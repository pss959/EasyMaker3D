#include "Managers/SceneContext.h"
#include "Models/Model.h"
#include "SG/Search.h"
#include "Tests/Sim/SimTestBase.h"
#include "Tests/Testing.h"
#include "Tools/Tool.h"
#include "Widgets/Widget.h"

/// \ingroup Tests
class HoverSimTest_ : public SimTestBase {
  protected:
    virtual void TestResults() override {
        // The ComplexityTool should be active and attached to the
        // CylinderModel.
        const auto tool = SG::FindTypedNodeInScene<Tool>(
            *context.scene_context->scene, "ComplexityTool");
        const auto model = tool->GetModelAttachedTo();
        EXPECT_NOT_NULL(model.get());
        EXPECT_EQ("Cylinder_1", model->GetName());

        // Its Slider1DWidget should not be hover-scaled.
        const auto sw = SG::FindTypedNodeUnderNode<Widget>(*tool, "Slider");
        EXPECT_EQ(Vector3f(1, 1, 1), sw->GetScale());
    }
};

TEST_F(HoverSimTest_, HoverSimTest) {
    RunScript("Hover");
}
