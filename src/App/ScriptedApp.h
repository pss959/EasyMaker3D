#pragma once

#include <concepts>
#include <vector>

#include "App/Application.h"
#include "App/ScriptBase.h"
#include "Math/Types.h"
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

    /// Returns the script. Asserts if Init() was not called to initialize it.
    const ScriptBase & GetScript() const;

    /// Returns the ScriptEmitter. Asserts if Init() was not called to
    /// initialize it.
    ScriptEmitter & GetEmitter() const;

    /// Derived classes must implement this to process a specific (derived)
    /// instruction, returning false on error.
    virtual bool ProcessInstruction(const ScriptBase::Instr &instr) = 0;

    /// Lets derived classes know when instructions have all been processed.
    /// The base class defines this to do nothing.
    virtual void InstructionsDone() {}

    /// Allows derived classes to pause or resume processing, meaning that
    /// instructions will not be processed and BeginFrame()/EndFrame() will not
    /// be called until unpaused. Returns true if the application is now
    /// paused.
    bool PauseOrUnpause();

    /// Lets derived classes know when a frame is starting. The base class
    /// defines this to do nothing.
    virtual void BeginFrame() {}

    /// Lets derived classes know when a frame is ending. The base class
    /// defines this to do nothing.
    virtual void EndFrame() {}

    /// Loads Settings from the given path, updating the SettingsManager and
    /// SessionManager. Returns false on error.
    bool LoadSettings(const FilePath &path);

    /// Convenience that casts a ScriptBase::Instr to the templated type.
    template <typename T>
    const T & GetTypedInstr_(const ScriptBase::Instr &instr) {
        static_assert(std::derived_from<T, ScriptBase::Instr> == true);
        return static_cast<const T &>(instr);
    }

    /// Sets \p rect to an image-plane rectangle surrounding the named Node
    /// with the given margin on all sides. Prints an error message and returns
    /// false if the Node is not found.
    bool GetNodeRect(const Str &name, float margin, Range2f &rect);

  private:
    OptionsPtr       options_;      ///< Derived Options instance.
    ScriptBasePtr    script_;       ///< Derived ScriptBase instance.
    ScriptEmitterPtr emitter_;      ///< Used to simulate mouse and key events.

    bool   is_paused_       = false;
    size_t cur_instruction_ = 0;
};
