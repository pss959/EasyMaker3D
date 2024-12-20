//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Parser/Scanner.h"

#include <cctype>
#include <ranges>
#include <sstream>
#include <vector>

#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/String.h"

namespace Parser {

// ----------------------------------------------------------------------------
// Scanner::Input_ definition and implementation.
// ----------------------------------------------------------------------------

/// Class wrapping a stack of istream instances. This is used to implement
/// pushing and popping streams seamlessly.
class Scanner::Input_ {
  public:
    Input_() {
        KLOG('f', "Parsed files are relative to \""
             << FilePath::GetResourceBasePath().ToString() + "\"");
    }
    ~Input_() {
        Clear();
    }
    void Clear() {
        while (! streams_.empty()) {
            delete streams_.back().sstream;
            streams_.pop_back();
        }
    }
    void PushFile(const FilePath &path, std::istream *input) {
        KLOG('f', Str(2 * open_file_count_, ' ')
             << "Parsing file \""
             << path.MakeRelativeTo(FilePath::GetResourceBasePath()).ToString()
             << "\"");
        Stream_ st;
        st.stream   = input;
        st.sstream  = nullptr;
        st.path     = path;
        st.cur_line = 1;
        streams_.push_back(st);
        ++open_file_count_;
    }
    void PushString(const Str &s, const Str &desc) {
        Stream_ st;
        st.sstream  = new std::istringstream(s);
        st.stream   = st.sstream;
        st.path     = desc.empty() ? "<string stream>" : "<" + desc + ">";
        st.cur_line = 1;
        streams_.push_back(st);
    }
    void Pop() {
        ASSERT(! streams_.empty());
        Stream_ &st = streams_.back();
        if (! st.sstream)
            --open_file_count_;
        delete st.sstream;
        streams_.pop_back();
    }
    bool GetCurrentPathAndLine(FilePath &path, int &line) {
        for (const auto &st: streams_ | std::views::reverse) {
            if (! st.sstream) {
                path = st.path;
                line = st.cur_line;
                return true;
            }
        }
        return false;
    }

    bool Get(char &c) {
        // Try this while EOF is hit and there are more streams.
        while (! streams_.empty()) {
            Stream_ &st = streams_.back();
            st.stream->get(c);
            if (! st.stream->eof()) // Got a real character.
                return true;

            // Got EOF. Pop the stream.
            Pop();
        }
        // If we get here, there are no more streams and no characters.
        return false;
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

    /// Returns the current path and line number for error messages.
    void GetPathAndLineNumber(FilePath &path, int &line) {
        if (streams_.empty()) {
            path = FilePath("NO FiLE");
            line = 0;
        }
        else {
            // Find the first stream that is a file, if any. If not, use
            // whatever the top stream is.
            if (! GetCurrentPathAndLine(path, line)) {
                path = Top_().path;
                line = Top_().cur_line;
            }
        }
    }
    /// Returns the current line number.
    int GetCurLine() {
        return streams_.empty() ? 0 : Top_().cur_line;
    }

  private:
    /// This struct stores everything necessary to manage an input stream.
    struct Stream_ {
        // Stream to read from.
        std::istream       *stream;

        /// istringstream instance if this stream wraps a string. This pointer
        /// is stored so the istringstream can be deleted.
        std::istringstream *sstream;

        /// Path to file stream is associated with, if any.
        FilePath           path;

        /// Current line in the file or string input.
        int                cur_line;
    };

    /// Stack of all input streams. Implemented as a vector because we need
    /// access to all items.
    std::vector<Stream_> streams_;

    /// Count of open files.
    size_t open_file_count_ = 0;

    Stream_ & Top_() {
        ASSERT(! streams_.empty());
        return streams_.back();
    }
};

// ----------------------------------------------------------------------------
// Scanner implementation.
// ----------------------------------------------------------------------------

Scanner::Scanner(const ConstantSubstFunc &func) :
    constant_substitution_func_(func),
    input_ptr_(new Input_),
    input_(*input_ptr_) {
}

Scanner::~Scanner() {
}

void Scanner::Clear() {
    input_.Clear();
}

void Scanner::PushInputStream(const FilePath &path, std::istream &in) {
    input_.PushFile(path, &in);
}

void Scanner::PushStringInput(const Str &input_string) {
    input_.PushString(input_string, "");
}

void Scanner::PopInputStream() {
    input_.Pop();
}

FilePath Scanner::GetCurrentPath() {
    FilePath path;
    int line;
    input_.GetCurrentPathAndLine(path, line);
    return path;
}

Str Scanner::ScanName(const Str &for_what) {
    SkipWhiteSpace_();
    Str s = "";
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
        Throw("Invalid empty name for " + for_what);
    if (! isalpha(s[0]) && s[0] != '_')
        Throw("Invalid name '" + s + "' for " + for_what);
    return s;
}

bool Scanner::ScanBool() {
    SkipWhiteSpace_();
    bool val = false;
    Str s;
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
    // Integers may be signed and are always base 10.
    Str s = ScanNumericString_();
    int n;
    if (Util::StringToInteger(s, n))
        return n;
    Throw("Invalid integer value");
    return 0;  // LCOV_EXCL_LINE [cannot happen]
}

unsigned Scanner::ScanUInteger() {
    // Unsigned integers may not be signed and may be base 8, 10, or 16.
    Str s = ScanNumericString_();
    if (! s.empty() && s[0] != '-' && s[0] != '+') {
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
    Throw("Invalid unsigned integer value");
    return 0;  // LCOV_EXCL_LINE [cannot happen]
}

float Scanner::ScanFloat() {
    Str s = ScanNumericString_();
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
    return 0.f;  // LCOV_EXCL_LINE [cannot happen]
}

Str Scanner::ScanQuotedString() {
    Str s;
    ScanExpectedChar('"');
    while (true) {
        char c;
        if (! input_.Get(c) || static_cast<int>(c) == EOF) {
            Throw("Found EOF inside quoted string");
        } else if (c == '"') {
            // End of quoted string.
            break;
        } else if (c == '\\') {
            // Handle escaped characters.
            input_.Get(c);
            switch (c) {
              case 'a': c = '\a'; break;
              case 'b': c = '\b'; break;
              case 'f': c = '\f'; break;
              case 'n': c = '\n'; break;
              case 'r': c = '\r'; break;
              case 't': c = '\t'; break;
              case 'v': c = '\v'; break;
              default:  break;  // Handles any case where we want to add c.
            }
            s += c;
        } else {
            s += c;
        }
    }
    return s;
}

Color Scanner::ScanColor() {
    Color color;

    // Check for quoted "#RRGGBB" and "#RRGGBBAA" formats
    if (PeekChar() == '"') {
        const Str s = ScanQuotedString();
        if (! color.FromHexString(s))
            Throw("Invalid color format: '" + s + "'");
    }
    else {
        // Scan a Vector4f. If any value is > 1, assume the 0-255 range.
        bool is_255 = false;
        for (int i = 0; i < 4; ++i) {
            color[i] = ScanFloat();
            if (color[i] > 1.f)
                is_255 = true;
        }

        if (is_255) {
            for (int i = 0; i < 4; ++i)
                color[i] = color[i] / 255.f;
        }
    }
    return color;
}

void Scanner::ScanExpectedChar(char expected_c) {
    SkipWhiteSpace_();
    char c;
    if (input_.Get(c) && c == expected_c)
        return;
    if (input_.IsAtEOF())
        Throw(Str("Expected '") + expected_c + "', got EOF");
    else
        Throw(Str("Expected '") + expected_c + "', got '" + c + "'");
}

char Scanner::PeekChar() {
    SkipWhiteSpace_();
    return static_cast<char>(input_.Peek());
}

void Scanner::Throw(const Str &msg) {
    FilePath path;
    int line = 0;
    input_.GetCurrentPathAndLine(path, line);
    if (line >= 1)
        throw Exception(path, line, msg);
    else
        throw Exception(msg);
}

Str Scanner::ScanNumericString_() {
    SkipWhiteSpace_();
    // Read letters, digits, and +/-/. characters into a string.
    Str  s;
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
                // Get the name of the constant.
                Str name = ScanName("constant");
                ASSERT(constant_substitution_func_);

                // Push the substituted value string on top of the input.
                input_.PushString(constant_substitution_func_(name),
                                  "String for constant " + name);

                // Recurse in case there are nested constants.
                SkipWhiteSpace_();
            }
            else
                input_.PutBack(c);
            return;
        }
    }
}

}  // namespace Parser
