#pragma once

#include <concepts>
#include <vector>

#include "App/Application.h"
#include "App/ScriptBase.h"
#include "Util/Memory.h"

class Selection;
DECL_SHARED_PTR(ScriptBase);
DECL_SHARED_PTR(ScriptEmitter);

/// ScriptedApp is an abstract class derived from Application that adds
/// processing of a read-in script that specifies what to do.
///
/// \ingroup App
class ScriptedApp : public Application {
  public:
    /// This struct adds some additional options.
    struct Options : public Application::Options {
        virtual ~Options() {}  // Makes this polymorphic.
        bool remain = false;  ///< Leave the window up after the script is done.
        bool report = false;  ///< Report each instruction when executed.
    };
    DECL_SHARED_PTR(Options);

    /// Initializes the ScriptedApp with a (derived) Options instance and a
    /// (derived) Script instance. Derived classes should call this in addition
    /// to adding their own code. Returns false if anything goes wrong.
    virtual bool Init(const OptionsPtr &options, const ScriptBasePtr &script);

    /// Redefines this to add script processing during frames.
    virtual bool ProcessFrame(size_t render_count, bool force_poll) override;

    // Makes this available to derived applications.
    using Application::GetContext;

  protected:
    /// Returns the options. Asserts if Init() was not called to initialize
    /// them.
    const Options & GetOptions() const;

    /// Returns the ScriptEmitter. Asserts if Init() was not called to
    /// initialize it.
    ScriptEmitter & GetEmitter() const;

    /// Derived classes must implement this to process a specific (derived)
    /// instruction, returning false on error.
    virtual bool ProcessInstruction(const ScriptBase::Instr &instr) = 0;

    /// Convenience that casts a ScriptBase::Instr to the templated type.
    template <typename T>
    const T & GetTypedInstr_(const ScriptBase::Instr &instr) {
        static_assert(std::derived_from<T, ScriptBase::Instr> == true);
        return static_cast<const T &>(instr);
    }

  private:
    OptionsPtr       options_;      ///< Derived Options instance.
    ScriptBasePtr    script_;       ///< Derived ScriptBase instance.
    ScriptEmitterPtr emitter_;      ///< Used to simulate mouse and key events.
    size_t           cur_instruction_ = 0;

    bool ProcessInstruction_(const ScriptBase::Instr &instr);
};
