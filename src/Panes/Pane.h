#pragma once

#include <functional>

#include "Base/Memory.h"
#include "Items/PaneBackground.h"
#include "Items/PaneBorder.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Notifier.h"

class  IPaneInteractor;
struct Event;
DECL_SHARED_PTR(Pane);

/// Pane is an abstract base class for a rectangular 2D element that lives
/// inside a Panel. The Pane class manages automatic sizing and placement.
///
/// The Pane coordinate system assumes 1 unit is approximately the size of a
/// pixel in a full-screen window.
///
/// \todo Describe Pane size computations in more detail.
///
/// \ingroup Panes
class Pane : public SG::Node {
  public:
    /// Amount to offset items in Z to put them in front of others without
    /// Z-fighting.
    static const float kZOffset;

    /// \name Size-related functions
    ///@{

    /// Returns the minimum size defined for the pane.
    const Vector2f & GetMinSize() const { return min_size_; }

    /// Returns true if the width of this Pane should respond to size changes.
    bool IsWidthResizable() const { return resize_width_; }

    /// Returns true if the height of this Pane should respond to size changes.
    bool IsHeightResizable() const { return resize_height_; }

    /// Sets a flag indicating whether the width this Pane should respond to
    /// size changes.
    void SetWidthResizable(bool resizable) { resize_width_ = resizable; }

    /// Sets a flag indicating whether the height of this Pane should respond
    /// to size changes.
    void SetHeightResizable(bool resizable) { resize_height_ = resizable; }

    /// Returns the base size of the Pane, which computed if potentially out of
    /// date and adjusted to be at least the minimum size.
    const Vector2f & GetBaseSize() const;

    /// Returns the current base size of the Pane, which may be out of date.
    /// This is used primarily for testing and debugging.
    const Vector2f & GetCurrentBaseSize() const { return base_size_; }

    /// Returns true if the base size is known to be correct. This is used
    /// primarily for testing and debugging.
    bool IsBaseSizeUpToDate() const { return ! base_size_may_have_changed_; }

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

    /// Returns an IPaneInteractor instance to handle interaction for this
    /// Pane. The base class defines this to return null, meaning that the Pane
    /// is not interactive.
    virtual IPaneInteractor * GetInteractor() { return nullptr; }

    /// Returns true if the Pane has a background set.
    bool HasBackground() const { return background_.GetValue().get(); }

    /// Returns a string representing the Pane for debugging. Derived classes
    /// can add info. If is_brief is true, this stores just the description and
    /// current layout size along with any derived identifying data.
    virtual std::string ToString(bool is_brief) const;

  protected:
    Pane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Allows derived classes to set the minimum size of the Pane.
    void SetMinSize(const Vector2f &size);

    /// This is invoked when the base size of this Pane may have changed. If
    /// this is the first such change since the last time the base size was
    /// updated, this sets a flag and notifies observers.
    void BaseSizeChanged();

    /// This is called when the base size is potentially out of date. It
    /// computes and returns the new base size for the Pane.
    virtual Vector2f ComputeBaseSize() const = 0;

    /// This is called to when a new base size is computed. The base class
    /// defines this to store the new size. Derived classes can add their own
    /// behavior in addition to calling this.
    virtual void SetBaseSize(const Vector2f &new_base_size);

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

    /// Current base size of the Pane.
    Vector2f         base_size_{0, 0};

    /// Flag that is set when the base_size_changed_ Notifier is triggered.
    /// Mutable because this can be set by the (const) GetBaseSize() function.
    mutable bool     base_size_may_have_changed_ = false;

    /// Notifies when a possible change is made to the base size of this Pane.
    Util::Notifier<> base_size_changed_;

    /// Current layout size of this pane.
    Vector2f         layout_size_{0, 0};
};
