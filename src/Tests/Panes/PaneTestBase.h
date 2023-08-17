#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <vector>

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
    /// same as the class name. If the \p extra_type_names vector is not empty,
    /// this adds the named templates first.
    template <typename T>
    std::shared_ptr<T> ReadRealPane(const Str &type_name,
                                    const Str &pane_contents,
                                    const StrVec &extra_type_names = StrVec()) {
        static_assert(std::derived_from<T, Pane> == true);
        return ReadRealNode<T>(GetContentsString_(type_name, pane_contents),
                               type_name);
    }

  private:
    static Str GetContentsString_(const Str &type_name,
                                  const Str &pane_contents);
};
