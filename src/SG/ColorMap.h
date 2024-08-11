//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <unordered_map>
#include <vector>

#include "Math/Types.h"
#include "SG/NamedColor.h"
#include "SG/Object.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(ColorMap);
DECL_SHARED_PTR(NamedColor);

/// A ColorMap maps color names to Colors for a Scene. This allows special
/// colors to be named and used within the application. It contains a vector of
/// NamedColor instances defining the colors and also sets up a hash table for
/// quick color lookup by name.
///
/// Rather than passing an instance of this everywhere it is needed, this is
/// implemented as a semi-singleton. There are static functions that access the
/// most recently created instance. This is ugly, but having to pass a ColorMap
/// instance to every Widget, Tool, Pane, and Panel that needs turned out to be
/// much uglier.
///
/// \ingroup SG
class ColorMap : public Object {
  public:
    // Resets to before any instance was created. Useful for tests.
    static void Reset();

    /// Returns all NamedColor instances in the ColorMap.
    const std::vector<NamedColorPtr> & GetNamedColors() const {
        return named_colors_;
    }

    /// Returns the color corresponding to the given name. Asserts if it is not
    /// found.
    Color GetColor(const Str &name) const;

    /// Returns a color used to represent the given dimension.
    Color GetColorForDimension(int dim) const;

    ///
    /// \name Singleton API
    /// Each of these functions accesses the most recently created ColorMap
    /// instance.
    ///@{

    static Color SGetColor(const Str &name);
    static Color SGetColorForDimension(int dim);

    ///@}

  protected:
    ColorMap() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<NamedColor> named_colors_;
    ///@}

    /// Maps special color name to Color.
    std::unordered_map<Str, Color> map_;

    /// Static instance - the most recent one created.
    static const ColorMap *s_instance;

    friend class Parser::Registry;
};

}  // namespace SG
