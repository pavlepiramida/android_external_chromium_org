// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sync/internal_api/js_mutation_event_observer.h"

#include <string>

#include "base/location.h"
#include "base/logging.h"
#include "base/string_number_conversions.h"
#include "base/values.h"
#include "sync/js/js_event_details.h"
#include "sync/js/js_event_handler.h"

namespace syncer {

JsMutationEventObserver::JsMutationEventObserver()
    : weak_ptr_factory_(this) {}

JsMutationEventObserver::~JsMutationEventObserver() {
  DCHECK(CalledOnValidThread());
}

base::WeakPtr<JsMutationEventObserver> JsMutationEventObserver::AsWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

void JsMutationEventObserver::InvalidateWeakPtrs() {
  weak_ptr_factory_.InvalidateWeakPtrs();
}

void JsMutationEventObserver::SetJsEventHandler(
    const WeakHandle<JsEventHandler>& event_handler) {
  event_handler_ = event_handler;
}

namespace {

// Max number of changes we attempt to convert to values (to avoid
// running out of memory).
const size_t kChangeLimit = 100;

}  // namespace

void JsMutationEventObserver::OnChangesApplied(
    ModelType model_type,
    int64 write_transaction_id,
    const ImmutableChangeRecordList& changes) {
  if (!event_handler_.IsInitialized()) {
    return;
  }
  base::DictionaryValue details;
  details.SetString("modelType", ModelTypeToString(model_type));
  details.SetString("writeTransactionId",
                    base::Int64ToString(write_transaction_id));
  base::Value* changes_value = NULL;
  const size_t changes_size = changes.Get().size();
  if (changes_size <= kChangeLimit) {
    base::ListValue* changes_list = new base::ListValue();
    for (ChangeRecordList::const_iterator it =
             changes.Get().begin(); it != changes.Get().end(); ++it) {
      changes_list->Append(it->ToValue());
    }
    changes_value = changes_list;
  } else {
    changes_value =
        new base::StringValue(
            base::Uint64ToString(static_cast<uint64>(changes_size)) +
            " changes");
  }
  details.Set("changes", changes_value);
  HandleJsEvent(FROM_HERE, "onChangesApplied", JsEventDetails(&details));
}

void JsMutationEventObserver::OnChangesComplete(ModelType model_type) {
  if (!event_handler_.IsInitialized()) {
    return;
  }
  base::DictionaryValue details;
  details.SetString("modelType", ModelTypeToString(model_type));
  HandleJsEvent(FROM_HERE, "onChangesComplete", JsEventDetails(&details));
}

void JsMutationEventObserver::OnTransactionWrite(
    const syncable::ImmutableWriteTransactionInfo& write_transaction_info,
    ModelTypeSet models_with_changes) {
  DCHECK(CalledOnValidThread());
  if (!event_handler_.IsInitialized()) {
    return;
  }
  base::DictionaryValue details;
  details.Set("writeTransactionInfo",
              write_transaction_info.Get().ToValue(kChangeLimit));
  details.Set("modelsWithChanges",
              ModelTypeSetToValue(models_with_changes));
  HandleJsEvent(FROM_HERE, "onTransactionWrite", JsEventDetails(&details));
}

void JsMutationEventObserver::HandleJsEvent(
    const tracked_objects::Location& from_here,
    const std::string& name, const JsEventDetails& details) {
  if (!event_handler_.IsInitialized()) {
    NOTREACHED();
    return;
  }
  event_handler_.Call(from_here,
                      &JsEventHandler::HandleJsEvent, name, details);
}

}  // namespace syncer
