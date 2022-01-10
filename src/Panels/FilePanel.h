#pragma once

#include <memory>
#include <string>

#include "Enums/FileFormat.h"
#include "Panels/MainPanel.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

/// FilePanel is a derived MainPanel class that allows the user to select a
/// file or directory.
class FilePanel : public MainPanel {
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

    /// Sets the title displayed in the panel. The default is "Select a File".
    void SetTitle(const std::string &title);

    /// Sets the type of file target. The default is TargetType::kDirectory.
    void SetTargetType(TargetType type);

    /// Sets the initial path to display in the browser. This defaults to the
    /// user's home directory.
    void SetInitialPath(const FilePath &path);

    /// Enables or disables the file format dropdown.
    void SetFileFormatsEnabled(bool enabled);

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

    /// Returns the selected file format. This returns FileFormat::kUnknown if
    /// the file format dropdown was not enabled or if the panel was canceled.
    FileFormat GetFileFormat() const;

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

    /// This cannot be in Impl_ because it requires access to Panel functions.
    void TryAcceptPath_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<FilePanel> FilePanelPtr;
