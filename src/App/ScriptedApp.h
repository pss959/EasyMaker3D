#pragma once

#include <concepts>
#include <vector>

#include "App/Application.h"
#include "App/Script.h"
#include "Math/Types.h"
#include "SG/NodePath.h"
#include "Util/Memory.h"

class Selection;
DECL_SHARED_PTR(Script);
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
    /// Script instance. Derived classes should call this in addition to adding
    /// their own code. Returns false if anything goes wrong.
    virtual bool Init(const OptionsPtr &options, const ScriptPtr &script);

    /// Redefines this to add script processing during frames.
    virtual bool ProcessFrame(size_t render_count, bool force_poll) override;

    // Makes this available to derived applications.
    using Application::GetContext;

  protected:
    /// Returns the options. Asserts if Init() was not called to initialize
    /// them.
    const Options & GetOptions() const;

    /// Returns the script. Asserts if Init() was not called to initialize it.
    const Script & GetScript() const;

    /// Returns the ScriptEmitter. Asserts if Init() was not called to
    /// initialize it.
    ScriptEmitter & GetEmitter() const;

    /// Derived classes must implement this to process a specific (derived)
    /// instruction, returning false on error.
    virtual bool ProcessInstruction(const Script::Instr &instr) = 0;

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

    /// Convenience that casts a Script::Instr to the templated type.
    template <typename T>
    const T & GetTypedInstr_(const Script::Instr &instr) {
        static_assert(std::derived_from<T, Script::Instr> == true);
        return static_cast<const T &>(instr);
    }

    /// Sets \p rect to an image-plane rectangle surrounding the Node
    /// referenced by the given path string with given margin on all
    /// sides. Prints an error message and returns false if the Node is not
    /// found.
    bool GetNodeRect(const Str &path_string, float margin, Range2f &rect);

    /// Resolves a Node path string to an SG::NodePath.
    SG::NodePath GetNodePath(const Str &path_string);

  private:
    OptionsPtr       options_;      ///< Derived Options instance.
    ScriptPtr        script_;       ///< Script instance.
    ScriptEmitterPtr emitter_;      ///< Used to simulate mouse and key events.

    bool   is_paused_       = false;
    size_t cur_instruction_ = 0;
};
