// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync/glue/app_data_type_controller.h"

#include "base/metrics/histogram.h"
#include "base/logging.h"
#include "base/time.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/sync/profile_sync_service.h"
#include "chrome/browser/sync/profile_sync_factory.h"
#include "chrome/browser/sync/unrecoverable_error_handler.h"
#include "content/browser/browser_thread.h"

namespace browser_sync {

AppDataTypeController::AppDataTypeController(
    ProfileSyncFactory* profile_sync_factory,
    Profile* profile,
    ProfileSyncService* sync_service)
    : profile_sync_factory_(profile_sync_factory),
      profile_(profile),
      sync_service_(sync_service),
      state_(NOT_RUNNING) {
  DCHECK(profile_sync_factory);
  DCHECK(sync_service);
}

AppDataTypeController::~AppDataTypeController() {
}

void AppDataTypeController::Start(StartCallback* start_callback) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DCHECK(start_callback);
  if (state_ != NOT_RUNNING) {
    start_callback->Run(BUSY, FROM_HERE);
    delete start_callback;
    return;
  }

  start_callback_.reset(start_callback);

  profile_->InitExtensions();
  ProfileSyncFactory::SyncComponents sync_components =
      profile_sync_factory_->CreateAppSyncComponents(sync_service_,
                                                     this);
  model_associator_.reset(sync_components.model_associator);
  change_processor_.reset(sync_components.change_processor);

  if (!model_associator_->CryptoReadyIfNecessary()) {
    StartFailed(NEEDS_CRYPTO, FROM_HERE);
    return;
  }

  bool sync_has_nodes = false;
  if (!model_associator_->SyncModelHasUserCreatedNodes(&sync_has_nodes)) {
    StartFailed(UNRECOVERABLE_ERROR, FROM_HERE);
    return;
  }

  base::TimeTicks start_time = base::TimeTicks::Now();
  bool merge_success = model_associator_->AssociateModels();
  UMA_HISTOGRAM_TIMES("Sync.AppAssociationTime",
                      base::TimeTicks::Now() - start_time);
  if (!merge_success) {
    StartFailed(ASSOCIATION_FAILED, FROM_HERE);
    return;
  }

  sync_service_->ActivateDataType(this, change_processor_.get());
  state_ = RUNNING;
  FinishStart(!sync_has_nodes ? OK_FIRST_RUN : OK, FROM_HERE);
}

void AppDataTypeController::Stop() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  if (state_ == MODEL_STARTING || state_ == ASSOCIATING)
    FinishStart(ABORTED, FROM_HERE);
  DCHECK(!start_callback_.get());

  if (change_processor_ != NULL)
    sync_service_->DeactivateDataType(this, change_processor_.get());

  if (model_associator_ != NULL)
    model_associator_->DisassociateModels();

  change_processor_.reset();
  model_associator_.reset();

  state_ = NOT_RUNNING;
}

bool AppDataTypeController::enabled() {
  return true;
}

syncable::ModelType AppDataTypeController::type() {
  return syncable::APPS;
}

browser_sync::ModelSafeGroup AppDataTypeController::model_safe_group() {
  return browser_sync::GROUP_UI;
}

const char* AppDataTypeController::name() const {
  // For logging only.
  return "app";
}

DataTypeController::State AppDataTypeController::state() {
  return state_;
}

void AppDataTypeController::OnUnrecoverableError(
    const tracked_objects::Location& from_here,
    const std::string& message) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  UMA_HISTOGRAM_COUNTS("Sync.AppRunFailures", 1);
  sync_service_->OnUnrecoverableError(from_here, message);
}

void AppDataTypeController::FinishStart(StartResult result,
     const tracked_objects::Location& location) {
  start_callback_->Run(result, location);
  start_callback_.reset();
}

void AppDataTypeController::StartFailed(StartResult result,
    const tracked_objects::Location& location) {
  model_associator_.reset();
  change_processor_.reset();
  start_callback_->Run(result, location);
  start_callback_.reset();
  UMA_HISTOGRAM_ENUMERATION("Sync.AppStartFailures",
                            result,
                            MAX_START_RESULT);
}

}  // namespace browser_sync
