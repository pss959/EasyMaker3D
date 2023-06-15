#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "App/Args.h"
#include "App/ScriptedApp.h"
#include "Managers/PanelManager.h"
#include "Panels/FilePanel.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/FilePathList.h"
#include "Util/KLog.h"

/// \file
/// The snapimage application is used to create snapshot images for the public
/// documentation. See the usage string for details.
///
/// \ingroup Apps

// ----------------------------------------------------------------------------
// MockFilePathList_ class.
// ----------------------------------------------------------------------------

/// Derived FilePathList class that simulates a file system so that
/// documentation images are consistent and predictable.
class MockFilePathList_ : public FilePathList {
    /// Redefines this to simulate files.
    virtual void GetContents(std::vector<std::string> &subdirs,
                             std::vector<std::string> &files,
                             const std::string &extension,
                             bool include_hidden) const override;
    virtual bool IsValidDirectory(const FilePath &path) const {
        const std::string fn = path.GetFileName();
        return fn.starts_with("Dir") || fn == "stl" || fn == "maker";
    }
    virtual bool IsExistingFile(const FilePath &path) const {
        return true;
    }
};

void MockFilePathList_::GetContents(std::vector<std::string> &subdirs,
                                    std::vector<std::string> &files,
                                    const std::string &extension,
                                    bool include_hidden) const {
    const FilePath dir = GetCurrent();
    // Special case for dummy path used in doc.
    if (dir.ToString() == "/projects/maker/stl/") {
        files.push_back("Airplane.stl");
        files.push_back("Boat.stl");
        files.push_back("Car.stl");
    }
    else {
        subdirs.push_back("Dir0");
        subdirs.push_back("Dir1");
        files.push_back("FileA" + extension);
        files.push_back("FileB" + extension);
        files.push_back("FileC" + extension);
        files.push_back("FileD" + extension);
        files.push_back("FileE" + extension);
    }
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

constinit const char kUsageString[] =
R"(snapimage: Reads a script with instructions on how to create snapshot images
 for public documentation. See SnapScript.h for script details.
    Usage:
      snapimage [--fullscreen] [--klog=<klog_string>]
                [--remain] [--nosnap] SCRIPT

    Options:
      --fullscreen    Start with a full-screen window.
      --klog=<string> String to pass to KLogger::SetKeyString().
      --nosnap        Ignore snap commands (useful for testing).
      --remain        Keep the window alive after script processing.

    Script files are relative to PublicDoc/snaps/scripts.
    Image files are placed in PublicDoc/docs/images.
)";

int main(int argc, const char *argv[]) {
    Args args(argc, argv, kUsageString);

    ScriptedApp::Options options;

    const FilePath path("PublicDoc/snaps/scripts/" + args.GetString("SCRIPT"));
    if (! options.script.ReadScript(path))
        return -1;

    std::cout << "======= Processing Script file " << path.ToString() << "\n";

    KLogger::SetKeyString(args.GetString("--klog"));
    options.do_ion_remote      = true;
    options.enable_vr          = true;   // So controllers work properly.
    options.fullscreen         = args.GetBool("--fullscreen");
    options.nosnap             = args.GetBool("--nosnap");
    options.remain             = args.GetBool("--remain");
    options.report             = true;
    options.show_session_panel = false;

    // Note that this must have the same aspect ratio as fullscreen.
    options.window_size.Set(1024, 552);

    ScriptedApp app;
    if (! app.Init(options))
        return -1;

    // Use the MockFilePathList_ for the FilePanel and ImportToolPanel.
    const auto set_mock = [&](const std::string &panel_name){
        const auto &panel_mgr = *app.GetContext().panel_manager;
        auto panel = panel_mgr.GetTypedPanel<FilePanel>(panel_name);
        panel->SetFilePathList(new MockFilePathList_);
    };
    set_mock("FilePanel");
    set_mock("ImportToolPanel");

    try {
        app.MainLoop();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught Assertion failure: " << ex.what() << "\n";
        std::cerr << "*** STACK:\n";
        for (const auto &s: ex.GetStackTrace())
            std::cerr << "  " << s << "\n";
        return -1;
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception: " << ex.what() << "\n";
        return -1;
    }

    return 0;
}
