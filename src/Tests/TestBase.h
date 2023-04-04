#pragma once

#include <string>

#include "gtest/gtest.h"

#include "Items/UnitConversion.h"
#include "Math/Types.h"
#include "Parser/Registry.h"
#include "Util/FilePath.h"
#include "Util/General.h"

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

        /// Returns the contents of the file as a string.
        std::string GetContents();

      private:
        FilePath path_;
    };

    /// Class that temporarily disables the Util::is_in_unit_test flag for a
    /// test that needs to pretend it is not a test. The flag is disabled while
    /// this is in scope.
    class UnitTestFlagDisabler {
      public:
        UnitTestFlagDisabler()  { Util::is_in_unit_test = false; }
        ~UnitTestFlagDisabler() { Util::is_in_unit_test = true; }
    };

    // Close enough.
    static constexpr float kClose = 1e-4f;

    /// Returns the name of the current test case.
    std::string GetTestCaseName() const {
        const auto info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        return info->test_case_name();
    }

    /// Returns the name of the current test.
    std::string GetTestName() const {
        const auto info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        return info->name();
    }

    // Convenience to create an Object of the templated type.
    template <typename T>
    static std::shared_ptr<T> CreateObject(const std::string &name = "") {
        return Parser::Registry::CreateObject<T>(name);
    }

    /// Returns a FilePath to the named test file (in the Data directory).
    FilePath GetDataPath(const std::string &file_name);

    // Creates and returns a default UnitConversion instance.
    static UnitConversionPtr GetDefaultUC();

    /// Reads the named test file and returns the contents as a string. Asserts
    /// if the file cannot be opened. If add_data_extension is true, this adds
    /// the default data file extension.
    std::string ReadDataFile(const std::string &file_name,
                             bool add_data_extension = true);

    // FP testing predicates from Ion.
    static bool VectorsClose2(const Vector2f &v0, const Vector2f &v1);
    static bool PointsClose2(const Point2f &p0, const Point2f &p1);
    static bool VectorsClose(const Vector3f &v0, const Vector3f &v1);
    static bool PointsClose(const Point3f &p0, const Point3f &p1);
    static bool RotationsClose(const Rotationf &r0, const Rotationf &r1);

    // FP testing predicates from Ion with custom tolerance.
    static bool VectorsClose2T(const Vector2f &v0, const Vector2f &v1, float t);
    static bool PointsClose2T(const Point2f &p0, const Point2f &p1, float t);
    static bool VectorsCloseT(const Vector3f &v0, const Vector3f &v1, float t);
    static bool PointsCloseT(const Point3f &p0, const Point3f &p1, float t);
    static bool RotationsCloseT(const Rotationf &r0, const Rotationf &r1,
                                float t_degrees);

    // Loads a TriMesh from an STL file.
    TriMesh LoadTriMesh(const std::string &file_name);

    // Validates a TriMesh.
    static void ValidateMesh(const TriMesh &mesh, string desc);

    // Fixes a string by removing line feeds. Needed for Windows.
    static std::string FixString(const std::string &s);

    // Compares two strings for equality with verbose results on failure.
    static bool CompareStrings(const std::string &expected,
                               const std::string &actual);
};
