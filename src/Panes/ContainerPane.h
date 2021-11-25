#pragma once

#include <memory>

#include "Math/Types.h"
#include "Panes/Pane.h"

/// ContainerPane is an abstract base class for all derived Pane classes that
/// contain one or more other Pane instances.
class ContainerPane : public Pane {
  public:
    virtual void AddFields() override;

    /// Returns a vector of all contained Panes.
    const std::vector<PanePtr> GetPanes() const { return panes_; }

    virtual void PreSetUpIon()  override;
    virtual void PostSetUpIon() override;

  protected:
    ContainerPane() {}

    /// Convenience that calls SetRectInParent() based on the given size and
    /// upper-left corner position, assuming that GetSize() returns the correct
    /// current size for this.
    void SetSubPaneRect(Pane &pane, const Point2f &upper_left,
                        const Vector2f &size);

    /// Allow derived classes to replace Panes with new ones.
    void ReplacePanes(const std::vector<PanePtr> &panes);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Pane> panes_{"panes"};
    ///@}

    /// Stores the index of the first child that represents a Pane.
    size_t first_pane_child_index_;

    void ObservePanes_();
    void UnobservePanes_();
    void AddPanesAsChildren_();
    void RemovePanesAsChildren_();
};

typedef std::shared_ptr<ContainerPane> ContainerPanePtr;
