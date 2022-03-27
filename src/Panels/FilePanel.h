#pragma once

#include <string>
#include <vector>

#include "Memory.h"
#include "Panels/ToolPanel.h"
#include "Util/FilePath.h"

DECL_SHARED_PTR(FilePanel);

namespace Parser { class Registry; }

/// FilePanel is a derived ToolPanel class that allows the user to select a
/// file or directory.
class FilePanel : public ToolPanel {
  public:
    /// Types of target.
    enum class TargetType {
        kDirectory,      /// Only directories are available.
        kExistingFile,   /// Must be a file that already exists.
        kNewFile,        /// May be an existing file or a new file.
    };

    /// Resets all user-defined options and the results to their default
    /// values. This should be called if the caller is not sure of the current
    /// state of the FilePanel.
    void Reset();

    /// Sets a flag indicating a valid response result ("Accept" or "Cancel")
    /// should also close the FilePanel. The default is true.
    void SetResponseShouldClose(bool b) { response_should_close_ = b; }

    /// Sets the title displayed in the panel. The default is "Select a File".
    void SetTitle(const std::string &title);

    /// Sets the type of file target. The default is TargetType::kDirectory.
    void SetTargetType(TargetType type);

    /// Sets the initial path to display in the browser. This defaults to the
    /// user's home directory.
    void SetInitialPath(const FilePath &path);

    /// Sets the file formats to display in the file format dropdown. If the
    /// vector is empty, this disables the dropdown.
    void SetFileFormats(const std::vector<std::string> &formats);

    /// Sets the file extension that files must match to be shown in the
    /// browser and that will be appended to user-entered names if necessary.
    /// An empty string (the default) removes any name restriction.
    void SetExtension(const std::string &extension);

    /// Indicates that the given path is to be highlighted with red text and
    /// the given annotation string if it appears in the directory/file
    /// list. The annotation will appear after the special path. The default is
    /// empty for both, which turns off highlighting.
    void SetHighlightPath(const FilePath &path, const std::string &annotation);

    /// Accesses the resulting file path. This is empty if no file was selected
    /// or if the panel was canceled.
    const FilePath & GetPath() const;

    /// Returns the selected file format. This returns an empty string if no
    /// formats were passed to SetFileFormats() or if the panel was canceled.
    const std::string & GetFileFormat() const;

    /// Redefines this to also handle scrolling of file buttons.
    virtual bool HandleEvent(const Event &event) override;

  protected:
    FilePanel();

    virtual void InitInterface()   override;
    virtual void UpdateInterface() override;

  private:
    class PathList_;
    class Impl_;
    std::unique_ptr<Impl_> impl_;  /// Implementation instance.

    bool response_should_close_ = true;

    /// This cannot be in Impl_ because it requires access to Panel functions.
    void TryAcceptPath_();

    /// Calls ReportChange() with the given result. If response_should_close_
    /// is true, this also closes the FilePanel with the result.
    void ProcessResult_(const std::string &result);

    friend class Parser::Registry;
};
