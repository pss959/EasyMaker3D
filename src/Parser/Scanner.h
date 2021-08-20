#pragma once

#include <functional>
#include <istream>
#include <string>

#include "Parser/Exception.h"
#include "Util/FilePath.h"

namespace Parser {

//! Scanner class that the Parser uses to get tokens and characters.
class Scanner {
  public:
    class Input_;

    //! Typedef for the constant substitution function passed to the
    //! constructor.
    typedef std::function<std::string(const std::string &)> ConstantSubstFunc;

    //! The constructor is passed a function to invoke to substitute a value
    //! string for a constant of the form "$name". The function is passed the
    //! name following the dollar sign; it returns the value string.  The value
    //! string is scanned next before continuing on to whatever came after the
    //! constant.
    Scanner(const ConstantSubstFunc &func);
    ~Scanner();

    //! Clears all input streams that may still be around.
    void Clear();

    //! Starts scanning from the given input stream. The file path name to use
    //! for the stream is supplied for error messages.
    void PushInputStream(const Util::FilePath &path, std::istream &in);

    //! Pushes an input stream created to scan the given string.
    void PushStringInput(const std::string &input_string);

    //! Stops scanning from the current input stream, returning to the previous
    //! one.
    void PopInputStream();

    //! Returns the current path: the most recently pushed non-string input.
    Util::FilePath GetCurrentPath();

    //! Scans a name, which must consist only of alphanumeric characters or
    //! underscores, and must not start with a numeric character.
    std::string ScanName();

    //! Scans a boolean value, which is any case-insensitive version of "true',
    //! "t", "false", or "f".
    bool ScanBool();

    //! Scans an integer value.
    int ScanInteger();

    //! Scans an unsigned integer value.
    unsigned int ScanUInteger();

    //! Scans a floating-point value.
    float ScanFloat();

    //! Scans a double-quoted string. This does not (yet) handle escape
    //! sequences.
    std::string ScanQuotedString();

    //! Scans a single character. Throws an Exception if it is not the
    //! expected one.
    void ScanExpectedChar(char expected_c);

    //! Returns the next character in the input without using it up.
    char PeekChar();

    //! Throws an Exception with the given message. Adds the current file path
    //! and line number to the message.
    void Throw(const std::string &msg);

  private:
    //! Constant substitution function passed to the constructor.
    ConstantSubstFunc constant_substitution_func_;

    //! Managed input manager.
    std::unique_ptr<Input_> input_ptr_;
    //! Reference to input manager for convenience.
    Input_ &input_;

    //! Scans and returns a string possibly containing a valid numeric value.
    std::string ScanNumericString_();

    //! Skips all whitespace, including comments. Looks for newlines,
    //! incrementing the current line counter.
    void SkipWhiteSpace_();
};

}  // namespace Parser
