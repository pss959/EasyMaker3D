#include "Commands/CreatePrimitiveModelCommand.h"
#include "Commands/RotateCommand.h"
#include "Commands/ScaleCommand.h"
#include "Managers/CommandManager.h"
#include "Managers/SceneContext.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "Tests/AppTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

class ScaleCommandTest : public AppTestBase {
  protected:
    // Convenience to create and return a default BoxModel after verifying that
    // it has a default scale and translation.
    ModelPtr CreateBox() {
        auto cpc = CreateCommand<CreatePrimitiveModelCommand>();
        cpc->SetType(PrimitiveType::kBox);
        context.command_manager->AddAndDo(cpc);

        const auto &rm = *context.scene_context->root_model;
        EXPECT_LE(1U, rm.GetChildModelCount());
        const auto box = rm.GetChildModel(rm.GetChildModelCount() - 1);
        EXPECT_EQ(Vector3f(MS, MS, MS), box->GetScale());
        EXPECT_EQ(Vector3f(0,  MS,  0), box->GetTranslation());
        return box;
    }

    // Convenience to create and return a default TorusModel after verifying
    // that it has a default scale and translation.
    ModelPtr CreateTorus() {
        auto cpc = CreateCommand<CreatePrimitiveModelCommand>();
        cpc->SetType(PrimitiveType::kTorus);
        context.command_manager->AddAndDo(cpc);

        const auto &rm = *context.scene_context->root_model;
        EXPECT_LE(1U, rm.GetChildModelCount());
        const auto torus = rm.GetChildModel(rm.GetChildModelCount() - 1);
        const float hh = MS * TK::kTorusInnerRadius;
        EXPECT_EQ(Vector3f(MS, MS, MS), torus->GetScale());
        EXPECT_VECS_CLOSE(Vector3f(0,  hh,  0), torus->GetTranslation());
        return torus;
    }

    // Convenience to create and execute a ScaleCommand.
    void ApplyScaleCommand(const Model &model, ScaleCommand::Mode mode,
                           const Vector3f &ratios) {
        auto sc = CreateCommand<ScaleCommand>();
        sc->SetModelNames(std::vector<std::string>(1, model.GetName()));
        sc->SetMode(mode);
        sc->SetRatios(ratios);

        context.command_manager->AddAndDo(sc);
    }

    // Convenience to apply a RotateCommand to rotate 90 degrees around Z.
    void ApplyRotateCommand(const Model &model) {
        auto rc = CreateCommand<RotateCommand>();
        rc->SetModelNames(std::vector<std::string>(1, model.GetName()));
        rc->SetRotation(BuildRotation(0, 0, 1, 90));
        context.command_manager->AddAndDo(rc);
    }
};

TEST_F(ScaleCommandTest, CenterSymmetric) {
    // Create a Box.
    const auto &box = *CreateBox();

    // Apply a symmetric scale about the center.
    ApplyScaleCommand(box, ScaleCommand::Mode::kCenterSymmetric,
                      Vector3f(2, 3, 4));
    EXPECT_EQ(Vector3f(2 * MS, 3 * MS, 4 * MS), box.GetScale());
    EXPECT_EQ(Vector3f(0, MS, 0),               box.GetTranslation());

    // Rotate 90 degrees around Z and apply another scale.
    ApplyRotateCommand(box);
    EXPECT_EQ(Vector3f(2 * MS, 3 * MS, 4 * MS), box.GetScale());
    EXPECT_EQ(Vector3f(0, MS, 0),   box.GetTranslation());
    ApplyScaleCommand(box, ScaleCommand::Mode::kCenterSymmetric,
                      Vector3f(10, 20, 30));
    EXPECT_EQ(Vector3f(20 * MS, 60 * MS, 120 * MS), box.GetScale());
    EXPECT_EQ(Vector3f(0, MS, 0),                   box.GetTranslation());
}

TEST_F(ScaleCommandTest, BaseSymmetric) {
    // Create a Box and apply a symmetric scale about the center of the Model
    // base.
    const auto &box = *CreateBox();
    ApplyScaleCommand(box, ScaleCommand::Mode::kBaseSymmetric,
                      Vector3f(2, 3, 4));
    EXPECT_EQ(Vector3f(2 * MS, 3 * MS, 4 * MS), box.GetScale());
    EXPECT_EQ(Vector3f(0,      3 * MS, 0),      box.GetTranslation());

    // Try the same thing with a Torus.
    const auto &torus = *CreateTorus();
    ApplyScaleCommand(torus, ScaleCommand::Mode::kBaseSymmetric,
                      Vector3f(2, 3, 4));
    EXPECT_EQ(Vector3f(2 * MS, 3 * MS, 4 * MS), box.GetScale());
    EXPECT_VECS_CLOSE(Vector3f(0, 3 * MS * TK::kTorusInnerRadius, 0),
                      torus.GetTranslation());
}

TEST_F(ScaleCommandTest, BaseSymmetricRotated) {
    // Create a Box.
    const auto &box = *CreateBox();

    // Rotate 90 degrees around Z and then apply the scale. The translation
    // should accommodate the change in X size (which is now along the Y axis).
    ApplyRotateCommand(box);
    EXPECT_EQ(Vector3f(MS, MS, MS), box.GetScale());
    EXPECT_EQ(Vector3f(0, MS, 0),  box.GetTranslation());
    ApplyScaleCommand(box, ScaleCommand::Mode::kBaseSymmetric,
                      Vector3f(10, 20, 30));
    EXPECT_EQ(Vector3f(10 * MS, 20 * MS, 30 * MS), box.GetScale());
    EXPECT_EQ(Vector3f(0,       10 * MS, 0),       box.GetTranslation());
}

TEST_F(ScaleCommandTest, Asymmetric) {
    // Create a Box.
    const auto &box = *CreateBox();

    // Apply an asymmetric scale relative to the minimum side in X (-2) and Y
    // (0) and the maximum side (2) in Z.
    ApplyScaleCommand(box, ScaleCommand::Mode::kAsymmetric, Vector3f(2, 3, -4));
    EXPECT_EQ(Vector3f(2 * MS, 3 * MS, 4 * MS),  box.GetScale());
    EXPECT_EQ(Vector3f(MS,     3 * MS, -3 * MS), box.GetTranslation());

    // Rotate 90 degrees around Z and apply another scale.
    ApplyRotateCommand(box);
    EXPECT_EQ(Vector3f(2 * MS, 3 * MS, 4 * MS),  box.GetScale());
    EXPECT_EQ(Vector3f(MS,     3 * MS, -3 * MS), box.GetTranslation());
    ApplyScaleCommand(box, ScaleCommand::Mode::kAsymmetric,
                      Vector3f(10, 20, 30));
    EXPECT_EQ(Vector3f(20 * MS,  60 * MS, 120 * MS), box.GetScale());
    EXPECT_VECS_CLOSE(Vector3f(-56 * MS, 21 * MS, 113 * MS),
                      box.GetTranslation());
}
