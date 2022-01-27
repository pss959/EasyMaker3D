#pragma once

#include <string>

#include "Selection.h"
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
    /// There must be at least one item (a selected Model or a Target) present
    /// when calling AddInfo();
    struct Info {
        Selection      selection;
        PointTargetPtr point_target;
        EdgeTargetPtr  edge_target;
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
    /// Types of text to display.
    enum class TextType_ {
        kHeader,   ///< Header of some section.
        kError,    ///< Error text.
        kNormal,   ///< Plain old text.
    };

    /// ContainerPane holding all lines of text (TextPane instances).
    ContainerPanePtr contents_pane_;

    /// TextPane template that is instantiated for each line of text.
    TextPanePtr      text_pane_;

    /// Adds text for a Model represented by a SelPath.
    void AddModelInfo_(std::vector<PanePtr> &panes, const SelPath &sel_path);

    /// Adds a clone of the title or main TextPane with the given text.
    void AddTextPane_(std::vector<PanePtr> &panes, TextType_ type,
                      const std::string &text);

    friend class Parser::Registry;
};

typedef std::shared_ptr<InfoPanel> InfoPanelPtr;
