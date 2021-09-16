#include "SG/IonSetup.h"

#include <stack>

#include "Assert.h"
#include "SG/Exception.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/PassData.h"
#include "SG/PassRootNode.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
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
                                 ShaderProgram &cur_program);

    //! Returns the ShaderProgram with the given name in the RenderPass,
    //! throwing an exception if it is not found.
    ShaderProgram & FindShader_(const RenderPass &pass,
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

    // Create an empty Ion UniformBlock for each PassData. They will be
    // filled in later.
    for (const auto &pass_data: node.GetPassData()) {
        if (auto &block = pass_data->GetUniformBlock()) {
            if (! block->GetIonUniformBlock())
                InitIonUniformBlock_(*block);
            ion_node->AddUniformBlock(block->GetIonUniformBlock());
        }
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
    // Create all Ion ShaderPrograms for the RenderPass if necessary.
    for (auto &program: pass.GetShaderPrograms()) {
        if (! program->GetIonShaderProgram())
            program->CreateIonShaderProgram(tracker_, *shader_manager_);
    }

    // Start with the shader selected by the root node.
    const PassRootNodePtr root = pass.GetRootNode();
    ShaderProgram &program = FindShader_(pass, root->GetDefaultShaderName());

    // Get the Ion UniformBlock of global values from the shader and add it to
    // the root node of the RenderPass if not already done.
    ASSERT(root->GetIonNode());
    if (root->GetIonNode()->GetUniformBlocks().empty()) {
        auto block = program.GetUniformBlock();
        // Set up the Ion UniformBlock if not already done.
        if (! block->GetIonUniformBlock()) {
            InitIonUniformBlock_(*block);
            block->SetIonRegistry(program.GetIonShaderProgram()->GetRegistry());
            block->AddIonUniforms();
        }
        root->GetIonNode()->AddUniformBlock(block->GetIonUniformBlock());
    }

    // Traverse the graph to set up all nodes, including the root node.
    SetUpNodeForRenderPass_(pass, *root, program);
}

void IonSetup::Impl_::SetUpNodeForRenderPass_(RenderPass &pass, Node &node,
                                              ShaderProgram &cur_program) {
    ShaderProgram *program = &cur_program;

    // If any PassData in the node matches this RenderPass, create uniforms in
    // its Ion UniformBlock.
    for (const auto &pass_data: node.GetPassData()) {
        if (pass_data->GetName() == pass.GetName()) {
            // If the PassData specifies a different shader, use it for
            // this node and its children.
            if (! pass_data->GetShaderName().empty())
                program = &FindShader_(pass, pass_data->GetShaderName());

            // Add Ion uniforms to the PassData's UniformBlock using the
            // current program's registry.
            if (auto &block = pass_data->GetUniformBlock()) {
                ASSERT(block->GetIonUniformBlock());
                block->SetIonRegistry(
                    program->GetIonShaderProgram()->GetRegistry());
                block->AddIonUniforms();
            }
        }
    }

    // Recurse on children.
    for (const auto &child: node.GetChildren())
        SetUpNodeForRenderPass_(pass, *child, *program);
}

ShaderProgram & IonSetup::Impl_::FindShader_(const RenderPass &pass,
                                             const std::string &name) {
    ShaderProgramPtr program = pass.FindShaderProgram(name);
    if (! program)
        throw Exception("Shader program '" + name + "' does not exist in " +
                        pass.GetDesc());
    return *program;
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
