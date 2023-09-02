#pragma once

#include <concepts>
#include <memory>

#include "Panes/Pane.h"
#include "Tests/SceneTestBase.h"

/// Base class for Pane tests; it provides some conveniences to simplify and
/// clarify tests.
///
/// \ingroup Tests
class PaneTestBase : public SceneTestBase {
  protected:
    /// Calls ReadRealScene() using a contents string that sets up for a
    /// derived Pane of the templated type and name; returns the derived Pane
    /// instance. A string with the contents of the Pane is supplied.
    template <typename T>
    std::shared_ptr<T> ReadRealPane(const Str &type_name,
                                    const Str &pane_contents,
                                    bool set_up_ion = true) {
        static_assert(std::derived_from<T, Pane> == true);
        return ReadRealNode<T>(GetContentsString_(type_name, pane_contents),
                               type_name, set_up_ion);
    }

  private:
    static Str GetContentsString_(const Str &type_name,
                                  const Str &pane_contents);
};
