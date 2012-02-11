// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * Class that wraps low-level details of interacting with the client plugin.
 *
 * This abstracts a <embed> element and controls the plugin which does
 * the actual remoting work. It also handles differences between
 * client plugins versions when it is necessary.
 */

'use strict';

/** @suppress {duplicate} */
var remoting = remoting || {};

/**
 * @param {Element} container The element to add the plugin to.
 * @param {string} id Id to use for the plugin element .
 * @constructor
 */
remoting.ClientPlugin = function(container, id) {
  this.plugin = /** @type {remoting.ViewerPlugin} */
      document.createElement('embed');

  this.plugin.id = id;
  this.plugin.src = 'about://none';
  this.plugin.type = 'application/vnd.chromium.remoting-viewer';
  this.plugin.width = 0;
  this.plugin.height = 0;
  this.plugin.tabIndex = 0;  // Required, otherwise focus() doesn't work.
  container.appendChild(this.plugin);

  this.desktopWidth = 0;
  this.desktopHeight = 0;

  /** @param {string} iq The Iq stanza received from the host. */
  this.onOutgoingIqHandler = function (iq) {};
  /** @param {string} message Log message. */
  this.onDebugMessageHandler = function (message) {};
  /**
   * @param {number} status The plugin status.
   * @param {number} error The plugin error status, if any.
   */
  this.onConnectionStatusUpdateHandler = function(status, error) {};
  this.onDesktopSizeUpdateHandler = function () {};

  // Connect Event Handlers

  /** @type {remoting.ClientPlugin} */
  var that = this;

  /** @param {string} iq The IQ stanza to send. */
  this.plugin.sendIq = function(iq) { that.onSendIq_(iq); };

  /** @param {string} msg The message to log. */
  this.plugin.debugInfo = function(msg) { that.onDebugInfo_(msg); };

  /**
   * @param {number} status The plugin status.
   * @param {number} error The plugin error status, if any.
   */
  this.plugin.connectionInfoUpdate = function(status, error) {
    that.onConnectionInfoUpdate_(status, error);
  };
  this.plugin.desktopSizeUpdate = function() { that.onDesktopSizeUpdate_(); };
};

/** @param {string} iq The Iq stanza received from the host. */
remoting.ClientPlugin.prototype.onSendIq_ = function(iq) {
  this.onOutgoingIqHandler(iq);
}

  /** @param {string} message The IQ stanza to send. */
remoting.ClientPlugin.prototype.onDebugInfo_ = function(message) {
  this.onDebugMessageHandler(message);
}

/**
 * @param {number} status The plugin status.
 * @param {number} error The plugin error status, if any.
 */
remoting.ClientPlugin.prototype.onConnectionInfoUpdate_=
    function(status, error) {
  // Old plugins didn't pass the status and error values, so get
  // them directly. Note that there is a race condition inherent in
  // this approach.
  if (typeof(status) == 'undefined')
    status = this.plugin.status;
  if (typeof(error) == 'undefined')
    error = this.plugin.error;
  this.onConnectionStatusUpdateHandler(status, error);
};

remoting.ClientPlugin.prototype.onDesktopSizeUpdate_ = function() {
    this.desktopWidth = this.plugin.desktopWidth;
    this.desktopHeight = this.plugin.desktopHeight;
    this.onDesktopSizeUpdateHandler();
}

/**
 * Chromoting session API version (for this javascript).
 * This is compared with the plugin API version to verify that they are
 * compatible.
 *
 * @const
 * @private
 */
remoting.ClientPlugin.prototype.API_VERSION_ = 4;

/**
 * The oldest API version that we support.
 * This will differ from the |API_VERSION_| if we maintain backward
 * compatibility with older API versions.
 *
 * @const
 * @private
 */
remoting.ClientPlugin.prototype.API_MIN_VERSION_ = 2;


/**
 * Deletes the plugin.
 */
remoting.ClientPlugin.prototype.cleanup = function() {
  this.plugin.parentNode.removeChild(this.plugin);
};

/**
 * @return {HTMLElement} HTML element that correspods to the plugin.
 */
remoting.ClientPlugin.prototype.element = function() {
  return this.plugin;
};

/**
 * @return {boolean} True if the plugin was loaded succesfully.
 */
remoting.ClientPlugin.prototype.isLoaded = function() {
  return typeof this.plugin.connect === 'function';
}

/**
 * @return {boolean} True if the plugin and web-app versions are compatible.
 */
remoting.ClientPlugin.prototype.isSupportedVersion = function() {
  return this.API_VERSION_ >= this.plugin.apiMinVersion &&
      this.plugin.apiVersion >= this.API_MIN_VERSION_;
};

/**
 * @return {boolean} True if the plugin supports high-quality scaling.
 */
remoting.ClientPlugin.prototype.isHiQualityScalingSupported = function() {
  return this.plugin.apiVersion >= 3;
};

/**
 * @param {string} iq Incoming IQ stanza.
 */
remoting.ClientPlugin.prototype.onIncomingIq = function(iq) {
  if (this.plugin && this.plugin.onIq) {
    this.plugin.onIq(iq);
  } else {
    // plugin.onIq may not be set after the plugin has been shut
    // down. Particularly this happens when we receive response to
    // session-terminate stanza.
    remoting.debug.log(
        'plugin.onIq is not set so dropping incoming message.');
  }
};

/**
 * @param {string} hostJid The jid of the host to connect to.
 * @param {string} hostPublicKey The base64 encoded version of the host's
 *     public key.
 * @param {string} clientJid Local jid.
 * @param {string} sharedSecret The access code for IT2Me or the PIN
 *     for Me2Me.
 * @param {string} authenticationMethods Comma-separated list of
 *     authentication methods the client should attempt to use.
 * @param {string} authenticationTag A host-specific tag to mix into
 *     authentication hashes.
 */
remoting.ClientPlugin.prototype.connect = function(
    hostJid, hostPublicKey, clientJid, sharedSecret,
    authenticationMethods, authenticationTag) {
  if (this.plugin.apiVersion < 4) {
    // Client plugin versions prior to 4 didn't support the last two
    // parameters.
    this.plugin.connect(hostJid, hostPublicKey, clientJid, sharedSecret);
  } else {
    this.plugin.connect(hostJid, hostPublicKey, clientJid, sharedSecret,
                        authenticationMethods, authenticationTag);
  }
}

/**
 * @param {boolean} scaleToFit True if scale-to-fit should be enabled.
 */
remoting.ClientPlugin.prototype.setScaleToFit = function(scaleToFit) {
  // scaleToFit() will be removed in future versions of the plugin.
  if (!!this.plugin && typeof this.plugin.setScaleToFit === 'function')
    this.plugin.setScaleToFit(scaleToFit);
};


/**
 *
 */
remoting.ClientPlugin.prototype.releaseAllKeys = function() {
  this.plugin.releaseAllKeys();
};

/**
 * Returns an associative array with a set of stats for this connection.
 *
 * @return {Object.<string, number>} The connection statistics.
 */
remoting.ClientPlugin.prototype.getPerfStats = function() {
  var dict = {};
  dict[remoting.ClientSession.STATS_KEY_VIDEO_BANDWIDTH] =
      this.plugin.videoBandwidth;
  dict[remoting.ClientSession.STATS_KEY_VIDEO_FRAME_RATE] =
      this.plugin.videoFrameRate;
  dict[remoting.ClientSession.STATS_KEY_CAPTURE_LATENCY] =
      this.plugin.videoCaptureLatency;
  dict[remoting.ClientSession.STATS_KEY_ENCODE_LATENCY] =
      this.plugin.videoEncodeLatency;
  dict[remoting.ClientSession.STATS_KEY_DECODE_LATENCY] =
      this.plugin.videoDecodeLatency;
  dict[remoting.ClientSession.STATS_KEY_RENDER_LATENCY] =
      this.plugin.videoRenderLatency;
  dict[remoting.ClientSession.STATS_KEY_ROUNDTRIP_LATENCY] =
      this.plugin.roundTripLatency;
  return dict;
};
