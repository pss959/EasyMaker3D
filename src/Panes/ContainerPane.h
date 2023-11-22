#pragma once

#include <vector>

#include "Math/Types.h"
#include "Panes/Pane.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(ContainerPane);

/// ContainerPane is an abstract base class for Pane classes that contain one
/// or more other Pane instances (as opposed to LeafPane). It stores a
/// collection of sub-panes in the "panes" field.
///
/// The protected LayOutSubPanes() function is used to compute and set the size
/// and position of all contained Panes. It should be called whenever a change
/// is made to the size of the ContainerPane or if something changed in one of
/// the contained Panes to warrant laying out again.
///
/// \ingroup Panes
class ContainerPane : public Pane {
  public:
    virtual ~ContainerPane();

    /// Returns a vector of all contained Panes.
    const PaneVec & GetPanes() const { return panes_.GetValue(); }

    /// Removes the given Pane. Asserts if it is not found.
    void RemovePane(const PanePtr &pane);

    /// Replaces all contained Panes with new ones.
    void ReplacePanes(const PaneVec &panes);

    /// Redefines this to return all contained Panes.
    virtual PaneVec GetSubPanes() const override { return GetPanes(); }

  protected:
    ContainerPane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Derived classes must implement this to lay out contained Panes when the
    /// layout size of the ContainerPane changes.
    virtual void LayOutSubPanes() override = 0;

    /// Returns the SG::Node to add panes to as extra children. The base class
    /// defines this to return the ContainerPane itself.
    virtual SG::Node & GetExtraChildParent() { return *this; }

    /// Allows derived classes to hide the panes field so it can be reserved
    /// for internal use only.
    void HidePanesField() { panes_.SetHidden(true); }

    /// Derived classes can call this to set the scale and translation of a
    /// contained Pane so that it has the correct size and position relative to
    /// this ContainerPane, based on the given upper-left corner position and
    /// the current layout sizes of both Panes, which are assumed to be up to
    /// date. If offset_forward is true, the translation will include a small
    /// offset in Z to put the contained Pane in front.
    void PositionSubPane(Pane &sub_pane, const Point2f &upper_left,
                         bool offset_forward = false);

    /// Returns the result of ensuring the given size is not smaller than the
    /// minimum size of the given Pane.
    static Vector2f AdjustPaneSize(const Pane &pane, const Vector2f &size);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Pane> panes_;
    ///@}

    void ObservePanes_();
    void UnobservePanes_();

    /// Offsets each contained pane to move it in front.
    void OffsetPanes_();
};
