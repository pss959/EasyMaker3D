#include <ion/gfx/image.h>

#include "Base/Procedural.h"
#include "Math/Color.h"
#include "Math/Types.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

TEST(ProceduralTest, GenerateGridImage) {
    // Swap red and green from the usual, just for testing.
    const Color green(0, 1, 0);
    const Color   red(1, 0, 0);
    const auto image = GenerateGridImage(20, green, red);

    const size_t size = TK::kStageImageSize;
    EXPECT_EQ(size, image->GetWidth());
    EXPECT_EQ(size, image->GetHeight());
    EXPECT_EQ(ion::gfx::Image::Format::kRgb888, image->GetFormat());

    const auto get_pixel = [&](size_t r, size_t c){
        const auto *bytes = image->GetData()->GetData<const uint8>();
        const size_t i = 3 * (r * size + c);
        return Color(static_cast<float>(bytes[i + 0] / 255.f),
                     static_cast<float>(bytes[i + 1] / 255.f),
                     static_cast<float>(bytes[i + 2] / 255.f));
    };

    // Black pixel in the center.
    EXPECT_EQ(Color::Black(), get_pixel(size / 2, size / 2));

    // Green on the X axis.
    EXPECT_EQ(green, get_pixel(size / 2, 3 * size / 4));

    // Red on the Y axis.
    EXPECT_EQ(red,   get_pixel(3 * size / 4, size / 2));
}

TEST(ProceduralTest, GenerateColorRingImage) {
    const auto image = GenerateColorRingImage();
    const size_t size = TK::kColorRingImageSize;
    EXPECT_EQ(size, image->GetWidth());
    EXPECT_EQ(size, image->GetHeight());
    EXPECT_EQ(ion::gfx::Image::Format::kRgb888, image->GetFormat());
}
