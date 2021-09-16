#include "SG/IonSetup.h"

#include <stack>

#include "Assert.h"
#include "SG/Exception.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
#include "SG/ShaderNode.h"
#include "SG/ShaderProgram.h"
#include "SG/Shape.h"
#include "SG/TextNode.h"
#include "SG/Texture.h"
#include "SG/UniformBlock.h"
#include "SG/Visitor.h"

namespace SG {

using ion::gfx::ShaderInputRegistry;
using ion::gfx::ShaderInputRegistryPtr;
using ion::gfxutils::ShaderManagerPtr;
using ion::text::FontManager;

// ----------------------------------------------------------------------------
// IonSetup::Impl_ class.
// ----------------------------------------------------------------------------

//! The IonSetup::Impl_ class does all of the work for IonSetup. It is not a
//! Visitor because it does not need to maintain any path information during
//! traversal. Instead, it does its own recursive traversal of nodes.
//!
//! There are two phases to the setup:
//!  1. Create all corresponding render-pass-independent Ion objects.
//!  2. Set up all render-pass-dependent Ion objects.
class IonSetup::Impl_ : public Visitor {
  public:
    //! The constructor is given all of the necessary context.
    Impl_(Tracker &tracker, const ShaderManagerPtr &shader_manager,
          FontManager &font_manager) :
        tracker_(tracker),
        shader_manager_(shader_manager),
        font_manager_(font_manager) {}

    //! Sets up the given Scene.
    void SetUpScene(Scene &scene);

  private:
    Tracker          &tracker_;
    ShaderManagerPtr  shader_manager_;
    FontManager      &font_manager_;

    //! Recursive function that Implements the first phase of setup: creating
    //! Ion objects for all nodes under the given one, inclusive.
    void SetUpIonObjects_(Node &node);

    //! UniformBlock instances may need a bunch of special work when setting up
    //! Ion objects, such as creating textures, images, and samplers. This
    //! takes care of all of that.
    void InitIonUniformBlock_(UniformBlock &block);

    // Implements the second phase of setup: setting up all
    // render-pass-dependent Ion objects for the given RenderPass.
    void SetUpRenderPass_(RenderPass &pass);

    //! Recursive function that Implements the second phase for a Node. The
    //! current ShaderProgram in effect during the traversal is supplied.
    void SetUpNodeForRenderPass_(RenderPass &pass, Node &node,
                                 const ShaderProgramPtr &cur_program);

    //! Used by SetUpNodeForRenderPass_() for a regular Node. It creates
    //! uniforms in any UniformBlock that matches the given pass, if necessary.
    void SetUpUniformBlocksForRenderPass_(RenderPass &pass, Node &node,
                                          const ShaderProgramPtr &cur_program);

    //! Used by SetUpNodeForRenderPass_() for a ShaderNode. It installs the
    //! Ion ShaderProgram and UniformBlock for the named shader and returns the
    //! new ShaderProgram to use for the node's subgraph.
    ShaderProgramPtr SetUpShaderForRenderPass_(RenderPass &pass,
                                               ShaderNode &node);

    //! Returns the ShaderProgram with the given name in the RenderPass,
    //! throwing an exception if it is not found.
    ShaderProgramPtr FindShader_(const RenderPass &pass,
                                 const std::string &name);
};

// ----------------------------------------------------------------------------
// IonSetup::Impl_ functions.
// ----------------------------------------------------------------------------

void IonSetup::Impl_::SetUpScene(Scene &scene) {
    // Phase 0: Validate each pass.
    for (const auto &pass: scene.GetRenderPasses()) {
        if (! pass->GetRootNode())
            throw Exception("No root node for " + pass->GetDesc());
        if (pass->GetShaderPrograms().empty())
            throw Exception("No shader programs for " + pass->GetDesc());
    }

    // Phase 1:
    for (const auto &pass: scene.GetRenderPasses())
        SetUpIonObjects_(*pass->GetRootNode());

    // Phase 2:
    for (const auto &pass: scene.GetRenderPasses())
        SetUpRenderPass_(*pass);
}

void IonSetup::Impl_::SetUpIonObjects_(Node &node) {
    // If this Node already has an Ion node, no need to traverse under it.
    if (node.GetIonNode())
        return;

    // Create Ion node.
    node.CreateIonNode();
    auto &ion_node = node.GetIonNode();

    // Create a StateTable if necessary.
    if (auto &state_table = node.GetStateTable()) {
        if (! state_table->GetIonStateTable())
            state_table->CreateIonStateTable();
        ion_node->SetStateTable(state_table->GetIonStateTable());
    }

    // Create an empty Ion UniformBlock for each UniformBlock. They will be
    // filled in later.
    for (const auto &block: node.GetUniformBlocks()) {
        if (! block->GetIonUniformBlock())
            InitIonUniformBlock_(*block);
        ion_node->AddUniformBlock(block->GetIonUniformBlock());
    }

    // Add Shapes.
    for (const auto &shape: node.GetShapes()) {
        if (! shape->GetIonShape())
            shape->CreateIonShape();
        ion_node->AddShape(shape->GetIonShape());
    }

    // Deal with derived Node types.
    if (node.GetTypeName() == "TextNode") {
        static_cast<TextNode &>(node).AddIonText(font_manager_,
                                                 shader_manager_);
    }

    // Recurse on and add children.
    for (const auto &child: node.GetChildren()) {
        SetUpIonObjects_(*child);
        ASSERT(child->GetIonNode());
        ion_node->AddChild(child->GetIonNode());
    }
}

void IonSetup::Impl_::InitIonUniformBlock_(UniformBlock &block) {
    block.CreateIonUniformBlock();

    // Create Ion Textures, including their images and samplers.
    for (const auto &tex: block.GetTextures()) {
        if (! tex->GetIonTexture()) {
            tex->CreateIonTexture();
            auto &ion_tex = tex->GetIonTexture();
            if (auto &image = tex->GetImage())
                ion_tex->SetImage(0U, image->GetIonImage(tracker_));
            if (auto &sampler = tex->GetSampler())
                ion_tex->SetSampler(sampler->GetIonSampler());
        }
    }
}

void IonSetup::Impl_::SetUpRenderPass_(RenderPass &pass) {
    // Create all Ion ShaderPrograms and default UniformBlocks for the
    // RenderPass if necessary.
    for (auto &program: pass.GetShaderPrograms()) {
        if (! program->GetIonShaderProgram())
            program->CreateIonShaderProgram(tracker_, *shader_manager_);
        if (auto &block = program->GetUniformBlock()) {
            if (! block->GetIonUniformBlock()) {
                InitIonUniformBlock_(*block);
                block->SetIonRegistry(
                    program->GetIonShaderProgram()->GetRegistry());
                block->AddIonUniforms();
            }
        }
    }

    // Traverse the graph to set up all nodes, including the root node.
    SetUpNodeForRenderPass_(pass, *pass.GetRootNode(), nullptr);
}

void IonSetup::Impl_::SetUpNodeForRenderPass_(
    RenderPass &pass, Node &node, const ShaderProgramPtr &cur_program) {
    ShaderProgramPtr program = cur_program;

    // Handle ShaderNode specially.
    if (node.GetTypeName() == "ShaderNode") {
        ShaderProgramPtr new_program = SetUpShaderForRenderPass_(
            pass, static_cast<ShaderNode &>(node));
        if (new_program)
            program = new_program;
    }
    else {
        SetUpUniformBlocksForRenderPass_(pass, node, cur_program);
    }

    // Recurse on children.
    for (const auto &child: node.GetChildren())
        SetUpNodeForRenderPass_(pass, *child, program);
}

ShaderProgramPtr IonSetup::Impl_::SetUpShaderForRenderPass_(RenderPass &pass,
                                                            ShaderNode &node) {
    ShaderProgramPtr program;

    // Do nothing if the ShaderNode is for a different RenderPass.
    if (node.GetName() != pass.GetName())
        return program;

    // Install the ShaderProgram selected by the ShaderNode in the ShaderNode
    // and the corresponding Ion ShaderProgram in its Ion Node.
    program = FindShader_(pass, node.GetShaderName());
    node.SetShaderProgram(program);
    auto &ion_node = node.GetIonNode();
    ASSERT(ion_node);
    ion_node->SetShaderProgram(program->GetIonShaderProgram());

    // Install the Ion UniformBlock of global values from the shader if it has
    // one and add it to the node if not already done.
    if (ion_node->GetUniformBlocks().empty()) {
        if (auto &block = program->GetUniformBlock()) {
            ASSERT(block->GetIonUniformBlock());
            ion_node->AddUniformBlock(block->GetIonUniformBlock());
        }
    }
    return program;
}

void IonSetup::Impl_::SetUpUniformBlocksForRenderPass_(
    RenderPass &pass, Node &node, const ShaderProgramPtr &cur_program) {
    ASSERT(cur_program);
    // If any UniformBlock in the node matches this RenderPass, create uniforms
    // in its Ion UniformBlock.
    for (const auto &block: node.GetUniformBlocks()) {
        if (block->GetName() == pass.GetName()) {
            // Add Ion uniforms to the UniformBlock using the current program's
            // registry.
            ASSERT(block->GetIonUniformBlock());
            block->SetIonRegistry(
                cur_program->GetIonShaderProgram()->GetRegistry());
            block->AddIonUniforms();
        }
    }
}

ShaderProgramPtr IonSetup::Impl_::FindShader_(const RenderPass &pass,
                                              const std::string &name) {
    ShaderProgramPtr program = pass.FindShaderProgram(name);
    if (! program)
        throw Exception("Shader program '" + name + "' does not exist in " +
                        pass.GetDesc());
    return program;
}

// ----------------------------------------------------------------------------
// IonSetup functions.
// ----------------------------------------------------------------------------

IonSetup::IonSetup(Tracker &tracker, const ShaderManagerPtr &shader_manager,
                   FontManager &font_manager) :
    impl_(new Impl_(tracker, shader_manager, font_manager)) {
}

void IonSetup::SetUpScene(Scene &scene) {
    impl_-> SetUpScene(scene);
}

}  // namespace SG
