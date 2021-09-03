#pragma once

#include "Math/Types.h"
#include "SG/Change.h"
#include "SG/Hit.h"
#include "SG/Object.h"
#include "Util/Notification.h"

namespace SG {

//! Interactor is an abstract base class for all objects that can be interacted
//! with. SG just provides this base class; all derived classes are defined
//! elsewhere.
//!
//! A Node contains a pointer to an Interactor, allowing clients to attach
//! interaction of some sort to a Node.
class Interactor : public Object {
  protected:
    //! The constructor is protected to make this abstract.
    Interactor() {}
};

}  // namespace SG
