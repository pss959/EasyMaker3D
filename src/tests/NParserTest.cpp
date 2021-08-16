#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "NParser/Exception.h"
#include "NParser/Object.h"
#include "NParser/Parser.h"
#include "Testing.h"
#include "Util/General.h"

// Tests that a Parser::Exception is thrown and that its message contains
// the given string pattern.
#define TEST_THROW_(STMT, PATTERN) \
    TEST_THROW(STMT, NParser::Exception, PATTERN)

// Classes for testing parsing.
class Simple : public NParser::Object {
  public:
    int         ival;
    float       fval;
    std::string sval;

  private:
    const std::vector<FieldSpec> specs_{
        { "ival", NParser::ValueType::kInteger, 1,
          [&](const std::vector<NParser::Value> &vals){
              ival = std::get<int>(vals[0]); } },
        { "fval", NParser::ValueType::kFloat,   1,
          [&](const std::vector<NParser::Value> &vals){
              fval = std::get<float>(vals[0]); } },
        { "sval", NParser::ValueType::kString, 1,
          [&](const std::vector<NParser::Value> &vals){
              sval = std::get<std::string>(vals[0]); } },
    };

    virtual const std::vector<FieldSpec> & GetFieldSpecs() const override {
        return specs_;
    }
};

class NParserTest : public TestBase {
 protected:
    // Sets up a parser.
    NParser::Parser parser;
};

TEST_F(NParserTest, Simple) {
    const std::string input =
        "# Full-line comment\n"
        "Simple {\n"
        "  ival: 13, # In-line comment\n"
        "  fval: 3.4,\n"
        "  sval: \"A quoted string\",\n"
        "}\n";

    parser.RegisterObject("Simple", []{ return new Simple; });
    NParser::ObjectPtr obj = parser.ParseString(input);
    EXPECT_NOT_NULL(obj.get());
    std::shared_ptr<Simple> sp =
        Util::CastToDerived<NParser::Object, Simple>(obj);
    EXPECT_NOT_NULL(sp.get());
    EXPECT_EQ(13,   sp->ival);
    EXPECT_EQ(3.4f, sp->fval);
    EXPECT_EQ("A quoted string", sp->sval);
}
