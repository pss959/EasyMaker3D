#pragma once

#include <functional>
#include <map>
#include <vector>

#include "Base/Event.h"
#include "Enums/Action.h"
#include "Enums/Hand.h"
#include "Math/Types.h"
#include "Util/FilePath.h"
#include "Util/Memory.h"

/// The Script class reads and stores scripted instructions used to generate
/// images and videos for public documentation. Note that not all instruction
/// types are used for both snaps and videos.
///
/// A script contains any number of lines with one of the following
/// instructions:
///
/// \htmlinclude[block] Script.html
///
/// \ingroup App
class Script {
  public:
    /// Base Instruction struct.
    struct Instr {
        Str name;             ///< Name of the instruction.
        int line_number = 0;  ///< Line it appeared on (for messages).
        virtual ~Instr() {}   // Makes deletion work properly.
    };
    DECL_SHARED_PTR(Instr);

    struct ActionInstr : public Instr {
        Action action;
    };
    struct CaptionInstr : public Instr {
        Point2f pos;
        float   duration;
        Str     text;
    };
    struct ClickInstr : public Instr {
        Event::Device device;
        Event::Button button;
    };
    struct DragInstr : public Instr {
        Vector2f      motion;
        float         duration;
        Event::Button button;
    };
    struct DragStartInstr : public Instr {
        Vector2f motion;
        float    duration;
    };
    struct DragEndInstr : public Instr {
        // No data.
    };
    struct FilesInstr : public Instr {
        Str setting;
    };
    struct FocusInstr : public Instr {
        Str pane_name;
    };
    struct HandModelInstr : public Instr {
        Hand hand;
        Str  controller;
    };
    struct HandMoveInstr : public Instr {
        Hand          hand;
        Vector3f      trans;
        Rotationf     rot;
        float         duration;
        Event::Button button;
    };
    struct HandPointInstr : public Instr {
        Hand          hand;
        Str           path_string;
        float         duration;
        Event::Button button;
    };
    struct HandPosInstr : public Instr {
        Hand      hand;
        Point3f   pos;
        Rotationf rot;
        float     duration;
    };
    struct HandTouchInstr : public Instr {
        Hand  hand;
        Str   path_string;
        float duration;
    };
    struct HighlightInstr : public Instr {
        StrVec path_strings;
        float  margin;
        float  duration;
    };
    struct KeyInstr : public Instr {
        Str key_string;
    };
    struct LoadInstr : public Instr {
        Str file_name;
    };
    struct MoveOverInstr : public Instr {
        Str      path_string;
        float    duration;
        Vector2f offset;
    };
    struct MoveToInstr : public Instr {
        Point2f pos;
        float   duration;
    };
    struct SectionInstr : public Instr {
        Str tag;
        Str title;
    };
    struct SelectInstr : public Instr {
        StrVec names;
    };
    struct SettingsInstr : public Instr {
        Str file_name;
    };
    struct SnapInstr : public Instr {
        Range2f rect;
        Str     file_name;
    };
    struct SnapObjInstr : public Instr {
        Str   path_string;
        float margin;
        Str   file_name;
    };
    struct StageInstr : public Instr {
        float  scale;
        Anglef angle;
    };
    struct StateInstr : public Instr {
        Str  setting;
        bool is_on;
    };
    struct StopInstr : public Instr {
        // No data.
    };
    struct ViewInstr : public Instr {
        float    gantry_height;
        Vector3f dir;
    };
    struct WaitInstr : public Instr {
        float duration;
    };
    DECL_SHARED_PTR(ActionInstr);
    DECL_SHARED_PTR(CaptionInstr);
    DECL_SHARED_PTR(ClickInstr);
    DECL_SHARED_PTR(DragInstr);
    DECL_SHARED_PTR(DragStartInstr);
    DECL_SHARED_PTR(DragEndInstr);
    DECL_SHARED_PTR(FilesInstr);
    DECL_SHARED_PTR(FocusInstr);
    DECL_SHARED_PTR(HandModelInstr);
    DECL_SHARED_PTR(HandMoveInstr);
    DECL_SHARED_PTR(HandPointInstr);
    DECL_SHARED_PTR(HandPosInstr);
    DECL_SHARED_PTR(HandTouchInstr);
    DECL_SHARED_PTR(HighlightInstr);
    DECL_SHARED_PTR(KeyInstr);
    DECL_SHARED_PTR(LoadInstr);
    DECL_SHARED_PTR(MoveOverInstr);
    DECL_SHARED_PTR(MoveToInstr);
    DECL_SHARED_PTR(SectionInstr);
    DECL_SHARED_PTR(SelectInstr);
    DECL_SHARED_PTR(SettingsInstr);
    DECL_SHARED_PTR(SnapInstr);
    DECL_SHARED_PTR(SnapObjInstr);
    DECL_SHARED_PTR(StageInstr);
    DECL_SHARED_PTR(StateInstr);
    DECL_SHARED_PTR(StopInstr);
    DECL_SHARED_PTR(ViewInstr);
    DECL_SHARED_PTR(WaitInstr);

    Script();

    /// Reads instructions from the script at the given FilePath. Returns false
    /// on error.
    bool ReadScript(const FilePath &path);

    /// Returns the path to the file the script was read from.
    const FilePath & GetPath() const { return file_path_; }

    /// Returns the instructions in the script.
    const std::vector<InstrPtr> & GetInstructions() const {
        return instructions_;
    }

  private:
    using InstrFunc_ = std::function<InstrPtr(const StrVec &)>;
    using FuncMap_   = std::map<Str, InstrFunc_>;

    /// This maps instruction names to functions.
    FuncMap_              func_map_;

    FilePath              file_path_;     ///< Script file being processed.
    size_t                line_number_;   ///< Current line number in the file.
    std::vector<InstrPtr> instructions_;  ///< Resulting instructions.

    /// Reports an error.
    void Error_(const Str &message);

    /// Removes comments from input lines. Returns false on error.
    bool RemoveComments_(StrVec &lines);

    /// Parses one line of the script. Returns false on error.
    bool ParseLine_(const Str &line);

    /// Parses an include instruction and processes the included file. Returns
    /// false on error.
    bool ParseInclude_(const StrVec &words);

    /// Processes the named included file, returning false on error.
    bool ProcessInclude_(const Str &file, float timescale);

    /// \name Parsing helpers.
    /// Each of these parses an item from one or more words, returning false on
    /// parse error.
    ///@{
    bool ParseVector3f_(const StrVec &words, size_t index, Vector3f &v);
    bool ParseFloat_(const Str &s, float &f);
    bool ParseFloat01_(const Str &s, float &f);
    bool ParseN_(const Str &s, size_t &n);
    ///@}

    /// \name Instruction parsing.
    /// Each of these parses and returns an instruction of a specific type.
    ///@{
    InstrPtr ParseAction_(const StrVec &words);
    InstrPtr ParseCaption_(const StrVec &words);
    InstrPtr ParseClick_(const StrVec &words);
    InstrPtr ParseDrag_(const StrVec &words);
    InstrPtr ParseDragStart_(const StrVec &words);
    InstrPtr ParseDragEnd_(const StrVec &words);
    InstrPtr ParseFiles_(const StrVec &words);
    InstrPtr ParseFocus_(const StrVec &words);
    InstrPtr ParseHandModel_(const StrVec &words);
    InstrPtr ParseHandMove_(const StrVec &words);
    InstrPtr ParseHandPoint_(const StrVec &words);
    InstrPtr ParseHandPos_(const StrVec &words);
    InstrPtr ParseHandTouch_(const StrVec &words);
    InstrPtr ParseHighlight_(const StrVec &words);
    InstrPtr ParseKey_(const StrVec &words);
    InstrPtr ParseLoad_(const StrVec &words);
    InstrPtr ParseMoveOver_(const StrVec &words);
    InstrPtr ParseMoveTo_(const StrVec &words);
    InstrPtr ParseSection_(const StrVec &words);
    InstrPtr ParseSelect_(const StrVec &words);
    InstrPtr ParseSettings_(const StrVec &words);
    InstrPtr ParseSnap_(const StrVec &words);
    InstrPtr ParseSnapObj_(const StrVec &words);
    InstrPtr ParseStage_(const StrVec &words);
    InstrPtr ParseState_(const StrVec &words);
    InstrPtr ParseStop_(const StrVec &words);
    InstrPtr ParseView_(const StrVec &words);
    InstrPtr ParseWait_(const StrVec &words);
    ///@}

    /// Scales the duration in the given instruction (if it has one).
    static void ScaleDuration_(Instr &instr, float scale);

    /// Deals with multi-line and token substitution in captions.
    static Str FixCaptionText_(const StrVec &words);

    /// Returns the button for a controller motion instruction.
    static Event::Button GetControllerButton(const StrVec &words, size_t index);
};
