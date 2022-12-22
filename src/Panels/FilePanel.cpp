#include "Panels/FilePanel.h"

#include <vector>

#include <ion/base/stringutils.h>

#include "Panes/ButtonPane.h"
#include "Panes/CheckboxPane.h"
#include "Panes/DropdownPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "SG/ColorMap.h"
#include "SG/Search.h"
#include "Util/Enum.h"
#include "Util/FilePathList.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Widgets/PushButtonWidget.h"

// ----------------------------------------------------------------------------
// FilePanel::Impl_ class definition.
// ----------------------------------------------------------------------------

class FilePanel::Impl_ {
  public:
    typedef std::function<void(const PanePtr &)> FocusFunc;

    Impl_();

    /// Sets a FilePathList instance to use instead of a default FilePathList.
    /// This can be called to use a mock version to simulate a file system.
    void SetFilePathList(FilePathList *list) {
        ASSERT(list);
        path_list_.reset(list);
    }

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
    void GoInDirection(FilePathList::Direction dir);
    bool AcceptPath();
    ///@}

    bool HandleEvent(const Event &event, const PanePtr &focused_pane);
    void InitInterface(ContainerPane &root_pane);
    void UpdateInterface();
    void UpdateForPaneSizeChange(const PanePtr &focused_pane);

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

    std::shared_ptr<FilePathList> path_list_;

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
    ButtonPanePtr dir_button_panes_[Util::EnumCount<FilePathList::Direction>()];

    PathStatus_ GetPathStatus_(const FilePath &path);
    void    OpenPath_(const FilePath &path);
    void    OpenDirectory_(const FilePath &path);
    void    SelectFile_(const FilePath &path);
    void    UpdateFiles_(bool scroll_to_highlighted_file);
    int     CreateFileButtons_(const std::vector<std::string> &names,
                               bool are_dirs, std::vector<PanePtr> &buttons);
    PanePtr CreateFileButton_(size_t index, const std::string &name,
                              bool is_dir, bool is_highlighted);
    void    UpdateButtons_(PathStatus_ path_status);
    void    FocusFileButton_();
    void    ScrollToViewFileButton_(size_t index);

    /// If the given Pane is a file button, this sets index to its index and
    /// returns true. Otherwise, returns false.
    bool    GetFileButtonIndex_(const Pane &pane, size_t &index) const;
};

// ----------------------------------------------------------------------------
// FilePanel::Impl_ functions.
// ----------------------------------------------------------------------------

FilePanel::Impl_::Impl_() :  path_list_ (new FilePathList) {
}

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

void FilePanel::Impl_::GoInDirection(FilePathList::Direction dir) {
    OpenPath_(path_list_->GoInDirection(dir));
}

bool FilePanel::Impl_::AcceptPath() {
    // Set the result path. If the TextInputPane has a different path and it is
    // valid, use it. Otherwise, use the current path.
    result_path_ = path_list_->GetCurrent();
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
    return target_type_ != TargetType::kNewFile ||
        ! path_list_->IsExistingFile(result_path_);
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
    for (auto dir: Util::EnumValues<FilePathList::Direction>()) {
        const std::string name = Util::EnumToWords(dir);
        auto but = root_pane.FindTypedPane<ButtonPane>(name);
        dir_button_panes_[Util::EnumInt(dir)] = but;
    }

    // Update the file list when the user toggles "Show Hidden Files".
    hidden_files_pane_->GetStateChanged().AddObserver(
        this, [&](){ UpdateFiles_(true); });

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

    path_list_->Init(initial_path_);

    // Open the initial path to set up the file area.
    OpenPath_(initial_path_);

    UpdateButtons_(GetPathStatus_(path_list_->GetCurrent()));
}

void FilePanel::Impl_::UpdateForPaneSizeChange(const PanePtr &focused_pane) {
    // Now that file button placement is known, scroll to the focused or
    // highlighted button, if any.
    if (focused_pane)
        UpdateFocus(focused_pane);
    else if (highlighted_index_ >= 0)
        ScrollToViewFileButton_(highlighted_index_);
}

void FilePanel::Impl_::UpdateFocus(const PanePtr &pane) {
    // Scroll to view a file button if necessary.
    size_t index;
    if (GetFileButtonIndex_(*pane, index))
        ScrollToViewFileButton_(index);
}

FilePanel::Impl_::PathStatus_
FilePanel::Impl_::GetPathStatus_(const FilePath &path) {
    const bool is_dir = path_list_->IsValidDirectory(path);
    PathStatus_ status;
    switch (target_type_) {
      case TargetType::kDirectory:
        status = is_dir ? PathStatus_::kAcceptable : PathStatus_::kInvalid;
        break;

      case TargetType::kExistingFile:
        status = path_list_->IsExistingFile(path) ?
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
    if (path_list_->IsValidDirectory(path))
        OpenDirectory_(path);
    else
        SelectFile_(path);
}

void FilePanel::Impl_::OpenDirectory_(const FilePath &path) {
    std::string path_string = path.ToString();
    if (! ion::base::EndsWith(path_string, FilePath::GetSeparator()))
        path_string += FilePath::GetSeparator();

    input_pane_->SetInitialText(path_string);

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
    UpdateFiles_(true);
    UpdateButtons_(GetPathStatus_(path));

    // Set the focus on the Accept button.
    focus_func_(accept_button_pane_);
}

void FilePanel::Impl_::UpdateFiles_(bool scroll_to_highlighted_file) {
    // Get sorted lists of directories and files in the current directory.
    const bool include_hidden = hidden_files_pane_->GetState();
    std::vector<std::string> subdirs;
    std::vector<std::string> files;
    path_list_->GetContents(subdirs, files, extension_, include_hidden);

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

    // Scroll to and focus on the highlighted file, if any. Otherwise, reset
    // the scroll.
    if (scroll_to_highlighted_file && highlighted_index_ >= 0) {
        ScrollToViewFileButton_(highlighted_index_);
        FocusFileButton_();
    }
    else {
        file_list_pane_->ScrollTo(0);
        focus_func_(cancel_button_pane_);
    }
}

int FilePanel::Impl_::CreateFileButtons_(
    const std::vector<std::string> &names,
    bool are_dirs, std::vector<PanePtr> &buttons) {

    auto is_highlighted_path = [&](const std::string &name){
        return highlight_path_ &&
            FilePath::Join(path_list_->GetCurrent(), name) == highlight_path_;
    };

    size_t cur_index         = 0;
    int    highlighted_index = -1;
    for (const auto &name: names) {
        const bool is_highlighted = is_highlighted_path(name);
        buttons.push_back(
            CreateFileButton_(cur_index, name, are_dirs, is_highlighted));
        if (is_highlighted)
            highlighted_index = cur_index;
        ++cur_index;
    }

    return highlighted_index;
}

PanePtr FilePanel::Impl_::CreateFileButton_(size_t index,
                                            const std::string &name,
                                            bool is_dir, bool is_highlighted) {
    auto but = file_button_pane_->CloneTyped<ButtonPane>(
        true, (is_dir ? "Dir_" : "File_") + Util::ToString(index));
    auto text = but->FindTypedPane<TextPane>("ButtonText");
    text->SetText(is_highlighted ? name + highlight_annotation_ : name);

    const std::string color_name = is_highlighted ? "FileHighlightColor" :
        is_dir ? "FileDirectoryColor" : "FileColor";
    text->SetColor(SG::ColorMap::SGetColor(color_name));

    but->GetButton().GetClicked().AddObserver(
        this, [this, name](const ClickInfo &){
            OpenPath_(path_list_->AddPath(name));
        });
    but->SetEnabled(true);

    return but;
}

void FilePanel::Impl_::UpdateButtons_(PathStatus_ path_status) {
    accept_button_pane_->GetButton().SetInteractionEnabled(
        path_status == PathStatus_::kAcceptable);

    for (auto dir: Util::EnumValues<FilePathList::Direction>()) {
        auto &but = dir_button_panes_[Util::EnumInt(dir)]->GetButton();
        but.SetInteractionEnabled(path_list_->CanGoInDirection(dir));
    }
}

void FilePanel::Impl_::FocusFileButton_() {
    // If there is a highlighted path, focus it. Otherwise, focus the first
    // button.
    const size_t index = highlighted_index_ < 0 ? 0 :
        static_cast<size_t>(highlighted_index_);
    const auto &panes = file_list_pane_->GetContentsPane()->GetPanes();
    ASSERT(index < panes.size());
    focus_func_(panes[index]);
}

void FilePanel::Impl_::ScrollToViewFileButton_(size_t index) {
    const auto &panes = file_list_pane_->GetContentsPane()->GetPanes();
    file_list_pane_->ScrollToShowSubPane(*panes[index]);
}

bool FilePanel::Impl_::GetFileButtonIndex_(const Pane &pane,
                                           size_t &index) const {
    const auto &panes = file_list_pane_->GetContentsPane()->GetPanes();
    for (size_t i = 0; i < panes.size(); ++i) {
        if (panes[i].get() == &pane) {
            index = i;
            return true;
        }
    }
    return false;
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

void FilePanel::SetFilePathList(FilePathList *list) {
    impl_->SetFilePathList(list);
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
    for (auto dir: Util::EnumValues<FilePathList::Direction>()) {
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

void FilePanel::UpdateForPaneSizeChange() {
    impl_->UpdateForPaneSizeChange(GetFocusedPane());
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
        AskQuestion(msg, func, true);
    }
}
