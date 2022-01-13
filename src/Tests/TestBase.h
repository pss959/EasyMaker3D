#pragma once

#include <string>

#include "gtest/gtest.h"

#include "Math/Types.h"
#include "UnitConversion.h"
#include "Util/FilePath.h"

/// Base test fixture that supplies some useful functions for tests.
class TestBase : public ::testing::Test {
 protected:
    /// Class that creates a temporary file in the constructor and removes it
    /// in the destructor. Used for tests.
    class TempFile {
      public:
        /// Creates a temporary file with the given contents.
        TempFile(const std::string &input_string);

        /// Removes the temporary file.
        ~TempFile();

        /// Returns the path to the temporary file.
        const FilePath GetPath() { return path_; }

      private:
        FilePath path_;
    };

    // Close enough.
    static constexpr float kClose = 1e-4f;

    /// Returns a FilePath to the named test file (in the Data directory).
    FilePath GetDataPath(const std::string &file_name);

    // Creates and returns a default UnitConversion instance.
    static UnitConversionPtr GetDefaultUC();

    /// Reads the named test file and returns the contents as a string. Asserts
    /// if the file cannot be opened.
    std::string ReadDataFile(const std::string &file_name);

    // FP testing predicates from Ion.
    static bool VectorsClose(const Vector3f &v0, const Vector3f &v1);
    static bool PointsClose(const Point3f &p0, const Point3f &p1);

    // FP testing predicates from Ion with custom tolerance.
    static bool VectorsCloseT(const Vector3f &v0, const Vector3f &v1, float t);
    static bool PointsCloseT(const Point3f &p0, const Point3f &p1, float t);

    // Loads a TriMesh from an STL file.
    TriMesh LoadTriMesh(const std::string &file_name);

    // Validates a TriMesh.
    static void ValidateMesh(const TriMesh &mesh, string desc);

    // Fixes a string by removing line feeds. Needed for Windows.
    static std::string FixString(const std::string &s);
};
