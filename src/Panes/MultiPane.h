#pragma once

#include <memory>

#include "Panes/Pane.h"

/// MultiPane is an abstract base class for all derived Pane classes that
/// contain one or more other Pane instances.
class MultiPane : public Pane {
  public:
    virtual void AddFields() override;

    /// Returns a vector of all contained Panes.
    const std::vector<PanePtr> GetPanes() const { return panes_; }

  protected:
    MultiPane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Pane> panes_{"panes"};
    ///@}
};

typedef std::shared_ptr<MultiPane> MultiPanePtr;
