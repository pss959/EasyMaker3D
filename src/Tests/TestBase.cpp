#include "Tests/TestBase.h"

#include <algorithm>
#include <cstddef>
#include <fstream>

#include <ion/math/matrixutils.h>
#include <ion/math/vectorutils.h>

#include "IO/STLReader.h"
#include "Math/MeshValidation.h"
#include "Math/TextUtils.h"
#include "Tests/Testing.h"
#include "Tests/Util/FakeFileSystem.h"
#include "Tests/Util/FakeFontSystem.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/Read.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// Constructor, destructor, and setup.
// ----------------------------------------------------------------------------

TestBase::TestBase() {
    // Install a FakeFileSystem and FakeFontSystem. These can be changed later
    // if necessary.
    fake_file_system_.reset(new FakeFileSystem);
    FileSystem::Install(fake_file_system_);

    fake_font_system_.reset(new FakeFontSystem);
    FontSystem::Install(fake_font_system_);
}

TestBase::~TestBase() {}

void TestBase::UseRealFileSystem(bool b) {
    FileSystem::Install(b ? FileSystemPtr(new FileSystem) : fake_file_system_);
}

void TestBase::UseRealFontSystem(bool b) {
    FontSystem::Install(b ? FontSystemPtr(new FontSystem) : fake_font_system_);
}

std::shared_ptr<FakeFileSystem> TestBase::GetFakeFileSystem() {
    auto fs = std::dynamic_pointer_cast<FakeFileSystem>(
        FileSystem::GetInstalled());
    ASSERTM(fs, "FakeFileSystem is not installed");
    return fs;
}

std::shared_ptr<FakeFontSystem> TestBase::GetFakeFontSystem() {
    auto fs = std::dynamic_pointer_cast<FakeFontSystem>(
        FontSystem::GetInstalled());
    ASSERTM(fs, "FakeFontSystem is not installed");
    return fs;
}

// ----------------------------------------------------------------------------
// Test introspection.
// ----------------------------------------------------------------------------

Str TestBase::GetTestCaseName() const {
    const auto info = ::testing::UnitTest::GetInstance()->current_test_info();
    return info->test_case_name();
}

Str TestBase::GetTestName() const {
    const auto info = ::testing::UnitTest::GetInstance()->current_test_info();
    return info->name();
}

// ----------------------------------------------------------------------------
// Finding and reading files.
// ----------------------------------------------------------------------------

FilePath TestBase::GetDataPath(const Str &file_name) {
    return FilePath::Join(FilePath::GetTestDataPath(), FilePath(file_name));
}

Str TestBase::ReadDataFile(const Str &file_name) {
    Str s;
    const FilePath path = GetDataPath(file_name);
    EXPECT_TRUE(Util::ReadFile(path, s)) << "Path: " << path.ToString();
    return s;
}

Str TestBase::ReadResourceFile(const Str &file_path) {
    Str s;
    const FilePath path = FilePath::Join(FilePath::GetResourceBasePath(),
                                         FilePath(file_path));
    EXPECT_TRUE(Util::ReadFile(path, s)) << "Path: " << path.ToString();
    return s;
}

// ----------------------------------------------------------------------------
// Comparison helpers.
// ----------------------------------------------------------------------------

bool TestBase::VectorsClose2(const Vector2f &v0, const Vector2f &v1) {
    return ion::math::VectorsAlmostEqual(v0, v1, kClose);
}

bool TestBase::PointsClose2(const Point2f &p0, const Point2f &p1) {
    return ion::math::PointsAlmostEqual(p0, p1, kClose);
}

bool TestBase::VectorsClose(const Vector3f &v0, const Vector3f &v1) {
    return ion::math::VectorsAlmostEqual(v0, v1, kClose);
}

bool TestBase::PointsClose(const Point3f &p0, const Point3f &p1) {
    return ion::math::PointsAlmostEqual(p0, p1, kClose);
}

bool TestBase::RotationsClose(const Rotationf &r0, const Rotationf &r1) {
    return RotationsCloseT(r0, r1, .01f);
}

bool TestBase::MatricesClose(const Matrix4f &m0, const Matrix4f &m1) {
    return MatricesCloseT(m0, m1, kClose);
}

bool TestBase::VectorsClose2T(const Vector2f &v0, const Vector2f &v1, float t) {
    return ion::math::VectorsAlmostEqual(v0, v1, t);
}

bool TestBase::PointsClose2T(const Point2f &p0, const Point2f &p1, float t) {
    return ion::math::PointsAlmostEqual(p0, p1, t);
}

bool TestBase::VectorsCloseT(const Vector3f &v0, const Vector3f &v1, float t) {
    return ion::math::VectorsAlmostEqual(v0, v1, t);
}

bool TestBase::PointsCloseT(const Point3f &p0, const Point3f &p1, float t) {
    return ion::math::PointsAlmostEqual(p0, p1, t);
}

bool TestBase::RotationsCloseT(const Rotationf &r0, const Rotationf &r1,
                               float t_degrees) {
    Vector3f v0, v1;
    Anglef   a0, a1;
    r0.GetAxisAndAngle(&v0, &a0);
    r1.GetAxisAndAngle(&v1, &a1);
    return VectorsClose(v0, v1) &&
        ion::math::AlmostEqual(a0, a1, Anglef::FromDegrees(t_degrees));
}

bool TestBase::MatricesCloseT(const Matrix4f &m0, const Matrix4f &m1, float t) {
    return ion::math::MatricesAlmostEqual(m0, m1, t);
}

bool TestBase::CompareStrings(const Str &expected, const Str &actual) {
    size_t index;
    if (! Util::CompareStrings(actual, expected, index)) {
        // Outputs a character that causes strings to be different.
        auto output_char = [](char c){
            std::cerr << "'" << c << "' == 0x"
                      << std::hex << static_cast<int>(c) << std::dec;
        };

        if (index < actual.size() && index < expected.size()) {
            EXPECT_NE(actual[index], expected[index]);
        }
        std::cerr << "*** Strings differ at index " << index << "\n";
        std::cerr << "*** Actual:\n"   << actual << "\n";
        std::cerr << "*** Expected:\n" << expected << "\n";
        std::cerr << "***   (";
        if (index < expected.size())
            output_char(expected[index]);
        else
            std::cerr << "EOF";
        std::cerr << ") vs. (";
        if (index < actual.size())
            output_char(actual[index]);
        else
            std::cerr << "EOF";
        std::cerr << ")\n";
        /* Uncomment for extra help.
           std::cerr << "*** 1-line Expected:" <<
           Util::ReplaceString(expected, "\n", "|") << "\n";
           std::cerr << "*** 1-line Actual:  " <<
           Util::ReplaceString(actual,   "\n", "|") << "\n";
        */
        return false;
    }
    return true;
}

bool TestBase::CompareData(const void *expected, size_t size,
                           const void *actual) {
    // Compare 1 byte at a time.
    const auto e = reinterpret_cast<const std::byte *>(expected);
    const auto a = reinterpret_cast<const std::byte *>(actual);

    for (size_t i = 0; i < size; ++i) {
        if (a[i] != e[i]) {
            std::cerr << "*** Data buffers differ at byte " << i
                      << ": expected 0x" << std::hex << static_cast<int>(e[i])
                      << ", got 0x" << std::hex << static_cast<int>(a[i])
                      << std::dec << "\n";
            return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// TriMesh helpers.
// ----------------------------------------------------------------------------

TriMesh TestBase::LoadTriMesh(const Str &file_name) {
    const FilePath path = GetDataPath(file_name);
    Str error;
    TriMesh mesh = ReadSTLFile(path, 1, error);
    ASSERTM(! mesh.points.empty(),
            "Loaded from " + path.ToString() + ": " + error);
    ValidateMesh(mesh, "Imported from '" + path.ToString() + "'");
    return mesh;
}

void TestBase::ValidateMesh(const TriMesh &mesh, string desc) {
    const MeshValidityCode ret = ValidateTriMesh(mesh);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ret)
        << "Invalid " << desc << " mesh: " << Util::EnumName(ret);
}

bool TestBase::MeshHasPoint(const TriMesh &mesh, const Point3f &p) {
    const auto is_close =
        [&p](const Point3f &mp){ return PointsClose(p, mp); };
    return std::any_of(mesh.points.begin(), mesh.points.end(), is_close);
}

// ----------------------------------------------------------------------------
// Other conveniences.
// ----------------------------------------------------------------------------

Rotationf TestBase::BuildRotation(const Vector3f &axis, float deg) {
    return Rotationf::FromAxisAndAngle(axis, Anglef::FromDegrees(deg));
}

Rotationf TestBase::BuildRotation(float x, float y, float z, float deg) {
    return BuildRotation(Vector3f(x, y, z), deg);
}

Str TestBase::FixString(const Str &s) {
    const char lf = '\r';
    Str fs = s;
    fs.erase(std::remove(fs.begin(), fs.end(), lf), fs.end());
    return fs;
}
