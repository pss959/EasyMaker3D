//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Commands/ChangeTaperCommand.h"
#include "Math/Profile.h"
#include "Tools/PanelTool.h"

/// TaperTool is a specialized Tool that is used to edit the Taper data for
/// selected TaperedModel instances. It is derived from PanelTool because it
/// uses a TaperToolPanel to provide interaction.
///
/// \ingroup Tools
class TaperTool : public PanelTool {
  protected:
    TaperTool() {}

    virtual bool CanAttach(const Selection &sel) const override;

    virtual Str GetPanelName() const override {
        return "TaperToolPanel";
    }

    virtual void InitPanel() override;
    virtual void PanelChanged(const Str &key,
                              ToolPanel::InteractionType type) override;

  private:
    /// Command used to modify the TaperedModel data.
    ChangeTaperCommandPtr command_;

    /// Taper at the start of a drag.
    Taper start_taper_;

    friend class Parser::Registry;
};
