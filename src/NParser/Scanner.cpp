#include "NParser/Scanner.h"

#include <assert.h>

#include <cctype>
#include <sstream>
#include <stack>

#include "Util/String.h"

namespace NParser {

// ----------------------------------------------------------------------------
// Scanner::Input_ definition and implementation.
// ----------------------------------------------------------------------------

//! Class wrapping a stack of istream instances. This is used to implement
//! pushing and popping streams seamlessly.
class Scanner::Input_ {
  public:
    ~Input_() {
        while (! streams_.empty()) {
            delete streams_.top().sstream;
            streams_.pop();
        }
    }

    void PushFile(const Util::FilePath &path, std::istream *input) {
        Stream_ st;
        st.stream   = input;
        st.sstream  = nullptr;
        st.path     = path;
        st.cur_line = 1;
        streams_.push(st);
    }
    void PushString(const std::string &s) {
        Stream_ st;
        st.sstream  = new std::istringstream(s);
        st.stream   = st.sstream;
        st.path     = "<string stream>";
        st.cur_line = 1;
        streams_.push(st);
    }
    void Pop() {
        assert(! streams_.empty());
        Stream_ &st = streams_.top();
        delete st.sstream;
        streams_.pop();
    }
    bool Get(char &c) {
        // Pop any inputs that are at EOF.
        while (! streams_.empty() && streams_.top().stream->eof())
            Pop();

        // If nothing left, can't get a character.
        if (streams_.empty()) {
            c = -1;
            return false;
        }

        // Get the next character from the new top input.
        Stream_ &st = streams_.top();
        st.stream->get(c);
        return ! st.stream->fail();
    }
    char Peek() {
        return streams_.empty() ? char(-1) : Top_().stream->peek();
    }
    bool IsAtEOF() {
        return streams_.empty() || Top_().stream->eof();
    }
    void PutBack(char c) {
        Top_().stream->putback(c);
    }
    void IncrementCurLine() {
        ++Top_().cur_line;
    }

    //! Returns the current path.
    const Util::FilePath & GetPath() {
        return Top_().path;
    }
    //! Returns the current line number.
    int GetCurLine() {
        return Top_().cur_line;
    }

  private:
    //! This struct stores everything necessary to manage an input stream.
    struct Stream_ {
        // Stream to read from.
        std::istream       *stream;

        //! istringstream instance if this stream wraps a string. This pointer
        //! is stored so the istringstream can be deleted.
        std::istringstream *sstream;

        //! Path to file stream is associated with, if any.
        Util::FilePath     path;

        //! Current line in the file or string input.
        int                cur_line;
    };

    //! Stack of all input streams.
    std::stack<Stream_> streams_;

    Stream_ & Top_() {
        assert(! streams_.empty());
        return streams_.top();
    }
};

// ----------------------------------------------------------------------------
// Scanner implementation.
// ----------------------------------------------------------------------------

Scanner::Scanner() :
    input_ptr_(new Input_),
    input_(*input_ptr_) {
}

Scanner::~Scanner() {
}

void Scanner::PushInputStream(const Util::FilePath &path, std::istream &in) {
    input_.PushFile(path, &in);
}

void Scanner::PushStringInput(const std::string &input_string) {
    input_.PushString(input_string);
}

void Scanner::PopInputStream() {
    input_.Pop();
}

std::string Scanner::ScanName() {
    SkipWhiteSpace_();
    std::string s = "";
    char c;
    while (input_.Get(c)) {
        if (isalnum(c) || c == '_') {
            s += c;
        }
        else {
            input_.PutBack(c);
            break;
        }
    }
    if (s.empty())
        Throw("Invalid empty type name");
    if (! isalpha(s[0]) && s[0] != '_')
        Throw("Invalid type name '" + s + "'");
    return s;
}

bool Scanner::ScanBool() {
    SkipWhiteSpace_();
    bool val;
    std::string s;
    char c;
    while (input_.Get(c) && isalpha(c))
        s += c;
    if (! isalpha(c))
        input_.PutBack(c);
    if (Util::StringsEqualNoCase(s, "t") ||
        Util::StringsEqualNoCase(s, "true")) {
        val = true;
    }
    else if (Util::StringsEqualNoCase(s, "f") ||
        Util::StringsEqualNoCase(s, "false")) {
        val = false;
    }
    else {
        Throw("Invalid bool value '" + s + "'");
    }
    return val;
}

int Scanner::ScanInteger() {
    std::string s = ScanNumericString_();
    if (! s.empty()) {
        // Figure out the base.
        int base = 10;
        if (s.size() > 1U && s[0] == '0') {
            if (s[1] == 'x' || s[1] == 'X')
                base = 16;
            else
                base = 8;
        }

        try {
            size_t chars_processed;
            int i = std::stoi(s, &chars_processed, base);
            if (chars_processed == s.size())  // No extra characters at the end.
                return i;
        }
        catch (std::exception &) {} // Fall through to Throw_ below.
    }
    Throw("Invalid integer value");
    return 0;  // LCOV_EXCL_LINE
}

float Scanner::ScanFloat() {
    std::string s = ScanNumericString_();
    if (! s.empty()) {
        try {
            size_t chars_processed;
            float f = std::stof(s, &chars_processed);
            if (chars_processed == s.size())  // No extra characters at the end.
                return f;
        }
        catch (std::exception &) {} // Fall through to Throw_ below.
    }
    Throw("Invalid float value");
    return 0.f;  // LCOV_EXCL_LINE
}

std::string Scanner::ScanQuotedString() {
    // For simplicity, this assumes that there are no escaped characters or
    // newlines in the string.
    std::string s;
    ScanExpectedChar('"');
    char c;
    while (input_.Get(c) && c != '"')
        s += c;
    return s;
}

void Scanner::ScanExpectedChar(char expected_c) {
    SkipWhiteSpace_();
    char c;
    if (input_.Get(c) && c == expected_c)
        return;
    if (input_.IsAtEOF())
        Throw(std::string("Expected '") + expected_c + "', got EOF");
    else
        Throw(std::string("Expected '") + expected_c + "', got '" + c + "'");
}

char Scanner::PeekChar() {
    SkipWhiteSpace_();
    return (char) input_.Peek();
}

void Scanner::Throw(const std::string &msg) {
    throw Exception(input_.GetPath(), input_.GetCurLine(), msg);
}

std::string Scanner::ScanNumericString_() {
    SkipWhiteSpace_();
    // Read letters, digits, and +/-/. characters into a string.
    std::string s;
    char c;
    while (input_.Get(c)) {
        if (isalnum(c) || c == '+' || c == '-' || c == '.')
            s += c;
        else {
            input_.PutBack(c);
            break;
        }
    }
    return s;
}

void Scanner::SkipWhiteSpace_() {
    char c;
    while (input_.Get(c)) {
        // Check for comments; read to end of line.
        if (c == '#') {
            while (input_.Get(c) && c != '\n')
                ;
        }
        if (c == '\n')
            input_.IncrementCurLine();

        else if (! isspace(c)) {
            // Check for constant token substitution.
            if (c == '$') {
                // XXXX SubstituteConstant_();
                // Recurse in case there are nested constants.
                SkipWhiteSpace_();
            }
            else
                input_.PutBack(c);
            return;
        }
    }
}

}  // namespace NParser
