#include "SG/Exception.h"
#include "SG/FileImage.h"
#include "SG/IonContext.h"
#include "SG/ProceduralImage.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ImageTest : public SceneTestBase {};

TEST_F(ImageTest, DefaultFileImage) {
    auto im = CreateObject<SG::FileImage>();

    EXPECT_NOT_NULL(im);
    EXPECT_FALSE(im->GetFilePath());

    auto context = GetIonContext();
    im->SetUpIon(context);

    EXPECT_NULL(im->GetIonImage().Get());
}

TEST_F(ImageTest, FileImage) {
    auto im = CreateObject<SG::FileImage>();

    EXPECT_NOT_NULL(im);
    EXPECT_FALSE(im->GetFilePath());

    im->SetFilePath(GetDataPath("testimage.jpg"));

    auto context = GetIonContext();
    im->SetUpIon(context);

    EXPECT_NOT_NULL(im->GetIonImage().Get());
}

TEST_F(ImageTest, SubImage) {
    const std::string input = R"(
FileImage {
  path: "Frame.jpg",
  sub_images: [
    SubImage "Sub" {
      texture_scale: 2 3,
      texture_offset: 4 -5,
    },
  ],
},
)";
    auto im = ParseObject<SG::Image>(input);

    EXPECT_NOT_NULL(im);

    EXPECT_NOT_NULL(im->FindSubImage("Sub"));
    EXPECT_NULL(im->FindSubImage("NoSuchSub"));
}

TEST_F(ImageTest, BadFileImage) {
    auto im = CreateObject<SG::FileImage>();

    EXPECT_NOT_NULL(im);
    EXPECT_FALSE(im->GetFilePath());

    im->SetFilePath("/no/such/path");

    auto context = GetIonContext();
    TEST_THROW(im->SetUpIon(context), SG::Exception,
               "Unable to open or read image file");
}

TEST_F(ImageTest, ProceduralImage) {
    using ion::gfx::Image;
    using ion::gfx::ImagePtr;

    auto im = CreateObject<SG::ProceduralImage>();
    EXPECT_NOT_NULL(im);
    EXPECT_NULL(im->GetIonImage().Get());

    // With no function, default is a 1x1 image.
    auto context = GetIonContext();
    im->SetUpIon(context);
    auto image = im->GetIonImage();
    EXPECT_NOT_NULL(image.Get());
    EXPECT_EQ(1U, image->GetWidth());
    EXPECT_EQ(1U, image->GetHeight());

    // Set a function and regenerate.
    auto create_image = [](){
        ImagePtr image(new Image);
        auto data = ion::base::DataContainer::Create<uint8>(
            nullptr, nullptr, false, image->GetAllocator());
        image->Set(Image::kRgb888, 4, 4, data);
        return image;
    };
    im->SetFunction(create_image);
    im->RegenerateImage();
    image = im->GetIonImage();
    EXPECT_NOT_NULL(image.Get());
    EXPECT_EQ(4U, image->GetWidth());
    EXPECT_EQ(4U, image->GetHeight());
}

