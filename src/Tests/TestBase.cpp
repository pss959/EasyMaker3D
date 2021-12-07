#include "TestBase.h"

#include <algorithm>
#include <fstream>
#include <random>

#include <ion/math/vectorutils.h>

#include "IO/STLReader.h"
#include "Math/MeshValidation.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/Read.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

// Returns a string of random characters of the given length. This is used
// because std::filesystem has no current way of generating a unique file name,
// and I don't want to depend on boost::filesystem. This was stolen from the
// internet.
static std::string RandomString_(size_t length) {
    auto random_char = []() -> char {
        const char alphanumeric[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
            ;
        return alphanumeric[rand() % (sizeof(alphanumeric) - 1)];
    };
    std::string str(length, '\0');
    std::generate_n(str.begin(), length, random_char);
    return str;
}

// Returns a unique file name for temporary files of the form 'NNNN-NNNN-NNNN'.
static std::string GetTempFileName_() {
    return RandomString_(4) + "-" + RandomString_(4) + "-" + RandomString_(4);
}

// ----------------------------------------------------------------------------
// TestBase::TempFile implementation.
// ----------------------------------------------------------------------------

TestBase::TempFile::TempFile(const std::string &input_string) {
    path_ = Util::FilePath::Join(
        Util::FilePath::GetTempFilePath(), Util::FilePath(GetTempFileName_()));

    std::ofstream out(path_.ToNativeString());
    ASSERT(out.is_open());
    out << input_string;
    out.close();
}

TestBase::TempFile::~TempFile() {
    path_.Remove();
}

// ----------------------------------------------------------------------------
// TestBase implementation.
// ----------------------------------------------------------------------------

Util::FilePath TestBase::GetDataPath(const std::string &file_name) {
    return Util::FilePath::Join(Util::FilePath::GetTestDataPath(),
                                Util::FilePath(file_name));
}

std::string TestBase::ReadDataFile(const std::string &file_name) {
    std::string s;
    EXPECT_TRUE(Util::ReadFile(GetDataPath(file_name), s));
    return s;
}

bool TestBase::VectorsClose(const Vector3f &v0, const Vector3f &v1) {
    return ion::math::VectorsAlmostEqual(v0, v1, kClose);
}

bool TestBase::PointsClose(const Point3f &p0, const Point3f &p1) {
    return ion::math::PointsAlmostEqual(p0, p1, kClose);
}

bool TestBase::VectorsCloseT(const Vector3f &v0, const Vector3f &v1, float t) {
    return ion::math::VectorsAlmostEqual(v0, v1, t);
}

bool TestBase::PointsCloseT(const Point3f &p0, const Point3f &p1, float t) {
    return ion::math::PointsAlmostEqual(p0, p1, t);
}

TriMesh TestBase::LoadTriMesh(const std::string &file_name) {
    const Util::FilePath path = GetDataPath(file_name);
    std::string error;
    TriMesh mesh = ReadSTLFile(path, UnitConversion(), error);
    ASSERTM(! mesh.points.empty(),
            "Loaded from " + path.ToString() + ": " + error);
    ValidateMesh(mesh, "Imported from '" + path.ToString() + "'");
    return mesh;
}

void TestBase::ValidateMesh(const TriMesh &mesh, string desc) {
    const MeshValidityCode ret = IsMeshValid(mesh);
    EXPECT_TRUE(ret == MeshValidityCode::kValid)
        << "Invalid " << desc << " mesh: " << Util::EnumName(ret);
}

std::string TestBase::FixString(const std::string &s) {
    const char lf = '\r';
    std::string fs = s;
    fs.erase(std::remove(fs.begin(), fs.end(), lf), fs.end());
    return fs;
}
