//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include <vector>

#include "IO/OFFWriter.h"
#include "Math/MeshUtils.h"
#include "Models/BoxModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/TempFile.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class WriteOFFTest : public SceneTestBase {
  protected:
    /// Writes the given Model(s) in OFF format, returning the string
    /// representing the contents of the output file.
    Str WriteModelsAsOFF(const std::vector<ModelPtr> &models,
                        const std::vector<Color> &colors) {
        std::vector<TriMesh> meshes;
        for (const auto &model: models)
            meshes.push_back(TransformMesh(model->GetMesh(),
                                           model->GetModelMatrix()));

        TempFile tmp("");
        EXPECT_TRUE(WriteOFFFile(meshes, colors, tmp.GetPath()));

        return tmp.GetContents();
    }
};

TEST_F(WriteOFFTest, OneBoxNoColor) {
const Str expected = R"(OFF
8 12 0
-2 -3 -4
-2 -3 4
-2 3 -4
-2 3 4
2 -3 -4
2 -3 4
2 3 -4
2 3 4
3 0 1 2
3 1 3 2
3 2 3 6
3 3 7 6
3 4 6 5
3 7 5 6
3 0 4 5
3 1 0 5
3 1 5 7
3 1 7 3
3 0 2 6
3 0 6 4
)";

    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    // (Different sizes to check the coordinate system conversions.)
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 3, 4));

    const Str actual = WriteModelsAsOFF(std::vector<ModelPtr>{ box },
                                        std::vector<Color>());
    EXPECT_TRUE(CompareStrings(expected, actual));
}

TEST_F(WriteOFFTest, OneBoxWithColor) {
const Str expected = R"(OFF
8 12 0
-2 -3 -4
-2 -3 4
-2 3 -4
-2 3 4
2 -3 -4
2 -3 4
2 3 -4
2 3 4
3 0 1 2 255 0 0
3 1 3 2 255 0 0
3 2 3 6 255 0 0
3 3 7 6 255 0 0
3 4 6 5 255 0 0
3 7 5 6 255 0 0
3 0 4 5 255 0 0
3 1 0 5 255 0 0
3 1 5 7 255 0 0
3 1 7 3 255 0 0
3 0 2 6 255 0 0
3 0 6 4 255 0 0
)";

    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    // (Different sizes to check the coordinate system conversions.)
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 3, 4));

    const Str actual = WriteModelsAsOFF(std::vector<ModelPtr>{ box },
                                        std::vector<Color>{ Color(1, 0, 0) });
    EXPECT_TRUE(CompareStrings(expected, actual));
}

TEST_F(WriteOFFTest, TextTwoBoxesWithColor) {
const Str expected = R"(OFF
16 24 0
-2 -3 -4
-2 -3 4
-2 3 -4
-2 3 4
2 -3 -4
2 -3 4
2 3 -4
2 3 4
8 -2 -2
8 -2 2
8 2 -2
8 2 2
12 -2 -2
12 -2 2
12 2 -2
12 2 2
3 0 1 2 0 255 0
3 1 3 2 0 0 255
3 2 3 6 0 255 0
3 3 7 6 0 0 255
3 4 6 5 0 255 0
3 7 5 6 0 0 255
3 0 4 5 0 255 0
3 1 0 5 0 0 255
3 1 5 7 0 255 0
3 1 7 3 0 0 255
3 0 2 6 0 255 0
3 0 6 4 0 0 255
3 8 9 10 0 255 0
3 9 11 10 0 0 255
3 10 11 14 0 255 0
3 11 15 14 0 0 255
3 12 14 13 0 255 0
3 15 13 14 0 0 255
3 8 12 13 0 255 0
3 9 8 13 0 0 255
3 9 13 15 0 255 0
3 9 15 11 0 0 255
3 8 10 14 0 255 0
3 8 14 12 0 0 255
)";

    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    auto box0 = Model::CreateModel<BoxModel>();
    box0->SetScale(Vector3f(2, 3, 4));

    // Create a 2x2x2 Box centered on (10,0,0).
    auto box1 = Model::CreateModel<BoxModel>();
    box1->SetUniformScale(2);
    box1->SetTranslation(Vector3f(10, 0, 0));

    const Str actual = WriteModelsAsOFF(std::vector<ModelPtr>{ box0, box1 },
                                        std::vector<Color>{
                                            Color(0, 1, 0),
                                            Color(0, 0, 1)
                                        });
    EXPECT_TRUE(CompareStrings(expected, actual));
}

TEST_F(WriteOFFTest, BadPath) {
    auto box = Model::CreateModel<BoxModel>();
    EXPECT_FALSE(WriteOFFFile(std::vector<TriMesh>{ box->GetMesh() },
                              std::vector<Color>(),
                              "/this/is/a/bad/path.stl"));
}
