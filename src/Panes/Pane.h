#pragma once

#pragma once

#include <functional>

#include "Items/PaneBackground.h"
#include "Items/PaneBorder.h"
#include "Math/Types.h"
#include "Memory.h"
#include "SG/Node.h"
#include "Util/Notifier.h"

struct Event;

DECL_SHARED_PTR(Pane);

/// Pane is an abstract base class for a rectangular 2D element that lives
/// inside a Panel. The Pane class manages automatic sizing and placement.
///
/// The Pane coordinate system assumes 1 unit is approximately the size of a
/// pixel in a full-screen window.
///
/// XXXX Describe sizing.
class Pane : public SG::Node {
  public:
    /// \name Size-related functions
    ///@{

    /// Returns the minimum size defined for the pane.
    const Vector2f & GetMinSize() const { return min_size_; }

    /// Returns true if the width of this Pane should respond to size changes.
    bool IsWidthResizable() const { return resize_width_; }

    /// Returns true if the height of this Pane should respond to size changes.
    bool IsHeightResizable() const { return resize_height_; }

    /// Returns the base size of the Pane, which computed if potentially out of
    /// date and adjusted to be at least the minimum size.
    const Vector2f & GetBaseSize() const;

    /// Returns the current base size of the Pane, which may be out of date.
    /// This is used primarily for testing and debugging.
    const Vector2f & GetCurrentBaseSize() const { return base_size_; }

    /// Returns a Notifier invoked when the base size of this Pane may have
    /// changed. Note that this propagates upward through ContainerPane
    /// instances.
    Util::Notifier<> & GetBaseSizeChanged() { return base_size_changed_; }

    /// Sets the layout size of the Pane. Derived classes may add other
    /// behavior, but should call this one.
    virtual void SetLayoutSize(const Vector2f &size);

    /// Returns the current layout size of the Pane.
    const Vector2f & GetLayoutSize() const { return layout_size_; }

    ///@}

    /// \name Interaction-related functions
    /// If IsInteractive() returns true for a derived class, it indicates that
    /// the derived Pane supports user interaction.
    ///@{

    /// Typedef for a function that can be invoked by the Pane to get focus.
    typedef std::function<void(const Pane &)> FocusFunc;

    /// Returns true if this Pane represents an interactive element, such as a
    /// button or slider. The base class defines this to return false.
    virtual bool IsInteractive() const;

    /// If IsInteractive() returns true, this can be called to determine if the
    /// Pane is enabled.
    virtual bool IsInteractionEnabled() const;

    /// If IsInteractive() returns true, this can be called to activate the
    /// Pane. This is called when the user hits the Enter key with the focus on
    /// this Pane or if the user clicks on this Pane.
    virtual void Activate();

    /// If IsInteractive() returns true, this can be called to deaactivate the
    /// Pane. This is called when focus leaves the Pane. However, Activate()
    /// may not have been called, so derived classes should not assume it was.
    /// The base class implements this to do nothing.
    virtual void Deactivate() {}

    /// If IsInteractive() returns true, this can be called to handle the given
    /// Event. The base class defines this to just return false.
    virtual bool HandleEvent(const Event &event) { return false; }

    /// If IsInteractive() returns true, this can be used to set a function
    /// that the derived Pane can use to set focus to itself.
    void SetFocusFunc(const FocusFunc &func) { focus_func_ = func; }

    /// Interactive derived classes can call this to take the focus.
    void TakeFocus();

    ///@}

    /// Returns true if the Pane has a background set.
    bool HasBackground() const { return background_.GetValue().get(); }

    /// Returns a string representing the Pane for debugging. Derived classes
    /// can add info.
    virtual std::string ToString() const;

  protected:
    Pane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Allows derived classes to set the minimum size of the Pane.
    void SetMinSize(const Vector2f &size);

    /// Allows derived classes to change the resize_width_ field.
    void SetWidthResizable(bool resizable) { resize_width_ = resizable; }

    /// Allows derived classes to change the resize_height_ field.
    void SetHeightResizable(bool resizable) { resize_height_ = resizable; }

    /// This is invoked when the base size of this Pane may have changed. If
    /// this is the first such change since the last time the base size was
    /// updated, this sets a flag and notifies observers.
    void BaseSizeChanged();

    /// This is called when the base size is potentially out of date. It
    /// computes and returns the new base size for the Pane.
    virtual Vector2f ComputeBaseSize() const = 0;

    /// Returns the SG::Node to add auxiliary items to as children, such as
    /// borders and background. The base class defines this to return the Pane
    /// itself.
    virtual SG::Node & GetAuxParent() { return *this; }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f>            min_size_{"min_size", {1, 1}};
    Parser::TField<bool>                resize_width_{"resize_width", false};
    Parser::TField<bool>                resize_height_{"resize_height", false};
    Parser::ObjectField<PaneBackground> background_{"background"};
    Parser::ObjectField<PaneBorder>     border_{"border"};
    ///@}

    /// Current base size of the Pane. Mutable because this can be set by the
    /// (const) GetBaseSize() function.
    mutable Vector2f base_size_{0, 0};

    /// Flag that is set when the base_size_changed_ Notifier is triggered.
    /// Mutable because this can be set by the (const) GetBaseSize() function.
    mutable bool     base_size_may_have_changed_ = false;

    /// Notifies when a possible change is made to the base size of this Pane.
    Util::Notifier<> base_size_changed_;

    /// Current layout size of this pane.
    Vector2f         layout_size_{0, 0};

    /// Function to invoke to take focus.
    FocusFunc        focus_func_;
};
