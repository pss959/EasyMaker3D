#include "Procedural.h"

#include "Assert.h"

/// Helper class for generating RGB images.
class ImageStore_ {
  public:
    /// An RGB pixel.
    struct Pixel {
        uint8 r, g, b;

        // Default constructor sets to black.
        Pixel() : r(0), g(0), b(0) {}
        // Constructor taking RGB values.
        Pixel(uint8 r_in, uint8 g_in, uint8 b_in) : r(r_in), g(g_in), b(b_in) {}
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
        for (int col = 0; col < width_; ++col)
            for (int r = row; r < row + line_width; ++r)
                Set(col, r, pix);
    }

    /// Draws a line parallel to Y with the given line width at the given
    /// column.
    void AddYLine(int col, int line_width, const Pixel &pix) {
        for (int row = 0; row < height_; ++row)
            for (int c = col; c < col + line_width; ++c)
                Set(c, row, pix);
    }

    /// Stores the result in an Ion DataContainer and returns it.
    ion::base::DataContainerPtr GetDataContainer() const {
        return ion::base::DataContainer::CreateAndCopy(
            data_.data(), data_.size(), true, ion::base::AllocatorPtr());
    }

  private:
    const int width_;
    const int height_;
    std::vector<Pixel> data_;
};

ion::gfx::ImagePtr GenerateGridImage(float radius) {
    const int kSize         = 1024;   // Size of the image in each dimension.
    const int kOriginRadius = 5;      // Center circle radius.

    ImageStore_ store(kSize, kSize);

    // Start with all white.
    for (int row = 0; row < kSize; ++row)
        for (int col = 0; col < kSize; ++col)
            store.Set(row, col, 255, 255, 255);

    // Add X/Y axis lines through the center.
    // XXXX Get colors from ColorDict
    const int center = kSize / 2;
    store.AddXLine(center, 3, ImageStore_::Pixel(245, 20, 20));
    store.AddYLine(center, 3, ImageStore_::Pixel(20, 245, 20));

    // Small black square to mark the origin.
    const int m0 = center - kOriginRadius;
    const int m1 = center + kOriginRadius;
    for (int row = m0; row <= m1; ++row)
        for (int col = m0; col <= m1; ++col)
            store.Set(col, row, ImageStore_::Pixel(0, 0, 0));

    // Adds grid lines in both directions with the given spacing and width.
    auto grid_func = [&store, center](int spacing, int width,
                                      const ImageStore_::Pixel &pix){
        for (int col = center - spacing; col >= 0; col -= spacing)
            store.AddYLine(col, width, pix);
        for (int col = center + spacing; col < kSize; col += spacing)
            store.AddYLine(col, width, pix);
        for (int row = center - spacing; row >= 0; row -= spacing)
            store.AddXLine(row, width, pix);
        for (int row = center + spacing; row < kSize; row += spacing)
            store.AddXLine(row, width, pix);
    };

    // Each grid square represents 1 unit. Compute the number of pixels per
    // unit.
    const int ppu = static_cast<int>(kSize / (2.f * radius));

    // Add all grid lines.
    grid_func(1  * ppu, 1, ImageStore_::Pixel(204, 204, 204));
    grid_func(5  * ppu, 1, ImageStore_::Pixel(102, 102, 102));
    grid_func(10 * ppu, 1, ImageStore_::Pixel(0,   0,   0));

    ion::gfx::ImagePtr image(new ion::gfx::Image);
    image->Set(ion::gfx::Image::kRgb888, kSize, kSize,
               store.GetDataContainer());
    return image;
}
