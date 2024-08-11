//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Util/FilePath.h"
namespace Parser { class Object; }

/// Class that creates a temporary file in the constructor and removes it in
/// the destructor.
/// \ingroup Tests
class TempFile {
  public:
    /// Creates a temporary file with the given contents.
    explicit TempFile(const Str &input_string);

    /// Creates a temporary file containing the result of using a
    /// Parser::Writer to write the given object.
    explicit TempFile(const Parser::Object &obj);

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

    static FilePath GetRandomFilePath_();
    static Str      RandomString_(size_t length);
};

