//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include <ion/gfx/image.h>

#include "Debug/Timer.h"
#include "Tests/SceneTestBase.h"
#include "Tests/TempFile.h"
#include "Tests/Testing.h"
#include "App/VideoWriter.h"

/// \ingroup Tests
class TimingTest : public SceneTestBase {
  protected:
    void RunTest(VideoWriter::Format format);
  private:
    ion::gfx::ImagePtr BuildImage_(int width, int height, int frame);
};

void TimingTest::RunTest(VideoWriter::Format format) {
    // Constants affecting the test.
    const int kWidth      = 600;
    const int kHeight     = 400;
    const int kFPS        = 30;
    const int kFrameCount = 100;

    Debug::StopTimer create_image_timer("Create images");
    Debug::StopTimer    add_image_timer("Add images   ");
    Debug::StopTimer        write_timer("Write to file");

    VideoWriter vw(format);

    TempFile tmp("");
    vw.Init(tmp.GetPath(), Vector2ui(kWidth, kHeight), kFPS);

    // Add a bunch of images representing frames.
    for (int i = 0; i < kFrameCount; ++i) {
        create_image_timer.Start();
        auto image = BuildImage_(kWidth, kHeight, i);
        create_image_timer.Stop();

        add_image_timer.Start();
        vw.AddImage(*image);
        add_image_timer.Stop();
    }

    write_timer.Start();
    vw.WriteToFile();
    write_timer.Stop();

    create_image_timer.Report();
    add_image_timer.Report();
    write_timer.Report();
}

ion::gfx::ImagePtr TimingTest::BuildImage_(int width, int height, int frame) {
    const size_t data_size = width * height * 3;
    uint8_t      data[data_size];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const int t = 3 * (y * width + x);
            data[t + 0] = (((frame + y) / 25) % 4) * 80;
            data[t + 1] = (((frame + x) / 25) % 4) * 80;
            data[t + 2] = 120;
        }
    }
    ion::gfx::ImagePtr image(new ion::gfx::Image);
    image->Set(ion::gfx::Image::kRgb888, width, height,
               ion::base::DataContainer::CreateAndCopy(
                   data, data_size, true, ion::base::AllocatorPtr()));
    return image;
}

TEST_F(TimingTest, CaptureRGBMP4) {
    RunTest(VideoWriter::Format::kRGBMP4);
}

TEST_F(TimingTest, CaptureYUVMP4) {
    RunTest(VideoWriter::Format::kYUVMP4);
}

TEST_F(TimingTest, CaptureWEBM) {
    RunTest(VideoWriter::Format::kWEBM);
}
