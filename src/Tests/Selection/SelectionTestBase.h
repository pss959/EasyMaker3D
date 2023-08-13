#pragma once

#include "Models/BoxModel.h"
#include "Models/HullModel.h"
#include "Models/RootModel.h"
#include "SG/NodePath.h"
#include "Selection/SelPath.h"
#include "Tests/SceneTestBase.h"

/// Base class for tests dealing with selections and SelPath instances.
class SelectionTestBase : public SceneTestBase {
  protected:
    using ModelVec = std::vector<ModelPtr>;  ///< Shorthand.

    SG::NodePtr  top;   ///< Node above selection paths.
    RootModelPtr root;  ///< RootModel at top of SelPath instances.
    HullModelPtr par0;  ///< HullModel used as a parent.
    HullModelPtr par1;  ///< HullModel used as a parent.
    BoxModelPtr  box0;  ///< BoxModel used as a child of hull.
    BoxModelPtr  box1;  ///< BoxModel used as a child of hull.

    /// The constructor sets up the scene and all nodes stored here.
    SelectionTestBase() {
        root = ReadRealNode<RootModel>(
            R"(children: [<"nodes/ModelRoot.emd">])", "ModelRoot");
        top  = CreateObject<SG::Node>("Top");
        par0 = Model::CreateModel<HullModel>("Par0");
        par1 = Model::CreateModel<HullModel>("Par1");
        box0 = Model::CreateModel<BoxModel>("Box0");
        box1 = Model::CreateModel<BoxModel>("Box1");
        par0->SetOperandModels(ModelVec{ box0, box1 });
        par1->SetOperandModels(ModelVec{ box0, box1 });
        root->AddChildModel(par0);
        root->AddChildModel(par1);
        top->AddChild(root);
    }

    /// Convenience that creates a SelPath from a vector of Model pointers.
    static SelPath BuildSelPath(const ModelVec &models) {
        SG::NodePath npath;
        for (const auto &m: models)
            npath.push_back(m);
        return SelPath(npath);
    }
};
