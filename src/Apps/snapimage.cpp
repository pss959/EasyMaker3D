#include <string>
#include <vector>

#include <stblib/stb_image_write.h>

#include "App/Application.h"
#include "App/Args.h"
#include "App/Renderer.h"
#include "App/SnapScript.h"
#include "Math/Types.h"

// ----------------------------------------------------------------------------
// Derived Application class that adds snapshot processing. Reads a SnapScript
// that specifies what to do.
// ----------------------------------------------------------------------------

class SnapshotApp_ : public Application {
  public:
    void SetScript(const SnapScript &script) { script_ = script; }

    virtual bool ProcessFrame(size_t render_count) override;

  private:
    SnapScript script_;

    void SaveImage_(int width, int height, const std::vector<uint8> &pixels,
                    const std::string &file_name);
};

bool SnapshotApp_::ProcessFrame(size_t render_count) {
    const bool ret = Application::ProcessFrame(render_count);
    if (ret) {
        // XXXX
        std::vector<uint8> pixels = GetRenderer().ReadPixels(600, 300, 128, 64);
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
R"(snapimage: Reads a script with instructions on how to create snapshot images
 for public documentation. See SnapScript.h for script details.
    Usage:
      snapimage SCRIPT

    Script files are relative to PublicDoc/snaps/scripts.
)";

int main(int argc, const char *argv[]) {
    Args args(argc, argv, kUsageString);

    SnapScript script;
    if (! script.ReadScript("PublicDoc/snaps/scripts/" +
                            args.GetString("SCRIPT")))
        return -1;

    Application::Options options;
    options.window_size.Set(1066, 600);
    options.show_session_panel = false;

    SnapshotApp_ app;
    if (! app.Init(options))
        return -1;

    app.SetScript(script);
    app.MainLoop();

    return 0;
}
