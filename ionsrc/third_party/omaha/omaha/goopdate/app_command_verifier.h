// Copyright 2012 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ========================================================================

#ifndef OMAHA_GOOPDATE_APP_COMMAND_VERIFIER_H__
#define OMAHA_GOOPDATE_APP_COMMAND_VERIFIER_H__

#include <atlbase.h>
#include "base/basictypes.h"

namespace omaha {

class AppCommandVerifier {
 public:
  AppCommandVerifier() {}
  virtual ~AppCommandVerifier() {}

  virtual HRESULT VerifyExecutable(const CString& path) = 0;
 private:
  DISALLOW_COPY_AND_ASSIGN(AppCommandVerifier);
};

}  // namespace omaha

#endif  // OMAHA_GOOPDATE_APP_COMMAND_VERIFIER_H__
