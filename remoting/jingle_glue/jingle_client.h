// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_JINGLE_GLUE_JINGLE_CLIENT_H_
#define REMOTING_JINGLE_GLUE_JINGLE_CLIENT_H_

#include <string>

#include "remoting/jingle_glue/jingle_channel.h"
#include "talk/xmpp/xmppclient.h"

class MessageLoop;

namespace talk_base {
class NetworkManager;
}  // namespace talk_base

namespace buzz {
class PreXmppAuth;
}  // namespace buzz

namespace cricket {
class BasicPortAllocator;
class SessionManager;
class TunnelSessionClient;
class SessionManagerTask;
class Session;
}  // namespace cricket

namespace remoting {

class JingleClient : public base::RefCountedThreadSafe<JingleClient>,
                     public sigslot::has_slots<> {
 public:
  enum State {
    START,  // Initial state.
    CONNECTING,
    CONNECTED,
    CLOSED,
  };

  class Callback {
   public:
    virtual ~Callback() {}

    // Called when state of the connection is changed.
    virtual void OnStateChange(JingleClient* client, State state) = 0;

    // Called when a client attempts to connect to the machine. If the
    // connection should be accepted, must return true and must set
    // channel_callback to the callback for the new channel.
    virtual bool OnAcceptConnection(
        JingleClient* client, const std::string& jid,
        JingleChannel::Callback** channel_callback) = 0;

    // Called when a new client connects to the host. Ownership of the |channel|
    // is transfered to the callee.
    virtual void OnNewConnection(JingleClient* client,
                                 scoped_refptr<JingleChannel> channel) = 0;
  };

  // Creates a JingleClient object that executes on |thread|.  This does not
  // take ownership of |thread| and expects that the thread is started before
  // the constructor is called, and only stopped after the JingleClient object
  // has been destructed.
  JingleClient(JingleThread* thread);
  virtual ~JingleClient();

  // Starts the XMPP connection inialization. Must be called only once.
  // |callback| specifies callback object for the client and must not be NULL.
  void Init(const std::string& username, const std::string& auth_token,
            const std::string& auth_token_service, Callback* callback);

  // Creates new JingleChannel connected to the host with the specified jid.
  // The result is returned immediately but the channel fails if the host
  // rejects connection. |host_jid| must be a full jid (includes resource ID).
  // Ownership of the result is transfered to the caller. The channel must
  // be closed/destroyed before JingleClient is destroyed.
  JingleChannel* Connect(const std::string& host_jid,
                         JingleChannel::Callback* callback);

  // Closes XMPP connection and stops the thread. Must be called before the
  // object is destroyed.
  void Close();

  // Returns JID with resource ID. Empty string is returned if full JID is not
  // known yet, i.e. authentication hasn't finished.
  std::string GetFullJid();

  // Current state of the client.
  State state() { return state_; }

  // Returns XmppClient object for the xmpp connection or NULL if not connected.
  buzz::XmppClient* xmpp_client() { return client_; }

  // Message loop used by this object to execute tasks.
  MessageLoop* message_loop();

 private:
  void OnConnectionStateChanged(buzz::XmppEngine::State state);

  void OnIncomingTunnel(cricket::TunnelSessionClient* client, buzz::Jid jid,
                        std::string description, cricket::Session* session);

  void DoInitialize(const std::string& username,
                    const std::string& auth_token,
                    const std::string& auth_token_service);

  // Used by Connect().
  void DoConnect(scoped_refptr<JingleChannel> channel,
                 const std::string& host_jid,
                 JingleChannel::Callback* callback);

  // Used by Close().
  void DoClose();

  // Updates current state of the connection. Must be called only in
  // the jingle thread.
  void UpdateState(State new_state);

  buzz::PreXmppAuth* CreatePreXmppAuth(
      const buzz::XmppClientSettings& settings);

  buzz::XmppClient* client_;
  JingleThread* thread_;
  State state_;
  Callback* callback_;

  Lock full_jid_lock_;
  std::string full_jid_;

  scoped_ptr<talk_base::NetworkManager> network_manager_;
  scoped_ptr<cricket::BasicPortAllocator> port_allocator_;
  scoped_ptr<cricket::SessionManager> session_manager_;
  scoped_ptr<cricket::TunnelSessionClient> tunnel_session_client_;

  DISALLOW_COPY_AND_ASSIGN(JingleClient);
};

}  // namespace remoting

#endif  // REMOTING_JINGLE_GLUE_JINGLE_CLIENT_H_
