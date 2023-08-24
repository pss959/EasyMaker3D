#pragma once

#include <concepts>
#include <memory>

#include "Panes/Pane.h"
#include "Panels/Panel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// Base class for Panel tests; it provides some conveniences to simplify and
/// clarify tests.
///
/// \ingroup Tests
class PanelTestBase : public SceneTestBase {
  protected:
    /// Calls ReadRealScene() using a contents string that sets up for a
    /// derived Panel of the templated type and name, sets up a test context in
    /// it, and returns the derived Panel
    /// instance.
    template <typename T>
    std::shared_ptr<T> ReadRealPanel(const Str &type_name) {
        static_assert(std::derived_from<T, Panel> == true);
        auto panel = ReadRealNode<T>(GetContentsString_(), type_name);
        SetTestContext_(*panel);
        return panel;
    }

    /// Finds and returns the sub-pane in the Panel with the given name and
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> FindPane(const Panel &panel, const Str &name) {
        static_assert(std::derived_from<T, Pane> == true);
        auto pane = std::dynamic_pointer_cast<T>(FindPane_(panel, name));
        EXPECT_NOT_NULL(pane);
        return pane;
    }

  private:
    static Str  GetContentsString_();
    static void SetTestContext_(Panel &panel);
    SG::NodePtr FindPane_(const Panel &panel, const Str &name);
};
