//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <memory>

#include "gtest/gtest.h"

#include "Math/Types.h"
#include "Util/FilePath.h"

class FakeFileSystem;
class FakeFontSystem;
struct TriMesh;

/// Base test fixture that supplies some useful functions for tests.
///
/// \ingroup Tests
class TestBase : public ::testing::Test {
 protected:
    // ------------------------------------------------------------------------
    // Constructor, destructor, and setup.
    // ------------------------------------------------------------------------

    /// The constructor installs a FakeFileSystem and FakeFontSystem for tests
    /// to use.
    TestBase();

    /// The destructor restores the real FileSystem and FontSystem instances.
    virtual ~TestBase();

    /// If the flag is true, this installs a real FileSystem; otherwise, it
    /// restores the FakeFileSystem. Note that using a real FileSystem makes
    /// tests sensitive to actual file and directory conditions, which is not
    /// ideal in most cases.
    void UseRealFileSystem(bool b);

    /// If the flag is true, this installs a real FontSystem; otherwise, it
    /// restores the FakeFontSystem. Note that using a real FontSystem can slow
    /// things down considerably and should be used only when actual font
    /// images or outlines are required.
    void UseRealFontSystem(bool b);

    /// Convenience that returns the FakeFileSystem when it is in use. Asserts
    /// if it is not.
    std::shared_ptr<FakeFileSystem> GetFakeFileSystem();

    /// Convenience that returns the FakeFontSystem when it is in use. Asserts
    /// if it is not.
    std::shared_ptr<FakeFontSystem> GetFakeFontSystem();

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

    /// Returns a Normalized Vector3f.
    static Vector3f Normalized(const Vector3f &v);
    /// Returns a Normalized Vector3f from components.
    static Vector3f Normalized(float x, float y, float z);

    /// Returns a Rotationf from axis and angle (degree) values.
    static Rotationf BuildRotation(const Vector3f &axis, float deg);
    /// Returns a Rotationf from axis components and angle (degree) values.
    static Rotationf BuildRotation(float x, float y, float z, float deg);

    /// Fixes a string by removing line feeds. Needed for Windows. Also strips
    /// comments (for files with license comments).
    static Str FixString(const Str &s);

    /// Strips comments from a string (to help with files that have license
    /// comments). Comments start with the \p start string.
    static Str RemoveComments(const Str &s, const Str &start);

  private:
    /// Stores a FakeFileSystem when needed.
    std::shared_ptr<FakeFileSystem> fake_file_system_;

    /// Stores a FakeFontSystem when needed.
    std::shared_ptr<FakeFontSystem> fake_font_system_;
};
