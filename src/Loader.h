#pragma once

#include <string>
#include <vector>

#include <ion/gfx/node.h>
#include <ion/gfx/shape.h>

#include "ExceptionBase.h"
#include "Parser.h"
#include "Util.h"

//! The Loader class loads resources of different types from files, returning
//! an Ion pointer to the results. All files are specified by string path
//! relative to the \c RESOURCE_DIR directory.  Failure of any loading function
//! results in a Loader::Exception being thrown.
class Loader {
  public:
    //! Exception thrown when any loading function fails.
    class Exception : public ExceptionBase {
      public:
        Exception(const std::string &path, const std::string &msg) :
            ExceptionBase(path, "Error loading: " + msg) {}
        Exception(const std::string &path, int line_number,
                  const std::string &msg) :
            ExceptionBase(path, line_number, "Error loading: " + msg) {}
    };

    //! Loads an Ion node subgraph from a JSON file, returning an Ion NodePtr
    //! to the result.
    ion::gfx::NodePtr LoadNode(const std::string &path);

  private:
#if XXXX
    //! Builds a full path to a resource file.
    static std::string FullPath(const std::string &path) {
        return std::string(RESOURCE_DIR) + '/' + path;
    }

    //! Uses a Parser to parse the given file.
    static Parser::ObjectPtr ParseFile_(const std::string &path);

    // XXXX Parser object extraction functions.
    ion::gfx::NodePtr       ExtractNode_(const Parser::Object &obj);
    ion::gfx::StateTablePtr ExtractStateTable_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractShape_(const Parser::Object &obj);

    template <typename EnumType>
    void SetEnumField_(const Parser::Object &obj,
                       const std::string &value_string,
                       const std::string &type_string, EnumType &result) {
        if (! Util::EnumFromString<EnumType>(value_string, result))
            ThrowEnumException_(obj, type_string, value_string);
    }

    void ThrowTypeMismatch_(const Parser::Object &obj,
                            const std::string &expected_type);
    void ThrowBadField_(const Parser::Object &obj, const Parser::Field &field);
    void ThrowEnumException_(const Parser::Object &obj,
                             const std::string &enum_type_name,
                             const std::string &value_string);
#endif
};
