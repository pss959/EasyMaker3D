#include "Parser.h"

#include <cctype>
#include <fstream>

Parser::Parser() {
}

Parser::~Parser() {
}

void Parser::MergeFieldTypeMaps(const FieldTypeMap &from_map,
                                FieldTypeMap &to_map) {
    // Check for conflicts first.
    for (const auto &entry: from_map) {
        auto it = to_map.find(entry.first);
        if (it != to_map.end() && it->second != entry.second) {
            throw Exception(std::string("Conflict merging types for field '" +
                                        entry.first + "'"));
        }
    }
    to_map.insert(from_map.begin(), from_map.end());
}

Parser::ObjectPtr Parser::ParseFile(const std::string &path,
                                    const FieldTypeMap &field_type_map) {
    // Save the path to store in objects and for error messages.
    path_ = path;

    field_type_map_ = &field_type_map;

    std::ifstream in(path);
    if (in.fail())
        Throw_("Failed to open file");

    cur_line_ = 1;
    return ParseObject_(in);
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

void Parser::ParseObjects_(std::istream &in, std::vector<ObjectPtr> &objects) {
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
}

void Parser::ParseFields_(std::istream &in, std::vector<Field> &fields) {
    while (true) {
        Field field;
        field.name = ParseName_(in);
        ParseChar_(in, ':');

        // Get the expected type for the field.
        field.type = GetFieldType_(field.name);

        // Parse the value based on the type.
        ParseFieldValue_(in, field);

        fields.push_back(field);

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

void Parser::ParseFieldValue_(std::istream &in, Field &field) {
    SkipWhiteSpace_(in);
    switch (field.type) {
      case Field::Type::kBool:
        field.bool_val = ParseBool_(in);
        break;
      case Field::Type::kString:
        field.string_val = ParseQuotedString_(in);
        break;
      case Field::Type::kInteger:
        if (! (in >> field.integer_val))
            Throw_("Invalid integer value");
        break;
      case Field::Type::kScalar:
        if (! (in >> field.scalar_val))
            Throw_("Invalid scalar value");
        break;
      case Field::Type::kVector2:
        if (! (in >> field.vector2_val[0] >> field.vector2_val[1]))
            Throw_("Invalid vector2 value");
        break;
      case Field::Type::kVector3:
        if (! (in >> field.vector3_val[0] >> field.vector3_val[1]
               >> field.vector3_val[2]))
            Throw_("Invalid vector3 value");
        break;
      case Field::Type::kVector4:
        if (! (in >> field.vector4_val[0] >> field.vector4_val[1]
               >> field.vector4_val[2] >> field.vector4_val[3]))
            Throw_("Invalid vector4 value");
        break;
      case Field::Type::kObject:
        field.object_val = ParseObject_(in);
        break;
      case Field::Type::kObjects:
        ParseObjects_(in, field.objects_val);
        break;
      default:
        Throw_("Unexpected field type");
    }
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

Parser::Field::Type Parser::GetFieldType_(const std::string &name) {
    auto it = field_type_map_->find(name);
    if (it == field_type_map_->end())
        Throw_(std::string("Unknown field name '") + name + "'");
    return it->second;
}

void Parser::Throw_(const std::string &msg) {
    throw Exception(path_, cur_line_, msg);
}
