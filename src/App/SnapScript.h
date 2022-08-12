#pragma once

#include <string>
#include <vector>

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
///  **load** *file_name*                 | Loads a session.
///  **snap** *x* *y* *w* *h* *file_name* | Saves a snapshot to a file.
///  **undo** *n*                         | Undoes the last n commands.
///  **redo** *n*                         | Redoes n commands.
///
/// Notes:
/// \li All whitespace is ignored.
/// \li Session files are relative to \c PublicDoc/snaps/sessions.
/// \li Saved snapshot files are relative to \c PublicDoc/snaps/images.
/// \li Snap coordinates are floating point fractions in the range (0,1) in
///     both dimensions.
///
/// \ingroup App
class SnapScript {
  public:
    struct Instruction {
        std::string type;        // Type of instruction.
        std::string file_name;   // For "load" or "snap".
        Range2f     rect;        // For "snap".
        size_t      count;       // For "undo" or "redo".
    };

    /// Reads a script from a file relative to the PublicDoc/snaps/scripts
    /// directory to fill in instructions. Returns false on error.
    bool ReadScript(const std::string &file_name);

    /// Returns the instructions in the script.
    const std::vector<Instruction> & GetInstructions() const {
        return instructions_;
    }

  private:
    std::string               file_name_;
    size_t                    line_number_;
    std::vector<Instruction>  instructions_;

    bool ProcessLine_(const std::string &line);
    bool Error_(const std::string &message);

    static bool ParseFloat01_(const std::string &s, float &f);
    static bool ParseN_(const std::string &s, size_t &n);
};
