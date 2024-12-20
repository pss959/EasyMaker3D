//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Panels/Panel.h"
#include "Panes/Pane.h"
#include "Selection/Selection.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(ContainerPane);
DECL_SHARED_PTR(EdgeTarget);
DECL_SHARED_PTR(InfoPanel);
DECL_SHARED_PTR(Pane);
DECL_SHARED_PTR(PointTarget);
DECL_SHARED_PTR(TextPane);

namespace Parser { class Registry; }

/// InfoPanel is a derived Panel class that displays information about selected
/// Models and current Targets.
///
/// \ingroup Panels
class InfoPanel : public Panel {
  public:
    /// Struct containing all information that can appear in the InfoPanel.
    /// There must be at least one item (a selected Model or a Target) present
    /// when calling AddInfo();
    struct Info {
        Selection          selection;               ///< May be empty.
        const PointTarget *point_target = nullptr;  ///< May be null.
        const EdgeTarget  *edge_target  = nullptr;  ///< May be null.
    };

    /// Sets the data sources to add info for. The Info instance must have at
    /// least one Model or Target specified.
    void SetInfo(const Info &info);

  protected:
    InfoPanel() {}

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

    /// SpacerPane that is instantiated to separate text sections.
    PanePtr          separator_pane_;

    /// Adds text for a Model represented by a SelPath.
    void AddModelInfo_(Pane::PaneVec &panes, const SelPath &sel_path);

    /// Adds text for a PointTarget.
    void AddPointTargetInfo_(Pane::PaneVec &panes, const PointTarget &pt);

    /// Adds text for a EdgeTarget.
    void AddEdgeTargetInfo_(Pane::PaneVec &panes, const EdgeTarget &et);

    /// Adds a clone of the title or main TextPane with the given text.
    void AddTextPane_(Pane::PaneVec &panes, TextType_ type,
                      const Str &label, const Str &text);

    /// Adds a clone of the separator pane if panes is not empty.
    void AddSeparator_(Pane::PaneVec &panes);

    friend class Parser::Registry;
};
