#include "SG/ShaderProgram.h"

#include <ion/gfxutils/shadersourcecomposer.h>

#include "SG/Exception.h"

using ion::gfxutils::StringComposer;
using ion::gfxutils::ShaderSourceComposerPtr;
using ion::gfxutils::ShaderManager;

namespace SG {

void ShaderProgram::AddFields() {
    AddField(inherit_uniforms_);
    AddField(uniform_defs_);
    AddField(vertex_source_);
    AddField(geometry_source_);
    AddField(fragment_source_);
}

void ShaderProgram::SetUpIon(IonContext &context) {
    if (! ion_program_) {
        // Use the shader name as a base for Ion shader names.
        const std::string &name = GetName();

        // If there are any UniformDef instances, create a new registry to hold
        // them and add them.
        ion::gfx::ShaderInputRegistryPtr cur_reg = context.registry_stack.top();
        ASSERT(cur_reg);
        ion::gfx::ShaderInputRegistryPtr reg;
        if (GetUniformDefs().empty()) {
            reg = cur_reg;
        }
        else {
            reg.Reset(new ion::gfx::ShaderInputRegistry);
            if (ShouldInheritUniforms())
                reg->Include(cur_reg);
            else
                reg->IncludeGlobalRegistry();

            for (const auto &def: GetUniformDefs()) {
                def->SetUpIon(context);
                reg->Add<ion::gfx::Uniform>(def->GetIonSpec());
            }
        }

        // Update all ShaderSource instances.
        if (GetVertexSource())
            GetVertexSource()->SetUpIon(context);
        if (GetGeometrySource())
            GetGeometrySource()->SetUpIon(context);
        if (GetFragmentSource())
            GetFragmentSource()->SetUpIon(context);

        //! Helper function.
        auto comp_func = [](const ShaderSourcePtr src,
                            const std::string &name) {
            ShaderSourceComposerPtr sscp;
            if (src && ! src->GetSourceString().empty())
                sscp.Reset(new StringComposer(name, src->GetSourceString()));
            return sscp;
        };

        // Create a StringComposer for each supplied source.
        ShaderManager::ShaderSourceComposerSet composer_set;
        composer_set.vertex_source_composer =
            comp_func(vertex_source_,   name + "_vp");
        composer_set.geometry_source_composer =
            comp_func(geometry_source_, name + "_gp");
        composer_set.fragment_source_composer =
            comp_func(fragment_source_, name + "_fp");

        // There has to be a vertex program for this to work.
        if (! composer_set.vertex_source_composer)
            throw Exception("No vertex program for shader '" + name + "'");

        ion_program_ = context.shader_manager->CreateShaderProgram(
            name, reg, composer_set);
        if (! ion_program_->GetInfoLog().empty())
            throw Exception("Unable to compile shader program: " +
                            ion_program_->GetInfoLog());
    }
}

}  // namespace SG
