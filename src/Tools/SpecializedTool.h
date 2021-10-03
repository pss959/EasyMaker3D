#pragma once

#include <functional>
#include <memory>

#include "Tools/Tool.h"

/// SpecializedTool is a derived Tool class that serves as an abstract base
/// class for all interactive tools that can be attached only to specific types
/// of Models.
//
/// \ingroup Tools
class SpecializedTool : public Tool {
  public:
    /// Typedef for completion function passed to SetCompletionFunc().
    typedef std::function<void(const Selection &)> CompletionFunc;

    /// Some derived classes (such as those that use panels) allow the user to
    /// indicate that they are done with interaction. When this happens, this
    /// completion function will be invoked. It will be passed the Selection
    /// that the tool was attached to.
    void SetCompletionFunc(const CompletionFunc &func) {
        completion_func_ = func;
    }

  protected:
    virtual bool CanAttach(const Selection &sel) const override;

  private:
    CompletionFunc completion_func_;

    /// Derived classes must implement to indicate whether the SpecializedTool
    /// can be attached to the given Model.
    virtual bool CanAttachToModel(const Model &model) const = 0;

    /// This allows derived classes to invoke the completion function.
    virtual void Finish();
};

typedef std::shared_ptr<SpecializedTool> SpecializedToolPtr;
