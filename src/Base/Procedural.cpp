#include "Base/Procedural.h"

#include "Math/ColorRing.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

namespace {

/// Manages pixels for generating RGB images.
class ImageStore_ {
  public:
    /// An RGB pixel.
    struct Pixel {
        uint8 r, g, b;

        // Default constructor sets to black.
        Pixel() : r(0), g(0), b(0) {}

        // Constructor taking RGB values.
        Pixel(uint8 r_in, uint8 g_in, uint8 b_in) : r(r_in), g(g_in), b(b_in) {}

        // Constructor taking a Color.
        Pixel(const Color &color) :
            r(FloatToUint8(color[0])),
            g(FloatToUint8(color[1])),
            b(FloatToUint8(color[2])) {}

        static uint8 FloatToUint8(float f) {
            return static_cast<uint8>(255 * f);
        }
    };

    /// The constructor is passed the image size. It creates a store of the
    /// appropriate size with all black pixels.
    ImageStore_(int width, int height) : width_(width), height_(height) {
        data_.resize(width * height);
    }

    /// Sets a pixel.
    void Set(int row, int col, const Pixel &pix) {
        ASSERT(col >= 0 && col < width_);
        ASSERT(row >= 0 && row < height_);
        data_[row * width_ + col] = pix;
    }

    /// Sets a pixel by RGB.
    void Set(int row, int col, uint8 r, uint8 g, uint8 b) {
        Set(row, col, Pixel(r, g, b));
    }

    /// Draws a line parallel to X with the given line width at the given row.
    void AddXLine(int row, int line_width, const Pixel &pix) {
        const int hw0 = line_width / 2;
        const int hw1 = std::max(1, hw0);
        for (int col = 0; col < width_; ++col)
            for (int r = row - hw0; r < row + hw1; ++r)
                Set(col, r, pix);
    }

    /// Draws a line parallel to Y with the given line width at the given
    /// column.
    void AddYLine(int col, int line_width, const Pixel &pix) {
        const int hw0 = line_width / 2;
        const int hw1 = std::max(1, hw0);
        for (int row = 0; row < height_; ++row)
            for (int c = col - hw0; c < col + hw1; ++c)
                Set(c, row, pix);
    }

    /// Creates and returns an Ion Image representing the data.
    ion::gfx::ImagePtr GetIonImage() {
        ion::gfx::ImagePtr image(new ion::gfx::Image);
        image->Set(ion::gfx::Image::kRgb888, width_, height_,
                   ion::base::DataContainer::CreateAndCopy(
                       data_.data(), data_.size(), true,
                       ion::base::AllocatorPtr()));
        return image;
    }

  private:
    const int width_;
    const int height_;
    std::vector<Pixel> data_;
};

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

ion::gfx::ImagePtr GenerateGridImage(float radius,
                                     const Color &x_color,
                                     const Color &y_color) {
    const int size = TK::kStageImageSize;
    ImageStore_ store(size, size);

    // Start with all white.
    for (int row = 0; row < size; ++row)
        for (int col = 0; col < size; ++col)
            store.Set(row, col, 255, 255, 255);

    // Small black square to mark the origin.
    const int center = size / 2;
    const int m0 = center - TK::kStageOriginRadius;
    const int m1 = center + TK::kStageOriginRadius;
    for (int row = m0; row <= m1; ++row)
        for (int col = m0; col <= m1; ++col)
            store.Set(row, col, ImageStore_::Pixel(0, 0, 0));

    // Adds grid lines in both directions with the given spacing and width.
    auto grid_func = [&store, center, size](int spacing, int width,
                                            const ImageStore_::Pixel &pix){
        for (int col = center - spacing; col >= 0; col -= spacing)
            store.AddYLine(col, width, pix);
        for (int col = center + spacing; col < size; col += spacing)
            store.AddYLine(col, width, pix);
        for (int row = center - spacing; row >= 0; row -= spacing)
            store.AddXLine(row, width, pix);
        for (int row = center + spacing; row < size; row += spacing)
            store.AddXLine(row, width, pix);
    };

    // Each grid square represents 1 unit. Compute the number of pixels per
    // unit.
    const int ppu = static_cast<int>(size / (2.f * radius));

    // Add all grid lines.
    grid_func(1  * ppu, 1, ImageStore_::Pixel(204, 204, 204));
    grid_func(5  * ppu, 1, ImageStore_::Pixel(102, 102, 102));
    grid_func(10 * ppu, 1, ImageStore_::Pixel(0,   0,   0));

    // Add X/Y axis lines through the center. Do this last so they are on top.
    store.AddXLine(center, 5, ImageStore_::Pixel(y_color));
    store.AddYLine(center, 5, ImageStore_::Pixel(x_color));

    return store.GetIonImage();
}

ion::gfx::ImagePtr GenerateColorRingImage() {
    const int size = TK::kColorRingImageSize;
    ImageStore_ store(size, size);

    // This stores the current point in the range (-1,1). Note that Y has to be
    // negated so that 1 is at the top of the image.
    Point2f point(0, 0);
    for (int row = 0; row < size; ++row) {
        point[1] = -1 + 2 * static_cast<float>(row) / (size - 1);
        for (int col = 0; col < size; ++col) {
            point[0] = -1 + 2 * static_cast<float>(col) / (size - 1);
            const Color c = ColorRing::GetColorForPoint(point);
            store.Set(row, col, c[0] * 255, c[1] * 255, c[2] * 255);
        }
    }

    return store.GetIonImage();
}
