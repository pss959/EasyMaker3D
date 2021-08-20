#pragma once

#include <istream>
#include <string>

#include "Parser/Exception.h"
#include "Util/FilePath.h"

namespace Parser {

//! Scanner. XXXX
class Scanner {
  public:
    class Input_;

    //! Managed input manager.
    std::unique_ptr<Input_> input_ptr_;
    //! Reference to input manager for convenience.
    Input_ &input_;

    Scanner();
    ~Scanner();

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
    //! Scans and returns a string possibly containing a valid numeric value.
    std::string ScanNumericString_();

    //! Skips all whitespace, including comments. Looks for newlines,
    //! incrementing the current line counter.
    void SkipWhiteSpace_();
};

}  // namespace Parser
