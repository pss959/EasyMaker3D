#pragma once

#include <memory>

#include "Math/Types.h"
#include "Panes/Pane.h"

/// MultiPane is an abstract base class for all derived Pane classes that
/// contain one or more other Pane instances.
class MultiPane : public Pane {
  public:
    virtual void AddFields() override;

    /// Returns a vector of all contained Panes.
    const std::vector<PanePtr> GetPanes() const { return panes_; }

    /// Redefines this to add contained Panes as children.
    virtual void PreSetUpIon() override;

  protected:
    MultiPane() {}

    /// Convenience that calls SetRectInParent() based on the given size and
    /// upper-left corner position, assuming that GetSize() returns the correct
    /// current size for this.
    void SetSubPaneRect(Pane &pane, const Point2f &upper_left,
                        const Vector2f &size);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Pane> panes_{"panes"};
    ///@}
};

typedef std::shared_ptr<MultiPane> MultiPanePtr;
