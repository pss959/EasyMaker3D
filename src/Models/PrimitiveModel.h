//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Models/Model.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(PrimitiveModel);

/// PrimitiveModel is an abstract derived Model class representing a primitive
/// Model of some sort. There are derived classes for each specific primitive
/// type.
///
/// \ingroup Models
class PrimitiveModel : public Model {
  protected:
    /// Protected constructor to make this abstract.
    PrimitiveModel() {}
};

