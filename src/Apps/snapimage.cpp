#include <string>
#include <vector>

#include <ion/gfx/image.h>
#include <ion/image/conversionutils.h>

#include "App/Application.h"
#include "App/Args.h"
#include "App/Renderer.h"
#include "App/SceneContext.h"
#include "App/Selection.h"
#include "App/SnapScript.h"
#include "Debug/Shortcuts.h"
#include "Items/Controller.h"
#include "Managers/CommandManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "SG/Search.h"
#include "SG/WindowCamera.h"
#include "Tests/TestContext.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/Read.h"
#include "Util/Write.h"
#include "Widgets/StageWidget.h"

// ----------------------------------------------------------------------------
// Derived Application class that adds snapshot processing. Reads a SnapScript
// that specifies what to do.
// ----------------------------------------------------------------------------

class SnapshotApp_ : public Application {
  public:
    virtual bool Init(const Options &options) override;
    void SetRemain(bool remain) { remain_ = remain; }
    void SetScript(const SnapScript &script) { script_ = script; }

    virtual bool ProcessFrame(size_t render_count) override;

  private:
    Vector2i    window_size_;    // From Options.
    TestContext test_context_;   // For accessing managers.
    SnapScript  script_;
    size_t      cur_instruction_ = 0;
    bool        remain_ = false;

    bool LoadSession_(const std::string &file_name);
    bool AddHand_(const SnapScript::Instruction &instr);
    bool TakeSnapshot_(const Range2f &rect, const std::string &file_name);
    Selection BuildSelection_(const std::vector<std::string> &names);
};

bool SnapshotApp_::Init(const Options &options) {
    if (! Application::Init(options))
        return false;

    window_size_ = options.window_size;
    GetTestContext(test_context_);

    // Make sure there is no debug text visible.
    Debug::DisplayDebugText("");

    return true;
}

bool SnapshotApp_::ProcessFrame(size_t render_count) {
    if (! Application::ProcessFrame(render_count))
        return false;

    // Process instructions in the script.
    const size_t instr_count = script_.GetInstructions().size();
    bool keep_going   = cur_instruction_ < instr_count;
    bool render_again = false;
    while (keep_going && ! render_again) {
        const auto &instr = script_.GetInstructions()[cur_instruction_];

        std::cout << "=== Processing " << instr.type << " (instruction "
                  << (cur_instruction_ + 1) << " of " << instr_count << ")\n";

        if (instr.type == "hand") {
            if (! AddHand_(instr))
                return false;
            render_again = true;
        }
        else if (instr.type == "load") {
            if (! LoadSession_(instr.file_name))
                return false;
            // After loading a session, wait until the next frame to continue.
            render_again = true;
        }
        else if (instr.type == "redo") {
            for (size_t i = 0; i < instr.count; ++i)
                test_context_.command_manager->Redo();
        }
        else if (instr.type == "select") {
            test_context_.selection_manager->ChangeSelection(
                BuildSelection_(instr.names));
            // Selection change requires another render.
            render_again = true;
        }
        else if (instr.type == "snap") {
            if (! TakeSnapshot_(instr.rect, instr.file_name))
                return false;
        }
        else if (instr.type == "stage") {
            auto &stage = *test_context_.scene_context->stage;
            stage.SetScaleAndRotation(instr.stage_scale, instr.stage_angle);
            // Need to render to see the new stage.
            render_again = true;
        }
        else if (instr.type == "touch") {
            auto &sc = *test_context_.scene_context;
            sc.left_controller->SetTouchMode(instr.touch_on);
            sc.right_controller->SetTouchMode(instr.touch_on);
            ForceTouchMode(instr.touch_on);
            render_again = true;
        }
        else if (instr.type == "undo") {
            for (size_t i = 0; i < instr.count; ++i)
                test_context_.command_manager->Undo();
        }
        else if (instr.type == "view") {
            test_context_.scene_context->window_camera->SetOrientation(
                Rotationf::RotateInto(-Vector3f::AxisZ(), instr.view_dir));
            // View change requires another render.
            render_again = true;
        }
        else {
            ASSERTM(false, "Unknown instruction type: " + instr.type);
        }
        if (++cur_instruction_ == instr_count)
            keep_going = false;
    }
    return keep_going || remain_;
}

bool SnapshotApp_::LoadSession_(const std::string &file_name) {
    const FilePath path("PublicDoc/snaps/sessions/" + file_name);

    std::string error;
    if (! test_context_.session_manager->LoadSession(path, error)) {
        std::cerr << "*** Error loading session from '"
                  << path.ToString() << "'\n";
        return false;
    }
    std::cout << "  Loaded session from '" << path.ToString() << "'\n";
    return true;
}

bool SnapshotApp_::AddHand_(const SnapScript::Instruction &instr) {
    auto &controller = instr.hand == Hand::kLeft ?
        *test_context_.scene_context->left_controller :
        *test_context_.scene_context->right_controller;

    if (instr.hand_type == "None") {
        controller.SetEnabled(false);
        return true;
    }

    const std::string file_start = "models/controllers/" + instr.hand_type +
        "_" + Util::EnumToWord(instr.hand);

    const std::string mesh_file = file_start + ".tri";
    const std::string tex_file  = file_start + ".jpg";

    std::string mesh_data;
    Controller::CustomModel cm;
    if (! Util::ReadFile(mesh_file, mesh_data) ||
        ! cm.mesh.FromBinaryString(mesh_data)) {
        std::cerr << "*** Error reading controller mesh data from "
                  << mesh_file << "\n";
        return false;
    }

    cm.texture_image = Util::ReadImage(tex_file, false);
    if (! cm.texture_image) {
        std::cerr << "*** Error reading controller texture image from "
                  << tex_file << "\n";
        return false;
    }

    controller.UseCustomModel(cm);
    controller.SetEnabled(true);
    controller.SetGripGuideType(GripGuideType::kBasic);
    controller.ShowAll(true);
    controller.ShowGripHover(false, Point3f::Zero(), Color::White());
    controller.SetTranslation(instr.hand_pos);
    controller.SetRotation(Rotationf::RotateInto(Vector3f(0, 0, -1),
                                                 instr.hand_dir));
    return true;
}

bool SnapshotApp_::TakeSnapshot_(const Range2f &rect,
                                 const std::string &file_name) {
    const auto &minp = rect.GetMinPoint();
    const auto  size = rect.GetSize();

    const int x = static_cast<int>(minp[0] * window_size_[0]);
    const int y = static_cast<int>(minp[1] * window_size_[1]);
    const int w = static_cast<int>(size[0] * window_size_[0]);
    const int h = static_cast<int>(size[1] * window_size_[1]);

    const auto recti = Range2i::BuildWithSize(Point2i(x, y), Vector2i(w, h));
    const auto image = GetRenderer().ReadImage(recti);
    // Rows of image need to be inverted (GL vs stblib).
    ion::image::FlipImage(image);

    const FilePath path("PublicDoc/docs/images/" + file_name);
    if (! Util::WriteImage(path, *image, false)) {
        std::cerr << "*** Error saving snap image to = '"
                  << path.ToString() << "'\n";
        return false;
    }
    std::cout << "  Saved snap image to = '" << path.ToString() << "'\n";
    return true;
}

Selection SnapshotApp_::BuildSelection_(const std::vector<std::string> &names) {
    const auto &root_model = test_context_.scene_context->root_model;
    Selection sel;
    for (const auto &name: names) {
        SelPath path(SG::FindNodePathUnderNode(root_model, name, false));
        sel.Add(path);
    }
    return sel;
}

// ----------------------------------------------------------------------------
// Mainline.
// ----------------------------------------------------------------------------

static const char kUsageString[] =
R"(snapimage: Reads a script with instructions on how to create snapshot images
 for public documentation. See SnapScript.h for script details.
    Usage:
      snapimage [--remain] SCRIPT

    Options:
      --remain     Keep the window alive after script processing

    Script files are relative to PublicDoc/snaps/scripts.
    Image files are placed in PublicDoc/docs/images.
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
    options.do_ion_remote      = true;

    SnapshotApp_ app;
    if (! app.Init(options))
        return -1;

    app.SetRemain(args.GetBool("--remain"));
    app.SetScript(script);
    app.MainLoop();

    return 0;
}
