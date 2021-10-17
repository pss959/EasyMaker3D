#include "Managers/ActionManager.h"

#include "Assert.h"
#include "Commands/CreatePrimitiveModelCommand.h"
#include "Enums/PrimitiveType.h"

// ----------------------------------------------------------------------------
// ActionManager::Impl_ class.
// ----------------------------------------------------------------------------

class ActionManager::Impl_ {
  public:
    Impl_(const Context &context);

    bool CanApplyAction(Action action) const;
    void ApplyAction(Action action);
    bool ShouldQuit() const { return should_quit_; }

  private:
    const Context context_;
    bool          should_quit_ = false;

    /// Adds a command to create a primitive model of the given type.
    void CreatePrimitiveModel_(PrimitiveType type);
};

ActionManager::Impl_::Impl_(const Context &context) : context_(context) {
    ASSERT(context.command_manager);
    ASSERT(context.selection_manager);
    ASSERT(context.tool_manager);
    ASSERT(context.main_handler);
}

bool ActionManager::Impl_::CanApplyAction(Action action) const {
    // XXXX Need to flesh this out...
    switch (action) {

      case Action::kUndo:
        return context_.command_manager->CanUndo();
      case Action::kRedo:
        return context_.command_manager->CanRedo();

      default:
        // Anything else is assumed to always be possible.
        return true;
    }
}

void ActionManager::Impl_::ApplyAction(Action action) {
    ASSERT(CanApplyAction(action));

    // XXXX Need to flesh this out...
    switch (action) {
      case Action::kUndo:
        context_.command_manager->Undo();
        break;
      case Action::kRedo:
        context_.command_manager->Redo();
        break;
      case Action::kQuit:
        should_quit_ = true;
        break;

      case Action::kCreateBox:
        CreatePrimitiveModel_(PrimitiveType::kBox);
        break;
      case Action::kCreateCylinder:
        CreatePrimitiveModel_(PrimitiveType::kCylinder);
        break;
      case Action::kCreateSphere:
        CreatePrimitiveModel_(PrimitiveType::kSphere);
        break;
      case Action::kCreateTorus:
        CreatePrimitiveModel_(PrimitiveType::kTorus);
        break;

      default:
        // XXXX Do something for real.
        std::cerr << "XXXX Unimplemented action "
                  << Util::EnumName(action) << "\n";
    }
}

void ActionManager::Impl_::CreatePrimitiveModel_(PrimitiveType type) {
    CreatePrimitiveModelCommandPtr cpc =
        Parser::Registry::CreateObject<CreatePrimitiveModelCommand>(
            "CreatePrimitiveModelCommand");
    cpc->SetType(type);
    context_.command_manager->AddAndDo(cpc);
    context_.tool_manager->UseSpecializedTool(
        context_.selection_manager->GetSelection());
}

// ----------------------------------------------------------------------------
// ActionManager functions.
// ----------------------------------------------------------------------------

ActionManager::ActionManager(const Context &context) :
    impl_(new Impl_(context)) {
}

ActionManager::~ActionManager() {
}

bool ActionManager::CanApplyAction(Action action) const {
    return impl_->CanApplyAction(action);
}

void ActionManager::ApplyAction(Action action) {
    impl_->ApplyAction(action);
}

bool ActionManager::ShouldQuit() const {
    return impl_->ShouldQuit();
}
