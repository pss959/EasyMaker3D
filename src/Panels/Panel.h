#pragma once

#include <string>

#include "Panes/Pane.h"
#include "SG/Node.h"

/// Panel is an abstract base class for all panels used for 2D-ish interaction.
/// It can be attached to a Board to appear in the scene.
class Panel : public SG::Node {
  public:
    virtual void AddFields();
    virtual bool IsValid(std::string &details) override;

    /// Returns the root Pane for the Panel.
    const PanePtr & GetPane() const { return pane_; }

    /// Returns true if the Board containing the Panel should be movable. The
    /// base class defines this to return true.
    virtual bool IsMovable() const { return is_movable_; }

    /// Returns true if the Board containing the Panel should be resizable. The
    /// base class defines this to return false.
    virtual bool IsResizable() const { return is_resizable_; }

    virtual void PreSetUpIon() override;
    virtual void PostSetUpIon() override;

  protected:
    Panel() {}

    /// This function is invoked when a ButtonPane within the Panel is
    /// clicked. It is passed the name of the ButtonPane. The base class
    /// defines this to do nothing.
    virtual void ProcessButton(const std::string &name) {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<Pane> pane_{"pane"};
    Parser::TField<bool>      is_movable_{"is_movable",     true};
    Parser::TField<bool>      is_resizable_{"is_resizable", false};
    ///@}
};
