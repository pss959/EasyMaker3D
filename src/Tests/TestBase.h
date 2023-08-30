#pragma once

#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "Math/Types.h"
#include "Util/FilePath.h"
#include "Util/Flags.h"

class FakeFileSystem;
class FakeFontSystem;

/// Base test fixture that supplies some useful functions for tests.
/// \ingroup Tests
class TestBase : public ::testing::Test {
 protected:
    /// Flags passed to the constructor.
    enum class Flag : uint32_t {
        /// Use the real FileSystem instead of a FakeFileSystem for handling
        /// files. Note that this makes tests sensitive to actual file and
        /// directory conditions, which is not ideal.
        kUseRealFiles = 0x1,

        /// Use the real FontSystem instead of a FakeFontSystem for handling
        /// fonts. Note that this can slow things down a lot and should be used
        /// only when actual font images are required.
        kUseRealFonts = 0x2,
    };

    // ------------------------------------------------------------------------
    // Constructor and destructor.
    // ------------------------------------------------------------------------

    /// The constructor installs a FakeFileSystem and FakeFontSystem for tests
    /// to use unless flags are set.
    TestBase(const Util::Flags<Flag> &flags = Util::Flags<Flag>());

    /// The destructor restores the real FileSystem and FontSystem instances.
    virtual ~TestBase();

    // ------------------------------------------------------------------------
    // Test introspection.
    // ------------------------------------------------------------------------

    /// Returns the name of the current test case.
    Str GetTestCaseName() const;

    /// Returns the name of the current test.
    Str GetTestName() const;

    // ------------------------------------------------------------------------
    // Finding and reading files.
    // ------------------------------------------------------------------------

    /// Returns a FilePath to the named file below the Tests/Data directory.
    static FilePath GetDataPath(const Str &file_name);

    /// Reads the named file from the Tests/Data directory and returns the
    /// contents as a string. Asserts if the file cannot be opened.
    static Str ReadDataFile(const Str &file_name);

    /// Reads the file on the given path from the resources directory and
    /// returns the contents as a string. Asserts if the file cannot be opened.
    static Str ReadResourceFile(const Str &file_path);

    // ------------------------------------------------------------------------
    // Comparison helpers.
    // ------------------------------------------------------------------------

    /// Close enough for most floating-point comparisons.
    static constexpr float kClose = 1e-4f;

    /// FP testing predicates from Ion.
    static bool VectorsClose2(const Vector2f &v0, const Vector2f &v1);
    static bool PointsClose2(const Point2f &p0, const Point2f &p1);
    static bool VectorsClose(const Vector3f &v0, const Vector3f &v1);
    static bool PointsClose(const Point3f &p0, const Point3f &p1);
    static bool RotationsClose(const Rotationf &r0, const Rotationf &r1);
    static bool MatricesClose(const Matrix4f &m0, const Matrix4f &m1);

    /// FP testing predicates from Ion with custom tolerance.
    static bool VectorsClose2T(const Vector2f &v0, const Vector2f &v1, float t);
    static bool PointsClose2T(const Point2f &p0, const Point2f &p1, float t);
    static bool VectorsCloseT(const Vector3f &v0, const Vector3f &v1, float t);
    static bool PointsCloseT(const Point3f &p0, const Point3f &p1, float t);
    static bool RotationsCloseT(const Rotationf &r0, const Rotationf &r1,
                                float t_degrees);
    static bool MatricesCloseT(const Matrix4f &m0, const Matrix4f &m1, float t);

    /// Compares two strings for equality with verbose results on failure.
    static bool CompareStrings(const Str &expected, const Str &actual);

    /// Compares two data buffers for equality with verbose results on
    /// failure. The buffers are assumed to have the same size.
    static bool CompareData(const void *expected, size_t size,
                            const void *actual);

    // ------------------------------------------------------------------------
    // TriMesh helpers.
    // ------------------------------------------------------------------------

    /// Loads a TriMesh from an STL file.
    static TriMesh LoadTriMesh(const Str &file_name);

    /// Returns true if the given TriMesh contains a point close to the given
    /// one.
    static bool MeshHasPoint(const TriMesh &mesh, const Point3f &p);

    /// Validates a TriMesh.
    static void ValidateMesh(const TriMesh &mesh, string desc);

    // ------------------------------------------------------------------------
    // Other conveniences.
    // ------------------------------------------------------------------------

    /// Returns a Rotationf from axis and angle (degree) values.
    static Rotationf BuildRotation(const Vector3f &axis, float deg);
    static Rotationf BuildRotation(float x, float y, float z, float deg);

    /// Fixes a string by removing line feeds. Needed for Windows.
    static Str FixString(const Str &s);

  private:
    /// Stores a FakeFileSystem when needed.
    std::shared_ptr<FakeFileSystem> fake_file_system_;

    /// Stores a FakeFontSystem when needed.
    std::shared_ptr<FakeFontSystem> fake_font_system_;
};
