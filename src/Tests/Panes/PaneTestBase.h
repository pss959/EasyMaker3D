#pragma once

#include <concepts>
#include <memory>
#include <string>

#include "Panes/Pane.h"
#include "Tests/SceneTestBase.h"

/// Base class for Pane tests; it provides some conveniences to simplify and
/// clarify tests.
/// \ingroup Tests
class PaneTestBase : public SceneTestBase {
  protected:
    /// Calls ReadRealScene() using a contents string that sets up for a
    /// derived Pane of the templated type and name; returns the derived Pane
    /// instance. A string with the contents of the Pane is supplied. This
    /// assumes the template file name and name for the derived type are the
    /// same as the class name.
    template <typename T>
    std::shared_ptr<T> ReadRealPane(const std::string &type_name,
                                    const std::string &pane_contents) {
        static_assert(std::derived_from<T, Pane> == true);
        return ReadRealNode<T>(GetContentsString_(type_name, pane_contents),
                               type_name);
    }

  private:
    static std::string GetContentsString_(const std::string &type_name,
                                          const std::string &pane_contents);
};
