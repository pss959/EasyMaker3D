#pragma once

#include <string>
#include <vector>

#include "Base/Memory.h"
#include "Enums/Action.h"
#include "Enums/Hand.h"
#include "Math/Types.h"
#include "Util/FilePath.h"

/// The SnapScript class is used by the snapimage application to read and store
/// a script of instructions used to generate images for public documentation.
///
/// A script contains any number of lines with one of the following
/// instructions:
///
/// <table>
///   <tr>
///     <th>Command</th>
///     <th>Arguments</th>
///     <th>Effect</th>
///   </tr>
///   <tr>
///     <td>[whitespace]</td>
///     <td></td>
///     <td>Blank lines are ignored.</td>
///   </tr>
///   <tr>
///     <td><b>#</b></td>
///     <td>...</td>
///     <td>Comments are ignored.</td>
///   </tr>
///   <tr>
///     <td><b>action</b></td>
///     <td><i>name</i></td>
///     <td>Applies the named action.</td>
///   </tr>
///   <tr>
///     <td><b>click</b></td>
///     <td><i>x y</i></td>
///     <td>Simulates a mouse click at the given point in normalized window
///     coordinates.</td>
///   </tr>
///   <tr>
///     <td><b>drag</b></td>
///     <td><code>start</code>|<code>continue</code>|<code>end</code> <i>x
///     y</i></td>
///     <td>Simulates a mouse drag at the given point in normalized window
///     coordinates.</td>
///   </tr>
///   <tr>
///     <td><b>hand</b></td>
///     <td><code>L</code>|<code>R</code> <i>type posx posy posz dirx diry
///     dirz</i></td>
///     <td>Shows a controller for the given hand at the given position and
///     pointing along the given direction. *type* is one of
///     {<code>Oculus_Touch</code>, <code>Vive</code>, or <code>None</code>}.
///   </tr>
///   <tr>
///     <td><b>key</b></td>
///     <td><i>key [modifiers]</i></td>
///     <td>Simulates a keyboard key press and release for the given
///     string. The <i>modifiers</i> are either <code>ctrl</code> or
///     <code>alt</code>.
///   </tr>
///   <tr>
///     <td><b>load</b></td>
///     <td><i>file_name</i></td>
///     <td>Loads a session from the named file. Session files are relative
///     to <code>PublicDoc/snaps/sessions</code>.</td>
///   </tr>
///   <tr>
///     <td><b>select</b></td>
///     <td>[<i>name</i>, ...]</td>
///     <td>Selects the named models. If no names are given, deselects all
///     models.</td>
///   </tr>
///   <tr>
///     <td><b>settings</b></td>
///     <td><i>file_name</i></td>
///     <td>Loads settings from the named file. Settings files are relative
///     to <code>PublicDoc/snaps/settings</code>.</td>
///   </tr>
///   <tr>
///     <td><b>snap</b></td>
///     <td><i>x y w h file_name</i></td>
///     <td>Saves a snapshot image of the area of size <i>w</i>X<i>h</i> with
///     (<i>x</i>,<i>y</i>) at the lower left to the named file. All values are
///     in normalized window coordinates. Images are saved
///     in <code>PublicDoc/docs/images</code>.</td>
///   </tr>
///   <tr>
///     <td><b>stage</b></td>
///     <td><i>scale rot_angle</i></td>
///     <td>Scales and rotates the stage by the given values.</td>
///   </tr>
///   <tr>
///     <td><b>touch</b></td>
///     <td><code>on</code>|<code>off</code></td>
///     <td>Turns panel touch mode on or off.</td>
///   </tr>
///   <tr>
///     <td><b>view</b></td>
///     <td><i>dirx diry dirz</i></td>
///     <td>Changes the view to look along the given direction.</td>
///   </tr>
/// </table>
///
/// \ingroup App
class SnapScript {
  public:
    /// Handy typedef for multiple words or names.
    typedef std::vector<std::string> Words;

    /// Base Instruction struct.
    struct Instr {
        /// Types of instructions.
        enum class Type {
            kAction,
            kClick,
            kDrag,
            kHand,
            kKey,
            kLoad,
            kSelect,
            kSettings,
            kSnap,
            kStage,
            kTouch,
            kView,
        };

        Type type;
        virtual ~Instr() {}   // Makes deletion work properly.
    };
    struct ActionInstr : public Instr {
        Action action;
    };
    struct ClickInstr : public Instr {
        Point2f     pos;
    };
    struct DragInstr : public Instr {
        enum class Phase { kStart, kContinue, kEnd };
        Phase       phase;
        Point2f     pos;
    };
    struct HandInstr : public Instr {
        Hand        hand;
        std::string controller;
        Point3f     pos;
        Vector3f    dir;
    };
    struct KeyInstr : public Instr {
        std::string key;
        bool        is_ctrl_on;
        bool        is_alt_on;
    };
    struct LoadInstr : public Instr {
        std::string file_name;
    };
    struct SelectInstr : public Instr {
        Words names;
    };
    struct SettingsInstr : public Instr {
        std::string file_name;
    };
    struct SnapInstr : public Instr {
        Range2f     rect;
        std::string file_name;
    };
    struct StageInstr : public Instr {
        float  scale;
        Anglef angle;
    };
    struct TouchInstr : public Instr {
        bool is_on;
    };
    struct ViewInstr : public Instr {
        Vector3f dir;
    };

    DECL_SHARED_PTR(Instr);
    DECL_SHARED_PTR(ActionInstr);
    DECL_SHARED_PTR(ClickInstr);
    DECL_SHARED_PTR(DragInstr);
    DECL_SHARED_PTR(HandInstr);
    DECL_SHARED_PTR(KeyInstr);
    DECL_SHARED_PTR(LoadInstr);
    DECL_SHARED_PTR(SelectInstr);
    DECL_SHARED_PTR(SettingsInstr);
    DECL_SHARED_PTR(SnapInstr);
    DECL_SHARED_PTR(StageInstr);
    DECL_SHARED_PTR(TouchInstr);
    DECL_SHARED_PTR(ViewInstr);

    /// Reads instructions from the script at the given FilePath. Returns false
    /// on error.
    bool ReadScript(const FilePath &path);

    /// Returns the instructions in the script.
    const std::vector<InstrPtr> & GetInstructions() const {
        return instructions_;
    }

  private:
    FilePath              file_path_;
    size_t                line_number_;
    std::vector<InstrPtr> instructions_;

    bool ProcessLine_(const std::string &line);
    bool GetInstructionType_(const std::string &word, Instr::Type &type);

    InstrPtr ProcessAction_(const Words &words);
    InstrPtr ProcessClick_(const Words &words);
    InstrPtr ProcessDrag_(const Words &words);
    InstrPtr ProcessHand_(const Words &words);
    InstrPtr ProcessKey_(const Words &words);
    InstrPtr ProcessLoad_(const Words &words);
    InstrPtr ProcessSelect_(const Words &words);
    InstrPtr ProcessSettings_(const Words &words);
    InstrPtr ProcessSnap_(const Words &words);
    InstrPtr ProcessStage_(const Words &words);
    InstrPtr ProcessTouch_(const Words &words);
    InstrPtr ProcessView_(const Words &words);

    bool Error_(const std::string &message);

    static bool ParseVector3f_(const Words &words, size_t index,  Vector3f &v);
    static bool ParseFloat_(const std::string &s, float &f);
    static bool ParseFloat01_(const std::string &s, float &f);
    static bool ParseN_(const std::string &s, size_t &n);
};
