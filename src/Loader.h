#pragma once

#include <string>

#include <ion/gfx/node.h>

#include "ExceptionBase.h"
#include "Parser.h"

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
    //! Builds a full path to a resource file.
    static std::string FullPath(const std::string &path) {
        return std::string(RESOURCE_DIR) + '/' + path;
    }

    //! Uses a Parser to parse the given file.
    static Parser::ObjectPtr ParseFile_(const std::string &path);

    // XXXX Parser object interpretation functions.
    void ExpectType_(const Parser::Object &obj,
                     const std::string &type_name);
};
