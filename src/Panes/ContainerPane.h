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

    virtual void PreSetUpIon() override;
    virtual void PostSetUpIon() override;

  protected:
    ContainerPane() {}

    /// Returns the SG::Node to add the contained panes to as children. The
    /// base class defines this to return the ContainerPane itself. This is
    /// also the node that shapes are added to if color or border fields are
    /// set.
    virtual SG::Node & GetPaneParent() { return *this; }

    /// Convenience that calls SetRectInParent() based on the given size and
    /// upper-left corner position, assuming that GetSize() returns the correct
    /// current size for this.
    void SetSubPaneRect(Pane &pane, const Point2f &upper_left,
                        const Vector2f &size);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color>         color_{"color"};
    Parser::TField<Color>         border_color_{"border_color"};
    Parser::TField<float>         border_width_{"border_width"};
    Parser::ObjectListField<Pane> panes_{"panes"};
    ///@}
};

typedef std::shared_ptr<ContainerPane> ContainerPanePtr;
