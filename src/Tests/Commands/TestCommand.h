#pragma once

#include "Commands/Command.h"

DECL_SHARED_PTR(TestCommand);

/// Derived Command class for testing.
///
/// \ingroup Tests
class TestCommand : public Command {
  public:
    using Command::FixPath;  // Make this callable.

    /// Sets a flag indicating whether the command should be added as an orphan.
    void SetShouldBeAddedAsOrphan(bool b) { orphan_ = b; }

    /// Sets a flag indicating whether the command has undo/redo effect.
    void SetHasUndoRedoEffect(bool b) { undo_redo_ = b; }

    /// Sets the text field to test read/write.
    void SetText(const Str &s) { text_ = s; }

    /// Returns the text field contents.
    const Str & GetText() const { return text_; }

    virtual bool HasUndoEffect()         const override { return undo_redo_; }
    virtual bool HasRedoEffect()         const override { return undo_redo_; }
    virtual bool ShouldBeAddedAsOrphan() const override { return orphan_;    }
    virtual Str  GetDescription()        const override { return "TEST!";   }

    /// Creates an instance.
    static TestCommandPtr Create(const Str &name = "") {
        return Parser::Registry::CreateObject<TestCommand>(name);
    }

  protected:
    TestCommand() {}

    virtual void AddFields() override {
        AddField(text_.Init("text"));
        Command::AddFields();
    }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str> text_;  ///< For testing read/write.
    ///@}

    bool undo_redo_ = true;   ///< For testing undo/redo effects.
    bool orphan_    = false;  ///< For testing orphaned commands.
    friend class Parser::Registry;
};
