#pragma once

#include <concepts>
#include <vector>

#include "App/Application.h"
#include "App/Script.h"
#include "Math/Types.h"
#include "SG/NodePath.h"
#include "Util/Memory.h"

class Args;
class FilePath;
class Selection;
DECL_SHARED_PTR(ScriptEmitter);

/// ScriptedApp is a derived Application class that adds processing of a
/// read-in script that specifies what to do.
///
/// \ingroup App
class ScriptedApp : public Application {
  public:
    /// This struct adds some additional options.
    struct Options : public Application::Options {
        virtual ~Options() {}  // Makes this polymorphic.

        Str  vidformat = "webm";  ///< Video format as a string.
        bool dryrun    = false;   ///< Do not create images or videos.
        bool remain    = false;   ///< Keep the window after the script is done.
        bool report    = false;   ///< Report each instruction when executed.
        int  fps       = 30;      ///< Frames per second (default 30).
    };

    /// Initializes Options from command-line arguments.
    void InitOptions(const Args &args);

    /// Returns the Options.
    Options & GetOptions() { return options_; }

    /// Processes the Script named by the given FilePath. Returns false if
    /// anything goes wrong.
    bool ProcessScript(const FilePath &script_path);

  protected:
    /// Redefines this to add script processing during frames.
    virtual bool ProcessFrame(size_t render_count, bool force_poll) override;

    /// Processes one instruction.
    bool ProcessInstruction(const Script::Instr &instr);

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
    Options          options_;
    Script           script_;
    ScriptEmitterPtr emitter_;      ///< Used to simulate mouse and key events.

    bool   is_paused_       = false;
    size_t cur_instruction_ = 0;
};
