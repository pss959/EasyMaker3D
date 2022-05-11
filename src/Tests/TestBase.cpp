#include "TestBase.h"

#include <algorithm>
#include <fstream>
#include <random>

#include <ion/math/vectorutils.h>

#include "IO/STLReader.h"
#include "Math/MeshValidation.h"
#include "RegisterTypes.h"
#include "Testing.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/Read.h"
#include "Util/String.h"

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
    path_ = FilePath::Join(FilePath::GetTempFilePath(),
                           FilePath(GetTempFileName_()));

    if (! input_string.empty()) {
        std::ofstream out(path_.ToNativeString());
        ASSERT(out.is_open());
        out << input_string;
        out.close();
    }
}

TestBase::TempFile::~TempFile() {
    path_.Remove();
}

std::string TestBase::TempFile::GetContents() {
    std::string s;
    EXPECT_TRUE(Util::ReadFile(path_, s));
    return s;
}

// ----------------------------------------------------------------------------
// TestBase implementation.
// ----------------------------------------------------------------------------

FilePath TestBase::GetDataPath(const std::string &file_name) {
    return FilePath::Join(FilePath::GetTestDataPath(), FilePath(file_name));
}

UnitConversionPtr TestBase::GetDefaultUC() {
    RegisterTypes();
    return CreateObject<UnitConversion>();
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

bool TestBase::RotationsClose(const Rotationf &r0, const Rotationf &r1) {
    return RotationsCloseT(r0, r1, .01f);
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

TriMesh TestBase::LoadTriMesh(const std::string &file_name) {
    const FilePath path = GetDataPath(file_name);
    std::string error;
    TriMesh mesh = ReadSTLFile(path, *GetDefaultUC(), error);
    ASSERTM(! mesh.points.empty(),
            "Loaded from " + path.ToString() + ": " + error);
    ValidateMesh(mesh, "Imported from '" + path.ToString() + "'");
    return mesh;
}

void TestBase::ValidateMesh(const TriMesh &mesh, string desc) {
    const MeshValidityCode ret = IsMeshValid(mesh);
    EXPECT_ENUM_EQ(MeshValidityCode::kValid, ret)
        << "Invalid " << desc << " mesh: " << Util::EnumName(ret);
}

std::string TestBase::FixString(const std::string &s) {
    const char lf = '\r';
    std::string fs = s;
    fs.erase(std::remove(fs.begin(), fs.end(), lf), fs.end());
    return fs;
}

bool TestBase::CompareStrings(const std::string &expected,
                              const std::string &actual) {
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
