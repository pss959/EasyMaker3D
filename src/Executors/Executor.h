#pragma once

#include <functional>

#include "Base/Memory.h"
#include "Commands/Command.h"
#include "SelPath.h"
#include "Util/Assert.h"
#include "Widgets/Widget.h"

DECL_SHARED_PTR(AnimationManager);
DECL_SHARED_PTR(ClipboardManager);
DECL_SHARED_PTR(ColorManager);
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
        ColorManagerPtr     color_manager;
        CommandManagerPtr   command_manager;
        NameManagerPtr      name_manager;
        SelectionManagerPtr selection_manager;
        SettingsManagerPtr  settings_manager;
        TargetManagerPtr    target_manager;

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

    /// Convenience that converts the given Command instance to a derived
    /// Command, asserting that it is of that type.
    template <typename T> static T & GetTypedCommand(Command &command) {
        ASSERT(dynamic_cast<T *>(&command));
        return static_cast<T &>(command);
    }

    /// Convenience that converts the Model at the tail of the given SelPath to
    /// the derived Model type, asserting that it is of that type.
    template <typename T> static T & GetTypedModel(const SelPath &sel_path) {
        std::shared_ptr<T> tm = Util::CastToDerived<T>(sel_path.GetModel());
        ASSERT(tm);
        return *tm;
    }

    /// Uses the NameManager to create a unique name with the given prefix.
    std::string CreateUniqueName(const std::string &prefix);

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
