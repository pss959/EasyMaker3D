#include "Parser.h"

#include <cctype>
#include <fstream>

Parser::Parser() {
    InitFieldTypeMap_();
}

Parser::~Parser() {
}

Parser::ObjectPtr Parser::ParseFile(const std::string &path) {
    path_ = path;

    std::ifstream in(path);
    if (in.fail())
        Throw_("Failed to open file");

    cur_line_ = 1;
    return ParseObject_(in);
}

Parser::ObjectPtr Parser::ParseObject_(std::istream &in) {
    ObjectPtr obj(new Object);

    // Read the object type name.
    obj->type_name = ParseName_(in);
    std::cerr << "XXXX Got type name '" << obj->type_name << "'\n";
    ParseChar_(in, '{');
    ParseFields_(in, obj->fields);
    return obj;
}

void Parser::ParseObjects_(std::istream &in, std::vector<ObjectPtr> &objects) {
    ParseChar_(in, '[');
    while (true) {
        // If the next character is a closing brace, stop. An empty list of
        // objects is valid.
        if (PeekChar_(in) == '}')
            break;

        objects.push_back(ParseObject_(in));

        // Parse the trailing comma.
        char c = PeekChar_(in);
        if (c == ',')
            ParseChar_(in, ',');
    }
}

void Parser::ParseFields_(std::istream &in, std::vector<Field> &fields) {
    while (true) {
        std::string name = ParseName_(in);
        ParseChar_(in, ':');

        // Get the expected type for the field.
        Field field;
        field.type = GetFieldType_(name);

        // Parse the value based on the type.
        ParseFieldValue_(in, field);

        fields.push_back(field);

        // Parse the trailing comma.
        char c = PeekChar_(in);
        if (c == ',') {
            ParseChar_(in, ',');
            c = PeekChar_(in);
        }

        // If the next character is a closing brace, stop.
        if (c == '}')
            break;
    }
}

void Parser::ParseFieldValue_(std::istream &in, Field &field) {
    SkipWhiteSpace_(in);
    switch (field.type) {
      case Field::Type::kString:
        field.string_val = ParseQuotedString_(in);
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

void Parser::InitFieldTypeMap_() {
    struct Entry_ { std::string name; Field::Type type; };
    std::vector<Entry_> entries{
        { "bottom_radius", Field::Type::kScalar  },
        { "children",      Field::Type::kObjects },
        { "height",        Field::Type::kScalar  },
        { "name",          Field::Type::kString  },
        { "rotation",      Field::Type::kVector4 },
        { "scale",         Field::Type::kVector3 },
        { "shapes",        Field::Type::kObjects },
        { "top_radius",    Field::Type::kScalar  },
        { "translation",   Field::Type::kVector3 },
    };

    for (auto entry: entries)
        field_type_map_[entry.name] = entry.type;
}

Parser::Field::Type Parser::GetFieldType_(const std::string &name) {
    auto it = field_type_map_.find(name);
    if (it == field_type_map_.end())
        Throw_(std::string("Unknown field name '") + name + "'");
    return it->second;
}

void Parser::Throw_(const std::string &msg) {
    throw Exception(path_, cur_line_, msg);
}
