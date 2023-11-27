#pragma once

#include <functional>
#include <map>
#include <vector>

#include "Math/Types.h"
#include "Util/FilePath.h"
#include "Util/Memory.h"

/// ScriptBase is a base class for scripts used to generate images and videos
/// for public documentation.
///
/// \ingroup App
class ScriptBase {
  public:
    /// Base Instruction struct.
    struct Instr {
        Str name;             ///< Name of the instruction.
        int line_number = 0;  ///< Line it appeared on (for messages).
        virtual ~Instr() {}   // Makes deletion work properly.
    };

    DECL_SHARED_PTR(Instr);

    /// Reads instructions from the script at the given FilePath. Returns false
    /// on error.
    bool ReadScript(const FilePath &path);

    /// Returns the path to the file the script was read from.
    const FilePath & GetPath() const { return file_path_; }

    /// Returns the instructions in the script.
    const std::vector<InstrPtr> & GetInstructions() const {
        return instructions_;
    }

  protected:
    using InstrFunc = std::function<InstrPtr(const StrVec &)>;

    /// Derived classes can call this to register a function to invoke to
    /// process an instruction with the given name. The function is passed the
    /// words parsed from a line in the script file and should return a pointer
    /// to the resulting (derived) Instr, or null if there was an error.
    void RegisterInstrFunc(const Str &name, const InstrFunc &func);

    /// \name Parsing helpers.
    /// Each of these parses an item from one or more words, returning false on
    /// parse error.
    ///@{
    bool ParseVector3f(const StrVec &words, size_t index, Vector3f &v);
    bool ParseFloat(const Str &s, float &f);
    bool ParseFloat01(const Str &s, float &f);
    bool ParseN(const Str &s, size_t &n);
    //@}

    /// Reports an error.
    void Error(const Str &message);

  private:
    using FuncMap_ = std::map<Str, InstrFunc>;

    /// This maps instruction names to functions.
    FuncMap_              func_map_;

    FilePath              file_path_;     ///< Script file being processed.
    size_t                line_number_;   ///< Current line number in the file.
    std::vector<InstrPtr> instructions_;  ///< Resulting instructions.

    /// Processes one line of the script.
    bool ProcessLine_(const Str &line);

    /// Returns words from the given line, processing in-line comments.
    static StrVec GetWords_(const Str &line);
};
