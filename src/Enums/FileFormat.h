//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

/// Supported file formats for Model export.
///
/// \ingroup Enums
enum class FileFormat {
    kUnknown,    ///< Used to indicate that import could not be done.
    kTextSTL,    ///< Text (ASCII) STL.
    kBinarySTL,  ///< Binary STL.
    kOFF,        ///< OFF file.
};

/// Convenience that returns the file extension for the given format. Returns
/// an empty string for an unknown format.
inline Str GetFileFormatExtension(FileFormat format) {
    switch (format) {
      case FileFormat::kTextSTL:
      case FileFormat::kBinarySTL:
        return ".stl";
      case FileFormat::kOFF:
        return ".off";
      default:
        return "";
    }
}
