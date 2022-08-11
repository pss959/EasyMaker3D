#include "App/Application.h"
#include "App/Args.h"
#include "App/Renderer.h"

#include <../../ionsrc/Ion/third_party/stblib/stb_image_write.h>  // XXXX

// ----------------------------------------------------------------------------
// Derived Application class that adds snapshot processing.
// ----------------------------------------------------------------------------

class SnapshotApp_ : public Application {
  public:
    virtual bool ProcessFrame(size_t render_count) override;

  private:
    void SaveImage_(int width, int height, const std::vector<uint8> &pixels,
                    const std::string &file_name);
};

bool SnapshotApp_::ProcessFrame(size_t render_count) {
    const bool ret = Application::ProcessFrame(render_count);
    if (ret) {
        // XXXX
        std::vector<uint8> pixels = GetRenderer().ReadPixels(200, 300, 128, 64);
        SaveImage_(128, 64, pixels, "TEST");
        return false;
    }
    return ret;
}

void SnapshotApp_::SaveImage_(int width, int height,
                              const std::vector<uint8> &pixels,
                              const std::string &file_name) {
    const std::string &fn = file_name + ".jpg";
    const void *pp = reinterpret_cast<const void *>(&pixels[0]);
    stbi_write_jpg(fn.c_str(), width, height, 3, pp, 100);
    std::cerr << "=== Wrote image to = " << fn << "\n";
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

static const char kUsageString[] =
R"(snapimage: Optionally loads a session and takes a snapshot
    Usage:
      snapimage X Y WIDTH HEIGHT [--session=<session_file>]

    Window size is 1066x600. Session files are relative to PublicDoc/sessions/.
)";

int main(int argc, const char *argv[]) {
    Args args(argc, argv, kUsageString);

    Application::Options options;
    options.window_size.Set(1066, 600);

    SnapshotApp_ app;
    if (! app.Init(options))
        return -1;

    app.MainLoop();

    return 0;
}
