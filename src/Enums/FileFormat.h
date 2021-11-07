#pragma once

/// Supported file formats for Model export.
enum class FileFormat {
    kUnknown,    ///< Used to indicate that import could not be done.
    kTextSTL,    ///< Text (ASCII) STL.
    kBinarySTL,  ///< Binary STL.
};
