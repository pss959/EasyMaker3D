#pragma once

#include <string>

#include "Models/Model.h"
#include "Panels/Panel.h"
#include "Panes/ContainerPane.h"
#include "Panes/TextPane.h"
#include "Targets/EdgeTarget.h"
#include "Targets/PointTarget.h"

namespace Parser { class Registry; }

/// InfoPanel is a derived Panel class that displays information about selected
/// Models and current Targets.
class InfoPanel : public Panel {
  public:
    /// Struct containing all information that can appear in the InfoPanel.
    /// There must be at least one item (Model or Target) present when calling
    /// AddInfo();
    struct Info {
        std::vector<ModelPtr> models;
        PointTargetPtr        point_target;
        EdgeTargetPtr         edge_target;
    };

    /// Sets the data sources to add info for. The Info instance must have at
    /// least one Model or Target specified.
    void SetInfo(const Info &info);

  protected:
    InfoPanel() {}

    virtual void CreationDone() override;

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    /// ContainerPane holding all lines of text (TextPane instances).
    ContainerPanePtr contents_pane_;

    /// TextPane template that is instantiated for each line of text.
    TextPanePtr      text_pane_;

    /// Creates and returns a clone of the TextPane with the given name and
    /// text.
    PanePtr CreateTextPane_(const std::string &name, const std::string &text);

    friend class Parser::Registry;
};

typedef std::shared_ptr<InfoPanel> InfoPanelPtr;
