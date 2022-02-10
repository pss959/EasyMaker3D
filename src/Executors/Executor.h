﻿#pragma once

#include <functional>
#include <memory>

#include "Commands/Command.h"
#include "Managers/AnimationManager.h"
#include "Managers/ColorManager.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/TargetManager.h"
#include "Models/RootModel.h"
#include "SelPath.h"
#include "Util/Assert.h"

/// Executor is an abstract base class for any class that implements functions
/// to execute Commands.
///
/// \ingroup Executors
class Executor {
  public:
    /// The Context struct provides managers to derived Executor classes.
    struct Context {
        RootModelPtr        root_model;
        AnimationManagerPtr animation_manager;
        ColorManagerPtr     color_manager;
        NameManagerPtr      name_manager;
        SelectionManagerPtr selection_manager;
        TargetManagerPtr    target_manager;

        // std::shared_ptr<IActionProcessor> action_processor;
        // std::shared_ptr<FontManager>      font_manager;
        // std::shared_ptr<ModelManager>     model_manager;
        // std::shared_ptr<SettingsManager>  settings_manager;

        // Function to invoke to set up tooltips on Models.
        Widget::TooltipFunc tooltip_func;
    };
    typedef std::shared_ptr<Context> ContextPtr;

    /// Sets a Context containing managers that can be used by derived classes
    /// during operation.
    void SetContext(ContextPtr &context);

    /// Returns the Context passed to SetContext().
    const Context & GetContext() const {
        ASSERT(context_);
        return *context_;
    }

    /// Derived classes implement this to return the name of the type of
    /// derived Command that they execute.
    virtual std::string GetCommandTypeName() const = 0;

    /// Each derived class must implement this function to execute the given
    // command according to the CommandOperation.
    virtual void Execute(Command &command, Command::Op operation) = 0;

  protected:
    // ------------------------------------------------------------------------
    // Helper functions for derived classes.
    // ------------------------------------------------------------------------

    /// If the given name is not empty and differs from the name of the given
    /// Model, this changes the Model's name to match. This protects against
    /// inconsistencies in read-in session files if naming rules ever change.
    void FixModelName(Model &model, const std::string &name);

    /// Returns a SelPath to the model with the given name.
    SelPath FindPathToModel(const std::string &name);

    /// Attaches a click callback to the given model to change selection and
    /// sets up its tooltip.
    void AddModelInteraction(Model &model);

    /// Sets a random color for the Model.
    void SetRandomModelColor(Model &model);

  private:
    ContextPtr context_;
};

typedef std::shared_ptr<Executor> ExecutorPtr;
