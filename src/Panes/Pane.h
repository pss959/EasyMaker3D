#pragma once

#include <functional>
#include <string>
#include <vector>

#include "Base/Memory.h"
#include "Items/Border.h"
#include "Items/PaneBackground.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Flags.h"
#include "Util/Notifier.h"

class  IPaneInteractor;
struct Event;
struct TouchInfo;
DECL_SHARED_PTR(Pane);
DECL_SHARED_PTR(Widget);

/// Pane is an abstract base class for a rectangular 2D element that lives
/// inside a Panel. The Pane class manages automatic sizing and placement.
///
/// The Pane coordinate system assumes 1 unit is approximately the size of a
/// pixel in a full-screen window.
///
/// Every Pane maintains three sizes:
///  - The \em minimum \em size (GetMinSize()) represents the smallest size the
///    Pane can have. This is used to ensure that no Pane is too small to show
///    its contents. This value comes directly from a field in the Pane but can
///    be changed by derived classes with SetMinSize().
///  - The \em base \em size (GetBaseSize()) represents the size to use for the
///    Pane when computing a Pane layout. It will never be smaller than the
///    minimum size, but can be larger if the Pane is resizable.
///  - The \em layout \em size (GetLayoutSize()) is the size assigned to the
///    Pane by a ContainerPane when laying out its contents. It will never be
///    smaller than the base size, but can be larger if the Pane is resizable.
///
/// Pane layouts are recomputed only when necessary. Any action in a derived
/// class that might cause a change to the base size or contents of a Pane
/// causes the GetChanged() Notifier to trigger; this can be used to detect the
/// need to update the layout and anything else.
///
/// \ingroup Panes
class Pane : public SG::Node {
  public:
    /// Flag indicating a resizable dimension.
    enum class ResizeFlag { kWidth = 0x1, kHeight = 0x2 };
    using ResizeFlags = Util::Flags<ResizeFlag>;

    // Shorthand.
    using PaneVec = std::vector<PanePtr>;

    /// \name Size query.
    ///@{

    /// Returns a Notifier invoked when the base size or contents of this Pane
    /// may have changed, potentially requiring a layout update. Note that this
    /// propagates upward through ContainerPane instances.
    Util::Notifier<> & GetLayoutChanged() { return layout_changed_; }

    /// Returns a flag indicating whether the layout for the Pane may have
    /// changed since the last time SetLayoutSize() was called.
    bool WasLayoutChanged() const { return was_layout_changed_; }

    /// Returns flags indicating which dimensions are resizable.
    const ResizeFlags GetResizeFlags() const { return resize_flags_; }

    /// Returns the minimum size defined for the pane.
    const Vector2f & GetMinSize() const { return min_size_; }

    /// Returns the base size of the Pane, which computed first if
    /// necessary. This will never be smaller than the minimum size.
    const Vector2f & GetBaseSize() const;

    /// Returns the current base size of the Pane, which may be out of date.
    const Vector2f & GetCurrentBaseSize() const { return base_size_; }

    ///@}

    /// \name Size modification.
    ///@{

    /// Sets flags indicating which dimensions are resizable.
    void SetResizeFlags(const ResizeFlags &flags) { resize_flags_ = flags; }

    /// Sets the layout size of the Pane.
    void SetLayoutSize(const Vector2f &size);

    /// Returns the current layout size of the Pane.
    const Vector2f & GetLayoutSize() const { return layout_size_; }

    ///@}

    /// \name Position-related functions
    ///@{

    /// Sets the relative position (0-1) of the upper-left corner of this Pane
    /// within its parent. This is called during layout and is used for
    /// scrolling.
    void SetRelativePositionInParent(const Vector2f &pos) {
        rel_pos_in_parent_ = pos;
    }

    /// Returns the relative position (0-1) of this Pane within its parent.
    const Vector2f & GetRelativePositionInParent() const {
        return rel_pos_in_parent_;
    }

    ///@}

    /// \name Interaction functions
    ///@{

    /// Returns an IPaneInteractor instance to handle interaction for this
    /// Pane. The base class defines this to return null, meaning that the Pane
    /// is not interactive.
    virtual IPaneInteractor * GetInteractor() { return nullptr; }

    /// This is used for touch interaction to find the best Widget (if any)
    /// intersected by a touch sphere. If any Widget is intersected, this sets
    /// \p closest_distance to the distance to the closest one.
    virtual WidgetPtr GetTouchedWidget(const TouchInfo &info,
                                       float &closest_distance);

    ///@}

    /// \name Appearance functions
    ///@{

    /// Returns true if the Pane has a background set.
    bool HasBackground() const { return background_.GetValue().get(); }

    /// Returns the Border, if any.
    const BorderPtr & GetBorder() const { return border_; }

    ///@}

    /// \name Pane hierarchy functions
    ///@{

    /// Returns a vector of all sub-Panes. The base class defines this to
    /// return an empty vector.
    virtual PaneVec GetSubPanes() const { return PaneVec(); }

    /// Adds all sub-Panes (at any level) that are focusable to the given
    /// vector. The base class defines this to add all sub-Panes that have an
    /// IPaneInteractor, recursively.
    virtual void GetFocusableSubPanes(PaneVec &panes) const;

    /// Searches recursively for a sub-Pane with the given name.  Returns null
    /// if it is not found.
    PanePtr FindSubPane(const Str &name) const;

    /// Searches recursively for the sub-Pane with the given type and name.
    /// Asserts if it is not found.
    template <typename T>
    std::shared_ptr<T> FindTypedSubPane(const Str &name) const {
        static_assert(std::derived_from<T, Pane> == true);
        auto pane = FindSubPane(name);
        ASSERTM(pane, "Pane '" + name + "' not found in " + GetDesc());
        auto typed_pane = std::dynamic_pointer_cast<T>(pane);
        ASSERTM(typed_pane, "Wrong type for Pane '" + name +
                "' in " + GetDesc());
        return typed_pane;
    }

    ///@}

    /// Returns a string representing the Pane for debugging. Derived classes
    /// can add info. If is_brief is true, this returns just the description
    /// and current layout size along with any derived identifying data.
    virtual Str ToString(bool is_brief) const;

  protected:
    Pane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Allows derived classes to set the minimum size of the Pane.
    void SetMinSize(const Vector2f &size);

    /// This can be called by any Pane class when the base size or contents of
    /// this Pane may have changed. This sets a flag and notifies
    /// GetLayoutChanged() observers if the flag is not already set.
    void MarkLayoutAsChanged();

    /// This is called when the base size is potentially out of date. It
    /// computes and returns the new base size for the Pane.
    virtual Vector2f ComputeBaseSize() const = 0;

    /// Updates anything inside the Pane whem the layout size has changed. The
    /// base class defines this to do nothing.
    virtual void UpdateForLayoutSize(const Vector2f &size) {}

    /// Returns the SG::Node to add auxiliary items to as children, such as
    /// borders and background. The base class defines this to return the Pane
    /// itself.
    virtual SG::Node & GetAuxParent() { return *this; }

    /// Derived classes may implement this to lay out sub-Panes when the layout
    /// size of this Pane changes. The base class does nothing.
    virtual void LayOutSubPanes() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f>            min_size_;
    Parser::FlagField<ResizeFlag>       resize_flags_;
    Parser::ObjectField<PaneBackground> background_;
    Parser::ObjectField<Border>         border_;
    ///@}

    /// Current base size of the Pane.
    Vector2f         base_size_{0, 0};

    /// Flag that is set when MarkLayoutAsChanged() is called.
    bool             was_layout_changed_ = false;

    /// Notifies when a possible change is made to the base size or contents of
    /// this Pane.
    Util::Notifier<> layout_changed_;

    /// Current layout size of this pane.
    Vector2f         layout_size_{0, 0};

    /// Relative position (0-1) of the upper-left corner in the parent Pane.
    Vector2f         rel_pos_in_parent_{0, 0};
};
