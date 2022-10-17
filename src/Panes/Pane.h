#pragma once

#include <functional>
#include <string>

#include "Base/Memory.h"
#include "Items/PaneBackground.h"
#include "Items/PaneBorder.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Notifier.h"

class  IPaneInteractor;
struct Event;
DECL_SHARED_PTR(Pane);
DECL_SHARED_PTR(Widget);

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
    /// Typedef for an intersection function passed to
    /// Pane::GetIntersectedWidget(). This takes a Node to test and a distance
    /// (by reference); If there is an intersection with the touch sphere and
    /// the intersection distance is less than the current distance, this
    /// returns true and modifies the distance.
    typedef std::function<bool(const SG::Node &, float &)> IntersectionFunc;

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

    /// Sets the relative position (0-1) of this Pane within its parent. This
    /// is used for scrolling.
    void SetRelativePositionInParent(const Vector2f &pos) {
        rel_pos_in_parent_ = pos;
    }

    /// Returns the relative position (0-1) of this Pane within its parent.
    const Vector2f & GetRelativePositionInParent() const {
        return rel_pos_in_parent_;
    }

    /// Returns an IPaneInteractor instance to handle interaction for this
    /// Pane. The base class defines this to return null, meaning that the Pane
    /// is not interactive.
    virtual IPaneInteractor * GetInteractor() { return nullptr; }

    /// This is used for touch interaction to find the best Widget (if any)
    /// intersected by a touch sphere. The given IntersectionFunc can be used
    /// to test a Widget (or any other Node) to see if it is intersected.
    virtual WidgetPtr GetIntersectedWidget(const IntersectionFunc &func,
                                           float &closest_distance);

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
    Parser::TField<Vector2f>            min_size_;
    Parser::TField<bool>                resize_width_;
    Parser::TField<bool>                resize_height_;
    Parser::ObjectField<PaneBackground> background_;
    Parser::ObjectField<PaneBorder>     border_;
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

    /// Relative position (0-1) of the upper-left corner in the parent Pane.
    Vector2f         rel_pos_in_parent_{0, 0};
};
