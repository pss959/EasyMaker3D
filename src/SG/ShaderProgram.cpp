#include "SG/ShaderProgram.h"

#include <ion/gfxutils/shadersourcecomposer.h>

#include "SG/Exception.h"
#include "SG/ShaderSource.h"
#include "SG/SpecBuilder.h"
#include "SG/UniformDef.h"

using ion::gfxutils::StringComposer;
using ion::gfxutils::ShaderSourceComposerPtr;
using ion::gfxutils::ShaderManager;

namespace SG {

//! Helper function.
static ShaderSourceComposerPtr GetSourceComposer_(const ShaderSourcePtr src,
                                                  const std::string &name) {
    ShaderSourceComposerPtr sscp;
    if (src && ! src->GetSourceString().empty())
        sscp.Reset(new StringComposer(name, src->GetSourceString()));
    return sscp;
}

void ShaderProgram::SetUpIon(IonContext &context) {
    if (! ion_program_) {
        // Use the shader name as a base for Ion shader names.
        const std::string &name = GetName();

        // If there are any UniformDef instances, create a new registry to hold
        // them and add them.
        ion::gfx::ShaderInputRegistryPtr reg;
        if (uniform_defs_.empty()) {
            reg = context.current_registry;
        }
        else {
            reg.Reset(new ion::gfx::ShaderInputRegistry);
            reg->Include(context.current_registry);

            for (const auto &def: uniform_defs_) {
                def->SetUpIon(context);
                reg->Add<ion::gfx::Uniform>(def->GetIonSpec());
            }
        }

        // Update all ShaderSource instances.
        if (vertex_source_)
            vertex_source_->SetUpIon(context);
        if (geometry_source_)
            geometry_source_->SetUpIon(context);
        if (fragment_source_)
            fragment_source_->SetUpIon(context);

        // Create a StringComposer for each supplied source.
        ShaderManager::ShaderSourceComposerSet composer_set;
        composer_set.vertex_source_composer =
            GetSourceComposer_(vertex_source_,   name + "_vp");
        composer_set.geometry_source_composer =
            GetSourceComposer_(geometry_source_, name + "_gp");
        composer_set.fragment_source_composer =
            GetSourceComposer_(fragment_source_, name + "_fp");

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

Parser::ObjectSpec ShaderProgram::GetObjectSpec() {
    SG::SpecBuilder<ShaderProgram> builder;
    builder.AddObjectList<UniformDef>("uniform_defs",
                                      &ShaderProgram::uniform_defs_);
    builder.AddObject<ShaderSource>("vertex_source",
                                    &ShaderProgram::vertex_source_);
    builder.AddObject<ShaderSource>("geometry_source",
                                    &ShaderProgram::geometry_source_);
    builder.AddObject<ShaderSource>("fragment_source",
                                    &ShaderProgram::fragment_source_);
    return Parser::ObjectSpec{
        "Shader", true, []{ return new ShaderProgram; }, builder.GetSpecs() };
}

}  // namespace SG
