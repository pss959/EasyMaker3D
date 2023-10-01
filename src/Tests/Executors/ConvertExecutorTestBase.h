#pragma once

#include "Executors/Executor.h"
#include "Tests/Executors/ExecutorTestBase.h"

class ConvertCommand;
class ConvertExecutorBase;

/// Base class for conversion Executor tests, which are all similar; it
/// provides some conveniences to simplify and clarify tests.
///
/// \ingroup Tests
class ConvertExecutorTestBase : public ExecutorTestBase {
  protected:
    /// Tests conversion without result names in the command.
    void ConvertWithoutNames(ConvertCommand &cmd, ConvertExecutorBase &exec,
                             const Str &model_type_name,
                             const Str &name_prefix) {
        Convert_(cmd, exec, model_type_name, name_prefix, false);
    }

    /// Tests conversion with result names in the command.
    void ConvertWithNames(ConvertCommand &cmd, ConvertExecutorBase &exec,
                          const Str &model_type_name, const Str &name_prefix) {
        Convert_(cmd, exec, model_type_name, name_prefix, true);
    }

  private:
    /// Implements both of the above tests.
    void Convert_(ConvertCommand &cmd, ConvertExecutorBase &exec,
                  const Str &model_type_name, const Str &name_prefix,
                  bool has_names);
};
