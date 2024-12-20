//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <concepts>
#include <functional>

#include "Commands/Command.h"
#include "Selection/SelPath.h"
#include "Util/Assert.h"
#include "Util/Memory.h"
#include "Widgets/Widget.h"

DECL_SHARED_PTR(AnimationManager);
DECL_SHARED_PTR(ClipboardManager);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(Executor);
DECL_SHARED_PTR(NameManager);
DECL_SHARED_PTR(RootModel);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SettingsManager);
DECL_SHARED_PTR(TargetManager);

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
        ClipboardManagerPtr clipboard_manager;
        CommandManagerPtr   command_manager;
        NameManagerPtr      name_manager;
        SelectionManagerPtr selection_manager;
        SettingsManagerPtr  settings_manager;
        TargetManagerPtr    target_manager;

        // Function to invoke to set up tooltips on Models.
        Widget::TooltipFunc tooltip_func;
    };
    DECL_SHARED_PTR(Context);

    /// Sets a Context containing managers that can be used by derived classes
    /// during operation.
    void SetContext(ContextPtr &context);

    /// Version of SetContext() used for testing - not all parts need to be
    /// present in the Context.
    void SetTestContext(const ContextPtr &context);

    /// Returns the Context passed to SetContext().
    const Context & GetContext() const {
        ASSERT(context_);
        return *context_;
    }

    /// Derived classes implement this to return the name of the type of
    /// derived Command that they execute.
    virtual Str GetCommandTypeName() const = 0;

    /// Each derived class must implement this function to execute the given
    // command according to the CommandOperation.
    virtual void Execute(Command &command, Command::Op operation) = 0;

  protected:
    // ------------------------------------------------------------------------
    // Helper functions for derived classes.
    // ------------------------------------------------------------------------

    /// Convenience that converts the given Command instance to a derived
    /// Command, asserting that it is of that type.
    template <typename T> static T & GetTypedCommand(Command &command) {
        static_assert(std::derived_from<T, Command> == true);
        ASSERT(dynamic_cast<T *>(&command));
        return static_cast<T &>(command);
    }

    /// Convenience that converts the Model at the tail of the given SelPath to
    /// the derived Model type, asserting that it is of that type.
    template <typename T> static T & GetTypedModel(const SelPath &sel_path) {
        static_assert(std::derived_from<T, Model> == true);
        auto tm = std::dynamic_pointer_cast<T>(sel_path.GetModel());
        ASSERT(tm);
        return *tm;
    }

    /// Uses the NameManager to create a unique name with the given prefix.
    Str CreateUniqueName(const Str &prefix);

    /// Returns a SelPath to the model with the given name.
    SelPath FindPathToModel(const Str &name);

    /// Attaches a click callback to the given model to change selection and
    /// sets up its tooltip.
    void AddModelInteraction(Model &model);

    /// Positions a Model at the current point target or the origin.
    void InitModelPosition(Model &model);

    /// Sets a random color for the Model.
    void SetRandomModelColor(Model &model);

  private:
    ContextPtr context_;
};
