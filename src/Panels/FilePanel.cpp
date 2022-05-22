#include "Panels/FilePanel.h"

#include <vector>

#include "Panes/ButtonPane.h"
#include "Panes/CheckboxPane.h"
#include "Panes/DropdownPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "SG/ColorMap.h"
#include "SG/Search.h"
#include "Util/Enum.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Widgets/PushButtonWidget.h"

// ----------------------------------------------------------------------------
// FilePanel::PathList_ class definition.
// ----------------------------------------------------------------------------

class FilePanel::PathList_ {
  public:
    enum class Direction { kUp, kForward, kBack, kHome };

    /// Sets up with the initial absolute path to use.
    void Init(const FilePath &initial_path);

    /// Returns the current path.
    const FilePath & GetCurrent() const { return paths_[cur_index_]; }

    /// Returns true if it is possible to go in the given direction.
    bool CanGoInDirection(Direction dir) const;

    /// Goes to the new path in the given direction. Returns the new path.
    const FilePath & GoInDirection(Direction dir);

    /// Goes to a new path. If the path is not absolute, it is first made
    /// absolute by joining it to the current path. Returns the absolute path.
    const FilePath & GoToPath(const FilePath &path);

    /// Dumps current state for help with debugging.
    void Dump();

  private:
    std::vector<FilePath> paths_;
    size_t                cur_index_ = 0;

    /// If the given path is not absolute, this makes it absolute using the
    /// current path.
    FilePath MakeAbsolute_(const FilePath &path);
};

void FilePanel::PathList_::Init(const FilePath &initial_path) {
    ASSERT(initial_path.IsAbsolute());
    paths_.clear();
    paths_.push_back(initial_path);
    cur_index_ = 0;
}

bool FilePanel::PathList_::CanGoInDirection(Direction dir) const {
    switch (dir) {
      case Direction::kUp:
        return GetCurrent().GetParentDirectory().IsDirectory();
      case Direction::kForward:
        return cur_index_ + 1 < paths_.size();
      case Direction::kBack:
        return cur_index_ > 0;
      case Direction::kHome:
        return GetCurrent() != FilePath::GetHomeDirPath();
    }
    ASSERT(false);
    return false;
}

const FilePath & FilePanel::PathList_::GoInDirection(Direction dir) {
    ASSERT(CanGoInDirection(dir));
    switch (dir) {
      case Direction::kUp:
        GoToPath(GetCurrent().GetParentDirectory());
        break;
      case Direction::kForward:
        ++cur_index_;
        break;
      case Direction::kBack:
        --cur_index_;
        break;
      case Direction::kHome:
        GoToPath(FilePath::GetHomeDirPath());
        break;
      default:
        ASSERT(false);
    }
    return GetCurrent();
}

const FilePath & FilePanel::PathList_::GoToPath(const FilePath &path) {
    ASSERT(! paths_.empty());

    const FilePath full_path = MakeAbsolute_(path);

    if (full_path != paths_.back()) {
        // Remove anything after the current path. This gets rid of old history
        // when the user just went back to a previous path.
        paths_.erase(paths_.begin() + cur_index_ + 1, paths_.end());
        cur_index_ = paths_.size();
        paths_.push_back(full_path);
    }
    return GetCurrent();
}

FilePath FilePanel::PathList_::MakeAbsolute_(const FilePath &path) {
    if (path.IsAbsolute())
        return path;
    FilePath dir = GetCurrent();
    if (! dir.IsDirectory())
        dir = dir.GetParentDirectory();
    return FilePath::Join(dir, path);
}

void FilePanel::PathList_::Dump() {
    for (size_t i = 0; i < paths_.size(); ++i)
        std::cout << "[" << i << "] " << paths_[i].ToString()
                  << (i == cur_index_ ? " [CURRENT]\n" : "\n");
}

// ----------------------------------------------------------------------------
// FilePanel::Impl_ class definition.
// ----------------------------------------------------------------------------

class FilePanel::Impl_ {
  public:
    typedef std::function<void(const PanePtr &)> FocusFunc;

    /// Sets a function used to set focus.
    void SetFocusFunc(const FocusFunc &focus_func) { focus_func_ = focus_func; }

    /// \name Setup functions.
    ///@{
    void Reset();
    void SetTitle(const std::string &title) { title_ = title; }
    void SetTargetType(TargetType type) { target_type_ = type; }
    void SetInitialPath(const FilePath &path) { initial_path_ = path; }
    void SetFileFormats(const std::vector<FileFormat> &formats) {
        file_formats_ = formats;
    }
    void SetExtension(const std::string &extension) { extension_ = extension; }
    void SetHighlightPath(const FilePath &path, const std::string &annotation) {
        highlight_path_       = path;
        highlight_annotation_ = annotation;
    }
    ///@}

    /// \name Result query functions.
    ///@{
    const FilePath    & GetPath()       const { return result_path_; }
    FileFormat          GetFileFormat() const { return file_format_; }
    ///@}

    /// \name Button response functions.
    ///@{
    void GoInDirection(PathList_::Direction dir);
    bool AcceptPath();
    ///@}

    bool HandleEvent(const Event &event, const PanePtr &focused_pane);
    void InitInterface(ContainerPane &root_pane);
    void UpdateInterface();

    /// Makes sure that the focused Pane is in view. If the Pane is a file
    /// button and is out of view in the ScrollingPane, scrolls to view it.
    void UpdateFocus(const PanePtr &pane);

  private:
    /// Status of the current path displayed in the input pane.
    enum class PathStatus_ {
        /// The path is not valid for the current operation.
        kInvalid,
        /// The path is valid, but is not an acceptable type for the current
        /// operation.
        kValid,
        /// The path is valid and can be accepted for the current operation.
        kAcceptable,
    };

    FocusFunc               focus_func_;
    TargetType              target_type_;
    FilePath                initial_path_;
    std::vector<FileFormat> file_formats_;
    std::string             title_;
    std::string             extension_;
    FilePath                highlight_path_;
    std::string             highlight_annotation_;
    int                     highlighted_index_ = -1;  ///< -1 if no highlight.

    FilePath                result_path_;
    FileFormat              file_format_;

    PathList_               paths_;

    // Various parts.
    TextPanePtr      title_pane_;
    TextInputPanePtr input_pane_;
    ScrollingPanePtr file_list_pane_;
    ButtonPanePtr    file_button_pane_;
    PanePtr          format_label_pane_;
    DropdownPanePtr  format_pane_;
    CheckboxPanePtr  hidden_files_pane_;
    ButtonPanePtr    accept_button_pane_;
    ButtonPanePtr    cancel_button_pane_;

    // Directional buttons.
    ButtonPanePtr    dir_button_panes_[Util::EnumCount<PathList_::Direction>()];

    PathStatus_ GetPathStatus_(const FilePath &path);
    void    OpenPath_(const FilePath &path);
    void    OpenDirectory_(const FilePath &path);
    void    SelectFile_(const FilePath &path);
    void    UpdateFiles_(bool scroll_to_highlighted_file);
    int     CreateFileButtons_(const std::vector<std::string> &names,
                               bool are_dirs, std::vector<PanePtr> &buttons);
    PanePtr CreateFileButton_(const std::string &name, bool is_dir,
                              bool is_highlighted);
    void    UpdateButtons_(PathStatus_ path_status);
    void    FocusFileButton_();
    void    ScrollToViewFileButton_(size_t index);
};

// ----------------------------------------------------------------------------
// FilePanel::Impl_ functions.
// ----------------------------------------------------------------------------

void FilePanel::Impl_::Reset() {
    target_type_ = TargetType::kDirectory;
    initial_path_ = FilePath::GetHomeDirPath();
    file_formats_.clear();
    title_ = "Select a File";
    extension_.clear();
    highlight_path_.Clear();
    highlight_annotation_.clear();

    result_path_.Clear();
    file_format_ = FileFormat::kUnknown;
}

void FilePanel::Impl_::GoInDirection(PathList_::Direction dir) {
    OpenPath_(paths_.GoInDirection(dir));
}

bool FilePanel::Impl_::AcceptPath() {
    // Set the result path. If the TextInputPane has a different path and it is
    // valid, use it. Otherwise, use the current path.
    result_path_ = paths_.GetCurrent();
    FilePath input_path(input_pane_->GetText());
    if (! extension_.empty() && input_path.GetExtension() != extension_)
        input_path.AddExtension(extension_);
    if (input_path != result_path_ &&
        GetPathStatus_(input_path) == PathStatus_::kAcceptable)
        result_path_ = input_path;

    // Set the result file format.
    if (format_pane_->IsEnabled()) {
        ASSERT(format_pane_->GetChoiceIndex() >= 0);
        file_format_ = file_formats_[format_pane_->GetChoiceIndex()];
    }

    // If creating a new file and the file exists, return false to let the
    // FilePanel ask the user what to do.
    return target_type_ != TargetType::kNewFile || ! result_path_.Exists();
}

bool FilePanel::Impl_::HandleEvent(const Event &event,
                                   const PanePtr &focused_pane) {
    // ScrollWheel scrolls the file pane.
    if (event.flags.Has(Event::Flag::kPosition1D) &&
        event.device == Event::Device::kMouse) {
        file_list_pane_->ScrollBy(-event.position1D);  // Reverse scroll.
        return true;
    }

    return false;
}

void FilePanel::Impl_::InitInterface(ContainerPane &root_pane) {
    title_pane_         = root_pane.FindTypedPane<TextPane>("Title");
    input_pane_         = root_pane.FindTypedPane<TextInputPane>("Input");
    file_list_pane_     = root_pane.FindTypedPane<ScrollingPane>("FileList");
    hidden_files_pane_  = root_pane.FindTypedPane<CheckboxPane>("HiddenFiles");
    accept_button_pane_ = root_pane.FindTypedPane<ButtonPane>("Accept");
    cancel_button_pane_ = root_pane.FindTypedPane<ButtonPane>("Cancel");
    file_button_pane_   = root_pane.FindTypedPane<ButtonPane>("FileButton");
    format_label_pane_  = root_pane.FindPane("FormatLabel");
    format_pane_        = root_pane.FindTypedPane<DropdownPane>("Format");

    // Access and set up the direction buttons.
    for (auto dir: Util::EnumValues<PathList_::Direction>()) {
        const std::string name = Util::EnumToWords(dir);
        auto but = root_pane.FindTypedPane<ButtonPane>(name);
        dir_button_panes_[Util::EnumInt(dir)] = but;
    }

    // Remove the FileButton from the list of Panes so it does not show up.
    root_pane.RemovePane(file_button_pane_);

    // Install a validation function for the TextInputPane.
    input_pane_->SetValidationFunc([&](const std::string &path_string){
        const PathStatus_ status = GetPathStatus_(path_string);
        UpdateButtons_(status);
        return status != PathStatus_::kInvalid;
    });
}

void FilePanel::Impl_::UpdateInterface() {
    title_pane_->SetText(title_);
    input_pane_->SetInitialText(initial_path_.ToString());

    if (file_formats_.empty()) {
        format_label_pane_->SetEnabled(false);
        format_pane_->SetEnabled(false);
    }
    else {
        const auto choices = Util::ConvertVector<std::string, FileFormat>(
            file_formats_,
            [](const FileFormat &f){ return Util::EnumToWords(f); });
        format_pane_->SetChoices(choices, 0);
        format_label_pane_->SetEnabled(true);
        format_pane_->SetEnabled(true);
    }

    paths_.Init(initial_path_);

    // Open the initial path to set up the file area.
    OpenPath_(initial_path_);

    UpdateButtons_(GetPathStatus_(paths_.GetCurrent()));
}

void FilePanel::Impl_::UpdateFocus(const PanePtr &pane) {
    // Scroll to view a file button if necessary.
    if (pane->GetName() == "FileButton") {
        // Find the index of the pane.
        const auto &panes = file_list_pane_->GetContentsPane()->GetPanes();
        size_t index = panes.size();
        for (size_t i = 0; i < panes.size(); ++i) {
            if (panes[i] == pane) {
                index = i;
                break;
            }
        }
        ASSERT(index != panes.size());
        ScrollToViewFileButton_(index);
    }
}

FilePanel::Impl_::PathStatus_
FilePanel::Impl_::GetPathStatus_(const FilePath &path) {
    const bool is_dir = path.IsDirectory();
    PathStatus_ status;
    switch (target_type_) {
      case TargetType::kDirectory:
        status = is_dir ? PathStatus_::kAcceptable : PathStatus_::kInvalid;
        break;

      case TargetType::kExistingFile:
        status = path.Exists() ?
            (is_dir ? PathStatus_::kValid : PathStatus_::kAcceptable) :
            PathStatus_::kInvalid;
        break;

      case TargetType::kNewFile:
      default:
        status = is_dir ? PathStatus_::kValid : PathStatus_::kAcceptable;
        break;
    }
    return status;
}

void FilePanel::Impl_::OpenPath_(const FilePath &path) {
    ASSERT(path.IsAbsolute());
    if (path.IsDirectory())
        OpenDirectory_(path);
    else
        SelectFile_(path);
}

void FilePanel::Impl_::OpenDirectory_(const FilePath &path) {
    input_pane_->SetInitialText(path.ToString() + FilePath::GetSeparator());

    UpdateFiles_(true);
    UpdateButtons_(GetPathStatus_(path));

    // Focus on the Accept button if it is enabled. Otherwise, focus on the
    // first button in the file list if there are any. If neither is true,
    // focus on the Cancel button.
    if (accept_button_pane_->GetButton().IsInteractionEnabled())
        focus_func_(accept_button_pane_);
    else if (! file_list_pane_->GetContentsPane()->GetPanes().empty())
        FocusFileButton_();
    else
        focus_func_(cancel_button_pane_);
}

void FilePanel::Impl_::SelectFile_(const FilePath &path) {
    input_pane_->SetInitialText(path.ToString());
    UpdateButtons_(GetPathStatus_(path));

    // Set the focus on the Accept button.
    focus_func_(accept_button_pane_);
}

void FilePanel::Impl_::UpdateFiles_(bool scroll_to_highlighted_file) {
    // If the current path is a directory, use it. Otherwise, use its parent.
    FilePath dir = paths_.GetCurrent();
    if (! dir.IsDirectory())
        dir = dir.GetParentDirectory();

    // Get sorted lists of directories and files in the current directory.
    const bool include_hidden = hidden_files_pane_->GetState();
    std::vector<std::string> subdirs;
    std::vector<std::string> files;
    dir.GetContents(subdirs, files, extension_, include_hidden);

    // Ignore files if target must be a directory.
    if (target_type_ == TargetType::kDirectory)
        files.clear();

    // Create buttons for each of them and determine which if any is
    // highlighted.
    std::vector<PanePtr> buttons;
    const int hl_subdir_index = CreateFileButtons_(subdirs, true,  buttons);
    const int hl_file_index   = CreateFileButtons_(files,   false, buttons);

    // Cannot have both a subdir and file highlighted.
    ASSERT(hl_subdir_index < 0 || hl_file_index < 0);
    highlighted_index_ = hl_subdir_index >= 0 ? hl_subdir_index :
        hl_file_index >= 0 ? subdirs.size() + hl_file_index : -1;

    // Install the buttons.
    ASSERT(file_list_pane_->GetContentsPane());
    file_list_pane_->GetContentsPane()->ReplacePanes(buttons);

    // Scroll to the highlighted file, if any. Otherwise, reset the scroll.
    if (scroll_to_highlighted_file && highlighted_index_ >= 0)
        ScrollToViewFileButton_(highlighted_index_);
    else
        file_list_pane_->ScrollTo(0);
}

int FilePanel::Impl_::CreateFileButtons_(
    const std::vector<std::string> &names,
    bool are_dirs, std::vector<PanePtr> &buttons) {

    auto is_highlighted_path = [&](const std::string &name){
        return highlight_path_ &&
            FilePath::Join(paths_.GetCurrent(), name) == highlight_path_;
    };

    size_t cur_index         = 0;
    int    highlighted_index = -1;
    for (const auto &name: names) {
        const bool is_highlighted = is_highlighted_path(name);
        buttons.push_back(CreateFileButton_(name, are_dirs, is_highlighted));
        if (is_highlighted)
            highlighted_index = cur_index;
        ++cur_index;
    }

    return highlighted_index;
}

PanePtr FilePanel::Impl_::CreateFileButton_(const std::string &name,
                                            bool is_dir, bool is_highlighted) {
    auto but = file_button_pane_->CloneTyped<ButtonPane>(true);
    auto text = but->FindTypedPane<TextPane>("ButtonText");
    text->SetText(is_highlighted ? name + highlight_annotation_ : name);

    const std::string color_name = is_highlighted ? "FileHighlightColor" :
        is_dir ? "FileDirectoryColor" : "FileColor";
    text->SetColor(SG::ColorMap::SGetColor(color_name));

    but->GetButton().GetClicked().AddObserver(
        this, [this, name](const ClickInfo &){
            OpenPath_(paths_.GoToPath(name));
        });
    but->SetEnabled(true);

    return but;
}

void FilePanel::Impl_::UpdateButtons_(PathStatus_ path_status) {
    accept_button_pane_->GetButton().SetInteractionEnabled(
        path_status == PathStatus_::kAcceptable);

    for (auto dir: Util::EnumValues<PathList_::Direction>()) {
        auto &but = dir_button_panes_[Util::EnumInt(dir)]->GetButton();
        but.SetInteractionEnabled(paths_.CanGoInDirection(dir));
    }
}

void FilePanel::Impl_::FocusFileButton_() {
    // If there is a highlighted path, focus it. Otherwise, focus the first
    // button.
    const size_t index = highlighted_index_ < 0 ? 0 :
        static_cast<size_t>(highlighted_index_);
    focus_func_(file_list_pane_->GetContentsPane()->GetPanes()[index]);
}

void FilePanel::Impl_::ScrollToViewFileButton_(size_t index) {
    const size_t file_count =
        file_list_pane_->GetContentsPane()->GetPanes().size();
    const float scroll_pos = static_cast<float>(index) / file_count;
    file_list_pane_->ScrollTo(scroll_pos);
}

// ----------------------------------------------------------------------------
// FilePanel functions.
// ----------------------------------------------------------------------------

FilePanel::FilePanel() : impl_(new Impl_()) {
    Reset();

    // Allow the Impl_ to set the focused Pane, which requires calling the
    // protected SetFocus() function.
    auto focus_func = [&](const PanePtr &pane) { SetFocus(pane); };
    impl_->SetFocusFunc(focus_func);
}

FilePanel::~FilePanel() {
}

void FilePanel::Reset() {
    impl_->Reset();
}

bool FilePanel::HandleEvent(const Event &event) {
    return impl_->HandleEvent(event, GetFocusedPane()) ||
        ToolPanel::HandleEvent(event);
}

void FilePanel::UpdateFocus(const PanePtr &pane) {
    impl_->UpdateFocus(pane);
}

void FilePanel::InitInterface() {
    impl_->InitInterface(*GetPane());

    // The Impl_ class cannot call protected functions, so these need to be
    // done here.
    for (auto dir: Util::EnumValues<PathList_::Direction>()) {
        const std::string name = Util::EnumToWords(dir);
        AddButtonFunc(name, [this, dir](){ impl_->GoInDirection(dir); });
    }

    AddButtonFunc("Cancel", [this](){ ProcessResult("Cancel"); });
    AddButtonFunc("Accept", [this](){ TryAcceptPath_(); });
}

void FilePanel::UpdateInterface() {
    impl_->UpdateInterface();
}

void FilePanel::SetTitle(const std::string &title) {
    impl_->SetTitle(title);
}

void FilePanel::SetTargetType(TargetType type) {
    impl_->SetTargetType(type);
}

void FilePanel::SetInitialPath(const FilePath &path) {
    impl_->SetInitialPath(path);
}

void FilePanel::SetFileFormats(const std::vector<FileFormat> &formats) {
    impl_->SetFileFormats(formats);
}

void FilePanel::SetExtension(const std::string &extension) {
    impl_->SetExtension(extension);
}

void FilePanel::SetHighlightPath(const FilePath &path,
                                 const std::string &annotation) {
    impl_->SetHighlightPath(path, annotation);
}

const FilePath & FilePanel::GetPath() const {
    return impl_->GetPath();
}

FileFormat FilePanel::GetFileFormat() const {
    return impl_->GetFileFormat();
}

void FilePanel::ProcessResult(const std::string &result) {
    ReportChange(result, InteractionType::kImmediate);
    Close(result);
}

void FilePanel::TryAcceptPath_() {
    if (impl_->AcceptPath()) {
        ProcessResult("Accept");
    }
    else {
        // There is a conflict. Ask the user what to do.
        const std::string msg = "File \"" + impl_->GetPath().ToString() +
            "\" exists.\nDo you want to overwrite it?";
        auto func = [&](const std::string &answer){
            if (answer == "Yes")
                ProcessResult("Accept");
            // Otherwise, remain open for more interaction.
        };
        AskQuestion(msg, func);
    }
}
