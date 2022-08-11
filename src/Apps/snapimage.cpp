#include "App/Application.h"
#include "App/Args.h"

// ----------------------------------------------------------------------------
// Derived Application class that adds snapshot processing.
// ----------------------------------------------------------------------------

class SnapshotApp_ : public Application {
  public:
    virtual bool ProcessFrame(size_t render_count) override;
};

bool SnapshotApp_::ProcessFrame(size_t render_count) {
    const bool ret = Application::ProcessFrame(render_count);
    if (ret) {
        // XXXX Do something
    }
    return ret;
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
