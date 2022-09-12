#pragma once

#include <string>
#include <vector>

#include "Enums/Hand.h"
#include "Math/Types.h"

/// The SnapScript class is used by the snapimage application to read and store
/// a script of instructions used to generate images for public documentation.
///
/// A script contains any number of lines with one of the following
/// instructions:
///
///  Text                                 | Result
///  -------------------------------------|-------
///  [whitespace]                         | Blank lines are ignored.
///  <b>\#</b> ...                        | Comment (ignored).
///  **hand** *L/R* *type* *pos* *dir*    | Shows controller for the given hand.
///  **load** *file_name*                 | Loads a session file.
///  **redo** *n*                         | Redoes n commands.
///  **select** [*name* ...]              | Selects named models (or none).
///  **snap** *x* *y* *w* *h* *file_name* | Saves a snapshot to a file.
///  **stage** *scale* *rot_angle*        | Scales and rotates the stage.
///  **undo** *n*                         | Undoes the last n commands.
///  **view** *dir*                       | Views along the given direction.
///
/// Notes:
/// \li All whitespace is ignored.
/// \li Session files are relative to \c PublicDoc/snaps/sessions.
/// \li Saved snapshot files are relative to \c PublicDoc/snaps/images.
/// \li Snap coordinates are floating point fractions in the range (0,1) in
///     both dimensions.
/// \li All *pos* and *dir* arguments are 3D points or vectors.
/// \li The *type* for the **hand** instruction is one of "Oculus_Touch" or
///     "Vive"
///
/// \ingroup App
class SnapScript {
  public:
    struct Instruction {
        std::string              type;         ///< Type of instruction.
        std::string              file_name;    ///< For "load" or "snap".
        float                    stage_scale;  ///< For "stage".
        Anglef                   stage_angle;  ///< For "stage".
        Range2f                  rect;         ///< For "snap".
        Vector3f                 view_dir;     ///< For "view".
        size_t                   count;        ///< For "undo" or "redo".
        std::vector<std::string> names;        ///< For "select".
        Hand                     hand;         ///< For "hand".
        std::string              hand_type;    ///< For "hand".
        Point3f                  hand_pos;     ///< For "hand".
        Vector3f                 hand_dir;     ///< For "hand".
    };

    /// Reads a script from a file relative to the PublicDoc/snaps/scripts
    /// directory to fill in instructions. Returns false on error.
    bool ReadScript(const std::string &file_name);

    /// Returns the instructions in the script.
    const std::vector<Instruction> & GetInstructions() const {
        return instructions_;
    }

  private:
    typedef std::vector<std::string> Words_;

    std::string               file_name_;
    size_t                    line_number_;
    std::vector<Instruction>  instructions_;

    bool ProcessLine_(const std::string &line);

    bool ProcessHand_(const Words_ &words, Instruction &instr);
    bool ProcessLoad_(const Words_ &words, Instruction &instr);
    bool ProcessRedo_(const Words_ &words, Instruction &instr);
    bool ProcessSelect_(const Words_ &words, Instruction &instr);
    bool ProcessSnap_(const Words_ &words, Instruction &instr);
    bool ProcessStage_(const Words_ &words, Instruction &instr);
    bool ProcessUndo_(const Words_ &words, Instruction &instr);
    bool ProcessView_(const Words_ &words, Instruction &instr);

    bool Error_(const std::string &message);

    static bool ParseVector3f_(const Words_ &words, size_t index,  Vector3f &v);
    static bool ParseFloat_(const std::string &s, float &f);
    static bool ParseFloat01_(const std::string &s, float &f);
    static bool ParseN_(const std::string &s, size_t &n);
};
