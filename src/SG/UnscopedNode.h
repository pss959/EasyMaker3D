//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "SG/Node.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(UnscopedNode);

/// SG::UnscopedNode is a derived SG::Node that returns false for IsScoped().
/// This allows templates, constants, and objects defined within an
/// UnscopedNode to be accessible to other children of the containing scoped
/// Node.
///
/// \ingroup SG
class UnscopedNode : public Node {
  public:
    virtual bool IsScoped() const override { return false; }

  protected:
    UnscopedNode() {}

  private:
    friend class Parser::Registry;
};

}  // namespace SG
