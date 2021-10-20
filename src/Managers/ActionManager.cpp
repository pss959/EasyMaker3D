#include "Managers/ActionManager.h"

#include "Assert.h"
#include "Commands/CreatePrimitiveModelCommand.h"
#include "Enums/PrimitiveType.h"
#include "Parser/Writer.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Shape.h"

// ----------------------------------------------------------------------------
// ActionManager::Impl_ class.
// ----------------------------------------------------------------------------

class ActionManager::Impl_ {
  public:
    Impl_(const ContextPtr &context);

    void SetReloadFunc(const ReloadFunc &func) { reload_func_ = func; }
    bool CanApplyAction(Action action) const;
    void ApplyAction(Action action);
    bool ShouldQuit() const { return should_quit_; }

  private:
    ContextPtr            context_;
    bool                  should_quit_ = false;
    std::function<void()> reload_func_;

    /// Adds a command to create a primitive model of the given type.
    void CreatePrimitiveModel_(PrimitiveType type);

#if DEBUG
    /// \name Debugging functions.
    /// Each of these handles an Action (in debug builds only) that help debug
    /// the application.
    ///@{
    void PrintBounds_();
    void PrintMatrices_();
    void PrintScene_();
    void ReloadScene_();

    static void PrintNodeBounds_(const SG::Node &node, int level);
    static void PrintNodeMatrices_(const SG::Node &node, int level,
                                   const Matrix4f &start_matrix);

    static std::string Indent_(int level, bool add_horiz = true) {
        std::string s;
        for (int i = 1; i < level; ++i)
            s += "| ";
        if (level >= 1)
            s += add_horiz ? "|-" : "| ";
        return s;
    }
    ///@}
#endif
};

ActionManager::Impl_::Impl_(const ContextPtr &context) : context_(context) {
    ASSERT(context);
    ASSERT(context->command_manager);
    ASSERT(context->selection_manager);
    ASSERT(context->target_manager);
    ASSERT(context->tool_manager);
    ASSERT(context->main_handler);
}

bool ActionManager::Impl_::CanApplyAction(Action action) const {
    // XXXX Need to flesh this out...
    switch (action) {

      case Action::kUndo:
        return context_->command_manager->CanUndo();
      case Action::kRedo:
        return context_->command_manager->CanRedo();

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
        context_->command_manager->Undo();
        break;
      case Action::kRedo:
        context_->command_manager->Redo();
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

#if defined DEBUG
      case Action::kPrintBounds:   PrintBounds_();   break;
      case Action::kPrintMatrices: PrintMatrices_(); break;
      case Action::kPrintScene:    PrintScene_();    break;
      case Action::kReloadScene:   ReloadScene_();   break;
#endif

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
    context_->command_manager->AddAndDo(cpc);
    context_->tool_manager->UseSpecializedTool(
        context_->selection_manager->GetSelection());
}

void ActionManager::Impl_::PrintBounds_() {
    std::cout << "--------------------------------------------------\n";
    PrintNodeBounds_(*context_->scene->GetRootNode(), 0);
    std::cout << "--------------------------------------------------\n";
}

void ActionManager::Impl_::PrintMatrices_() {
    std::cout << "--------------------------------------------------\n";
    PrintNodeMatrices_(*context_->scene->GetRootNode(), 0, Matrix4f::Identity());
    std::cout << "--------------------------------------------------\n";
}

void ActionManager::Impl_::PrintScene_() {
    std::cout << "--------------------------------------------------\n";
    Parser::Writer writer;
    writer.SetAddressFlag(true);
    writer.WriteObject(*context_->scene, std::cout);
    std::cout << "--------------------------------------------------\n";
}

void ActionManager::Impl_::ReloadScene_() {
    ASSERT(reload_func_);
    reload_func_();
}

void ActionManager::Impl_::PrintNodeBounds_(const SG::Node &node, int level) {
    std::cout << Indent_(level) << node.GetDesc()
              << " " << node.GetBounds().ToString() << "\n";
    for (const auto &shape: node.GetShapes()) {
        std::cout << Indent_(level + 1) << shape->GetDesc() << " "
                  << shape->GetBounds().ToString() << "\n";
    }
    for (const auto &child: node.GetChildren())
        PrintNodeBounds_(*child, level + 1);
}

void ActionManager::Impl_::PrintNodeMatrices_(const SG::Node &node, int level,
                                              const Matrix4f &start_matrix) {
    std::cout << Indent_(level) << node.GetDesc() << " "
              << node.GetModelMatrix() << "\n";

    const Matrix4f combined = start_matrix * node.GetModelMatrix();
    std::cout << Indent_(level, false) << "        => " << combined << "\n";

    for (const auto &child: node.GetChildren())
        PrintNodeMatrices_(*child, level + 1, combined);
}

// ----------------------------------------------------------------------------
// ActionManager functions.
// ----------------------------------------------------------------------------

ActionManager::ActionManager(const ContextPtr &context) :
    impl_(new Impl_(context)) {
}

ActionManager::~ActionManager() {
}

void ActionManager::SetReloadFunc(const ReloadFunc &func) {
    impl_->SetReloadFunc(func);
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
