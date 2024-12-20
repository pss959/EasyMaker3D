//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <functional>
#include <istream>
#include <vector>

#include "Math/Color.h"
#include "Math/Types.h"
#include "Parser/Exception.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/Memory.h"

namespace Parser {

DECL_SHARED_PTR(Object);
DECL_SHARED_PTR(ObjectList);

/// Scanner class that the Parser uses to get tokens and characters.
///
/// \ingroup Parser
class Scanner {
  public:
    class Input_;

    /// Alias for the constant substitution function passed to the constructor.
    using ConstantSubstFunc = std::function<Str(const Str &)>;
    /// Alias for the function passed to scan Objects.
    using ObjectFunc        = std::function<ObjectPtr()>;
    /// Alias for the function passed to scan lists of Objects.
    using ObjectListFunc    = std::function<ObjectListPtr()>;

    /// The constructor is passed a function to invoke to substitute a value
    /// string for a constant of the form "$name". The function is passed the
    /// name following the dollar sign; it returns the value string.  The value
    /// string is scanned next before continuing on to whatever came after the
    /// constant.
    Scanner(const ConstantSubstFunc &func);
    ~Scanner();

    /// Clears all input streams that may still be around.
    void Clear();

    /// Starts scanning from the given input stream. The file path name to use
    /// for the stream is supplied for error messages.
    void PushInputStream(const FilePath &path, std::istream &in);

    /// Pushes an input stream created to scan the given string.
    void PushStringInput(const Str &input_string);

    /// Stops scanning from the current input stream, returning to the previous
    /// one.
    void PopInputStream();

    /// Returns the current path: the most recently pushed non-string input.
    FilePath GetCurrentPath();

    /// Scans a name, which must consist only of alphanumeric characters or
    /// underscores, and must not start with a numeric character.
    Str ScanName(const Str &for_what);

    /// Scans a boolean value, which is any case-insensitive version of "true',
    /// "t", "false", or "f".
    bool ScanBool();

    /// Scans an integer value.
    int ScanInteger();

    /// Scans an unsigned integer value.
    unsigned int ScanUInteger();

    /// Scans a floating-point value.
    float ScanFloat();

    /// Scans a double-quoted string. This does not (yet) handle escape
    /// sequences.
    Str ScanQuotedString();

    /// Scans an RGB color in any supported format:
    ///   - "#RRGGBB"    (hex)
    ///   - "#RRGGBBAA"  (hex)
    ///   - r g b a      (0-1 floats or 0-255 ints)
    Color ScanColor();

    /// Scans a single character. Throws an Exception if it is not the
    /// expected one.
    void ScanExpectedChar(char expected_c);

    /// Sets a function to invoke to scan an Object.
    void SetObjectFunction(const ObjectFunc &object_scan_func) {
        object_scan_func_ = object_scan_func;
    }

    /// Sets a function to invoke to scan a list of Objects.
    void SetObjectListFunction(const ObjectListFunc &object_list_scan_func) {
        object_list_scan_func_ = object_list_scan_func;
    }

    /// Scans and returns an object. This uses the supplied object parsing
    /// function. (Having this available through the Scanner makes it possible
    /// for fields containing Objects to parse their values.)
    ObjectPtr ScanObject() {
        ASSERT(object_scan_func_);
        return object_scan_func_();
    }

    /// Scans and returns a list of objects. This uses the supplied object list
    /// parsing function. (Having this available through the Scanner makes it
    /// possible for fields containing Object lists to parse their values.)
    ObjectListPtr ScanObjectList() {
        ASSERT(object_list_scan_func_);
        return object_list_scan_func_();
    }

    /// Returns the next character in the input without using it up.
    char PeekChar();

    /// Throws an Exception with the given message. Adds the current file path
    /// and line number to the message.
    void Throw(const Str &msg);

  private:
    /// Constant substitution function passed to the constructor.
    ConstantSubstFunc constant_substitution_func_;

    /// Managed input manager.
    std::unique_ptr<Input_> input_ptr_;
    /// Reference to input manager for convenience.
    Input_ &input_;

    /// Function used to scan objects opaquely.
    ObjectFunc     object_scan_func_;
    /// Function used to scan lists of objects opaquely.
    ObjectListFunc object_list_scan_func_;

    /// Scans and returns a string possibly containing a valid numeric value.
    Str ScanNumericString_();

    /// Skips all whitespace, including comments. Looks for newlines,
    /// incrementing the current line counter.
    void SkipWhiteSpace_();
};

}  // namespace Parser
