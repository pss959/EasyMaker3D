#include "Panels/FilePanel.h"

#include <vector>

#include "Panes/ButtonPane.h"
#include "Panes/CheckboxPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "SG/Search.h"
#include "Util/Enum.h"
#include "Util/General.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// FilePanel::PathList_ class definition.
// ----------------------------------------------------------------------------

class FilePanel::PathList_ {
  public:
    enum class Direction { kUp, kForward, kBack, kHome };

    typedef Util::FilePath Path;  ///< Shorthand.

    void         Init(const Path &initial_path);
    const Path & GetCurrent() const { return paths_[cur_index_]; }
    bool         CanGoInDirection(Direction dir) const;
    const Path & GoInDirection(Direction dir);
    const Path & GoToNewPath(const Path &path);

  private:
    std::vector<Path> paths_;
    size_t            cur_index_ = 0;
};

void FilePanel::PathList_::Init(const Path &initial_path) {
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
        return GetCurrent() != Path::GetHomeDirPath();
    }
    ASSERT(false);
    return false;
}

const Util::FilePath & FilePanel::PathList_::GoInDirection(Direction dir) {
    ASSERT(CanGoInDirection(dir));
    switch (dir) {
      case Direction::kUp:
        GoToNewPath(GetCurrent().GetParentDirectory());
        break;
      case Direction::kForward:
        ++cur_index_;
        break;
      case Direction::kBack:
        --cur_index_;
        break;
      case Direction::kHome:
        GoToNewPath(Path::GetHomeDirPath());
        break;
      default:
        ASSERT(false);
    }
    return GetCurrent();
}

const Util::FilePath & FilePanel::PathList_::GoToNewPath(const Path &path) {
    ASSERT(! paths_.empty());

    const Path full_path = path.IsAbsolute() ? path :
        Path::Join(paths_.back(), path);

    if (full_path != GetCurrent()) {
        paths_.push_back(full_path);
        cur_index_ = paths_.size() - 1;
    }
    return GetCurrent();
}

// ----------------------------------------------------------------------------
// FilePanel::Impl_ class definition.
// ----------------------------------------------------------------------------

class FilePanel::Impl_ {
  public:
    typedef Util::FilePath Path;  ///< Shorthand.

    /// \name Setup functions.
    ///@{
    void Reset();
    void SetTitle(const std::string &title) { title_ = title; }
    void SetTargetType(TargetType type) { target_type_ = type; }
    void SetInitialPath(const Path &path) { initial_path_ = path; }
    void SetFileFormatsEnabled(bool enable) { file_formats_enabled_ = enable; }
    void SetExtension(const std::string &extension) { extension_ = extension; }
    void SetHighlightPath(const Path &path, const std::string &annotation) {
        highlight_path_       = path;
        highlight_annotation_ = annotation;
    }
    ///@}

    /// \name Result query functions.
    ///@{
    const Path & GetPath()       const { return result_path_; }
    FileFormat   GetFileFormat() const { return file_format_; }
    ///@}

    /// \name Button response functions.
    ///@{
    void GoInDirection(PathList_::Direction dir);
    bool AcceptPath();
    ///@}

    void InitInterface(ContainerPane &root_pane);
    void UpdateInterface();

  private:
    TargetType  target_type_;
    Path        initial_path_;
    bool        file_formats_enabled_;
    std::string title_;
    std::string extension_;
    Path        highlight_path_;
    std::string highlight_annotation_;

    Path        result_path_;
    FileFormat  file_format_;

    PathList_   paths_;

    // Various parts.
    TextPanePtr      title_pane_;
    TextInputPanePtr input_pane_;
    ScrollingPanePtr file_list_pane_;
    ButtonPanePtr    file_button_pane_;
    CheckboxPanePtr  hidden_files_pane_;
    ButtonPanePtr    accept_button_pane_;

    // Directional buttons.
    ButtonPanePtr    dir_button_panes_[Util::EnumCount<PathList_::Direction>()];

    bool    CanAcceptPath_(const Path &path, bool is_final_target);
    void    OpenPath_(const Path &path, bool update_files);
    void    OpenDirectory_(const Path &path);
    void    SelectFile_(const Path &path, bool update_files);
    void    UpdateFiles_(bool scroll_to_highlighted_file);
    PanePtr CreateFileButton_(const std::string &name, bool is_dir);
    void    UpdateButtons_();
};

// ----------------------------------------------------------------------------
// FilePanel::Impl_ functions.
// ----------------------------------------------------------------------------

void FilePanel::Impl_::Reset() {
    target_type_ = TargetType::kDirectory;
    initial_path_ = Path::GetHomeDirPath();
    file_formats_enabled_ = false;
    title_ = "Select a File";
    extension_.clear();
    highlight_path_.Clear();
    highlight_annotation_.clear();

    result_path_.Clear();
    file_format_ = FileFormat::kUnknown;
}

void FilePanel::Impl_::GoInDirection(PathList_::Direction dir) {
    OpenPath_(paths_.GoInDirection(dir), true);
}

bool FilePanel::Impl_::AcceptPath() {
    // Set the results.
    result_path_ = paths_.GetCurrent();

    // XXXX Also set file_format_.

    // If creating a new file and the file exists, return false to let the
    // FilePanel ask the user what to do.
    return target_type_ != TargetType::kNewFile || ! result_path_.Exists();
}

void FilePanel::Impl_::InitInterface(ContainerPane &root_pane) {
    title_pane_         = root_pane.FindTypedPane<TextPane>("Title");
    input_pane_         = root_pane.FindTypedPane<TextInputPane>("Input");
    file_list_pane_     = root_pane.FindTypedPane<ScrollingPane>("FileList");
    hidden_files_pane_  = root_pane.FindTypedPane<CheckboxPane>("HiddenFiles");
    accept_button_pane_ = root_pane.FindTypedPane<ButtonPane>("Accept");
    file_button_pane_   = root_pane.FindTypedPane<ButtonPane>("FileButton");

    // Access and set up the direction buttons.
    for (auto dir: Util::EnumValues<PathList_::Direction>()) {
        const std::string name = Util::RemoveFirstN(Util::EnumName(dir), 1);
        auto but = root_pane.FindTypedPane<ButtonPane>(name);
        dir_button_panes_[Util::EnumInt(dir)] = but;
    }
}

void FilePanel::Impl_::UpdateInterface() {
    title_pane_->SetText(title_);
    input_pane_->SetInitialText(initial_path_.ToString());

    // XXXX Enable FileFormat dropdown once it is ready.

    paths_.Init(initial_path_);

    // Open the initial path to set up the file area.
    OpenPath_(initial_path_, true);

    UpdateButtons_();
}

bool FilePanel::Impl_::CanAcceptPath_(const Path &path, bool is_final_target) {
    const bool is_dir = path.IsDirectory();
    switch (target_type_) {
      case TargetType::kDirectory:
        return is_dir;

      case TargetType::kExistingFile:
        // Ok to be a directory if this is not the final target.
        return path.Exists() && ! (is_final_target && is_dir);

      case TargetType::kNewFile:
        // Only bad case is that this is the final target and is an existing
        // directory.
        return ! (is_final_target && is_dir);
    }
    return false;
}

void FilePanel::Impl_::OpenPath_(const Path &path, bool update_files) {
    Path new_path = paths_.GoToNewPath(path);

    if (new_path.IsDirectory())
        OpenDirectory_(new_path);
    else
        SelectFile_(new_path, update_files);
}

void FilePanel::Impl_::OpenDirectory_(const Path &path) {
    input_pane_->SetInitialText(path.ToString() + Path::GetSeparator());

    UpdateFiles_(true);
    UpdateButtons_();

#if XXXX
    // Focus on the Accept button if it is enabled. Otherwise, focus on the
    // first button in the chooser if there are any. If neither is true,
    // focus on the Cancel button.
    if (_acceptButton.IsEnabled())
        SetFocus(_acceptButton);
    else if (_fileContentsBox.GetElementCount() > 0)
        SetFocus(_fileContentsBox.GetElement(0));
    else
        SetFocus(_cancelButton);
#endif
}

void FilePanel::Impl_::SelectFile_(const Path &path, bool update_files) {
    input_pane_->SetInitialText(path.ToString());
    if (update_files)
        UpdateFiles_(true);

    UpdateButtons_();

    // Set the focus on the Accept button.
    // XXXX SetFocus(_acceptButton);
}

void FilePanel::Impl_::UpdateFiles_(bool scroll_to_highlighted_file) {
    // If the current path is a directory, use it. Otherwise, use its parent.
    Path dir = paths_.GetCurrent();
    if (! dir.IsDirectory())
        dir = dir.GetParentDirectory();

    // Get sorted lists of directories and files in the current directory.
    const bool include_hidden = hidden_files_pane_->GetState();
    std::vector<std::string> subdirs;
    std::vector<std::string> files;
    dir.GetContents(subdirs, files, include_hidden);

    // Create a vector containing a clone of the ButtonPane for each directory
    // and file.
    std::vector<PanePtr> buttons;
    for (const auto &subdir: subdirs)
        buttons.push_back(CreateFileButton_(subdir, true));
    for (const auto &file: files)
        buttons.push_back(CreateFileButton_(file, false));
    file_list_pane_->SetPanes(buttons);

    // Scroll to the highlighted file, if any.
    if (scroll_to_highlighted_file && highlight_path_) {
        // XXXX _scrollArea.SetScrollToElement(specialButton);
    }
}

PanePtr FilePanel::Impl_::CreateFileButton_(const std::string &name,
                                            bool is_dir) {
    auto but = file_button_pane_->CloneTyped<ButtonPane>(true);
    auto text = but->FindTypedPane<TextPane>("ButtonText");
    text->SetText(name);
    // XXXX Set color.

    but->GetButton().GetClicked().AddObserver(
        this, [this, name](const ClickInfo &){ OpenPath_(name, true); });
    but->SetEnabled(SG::Node::Flag::kTraversal, true);

    return but;
}

void FilePanel::Impl_::UpdateButtons_() {
    accept_button_pane_->GetButton().SetInteractionEnabled(
        CanAcceptPath_(paths_.GetCurrent(), true));

    for (auto dir: Util::EnumValues<PathList_::Direction>()) {
        auto &but = dir_button_panes_[Util::EnumInt(dir)]->GetButton();
        but.SetInteractionEnabled(paths_.CanGoInDirection(dir));
    }
}

// ----------------------------------------------------------------------------
// FilePanel functions.
// ----------------------------------------------------------------------------

FilePanel::FilePanel() : impl_(new Impl_) {
    Reset();
}

void FilePanel::Reset() {
    impl_->Reset();
}

void FilePanel::InitInterface() {
    impl_->InitInterface(*GetPane());

    // The Impl_ class cannot call protected functions, so these need to be
    // done here.
    for (auto dir: Util::EnumValues<PathList_::Direction>()) {
        const std::string name = Util::RemoveFirstN(Util::EnumName(dir), 1);
        AddButtonFunc(name, [this, dir](){ impl_->GoInDirection(dir); });
    }

    AddButtonFunc("Cancel", [this](){ Close("Cancel"); });
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

void FilePanel::SetInitialPath(const Util::FilePath &path) {
    impl_->SetInitialPath(path);
}

void FilePanel::SetFileFormatsEnabled(bool enable) {
    impl_->SetFileFormatsEnabled(enable);
}

void FilePanel::SetExtension(const std::string &extension) {
    impl_->SetExtension(extension);
}

void FilePanel::SetHighlightPath(const Util::FilePath &path,
                                 const std::string &annotation) {
    impl_->SetHighlightPath(path, annotation);
}

const Util::FilePath & FilePanel::GetPath() const {
    return impl_->GetPath();
}

FileFormat FilePanel::GetFileFormat() const {
    return impl_->GetFileFormat();
}

void FilePanel::TryAcceptPath_() {
    if (impl_->AcceptPath()) {
        Close("Accept");
    }
    else {
        // There is a conflict. Ask the user what to do.
        const std::string msg = "File \"" + impl_->GetPath().ToString() +
            "\" exists.\nDo you want to overwrite it?";
        auto func = [&](const std::string &answer){
            if (answer == "Yes")
                Close("Accept");
            // Otherwise, remain open for more interaction.
        };
        AskQuestion(msg, func);
    }
}
