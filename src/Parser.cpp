#include "Parser.h"

#include <cctype>
#include <fstream>

Parser::Parser(const std::vector<FieldSpec> &field_specs) :
    field_specs_(field_specs) {
    BuildFieldSpecMap_();
}

Parser::~Parser() {
}

Parser::ObjectPtr Parser::ParseFile(const std::string &path) {
    std::ifstream in(path);
    if (in.fail())
        Throw_("Failed to open file");

    return ParseStream(in);
}

Parser::ObjectPtr Parser::ParseStream(std::istream &in) {
    if (path_.empty())
        path_ = "<input stream>";
    cur_line_ = 1;
    return ParseObject_(in);
}

void Parser::BuildFieldSpecMap_() {
    assert(field_spec_map_.empty());

    // Build the FieldSpecMap_ from the specs, validating as we go:
    //   - Check for invalid counts.
    //   - Check for type conflicts (two fields with same name and different
    //     types or counts).
    for (const FieldSpec &spec: field_specs_) {
        if (spec.count <= 0) {
            throw Exception(std::string("FieldSpec for '") + spec.name +
                            "' has an invalid count: " +
                            Util::ToString(spec.count));
        }

        auto it = field_spec_map_.find(spec.name);
        if (it != field_spec_map_.end() && ! (*it->second == spec)) {
            throw Exception(std::string("Conflicting types/counts for field '" +
                                        spec.name + "'"));
        }

        field_spec_map_[spec.name] = &spec;
    }
}

Parser::ObjectPtr Parser::ParseObject_(std::istream &in) {
    ObjectPtr obj(new Object);

    // Read the object type name. This is the line the object is considered
    // defined on, so store the path and line number.
    obj->type_name = ParseName_(in);
    obj->path = path_;
    obj->line_number = cur_line_;

    ParseChar_(in, '{');
    if (PeekChar_(in) != '}')  // Valid to have an object with no fields.
        ParseFields_(in, obj->fields);
    ParseChar_(in, '}');
    return obj;
}

std::vector<Parser::ObjectPtr> Parser::ParseObjectList_(std::istream &in) {
    std::vector<ObjectPtr> objects;
    ParseChar_(in, '[');
    while (true) {
        // If the next character is a closing brace, stop. An empty list of
        // objects is valid.
        if (PeekChar_(in) == ']')
            break;

        objects.push_back(ParseObject_(in));

        // Parse the trailing comma.
        char c = PeekChar_(in);
        if (c == ',')
            ParseChar_(in, ',');
    }
    ParseChar_(in, ']');
    return objects;
}

void Parser::ParseFields_(std::istream &in, std::vector<FieldPtr> &fields) {
    while (true) {
        std::string name = ParseName_(in);
        ParseChar_(in, ':');

        // Get the expected type for the field.
        const FieldSpec &spec = GetFieldSpec_(name);

        // Parse the value based on the type.
        if (spec.count == 1)
            fields.push_back(ParseSingleFieldValue_(in, spec));
        else
            fields.push_back(ParseArrayFieldValue_(in, spec));

        // Parse the trailing comma.
        char c = PeekChar_(in);
        if (c == ',') {
            ParseChar_(in, ',');
            c = PeekChar_(in);
        }
        // If there was no comma, there must be a closing brace.
        else if (c != '}')
            Throw_(std::string("Expected ',' or '}', got '") + c + "'");

        // If the next character is a closing brace, stop.
        if (c == '}')
            break;
    }
}

Parser::FieldPtr Parser::ParseSingleFieldValue_(std::istream &in,
                                                const FieldSpec &spec) {
    SkipWhiteSpace_(in);
    return FieldPtr(new SingleField(spec, ParseValue_(in, spec)));
}

Parser::FieldPtr Parser::ParseArrayFieldValue_(std::istream &in,
                                               const FieldSpec &spec) {
    std::vector<Value> values;
    values.reserve(spec.count);
    for (uint32_t i = 0; i < spec.count; ++i)
        values.push_back(ParseValue_(in, spec));
    return FieldPtr(new ArrayField(spec, values));
}

Parser::Value Parser::ParseValue_(std::istream &in, const FieldSpec &spec) {
    SkipWhiteSpace_(in);

    Value value;
    switch (spec.type) {
      case ValueType::kBool:
        value = ParseBool_(in);
        break;
      case ValueType::kInteger:
        value = ParseInteger_(in);
        break;
      case ValueType::kFloat:
        value = ParseFloat_(in);
        break;
      case ValueType::kString:
        value = ParseQuotedString_(in);
        break;
      case ValueType::kObject:
        value = ParseObject_(in);
        break;
      case ValueType::kObjectList:
        value = ParseObjectList_(in);
        break;
      default:
        Throw_("Unexpected field type");
    }
    return value;
}

std::string Parser::ParseName_(std::istream &in) {
    SkipWhiteSpace_(in);
    std::string s = "";
    char c;
    while (in.get(c)) {
        if (isalnum(c) || c == '_') {
            s += c;
        }
        else {
            in.putback(c);
            break;
        }
    }
    if (s.empty())
        Throw_("Invalid empty type name");
    if (! isalpha(s[0]))
        Throw_("Invalid type name '" + s + "'");
    return s;
}

bool Parser::ParseBool_(std::istream &in) {
    bool val;
    std::string s;
    char c;
    while (in.get(c) && isalpha(c))
        s += c;
    if (! isalpha(c))
        in.putback(c);
    if (Util::StringsEqualNoCase(s, "t") ||
        Util::StringsEqualNoCase(s, "true")) {
        val = true;
    }
    else if (Util::StringsEqualNoCase(s, "f") ||
        Util::StringsEqualNoCase(s, "false")) {
        val = false;
    }
    else {
        Throw_("Invalid bool value '" + s + "'");
    }
    return val;
}

int Parser::ParseInteger_(std::istream &in) {
    int i;
    if (! (in >> i))
        Throw_("Invalid integer value");
    return i;
}

float Parser::ParseFloat_(std::istream &in) {
    float f;
    if (! (in >> f))
        Throw_("Invalid float value");
    return f;
}

std::string Parser::ParseQuotedString_(std::istream &in) {
    // For simplicity, this assumes that there are no escaped characters or
    // newlines in the string.
    std::string s;
    ParseChar_(in, '"');
    char c;
    while (in.get(c) && c != '"')
        s += c;
    return s;
}

void Parser::ParseChar_(std::istream &in, char expected_c) {
    SkipWhiteSpace_(in);
    char c;
    if (in.get(c) && c == expected_c)
        return;
    if (in.eof())
        Throw_(std::string("Expected '") + expected_c + "' got EOF");
    else
        Throw_(std::string("Expected '") + expected_c + "' got '" + c + "'");
}

char Parser::PeekChar_(std::istream &in) {
    SkipWhiteSpace_(in);
    return (char) in.peek();
}

void Parser::SkipWhiteSpace_(std::istream &in) {
    char c;
    while (in.get(c)) {
        // Check for comments; read to end of line.
        if (c == '#') {
            while (in.get(c) && c != '\n')
                ;
        }
        if (c == '\n')
            ++cur_line_;

        else if (! isspace(c)) {
            in.putback(c);
            return;
        }
    }
}

const Parser::FieldSpec & Parser::GetFieldSpec_(const std::string &name) {
    auto it = field_spec_map_.find(name);
    if (it == field_spec_map_.end())
        Throw_(std::string("Unknown field name '") + name + "'");
    return *it->second;
}

void Parser::Throw_(const std::string &msg) {
    throw Exception(path_, cur_line_, msg);
}
