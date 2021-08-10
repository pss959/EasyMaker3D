#pragma once

#include <istream>
#include <string>
#include <vector>

#include <ion/gfx/node.h>
#include <ion/gfx/shape.h>
#include <ion/gfxutils/shadermanager.h>

#include "ExceptionBase.h"
#include "Parser/Typedefs.h"
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

    Loader();
    ~Loader();

    //! Loads an Ion node subgraph from a resource file, returning an Ion
    //! NodePtr to the result.
    ion::gfx::NodePtr LoadNodeResource(const std::string &path);

    //! Loads an Ion node subgraph from a file specified by full path,
    //! returning an Ion NodePtr to the result.
    ion::gfx::NodePtr LoadNode(const std::string &path);

  private:
    //! Ion ShaderManager used to keep track of all shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager_;

    //! Builds a full path to a resource file for a named type.
    static std::string FullPath(const std::string &type,
                                const std::string &path) {
        return std::string(RESOURCE_DIR) + '/' + type + '/' + path;
    }

    //! Uses a Parser to parse the given file.
    static Parser::ObjectPtr ParseFile_(const std::string &path);

    // XXXX Parser object extraction functions.
    ion::gfx::NodePtr       ExtractNode_(const Parser::Object &obj);
    ion::gfx::StateTablePtr ExtractStateTable_(const Parser::Object &obj);
    ion::gfx::ShaderProgramPtr ExtractShaderProgram_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractShape_(const Parser::Object &obj);

    ion::gfx::ShapePtr      ExtractBox_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractCylinder_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractEllipsoid_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractPolygon_(const Parser::Object &obj);
    ion::gfx::ShapePtr      ExtractRectangle_(const Parser::Object &obj);

    void CheckObjectType_(const Parser::Object &obj,
                          const std::string &expected_type);

    void ThrowTypeMismatch_(const Parser::Object &obj,
                            const std::string &expected_type);
    void ThrowMissingField_(const Parser::Object &obj,
                            const std::string &field_name);
    void ThrowBadField_(const Parser::Object &obj, const Parser::Field &field);
    void ThrowEnumException_(const Parser::Object &obj,
                             const Parser::Field &field,
                             const std::string &enum_type_name);
};
