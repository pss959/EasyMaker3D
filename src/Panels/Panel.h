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

    /// Sets the size of the Panel. This can always be called to set the
    /// initial size and can also be used to resize the Panel if IsResizable()
    /// returns true.
    void SetSize(const Vector2f &size);

    /// Returns the minimum size of the Panel, which is computed by the root
    /// Pane.
    Vector2f GetMinSize() const;

    virtual void PreSetUpIon() override;
    virtual void PostSetUpIon() override;

  protected:
    Panel() {}

    /// This function is invoked when a ButtonPane within the Panel is
    /// clicked. It is passed the name of the ButtonPane. The base class
    /// defines this to do nothing.
    virtual void ProcessButton(const std::string &name) {}

    /// Closes the panel, reporting the given result string.
    void Close(const std::string &result);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<Pane> pane_{"pane"};
    Parser::TField<bool>      is_movable_{"is_movable",     true};
    Parser::TField<bool>      is_resizable_{"is_resizable", false};
    ///@}
};

typedef std::shared_ptr<Panel> PanelPtr;
