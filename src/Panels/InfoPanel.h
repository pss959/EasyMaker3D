#pragma once

#include <string>

#include "Panels/Panel.h"
#include "Panes/TextPane.h"

class EdgeTarget;
class Model;
class PointTarget;

namespace Parser { class Registry; }

/// InfoPanel is a derived Panel class that displays information about selected
/// Models and current Targets.
class InfoPanel : public Panel {
  public:
    /// Clears the InfoPanel of all current information text.
    void Reset();

    /// Adds a Model to show info about.
    void AddModel(const Model &model);

    /// Adds a PointTarget to show info about.
    void AddPointTarget(const PointTarget &pt);

    /// Adds a EdgeTarget to show info about.
    void AddEdgeTarget(const EdgeTarget &et);

  protected:
    InfoPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    /// Text string to display.
    std::string text_;

    /// TextPane containing all the formatted info.
    TextPanePtr text_pane_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<InfoPanel> InfoPanelPtr;
