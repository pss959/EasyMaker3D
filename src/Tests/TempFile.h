#pragma once

#include <string>

#include "Util/FilePath.h"

/// Class that creates a temporary file in the constructor and removes it in
/// the destructor.
/// \ingroup Tests
class TempFile {
  public:
    /// Creates a temporary file with the given contents.
    TempFile(const Str &input_string);

    /// Removes the temporary file.
    ~TempFile();

    /// Returns the path to the temporary file.
    const FilePath GetPath() { return path_; }

    /// Returns the contents of the file as a string.
    Str GetContents() const;

    /// Modifies the contents.
    void SetContents(const Str &new_contents);

  private:
    FilePath path_;

    static Str RandomString_(size_t length);
};

