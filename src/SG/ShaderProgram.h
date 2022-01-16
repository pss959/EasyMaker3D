#pragma once

#include <vector>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/shaderprogram.h>
#include <ion/gfxutils/shadermanager.h>

#include "SG/Object.h"
#include "SG/ShaderSource.h"
#include "SG/Typedefs.h"
#include "SG/UniformDef.h"

namespace Parser { class Registry; }

namespace SG {

/// A ShaderProgram object represents a compiled shader program associated with
/// a specific RenderPass object. It also manages a set of uniform definitions.
class ShaderProgram : public Object {
  public:
    virtual bool IsNameRequired() const override { return true; }

    const std::string & GetInheritFrom()    const { return inherit_from_;     }
    ShaderSourcePtr     GetVertexSource()   const { return vertex_source_;    }
    ShaderSourcePtr     GetGeometrySource() const { return geometry_source_;  }
    ShaderSourcePtr     GetFragmentSource() const { return fragment_source_;  }
    const std::vector<UniformDefPtr> & GetUniformDefs() const {
        return uniform_defs_;
    }

    /// Creates and stores an Ion ShaderProgram using the given Tracker and Ion
    /// ShaderManager.
    void SetUpIon(Tracker &tracker,
                  ion::gfxutils::ShaderManager &shader_manager);

    /// Returns the Ion ShaderProgram for this instance. This will be null
    /// until SetUpIon() is called.
    ion::gfx::ShaderProgramPtr GetIonShaderProgram() const {
        return ion_program_;
    }

  protected:
    ShaderProgram() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>         inherit_from_{"inherit_from"};
    Parser::ObjectField<ShaderSource>   vertex_source_{"vertex_source"};
    Parser::ObjectField<ShaderSource>   geometry_source_{"geometry_source"};
    Parser::ObjectField<ShaderSource>   fragment_source_{"fragment_source"};
    Parser::ObjectListField<UniformDef> uniform_defs_{"uniform_defs"};
    ///@}

    ion::gfx::ShaderProgramPtr ion_program_;

    /// Sets up the registry for the program, adds uniform definitions to it,
    /// and returns it.
    ion::gfx::ShaderInputRegistryPtr CreateRegistry_(
        ion::gfxutils::ShaderManager &shader_manager);

    /// Creates a StringComposer for the given shader source and returns it.
    ion::gfxutils::ShaderSourceComposerPtr CreateComposer_(
        const std::string &suffix, Tracker &tracker,
        const ShaderSourcePtr &source);

    friend class Parser::Registry;
};

}  // namespace SG
