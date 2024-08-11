//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <concepts>

#include "Panes/Pane.h"
#include "Panels/Panel.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/SceneTestBase.h"
#include "Util/Assert.h"
#include "Util/Memory.h"

/// Base class for Panel tests; it provides some conveniences to simplify and
/// clarify tests.
///
/// \ingroup Tests
class PanelTestBase : public SceneTestBase {
  protected:
    PanelInteractor pi;

    /// The constructor sets up the Panel::Context.
    PanelTestBase();
    virtual ~PanelTestBase();

    /// Sets a flag in the TestBoardAgent that prints useful information about
    /// Panel status changes to help with debugging.
    void SetPanelDebugFlag(bool b);

    /// Sets up to test a Panel of the templated type and name. This sets up a
    /// test Context in the Panel and stores it for later use. It also sets up
    /// a PanelInteractor for the derived class to used. The derived Panel is
    /// returned. The \p extra_contents string is added after Panels.emd.
    template <typename T> std::shared_ptr<T> InitPanel(
        const Str &type_name, const Str &extra_contents = "") {
        static_assert(std::derived_from<T, Panel> == true);
        auto panel = ReadRealNode<T>(GetContentsString_(extra_contents),
                                     type_name);
        StorePanelWithContext_(panel);
        return panel;
    }

    /// Stores the TestContext in the Scene and TestBoardAgent.
    void StoreContext();

    /// Returns the test Context created for the Panel.
    Panel::Context & GetContext() { return *test_context_; }

    /// \name Pane access helpers
    ///@{

    /// Finds and returns the sub-pane in the Panel with the given name.
    /// Asserts if not found.
    PanePtr FindPane(const Str &name) {
        auto panel = GetCurrentPanel();
        ASSERT(panel);
        return panel->GetPane()->FindSubPane(name);
    }

    /// Finds and returns the sub-pane in the Panel with the given name and
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> FindTypedPane(const Str &name) {
        auto panel = GetCurrentPanel();
        ASSERT(panel);
        return panel->GetPane()->FindTypedSubPane<T>(name);
    }

    ///@}

    /// Returns the result of the last call to Close() on the Panel and resets
    /// to empty.
    Str GetCloseResult();

    /// Returns the currently-open Panel.
    PanelPtr GetCurrentPanel();

  private:
    DECL_SHARED_PTR(TestBoardAgent);
    DECL_SHARED_PTR(TestSettingsAgent);

    Panel::ContextPtr    test_context_;
    TestBoardAgentPtr    test_board_agent_;
    TestSettingsAgentPtr test_settings_agent_;

    static Str GetContentsString_(const Str &extra_contents);
    void       StorePanelWithContext_(const PanelPtr &panel);
};
