#pragma once

#include <string>
#include <vector>

#include "Enums/FileFormat.h"
#include "Panels/ToolPanel.h"
#include "Util/FilePath.h"
#include "Util/Memory.h"

class FilePathList;
DECL_SHARED_PTR(FilePanel);

namespace Parser { class Registry; }

/// FilePanel is a derived ToolPanel class that allows the user to select a
/// file or directory.
///
/// ReportChange() keys: "Accept", "Cancel" (both immediate).
///
/// \ingroup Panels
class FilePanel : public ToolPanel {
  public:
    /// Types of target.
    enum class TargetType {
        kDirectory,      /// Only directories are available.
        kExistingFile,   /// Must be a file that already exists.
        kNewFile,        /// May be an existing file or a new file.
    };

    virtual ~FilePanel();

    /// Sets a FilePathList instance to use instead of a default FilePathList.
    /// This can be called to use a mock version to simulate a file system.
    void SetFilePathList(FilePathList *list);

    /// Resets all user-defined options and the results to their default
    /// values. This should be called if the caller is not sure of the current
    /// state of the FilePanel.
    void Reset();

    /// Sets the title displayed in the panel. The default is "Select a File".
    void SetTitle(const Str &title);

    /// Sets the type of file target. The default is TargetType::kDirectory.
    void SetTargetType(TargetType type);

    /// Sets the initial path to display in the browser. This defaults to the
    /// user's home directory.
    void SetInitialPath(const FilePath &path);

    /// Sets the file formats to display in the file format dropdown as a
    /// vector containing a subset of the FileFormat enum values. If the vector
    /// is empty, this disables the dropdown. It is disabled by default.
    void SetFileFormats(const std::vector<FileFormat> &formats);

    /// Sets the file extension that files must match to be shown in the
    /// browser and that will be appended to user-entered names if necessary.
    /// An empty string (the default) removes any name restriction.
    void SetExtension(const Str &extension);

    /// Indicates that the given path is to be highlighted with red text and
    /// the given annotation string if it appears in the directory/file
    /// list. The annotation will appear after the special path. The default is
    /// empty for both, which turns off highlighting.
    void SetHighlightPath(const FilePath &path, const Str &annotation);

    /// Accesses the resulting file path. This is empty if no file was selected
    /// or if the panel was canceled.
    const FilePath & GetPath() const;

    /// Returns the selected file format. This returns FileFormat::kUnknown if
    /// no formats were passed to SetFileFormats() or if the panel was
    /// canceled.
    FileFormat GetFileFormat() const;

    /// Redefines this to also handle scrolling of file buttons.
    virtual bool HandleEvent(const Event &event) override;

    /// Redefines this to make sure the focused Pane is in view (if a file
    /// button).
    virtual void UpdateFocus(const PanePtr &pane) override;

    /// Redefines this to return true.
    virtual bool IsCloseable() const override { return true; }

  protected:
    FilePanel();

    virtual void InitInterface()   override;
    virtual void UpdateInterface() override;

    /// Redefines this to scroll to the focused pane, if any.
    virtual void UpdateForPaneSizeChange();

    /// This is called when a response button is clicked. The base class
    /// defines this to Call ReportChange() with the given result and close the
    /// FilePanel with the result.
    virtual void ProcessResult(const Str &result);

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;  ///< Implementation instance.

    /// This cannot be in Impl_ because it requires access to Panel functions.
    void TryAcceptPath_();

    friend class Parser::Registry;
};
