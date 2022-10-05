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
/// \htmlinclude SnapScript.html
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
            kHandPos,
            kKey,
            kLoad,
            kMod,
            kSelect,
            kSettings,
            kSnap,
            kStage,
            kTouch,
            kView,
        };

        Type type;
        int  line_number = 0;
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
    };
    struct HandPosInstr : public Instr {
        Hand        hand;
        Point3f     pos;
        Rotationf   rot;
    };
    struct KeyInstr : public Instr {
        std::string key;
        bool        is_ctrl_on;
        bool        is_alt_on;
    };
    struct LoadInstr : public Instr {
        std::string file_name;
    };
    struct ModInstr : public Instr {
        bool is_on;
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
    DECL_SHARED_PTR(HandPosInstr);
    DECL_SHARED_PTR(KeyInstr);
    DECL_SHARED_PTR(LoadInstr);
    DECL_SHARED_PTR(ModInstr);
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
    InstrPtr ProcessHandPos_(const Words &words);
    InstrPtr ProcessKey_(const Words &words);
    InstrPtr ProcessLoad_(const Words &words);
    InstrPtr ProcessMod_(const Words &words);
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
