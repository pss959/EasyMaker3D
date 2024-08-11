//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Panels/Panel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(HelpPanel);

namespace Parser { class Registry; }

/// HelpPanel is a derived Panel class that implements help management.
///
/// \ingroup Panels
class HelpPanel : public Panel {
  protected:
    HelpPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    void OpenPage_(const Str &page_name);
    void OpenIssue_();

    friend class Parser::Registry;
};
