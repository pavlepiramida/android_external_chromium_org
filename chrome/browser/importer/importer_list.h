// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_IMPORTER_IMPORTER_LIST_H_
#define CHROME_BROWSER_IMPORTER_IMPORTER_LIST_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_vector.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string16.h"

namespace importer {
struct SourceProfile;
}

// ImporterList detects installed browsers and profiles via
// DetectSourceProfilesWorker(). ImporterList lives on the UI thread.
class ImporterList {
 public:
  ImporterList();
  ~ImporterList();

  // Detects the installed browsers and their associated profiles, then stores
  // their information in a list to be accessed via count() and
  // GetSourceProfileAt(). The detection runs asynchronously.
  //
  // |locale|: As in DetectSourceProfilesWorker().
  // |include_interactive_profiles|: True to include source profiles that
  // require user interaction to read.
  // |profiles_loaded_callback|: Assuming this ImporterList instance is still
  // alive, run the callback when the source profile detection finishes.
  void DetectSourceProfiles(const std::string& locale,
                            bool include_interactive_profiles,
                            const base::Closure& profiles_loaded_callback);

  // Returns the number of different source profiles you can import from.
  size_t count() const { return source_profiles_.size(); }

  // Returns the SourceProfile at |index|. The profiles are ordered such that
  // the profile at index 0 is the likely default browser. The SourceProfile
  // should be passed to ImporterHost::StartImportSettings().
  const importer::SourceProfile& GetSourceProfileAt(size_t index) const;

 private:
  // Called when the source profiles are loaded. Takes ownership of the
  // loaded profiles in |profiles| and calls |profiles_loaded_callback|.
  void SourceProfilesLoaded(
      const base::Closure& profiles_loaded_callback,
      const std::vector<importer::SourceProfile*>& profiles);

  // The list of profiles with the default one first.
  ScopedVector<importer::SourceProfile> source_profiles_;

  base::WeakPtrFactory<ImporterList> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ImporterList);
};

#endif  // CHROME_BROWSER_IMPORTER_IMPORTER_LIST_H_
