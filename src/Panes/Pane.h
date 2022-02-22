#pragma once

#pragma once

#include <memory>

#include "Items/PaneBackground.h"
#include "Items/PaneBorder.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Assert.h"
#include "Util/Notifier.h"

struct Event;

/// Pane is an abstract base class for a rectangular 2D element that lives
/// inside a Panel. The Pane class manages automatic sizing and placement.
///
/// The Pane coordinate system assumes 1 unit is approximately the size of a
/// pixel in a full-screen window.
class Pane : public SG::Node {
  public:
    /// \name Size-related functions
    ///@{

    /// Sets the minimum size of the Pane.
    void SetMinSize(const Vector2f &size);

    /// Sets the size of the Pane. Derived classes may add other behavior.
    virtual void SetSize(const Vector2f &size);

    /// Sets the size of the Pane and scales and translates so that it has the
    /// correct size and position relative to a ContainerPane, based on the
    /// given rectangle. If offset_forward is true, the translation will
    /// include a small offset in Z to put the Pane in front.
    void SetSizeWithinContainer(const Vector2f &size, const Range2f &rect,
                                bool offset_forward = false);

    /// Returns the current size of the Pane.
    const Vector2f & GetSize() const { return size_; }

    /// Returns the minimum size defined for the pane.
    const Vector2f & GetMinSize() const { return min_size_; }

    /// Returns the maximum size defined for the pane.
    const Vector2f & GetMaxSize() const { return max_size_; }

    /// Returns true if the width of this Pane should respond to size changes.
    bool IsWidthResizable() const { return resize_width_; }

    /// Returns true if the height of this Pane should respond to size changes.
    bool IsHeightResizable() const { return resize_height_; }

    /// Returns a Notifier that is invoked when the size of this Pane may have
    /// changed. It is passed the Pane that started the notification.
    Util::Notifier<const Pane &> & GetSizeChanged() { return size_changed_; }

    /// Returns the base size of the Pane, which is first computed if necessary
    /// and clamped to be between the min and max sizes. The base size is used
    /// by a ContainerPane to determine its own base size; this size may be
    /// expanded when the ContainerPane is resized.
    const Vector2f & GetBaseSize() const;

    /// Returns true if the Pane has a background set.
    bool HasBackground() const { return background_.GetValue().get(); }

    ///@}

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

    /// If IsInteractive() returns true, this can be called to deaactivate the
    /// Pane. This is called when focus leaves the Pane. However, Activate()
    /// may not have been called, so derived classes should not assume it was.
    /// The base class implements this to do nothing.
    virtual void Deactivate() {}

    /// If IsInteractive() returns true, this can be called to handle the given
    /// Event. The base class defines this to just return false.
    virtual bool HandleEvent(const Event &event) { return false; }

    ///@}

    /// Returns a string representing the Pane for debugging. Derived classes
    /// can add info.
    virtual std::string ToString() const;

  protected:
    Pane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Allows derived classes to change the resize_width_ field.
    void SetWidthResizable(bool resizable) { resize_width_ = resizable; }

    /// Allows derived classes to change the resize_height_ field.
    void SetHeightResizable(bool resizable) { resize_height_ = resizable; }

    /// Returns true if the current size of the Pane is known to be correct.
    bool IsSizeKnown() const {
        return size_ != Vector2f::Zero() && ! size_may_have_changed_;
    }

    /// This is invoked when the size of this Pane may have changed. The Pane
    /// that initiated the size change is passed in. The base class defines
    /// this to notify observers if this is the first notification since the
    /// size was last updated.
    virtual void SizeChanged(const Pane &initiating_pane);

    /// Computes and returns the base size for the Pane. The base class defines
    /// this to just use min_size_.
    virtual Vector2f ComputeBaseSize() const { return min_size_; }

    /// Returns the SG::Node to add auxiliary items to as children, such as
    /// borders and background. The base class defines this to return the Pane
    /// itself.
    virtual SG::Node & GetAuxParent() { return *this; }

    /// Returns the result of clamping the given size to the non-zero
    /// components of the min and max sizes of the given Pane.
    static Vector2f ClampSize(const Pane &pane, const Vector2f &size);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f>            min_size_{"min_size", {1, 1}};
    Parser::TField<Vector2f>            max_size_{"max_size", {0, 0}};
    Parser::TField<bool>                resize_width_{"resize_width", false};
    Parser::TField<bool>                resize_height_{"resize_height", false};
    Parser::ObjectField<PaneBackground> background_{"background"};
    Parser::ObjectField<PaneBorder>     border_{"border"};
    ///@}

    /// Notifies when a possible change is made to the size of this Pane. It is
    /// passed the Pane that initiated the change.
    Util::Notifier<const Pane &> size_changed_;

    /// Flag that is set when the size_changed_ Notifier is triggered.
    bool             size_may_have_changed_ = false;

    /// Current base size of the Pane.
    mutable Vector2f base_size_{0, 0};

    /// Current size of this pane.
    Vector2f         size_{0, 0};
};

typedef std::shared_ptr<Pane> PanePtr;
