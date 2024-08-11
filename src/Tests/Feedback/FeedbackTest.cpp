//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include <algorithm>

#include "Feedback/AngularFeedback.h"
#include "Feedback/FindFeedback.h"
#include "Feedback/LinearFeedback.h"
#include "Feedback/TooltipFeedback.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Delay.h"
#include "Util/General.h"

/// \ingroup Tests
class FeedbackTest : public SceneTestBase {
  protected:
    /// Returns contents for a real scene that includes feedback.
    static Str GetFeedbackContents() {
        return R"(
children: [<"nodes/templates/Border.emd">,
           <"nodes/Feedback.emd">]
)";
    }
};

TEST_F(FeedbackTest, AngularFeedback) {
    // Need to read the real thing.
    auto af = ReadRealNode<AngularFeedback>(GetFeedbackContents(),
                                            "AngularFeedback");

    EXPECT_FALSE(af->IsInWorldCoordinates());
    af->SetObjectToWorldMatrix(Matrix4f::Identity());

    // Set up the scene bounds function.
    af->SetSceneBoundsFunc([](){ return Bounds(Vector3f(40, 40, 40)); });

    // Have to dig into the AngularFeedback's scene graph to test results.
    auto stl  = SG::FindNodeUnderNode(*af,                    "StartLine");
    auto enl  = SG::FindNodeUnderNode(*af,                    "EndLine");
    auto arc  = SG::FindNodeUnderNode(*af,                    "Arc");
    auto text = SG::FindTypedNodeUnderNode<SG::TextNode>(*af, "Text");

    // Feedback for rotation around Z axis.
    af->SetColor(Color(0, 1, 1));
    af->SubtendArc(Point3f(0, 5, 2), 4, 2, Vector3f::AxisZ(), CircleArc(0, 90));
    EXPECT_EQ(Vector3f(0, 9, 2),                  af->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),              af->GetRotation());
    EXPECT_PTS_CLOSE(Point3f(12,       0,    0),  stl->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(24,  .15f, .15f),  stl->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(0,       12,    0),  enl->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.15f,  24, .15f),  enl->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(5.024f, 5.024f,  0),
                     arc->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(10.0517f, 10.0517f, .1f),
                      arc->GetBounds().GetSize());
    EXPECT_EQ(Rotationf::Identity(),             text->GetRotation());
    EXPECT_EQ(Color(0, 1, 1),                    text->GetColor());
    EXPECT_EQ("90",                              text->GetText());

    // Feedback for rotation around Y axis.
    af->SetColor(Color(1, 0, 1));
    af->SubtendArc(Point3f(4, 10, -6), 2, 1, Vector3f::AxisZ(),
                   CircleArc(90, -180));
    EXPECT_EQ(Vector3f(4, 12, -6),               af->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),             af->GetRotation());
    EXPECT_PTS_CLOSE(Point3f(0,      12,    0),  stl->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.15f, 24, .15f),  stl->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(0,      -12,    0), enl->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.15f,  24, .15f), enl->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(5.021f, 0,  0),     arc->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(10.0457f, 20.1f, .1f),
                      arc->GetBounds().GetSize());
    EXPECT_EQ(Rotationf::Identity(),             text->GetRotation());
    EXPECT_EQ(Color(1, 0, 1),                    text->GetColor());
    EXPECT_EQ("-180",                            text->GetText());

    // This should have no effect.
    af->Activate();
    af->Deactivate();
}

TEST_F(FeedbackTest, LinearFeedback) {
    // Need to read the real thing.
    auto lf = ReadRealNode<LinearFeedback>(GetFeedbackContents(),
                                           "LinearFeedback");

    EXPECT_FALSE(lf->IsInWorldCoordinates());
    lf->SetObjectToWorldMatrix(Matrix4f::Identity());

    // Set up the scene bounds function.
    lf->SetSceneBoundsFunc([](){ return Bounds(Vector3f(40, 40, 40)); });

    // Have to dig into the LinearFeedback's scene graph to test results.
    auto upr0 = SG::FindNodeUnderNode(*lf,                    "Upright0");
    auto upr1 = SG::FindNodeUnderNode(*lf,                    "Upright1");
    auto cbar = SG::FindNodeUnderNode(*lf,                    "Crossbar");
    auto text = SG::FindTypedNodeUnderNode<SG::TextNode>(*lf, "Text");

    // Feedback parallel to X axis.
    lf->SetColor(Color(0, 1, 1));
    lf->SpanPoints(Point3f(-1, 0, 0), Point3f(1, 0, 0));
    EXPECT_EQ(Vector3f(0, 0, 0),                 lf->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),             lf->GetRotation());
    EXPECT_PTS_CLOSE(Point3f(-1,    12.25f,  0), upr0->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.1f, 24.5f, .1f), upr0->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(1,     12.25f,  0), upr1->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.1f, 24.5f, .1f), upr1->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(0,     22.5f,   0), cbar->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(2,     .1f,  .1), cbar->GetBounds().GetSize());
    EXPECT_VECS_CLOSE(Vector3f(1,   25.5f,   0), text->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),             text->GetRotation());
    EXPECT_EQ(Color(0, 1, 1),                    text->GetColor());
    EXPECT_EQ("2",                               text->GetText());

    // Feedback parallel to -Y axis.
    lf->SetColor(Color(1, 0, 1));
    lf->SpanPoints(Point3f(0, 10, 0), Point3f(0, 0, 0));
    EXPECT_EQ(Vector3f(0, 0, 0),                 lf->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),             lf->GetRotation());
    EXPECT_PTS_CLOSE(Point3f(12.25f,   10,   0), upr0->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(24.5f, .1f, .1f), upr0->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(12.25f,    0,   0), upr1->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(24.5f, .1f, .1f), upr1->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(22.5f,     5,   0), cbar->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.1f,    10,  .1), cbar->GetBounds().GetSize());
    EXPECT_VECS_CLOSE(Vector3f(25.5f,   1,   0), text->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),             text->GetRotation());
    EXPECT_EQ(Color(1, 0, 1),                    text->GetColor());
    EXPECT_EQ("10",                              text->GetText());

    // Feedback parallel to Z axis. Text should be rotated.
    lf->SetColor(Color(1, 1, 0));
    lf->SpanPoints(Point3f(0, 4, -6), Point3f(0, 4, 6));
    EXPECT_EQ(Vector3f(0, 0, 0),                 lf->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),             lf->GetRotation());
    EXPECT_PTS_CLOSE(Point3f(0,    14.05f,  -6), upr0->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.1f, 20.1f, .1f), upr0->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(0,    14.05f,   6), upr1->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.1f, 20.1f, .1f), upr1->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(0,     22.1f,   0), cbar->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.1f,   .1f,  12), cbar->GetBounds().GetSize());
    EXPECT_VECS_CLOSE(Vector3f(0,   25.1f,   6), text->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),             text->GetRotation());
    EXPECT_EQ(Color(1, 1, 0),                    text->GetColor());
    EXPECT_EQ("12",                              text->GetText());

    // SpanLength() is the same as SpanPoints() except that the length can have
    // a sign. Repeat the -Y axis test here and check the differences.
    lf->SetColor(Color(1, 0, 1));
    lf->SpanLength(Point3f(0, 10, 0), -Vector3f::AxisY(), -10);
    EXPECT_EQ(Vector3f(0, 0, 0),                 lf->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),             lf->GetRotation());
    EXPECT_PTS_CLOSE(Point3f(12.25f,   10,   0), upr0->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(24.5f, .1f, .1f), upr0->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(12.25f,   20,   0), upr1->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(24.5f, .1f, .1f), upr1->GetBounds().GetSize());
    EXPECT_PTS_CLOSE(Point3f(22.5f,    15,   0), cbar->GetBounds().GetCenter());
    EXPECT_VECS_CLOSE(Vector3f(.1f,    10,  .1), cbar->GetBounds().GetSize());
    EXPECT_VECS_CLOSE(Vector3f(25.5f,  20,   0), text->GetTranslation());
    EXPECT_EQ(Rotationf::Identity(),             text->GetRotation());
    EXPECT_EQ(Color(1, 0, 1),                    text->GetColor());
    EXPECT_EQ("-10",                             text->GetText());

    // This should have no effect.
    lf->Activate();
    lf->Deactivate();
}

TEST_F(FeedbackTest, TooltipFeedback) {
    // Need to read the real thing.
    auto tf = ReadRealNode<TooltipFeedback>(GetFeedbackContents(),
                                            "TooltipFeedback");

    EXPECT_TRUE(tf->IsInWorldCoordinates());
    tf->SetDelay(.001f);
    tf->SetText("Help string");
    tf->SetColor(Color(1, 0, 1));

    EXPECT_VECS_CLOSE(Vector3f(12.1f, 1.6f, 1), tf->GetTextSize());

    // Activating will happen after the delay.
    EXPECT_FALSE(tf->IsEnabled());
    tf->Activate();
    EXPECT_FALSE(tf->IsEnabled());
    Util::DelayThread(.002f);
    EXPECT_TRUE(tf->IsEnabled());
    tf->Deactivate();
    EXPECT_FALSE(tf->IsEnabled());

    // Cannot activate when delay is 0.
    EXPECT_FALSE(tf->IsEnabled());
    tf->SetDelay(0);
    tf->Activate();
    EXPECT_FALSE(tf->IsEnabled());
    tf->Deactivate();
    EXPECT_FALSE(tf->IsEnabled());
}

TEST_F(FeedbackTest, FindFeedback) {
    auto scene = ReadRealScene(GetFeedbackContents());
    auto fb = FindFeedback(*scene->GetRootNode());
    EXPECT_EQ(3U, fb.size());

    // Sort by type name to make testing easier.
    std::sort(fb.begin(), fb.end(),
              [](const FeedbackPtr &fb0, const FeedbackPtr &fb1){
                  return fb0->GetTypeName() < fb1->GetTypeName();
              });
    EXPECT_TRUE(Util::IsA<AngularFeedback>(fb[0]));
    EXPECT_TRUE(Util::IsA<LinearFeedback>(fb[1]));
    EXPECT_TRUE(Util::IsA<TooltipFeedback>(fb[2]));
}
