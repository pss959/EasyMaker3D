/**
Copyright 2017 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef ION_BASE_STLALLOC_STLCONTAINER_H_
#define ION_BASE_STLALLOC_STLCONTAINER_H_

#include "ion/base/allocatable.h"

namespace ion {
namespace base {

// StlContainer is used as a base for all containers used in place of std
// equivalents. It provides a Finish() function for cleaning up placement
// allocation when the container's construction has completed.
class StlContainer {
 private:
  // This member is constructed last; it is used to reset placement allocation
  // for the container.
  struct Finisher { Finisher(); };
  Finisher finisher_;
};

}  // namespace base
}  // namespace ion

#endif  // ION_BASE_STLALLOC_STLCONTAINER_H_
