//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "App/SceneLoader.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/TempFile.h"
#include "Tests/Testing2.h"

class SceneLoaderTest : public TestBaseWithTypes {};

TEST_F(SceneLoaderTest, LoadScene) {
    TempFile tmp("Scene \"MyScene\" {}\n");

    SceneLoader loader;
    auto scene = loader.LoadScene(tmp.GetPath());
    EXPECT_NOT_NULL(scene);

    EXPECT_NOT_NULL(loader.GetShaderManager().Get());
}

TEST_F(SceneLoaderTest, LoadSceneFail) {
    TempFile tmp("Blah \"WrongType\" {}\n");
    SceneLoader loader;
    EXPECT_NULL(loader.LoadScene(tmp.GetPath()));
}
