// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/app_list/apps_model_builder.h"

#include <algorithm>
#include <vector>

#include "chrome/browser/extensions/extension_prefs.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/app_list/extension_app_item.h"
#include "chrome/common/chrome_notification_types.h"
#include "chrome/common/extensions/extension.h"
#include "content/public/browser/notification_service.h"

using extensions::Extension;

namespace {

typedef std::vector<ExtensionAppItem*> Apps;

bool AppPrecedes(const ExtensionAppItem* app1, const ExtensionAppItem* app2) {
  const syncer::StringOrdinal& page1 = app1->GetPageOrdinal();
  const syncer::StringOrdinal& page2 = app2->GetPageOrdinal();
  if (page1.LessThan(page2))
    return true;

  if (page1.Equals(page2))
    return app1->GetAppLaunchOrdinal().LessThan(app2->GetAppLaunchOrdinal());

  return false;
}

}  // namespace

AppsModelBuilder::AppsModelBuilder(Profile* profile,
                                   app_list::AppListModel::Apps* model,
                                   AppListController* controller)
    : profile_(profile),
      controller_(controller),
      model_(model) {
  extensions::ExtensionPrefs* extension_prefs =
      profile_->GetExtensionService()->extension_prefs();

  registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_LOADED,
      content::Source<Profile>(profile_));
  registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_UNLOADED,
      content::Source<Profile>(profile_));
  registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_LAUNCHER_REORDERED,
      content::Source<ExtensionSorting>(extension_prefs->extension_sorting()));
  registrar_.Add(this, chrome::NOTIFICATION_APP_INSTALLED_TO_APPLIST,
      content::Source<Profile>(profile_));

  pref_change_registrar_.Init(extension_prefs->pref_service());
  pref_change_registrar_.Add(extensions::ExtensionPrefs::kExtensionsPref, this);
}

AppsModelBuilder::~AppsModelBuilder() {
}

void AppsModelBuilder::Build() {
  DCHECK(model_ && model_->item_count() == 0);

  PopulateApps();
  HighlightApp();
}

void AppsModelBuilder::PopulateApps() {
  ExtensionService* service = profile_->GetExtensionService();
  if (!service)
    return;

  Apps apps;
  const ExtensionSet* extensions = service->extensions();
  for (ExtensionSet::const_iterator app = extensions->begin();
       app != extensions->end(); ++app) {
    if ((*app)->ShouldDisplayInLauncher())
      apps.push_back(new ExtensionAppItem(profile_, *app, controller_));
  }

#if defined(OS_CHROMEOS)
  // Explicitly add Talk extension if it's installed and enabled.
  // Add it here instead of in CreateSpecialApps() so it sorts naturally.
  // Prefer debug > alpha > beta > production version.
  const char* kTalkIds[] = {
      extension_misc::kTalkDebugExtensionId,
      extension_misc::kTalkAlphaExtensionId,
      extension_misc::kTalkBetaExtensionId,
      extension_misc::kTalkExtensionId,
  };
  for (size_t i = 0; i < arraysize(kTalkIds); ++i) {
    const Extension* talk = service->GetExtensionById(
        kTalkIds[i], false /*include_disabled*/);
    if (talk) {
      apps.push_back(new ExtensionAppItem(profile_, talk, controller_));
      break;
    }
  }
#endif  // OS_CHROMEOS

  if (apps.empty())
    return;

  std::sort(apps.begin(), apps.end(), &AppPrecedes);

  for (Apps::const_iterator it = apps.begin();
       it != apps.end();
       ++it) {
    model_->Add(*it);
  }
}

void AppsModelBuilder::ResortApps() {
  Apps apps;
  for (size_t i = 0; i < model_->item_count(); ++i)
    apps.push_back(GetAppAt(i));

  if (apps.empty())
    return;

  std::sort(apps.begin(), apps.end(), &AppPrecedes);

  // Adjusts the order of apps as needed in |model_| based on |apps|.
  for (size_t i = 0; i < apps.size(); ++i) {
    ExtensionAppItem* app_item = apps[i];

    const size_t insert_index = i;

    bool found = false;
    size_t index = 0;

    // Finds |app_item| in remaining unsorted part in |model_|.
    for (size_t j = insert_index; j < model_->item_count(); ++j) {
      if (GetAppAt(j) == app_item) {
        found = true;
        index = j;
        break;
      }
    }

    if (found) {
      if (index != insert_index) {
        model_->RemoveAt(index);
        model_->AddAt(insert_index, app_item);
      }
    } else {
      NOTREACHED();
    }
  }
}

void AppsModelBuilder::InsertApp(ExtensionAppItem* app) {
  DCHECK(model_);

  size_t start = 0;
  size_t end = model_->item_count();
  while (start < end) {
    size_t mid = (start + end) / 2;

    if (AppPrecedes(GetAppAt(mid), app))
      start = mid + 1;
    else
      end = mid;
  }
  model_->AddAt(start, app);
}

int AppsModelBuilder::FindApp(const std::string& app_id) {
  DCHECK(model_);

  for (size_t i = 0; i < model_->item_count(); ++i) {
    if (GetAppAt(i)->extension_id() == app_id)
      return i;
  }

  return -1;
}

void AppsModelBuilder::HighlightApp() {
  DCHECK(model_);
  if (highlight_app_id_.empty())
    return;

  int index = FindApp(highlight_app_id_);
  if (index == -1)
    return;

  model_->GetItemAt(index)->SetHighlighted(true);
  highlight_app_id_.clear();
}

ExtensionAppItem* AppsModelBuilder::GetAppAt(size_t index) {
  DCHECK_LT(index, model_->item_count());
  ChromeAppListItem* item =
      static_cast<ChromeAppListItem*>(model_->GetItemAt(index));
  DCHECK_EQ(item->type(), ChromeAppListItem::TYPE_APP);

  return static_cast<ExtensionAppItem*>(item);
}

void AppsModelBuilder::Observe(int type,
                               const content::NotificationSource& source,
                               const content::NotificationDetails& details) {
  switch (type) {
    case chrome::NOTIFICATION_EXTENSION_LOADED: {
      const Extension* extension =
          content::Details<const Extension>(details).ptr();
      if (!extension->ShouldDisplayInLauncher())
        return;

      if (FindApp(extension->id()) != -1)
        return;

      InsertApp(new ExtensionAppItem(profile_, extension, controller_));
      HighlightApp();
      break;
    }
    case chrome::NOTIFICATION_EXTENSION_UNLOADED: {
      const Extension* extension =
          content::Details<extensions::UnloadedExtensionInfo>(
              details)->extension;
      int index = FindApp(extension->id());
      if (index >= 0)
        model_->DeleteAt(index);
      break;
    }
    case chrome::NOTIFICATION_EXTENSION_LAUNCHER_REORDERED: {
      ResortApps();
      break;
    }
    case chrome::NOTIFICATION_APP_INSTALLED_TO_APPLIST: {
      highlight_app_id_ = *content::Details<const std::string>(details).ptr();
      HighlightApp();
      break;
    }
    case chrome::NOTIFICATION_PREF_CHANGED: {
      ResortApps();
      break;
    }
    default:
      NOTREACHED();
  }
}
