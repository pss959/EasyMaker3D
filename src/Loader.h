#pragma once

#include <string>

#include <ion/gfx/node.h>

#include "ExceptionBase.h"

//! The Loader class has static functions used to load resources of different
//! types from files, returning an Ion pointer to the results. All files are
//! specified by string path relative to the \c RESOURCE_DIR directory.
//! Failure of any loading function results in a Loader::Exception being
//! thrown.
class Loader {
  public:
    //! Exception thrown when any loading function fails.
    class Exception : public ExceptionBase {
      public:
        Exception(const std::string &path, const std::string &msg) :
            ExceptionBase("Error loading from '" + FullPath(path) +
                          "' : " + msg) {}
    };

    //! Loads an Ion node subgraph from a JSON file, returning an Ion NodePtr
    //! to the result.
    static ion::gfx::NodePtr LoadNode(const std::string &path);

  private:
    //! Builds a full path to a resource file.
    static std::string FullPath(const std::string &path) {
        return std::string(RESOURCE_DIR) + '/' + path;
    }
};
