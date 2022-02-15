#pragma once

#include <memory>

#include "Enums/Action.h"
#include "Widgets/PushButtonWidget.h"

namespace Parser { class Registry; }

/// IconWidget is a derived PushButtonWidget that has some fields that make it
/// easier to set up 3D icons for use on shelves.
class IconWidget : public PushButtonWidget {
  public:
    /// Returns the Action associated with the IconWidget.
    Action GetAction() const { return action_; }

    /// Returns a flag indicating whether the IconWidget is a toggle as opposed
    /// to a momentary button.
    bool IsToggle() const { return is_toggle_; }

    /// If IsToggle() returns true, this returns the current state of the
    /// toggle. Otherwise, it just returns false. It is false by default.
    bool GetToggleState() const { return toggle_state_; }

    /// If IsToggle() returns true, this sets the toggle state of the
    /// widget. Otherwise, it does nothing.
    void SetToggleState(bool state) { toggle_state_ = IsToggle() && state; }

    /// Returns the path to the file containing a shape to import for the
    /// icon. If this is not empty, the imported shape is added to the icon in
    /// addition to any other shapes.
    const std::string & GetImportPath() const { return import_path_; }

    /// Sizes and translates the icon to fit into a cube with the given size in
    /// each dimension and center point. The front of the icon should be flush
    /// with the front of the cube.
    virtual void FitIntoCube(float size, const Point3f &center);

    /// Redefines this to also toggle the state if IsToggle() is true.
    virtual void Click(const ClickInfo &info) override;

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
    Parser::EnumField<Action>   action_{"action", Action::kNone};
    Parser::TField<bool>        is_toggle_{"is_toggle", false};
    Parser::TField<std::string> import_path_{"import_path"};
    ///@}

    /// Current toggle state, which can be true only if IsToggle() is true.
    bool toggle_state_ = false;

    friend class Parser::Registry;
};

typedef std::shared_ptr<IconWidget> IconWidgetPtr;
