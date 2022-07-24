#pragma once

#include "Base/Memory.h"
#include "Enums/Action.h"
#include "Widgets/PushButtonWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(IconWidget);

/// IconWidget is a derived PushButtonWidget that has some fields that make it
/// easier to set up 3D icons for use on shelves.
///
/// \ingroup Widgets
class IconWidget : public PushButtonWidget {
  public:
    /// Returns the Action associated with the IconWidget.
    Action GetAction() const { return action_; }

    /// Returns the path to the file containing a shape to import for the
    /// icon. If this is not empty, the imported shape is added to the icon in
    /// addition to any other shapes.
    const std::string & GetImportPath() const { return import_path_; }

    /// Sizes and translates the icon to fit into a cube with the given size in
    /// each dimension and center point. The front of the icon should be flush
    /// with the front of the cube.
    virtual void FitIntoCube(float size, const Point3f &center);

  protected:
    IconWidget() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Toggle icons can hover while active.
    virtual bool SupportsActiveHovering() override { return IsToggle(); }

    /// Fits the given SG::Node into a cube for FitIntoCube().
    static void FitNodeIntoCube(SG::Node &node,
                                float size, const Point3f &center);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Action>   action_;
    Parser::TField<std::string> import_path_;
    ///@}

    friend class Parser::Registry;
};
