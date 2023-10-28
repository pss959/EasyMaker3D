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
#include "Util/FilePathList.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Widgets/PushButtonWidget.h"

// ----------------------------------------------------------------------------
// FilePanel::Impl_ class definition.
// ----------------------------------------------------------------------------

class FilePanel::Impl_ {
  public:
    using FocusFunc = std::function<void(const PanePtr &)>;

    Impl_();

    /// Sets a FilePathList instance to use instead of a default FilePathList.
    /// This can be called to use a mock version to simulate a file system.
    void SetFilePathList(FilePathList *list) {  // LCOV_EXCL_START [snaps only]
        ASSERT(list);
        path_list_.reset(list);
    }  // LCOV_EXCL_STOP

    /// Sets a function used to set focus. The function is passed a PanePtr for
    /// the Pane to focus.
    void SetFocusFunc(const FocusFunc &focus_func) { focus_func_ = focus_func; }

    /// \name Setup functions.
    ///@{
    void Reset();
    void SetTitle(const Str &title) { title_ = title; }
    void SetTargetType(TargetType type) { target_type_ = type; }
    void SetInitialPath(const FilePath &path) { initial_path_ = path; }
    void SetFileFormats(const std::vector<FileFormat> &formats) {
        file_formats_ = formats;
    }
    void SetExtension(const Str &extension) { extension_ = extension; }
    void SetHighlightPath(const FilePath &path, const Str &annotation) {
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
    void InitInterface(ContainerPane &root);
    void UpdateInterface();

    /// Changes the focus to the given Pane and makes sure that the Pane is in
    /// view: if the Pane is a file button and is not visible in the
    /// ScrollingPane, it scrolls to view it.
    void SetFocus(const PanePtr &pane);

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
    Str                     title_;
    Str                     extension_;
    FilePath                highlight_path_;
    Str                     highlight_annotation_;
    PanePtr                 highlighted_button_pane_;

    FilePath                result_path_;
    FileFormat              file_format_;

    std::shared_ptr<FilePathList> path_list_;

    /// \name Various parts.
    ///@{
    TextPanePtr      title_pane_;
    TextInputPanePtr input_pane_;
    ScrollingPanePtr file_list_pane_;
    ButtonPanePtr    file_button_pane_;
    PanePtr          format_label_pane_;
    DropdownPanePtr  format_pane_;
    CheckboxPanePtr  hidden_files_pane_;
    ButtonPanePtr    accept_button_pane_;
    ButtonPanePtr    cancel_button_pane_;
    ///@}

    // Directional buttons.
    ButtonPanePtr dir_button_panes_[Util::EnumCount<FilePathList::Direction>()];

    /// Returns a PathStatus_ for the given FilePath.
    PathStatus_ GetPathStatus_(const FilePath &path) const;

    /// Sets the FilePanel to show the given FilePath, which may be a file or
    /// directory. This is called when a file button or direction button is
    /// clicked.
    void ShowPath_(const FilePath &path);

    /// Updates the state of direction and result buttons when the path
    /// changes. The status of the current path is supplied.
    void UpdateButtons_(PathStatus_ path_status);

    /// Updates the files and directories displayed in the FilePanel.
    void UpdateFiles_();

    /// Creates a single ButtonPane for a file or directory.
    PanePtr CreateFileButton_(size_t index, const Str &name, bool is_dir);

    /// Updates focus based on the current state. This also scrolls if
    /// necessary to view the focused Pane.
    void UpdateFocus_();

    /// Returns true if the given directory or file name matches the given
    /// path.
    bool MatchesPath_(const Str &name, const FilePath &path) const;

    /// Returns true if the given Pane is a file button.
    bool IsFileButton_(const Pane &pane) const;

    /// Adds the extension, if any, to the given path if necessary, returning
    /// the result.
    FilePath AddExtension_(const FilePath &path) const;
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
    ShowPath_(path_list_->GoInDirection(dir));
}

bool FilePanel::Impl_::AcceptPath() {
    // Set the result path. If the TextInputPane has a different path and it is
    // valid, use it. Otherwise, use the current path.
    result_path_ = path_list_->GetCurrent();
    const FilePath input_path = AddExtension_(input_pane_->GetText());
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

void FilePanel::Impl_::InitInterface(ContainerPane &root) {
    title_pane_         = root.FindTypedSubPane<TextPane>("Title");
    input_pane_         = root.FindTypedSubPane<TextInputPane>("Input");
    file_list_pane_     = root.FindTypedSubPane<ScrollingPane>("FileList");
    hidden_files_pane_  = root.FindTypedSubPane<CheckboxPane>("HiddenFiles");
    accept_button_pane_ = root.FindTypedSubPane<ButtonPane>("Accept");
    cancel_button_pane_ = root.FindTypedSubPane<ButtonPane>("Cancel");
    file_button_pane_   = root.FindTypedSubPane<ButtonPane>("FileButton");
    format_label_pane_  = root.FindSubPane("FormatLabel");
    format_pane_        = root.FindTypedSubPane<DropdownPane>("Format");

    // Access and set up the direction buttons.
    for (auto dir: Util::EnumValues<FilePathList::Direction>()) {
        const Str name = Util::EnumToWords(dir);
        auto but = root.FindTypedSubPane<ButtonPane>(name);
        dir_button_panes_[Util::EnumInt(dir)] = but;
    }

    // When the user toggles "Show Hidden Files", update the file list and
    // focused Pane.
    auto hf_func = [&](){
        UpdateFiles_();
        UpdateFocus_();
    };
    hidden_files_pane_->GetStateChanged().AddObserver(this, hf_func);

    // Remove the FileButton from the list of Panes so it does not show up.
    root.RemovePane(file_button_pane_);

    // Install a validation function for the TextInputPane.
    input_pane_->SetValidationFunc([&](const Str &path_string){
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
        const auto choices = Util::ConvertVector<Str, FileFormat>(
            file_formats_,
            [](const FileFormat &f){ return Util::EnumToWords(f); });
        format_pane_->SetChoices(choices, 0);
        format_label_pane_->SetEnabled(true);
        format_pane_->SetEnabled(true);
    }

    path_list_->Init(initial_path_);

    // Show the initial path to set up the file area.
    ShowPath_(initial_path_);

    UpdateButtons_(GetPathStatus_(path_list_->GetCurrent()));
}

void FilePanel::Impl_::SetFocus(const PanePtr &pane) {
    // Scroll to view a file button if necessary.
    if (pane && IsFileButton_(*pane))
        file_list_pane_->ScrollToShowSubPane(*pane);
}

FilePanel::Impl_::PathStatus_
FilePanel::Impl_::GetPathStatus_(const FilePath &path) const {
    const bool is_dir = path_list_->IsValidDirectory(path);
    PathStatus_ status;
    switch (target_type_) {
      case TargetType::kDirectory:
        status = is_dir ? PathStatus_::kAcceptable : PathStatus_::kInvalid;
        break;

      case TargetType::kExistingFile:
        status = path_list_->IsExistingFile(AddExtension_(path)) ?
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

void FilePanel::Impl_::ShowPath_(const FilePath &path) {
    ASSERT(path.IsAbsolute());

    // Update the string in the TextInputPane.
    Str path_string = path.ToString();
    if (path_list_->IsValidDirectory(path) &&
        ! path_string.ends_with(FilePath::GetSeparator()))
        path_string += FilePath::GetSeparator();
    input_pane_->SetInitialText(path_string);

    // Update the direction and result buttons.
    UpdateButtons_(GetPathStatus_(path));

    // Update the file buttons.
    UpdateFiles_();

    // Update the focus.
    UpdateFocus_();
}

void FilePanel::Impl_::UpdateButtons_(PathStatus_ path_status) {
    accept_button_pane_->GetButton().SetInteractionEnabled(
        path_status == PathStatus_::kAcceptable);

    for (auto dir: Util::EnumValues<FilePathList::Direction>()) {
        auto &but = dir_button_panes_[Util::EnumInt(dir)]->GetButton();
        but.SetInteractionEnabled(path_list_->CanGoInDirection(dir));
    }
}

void FilePanel::Impl_::UpdateFiles_() {
    // Get sorted lists of directories and files in the current directory.
    const bool include_hidden = hidden_files_pane_->GetState();
    StrVec subdirs;
    StrVec files;
    path_list_->GetContents(subdirs, files, extension_, include_hidden);

    // Ignore files if target must be a directory.
    if (target_type_ == TargetType::kDirectory)
        files.clear();

    // This will be set by CreateFileButton_() if there is a highlighted
    // button.
    highlighted_button_pane_.reset();

    // Create buttons for each directory and file.
    Pane::PaneVec buttons;
    for (size_t index = 0; const auto &name: subdirs)
        buttons.push_back(CreateFileButton_(index++, name, true));
    for (size_t index = 0; const auto &name: files)
        buttons.push_back(CreateFileButton_(index++, name, false));

    // Populate the scrolling file list with the buttons.
    ASSERT(file_list_pane_->GetContentsPane());
    file_list_pane_->GetContentsPane()->ReplacePanes(buttons);
}

PanePtr FilePanel::Impl_::CreateFileButton_(size_t index, const Str &name,
                                            bool is_dir) {
    const bool is_highlighted = MatchesPath_(name, highlight_path_);
    const Str  but_name = (is_dir ? "Dir_" : "File_") + Util::ToString(index);

    auto but = file_button_pane_->CloneTyped<ButtonPane>(true, but_name);
    auto text = but->FindTypedSubPane<TextPane>("ButtonText");
    text->SetText(is_highlighted ? name + highlight_annotation_ : name);

    const Str color_name = is_highlighted ? "FileHighlightColor" :
        is_dir ? "FileDirectoryColor" : "FileColor";
    text->SetColor(SG::ColorMap::SGetColor(color_name));

    // Set the ButtonPane to NOT accept a focus change on activation. If it
    // did, a mouse press on a file button would change the focus with a
    // potential scrolling change, which would be very annoying.
    but->SetShouldFocusOnActivation(false);

    auto click_func = [&, name](const ClickInfo &){
        ShowPath_(path_list_->AddPath(name));
    };
    but->GetButton().GetClicked().AddObserver(this, click_func);
    but->SetEnabled(true);

    if (is_highlighted)
        highlighted_button_pane_ = but;

    return but;
}

void FilePanel::Impl_::UpdateFocus_() {
    // Access the file buttons.
    const auto &buttons = file_list_pane_->GetContentsPane()->GetPanes();

    // Focus on one of these Panes (in this priority order):
    //   - The button for the current path (if there is one).
    //   - The button for the highlighted path (if there is one).
    //   - The Accept button (if enabled).
    //   - The Cancel button.
    const auto &cur_path = path_list_->GetCurrent();
    PanePtr pane_to_focus;
    bool is_file_button = false;
    for (const auto &but: buttons) {
        const auto &text =
            but->FindTypedSubPane<TextPane>("ButtonText")->GetText();
        if (MatchesPath_(text, cur_path)) {
            // Current path always takes priority.
            pane_to_focus = but;
            is_file_button = true;
            break;
        }
        else if (but == highlighted_button_pane_) {
            // If current path is not found, this is used.
            pane_to_focus = but;
        }
    }
    if (! pane_to_focus) {
        // Neither was found.
        if (! buttons.empty())
            pane_to_focus = buttons[0];
        else if (accept_button_pane_->GetButton().IsInteractionEnabled())
            pane_to_focus = accept_button_pane_;
        else
            pane_to_focus = cancel_button_pane_;
    }
    ASSERT(pane_to_focus);
    focus_func_(pane_to_focus);

    // If the focused Pane is a file button, make sure it is
    // visible. Otherwise, scroll to the top.
    if (is_file_button)
        file_list_pane_->ScrollToShowSubPane(*pane_to_focus);
    else
        file_list_pane_->ScrollTo(0);
}

bool FilePanel::Impl_::MatchesPath_(const Str &name,
                                    const FilePath &path) const {
    return path &&
        FilePath(name).AppendRelative(path_list_->GetCurrent()) == path;
}

bool FilePanel::Impl_::IsFileButton_(const Pane &pane) const {
    const auto &panes = file_list_pane_->GetContentsPane()->GetPanes();
    return std::find_if(panes.begin(), panes.end(),
                        [&](const PanePtr &p){ return p.get() == &pane; }) !=
        panes.end();
}

FilePath FilePanel::Impl_::AddExtension_(const FilePath &path) const {
    FilePath ext_path = path;
    if (! extension_.empty() && ext_path.GetExtension() != extension_)
        ext_path.AddExtension(extension_);
    return ext_path;
}

// ----------------------------------------------------------------------------
// FilePanel functions.
// ----------------------------------------------------------------------------

FilePanel::FilePanel() : impl_(new Impl_()) {
    Reset();

    // Allow the Impl_ to set the focused Pane, which requires calling the
    // protected SetFocus() function.
    auto focus_func = [&](const PanePtr &pane){
        UpdateFocusablePanes();
        SetFocus(pane);
    };
    impl_->SetFocusFunc(focus_func);
}

FilePanel::~FilePanel() {
}

// LCOV_EXCL_START [snaps only]
void FilePanel::SetFilePathList(FilePathList *list) {
    impl_->SetFilePathList(list);
}
// LCOV_EXCL_STOP

void FilePanel::Reset() {
    impl_->Reset();
}

bool FilePanel::HandleEvent(const Event &event) {
    return impl_->HandleEvent(event, GetFocusedPane()) ||
        ToolPanel::HandleEvent(event);
}

void FilePanel::UpdateFocus(const PanePtr &pane) {
    impl_->SetFocus(pane);
}

void FilePanel::InitInterface() {
    auto root = std::dynamic_pointer_cast<ContainerPane>(GetPane());
    ASSERT(root);
    impl_->InitInterface(*root);

    // The Impl_ class cannot call protected functions, so these need to be
    // done here.
    for (auto dir: Util::EnumValues<FilePathList::Direction>()) {
        const Str name = Util::EnumToWords(dir);
        AddButtonFunc(name, [this, dir](){ impl_->GoInDirection(dir); });
    }

    AddButtonFunc("Cancel", [this](){ ProcessResult("Cancel"); });
    AddButtonFunc("Accept", [this](){ TryAcceptPath_(); });
}

void FilePanel::UpdateInterface() {
    impl_->UpdateInterface();
}

void FilePanel::SetTitle(const Str &title) {
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

void FilePanel::SetExtension(const Str &extension) {
    impl_->SetExtension(extension);
}

void FilePanel::SetHighlightPath(const FilePath &path,
                                 const Str &annotation) {
    impl_->SetHighlightPath(path, annotation);
}

const FilePath & FilePanel::GetPath() const {
    return impl_->GetPath();
}

FileFormat FilePanel::GetFileFormat() const {
    return impl_->GetFileFormat();
}

void FilePanel::UpdateForPaneSizeChange() {
    impl_->SetFocus(GetFocusedPane());
}

void FilePanel::ProcessResult(const Str &result) {
    ReportChange(result, InteractionType::kImmediate);
    Close(result);
}

void FilePanel::TryAcceptPath_() {
    if (impl_->AcceptPath()) {
        ProcessResult("Accept");
    }
    else {
        // There is a conflict. Ask the user what to do.
        const Str msg = "File \"" + impl_->GetPath().ToString() +
            "\" exists.\nDo you want to overwrite it?";
        auto func = [&](const Str &answer){
            if (answer == "Yes")
                ProcessResult("Accept");
            // Otherwise, remain open for more interaction.
        };
        AskQuestion(msg, func, true);
    }
}
