#include <string>
#include <vector>

#include <stblib/stb_image_write.h>

#include "App/Application.h"
#include "App/Args.h"
#include "App/Renderer.h"
#include "App/SnapScript.h"
#include "Base/Tuning.h"
#include "Math/Types.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"

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
    size_t     cur_instruction_ = 0;

    bool LoadSession_(const std::string &file_name);
    bool TakeSnapshot_(const Range2f &rect, const std::string &file_name);
};

bool SnapshotApp_::ProcessFrame(size_t render_count) {
    if (! Application::ProcessFrame(render_count))
        return false;

    // Process instructions in the script.
    const size_t instr_count = script_.GetInstructions().size();
    bool keep_going   = true;
    bool render_again = false;
    while (keep_going && ! render_again) {
        const auto &instr = script_.GetInstructions()[cur_instruction_];

        std::cerr << "XXXX Processing " << cur_instruction_
                  << "  " << instr.type << " of " << instr_count << "\n";

        if (instr.type == "load") {
            if (! LoadSession_(instr.file_name))
                return false;
            // After loading a session, wait until the next frame to continue.
            render_again = true;
        }
        else if (instr.type == "snap") {
            if (! TakeSnapshot_(instr.rect, instr.file_name))
                return false;
        }
        else {
            std::cerr << "XXXX SKIPPING " << instr.type << "\n";
        }
        if (++cur_instruction_ == instr_count)
            keep_going = false;
    }
    return keep_going;
}

bool SnapshotApp_::LoadSession_(const std::string &file_name) {
    std::string error;
    const FilePath path("PublicDoc/snaps/sessions/" + file_name);

    if (! LoadSession(path, error)) {
        std::cerr << "*** Error loading session from '"
                  << path.ToString() << "'\n";
        return false;
    }
    std::cerr << "=== Loaded session from '" << path.ToString() << "'\n";
    return true;
}

bool SnapshotApp_::TakeSnapshot_(const Range2f &rect,
                                 const std::string &file_name) {
    const size_t wh = TK::kWindowHeight;
    const size_t ww = static_cast<size_t>(TK::kWindowAspectRatio * wh);

    const auto &minp = rect.GetMinPoint();
    const auto  size = rect.GetSize();

    const size_t x = static_cast<size_t>(minp[0] * ww);
    const size_t y = static_cast<size_t>(minp[1] * wh);
    const size_t w = static_cast<size_t>(size[0] * ww);
    const size_t h = static_cast<size_t>(size[1] * wh);

    const auto pixels = GetRenderer().ReadPixels(x, y, w, h);

    const FilePath path("PublicDoc/snaps/images/" + file_name);
    const void *pp = reinterpret_cast<const void *>(&pixels[0]);
    if (! stbi_write_jpg(path.ToString().c_str(), w, h, 3, pp, 100)) {
        std::cerr << "*** Error saving snap image to = '"
                  << path.ToString() << "'\n";
        return false;
    }
    std::cerr << "=== Saved snap image to = '" << path.ToString() << "'\n";
    return true;
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
