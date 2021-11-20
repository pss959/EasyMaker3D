#pragma once

#pragma once

#include <memory>

#include "Assert.h"
#include "Items/PaneBackground.h"
#include "Items/PaneBorder.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Notifier.h"

/// Pane is an abstract base class for a rectangular 2D element that lives
/// inside a Panel. The Pane class manages automatic sizing and placement.
class Pane : public SG::Node {
  public:
    virtual void AddFields() override;

    /// Sets the size of the pane. Derived classes may add other behavior.
    virtual void SetSize(const Vector2f &size);

    /// Returns the current size of the Pane.
    const Vector2f & GetSize() const { return size_; }

    /// Returns the base size set for the pane.
    const Vector2f & GetBaseSize() const { return base_size_; }

    /// Returns the minimum size of the Pane (in stage coordinate units),
    /// computing it first if necessary.
    virtual const Vector2f & GetMinSize() const;

    /// Returns true if the width of this Pane should respond to size changes.
    bool IsWidthResizable() const { return resize_width_; }

    /// Returns true if the height of this Pane should respond to size changes.
    bool IsHeightResizable() const { return resize_height_; }

    /// Sets the Pane's rectangle within its parent in the range [0,1] in both
    /// dimensions. This also sets the scale and translation in the Pane so
    /// that it has the correct size and position relative to the parent.
    void SetRectInParent(const Range2f &rect);

    /// Returns the Pane's rectangle within its parent.
    const Range2f & GetRectInParent() const { return rect_in_parent_; }

    /// \name Interaction-related functions
    /// If IsInteractive() returns true for a derived class, it indicates that
    /// the derived Pane supports user interaction.
    ///@{

    /// Returns true if this Pane represents an interactive element, such as a
    /// button or slider. The base class defines this to return false.
    virtual bool IsInteractive() const { return false; }

    /// If IsInteractive() returns true, this can be called to determine if the
    /// Pane is enabled.
    virtual bool IsInteractionEnabled() const {
        ASSERTM(false, "Base class Pane::IsInteractionEnabled() called");
        return false;
    }

    /// If IsInteractive() returns true, this can be called to activate the
    /// Pane. This is called when the user hits the Enter key with the focus on
    /// this pane.
    virtual void Activate() {
        ASSERTM(false, "Base class Pane::Activate() called");
    }

    ///@}

    /// Returns a Notifier that is invoked when a change is made to the size of
    /// this Pane or any sub-pane. It is passed the Pane that initiated the
    /// change.
    Util::Notifier<const Pane &> & GetPaneSizeChanged() {
        return pane_size_changed_;
    }

    /// This is called when the size of this Pane or any observed Pane
    /// changes. The Pane that initiated the change is supplied.
    virtual void ProcessPaneSizeChange(const Pane &pane);

    virtual void PreSetUpIon() override;

  protected:
    Pane() {}

    /// Computes and returns the minimum size for the Pane. The base class
    /// defines this to just use the base size.
    virtual Vector2f ComputeMinSize() const { return base_size_; }

    /// Allows derived classes that compute a minimum size to set it when
    /// something changes since ComputeMinSize() was called.
    void SetMinSize(const Vector2f &size);

    /// Returns the SG::Node to add auxiliary items to as children, such as
    /// borders, background, or sub-panes. The base class defines this to
    /// return the Pane itself.
    virtual SG::Node & GetAuxParent() { return *this; }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f>            base_size_{"base_size", {1, 1}};
    Parser::TField<bool>                resize_width_{"resize_width", false};
    Parser::TField<bool>                resize_height_{"resize_height", false};
    Parser::ObjectField<PaneBackground> background_{"background"};
    Parser::ObjectField<PaneBorder>     border_{"border"};
    ///@}

    /// Notifies when a change is made to the size of this Pane or any of its
    /// observed Panes.
    Util::Notifier<const Pane &> pane_size_changed_;

    /// Minimum size of the Pane. It is initialized to (0,0) so that the
    /// minimum size is computed by the instance.
    mutable Vector2f min_size_{0, 0};

    /// Size of this pane in world coordinates.
    Vector2f size_{0, 0};

    /// Relative size and position of this pane within its parent in the range
    /// [0,1] in both dimensions.
    Range2f  rect_in_parent_{{0,0},{1,1}};
};

typedef std::shared_ptr<Pane> PanePtr;
