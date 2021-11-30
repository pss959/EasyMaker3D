#pragma once

#include <fstream>
#include <string>

#include "gtest/gtest.h"

#include <boost/filesystem.hpp>

#include <ion/math/vectorutils.h>

#include "IO/STLReader.h"
#include "Math/CGALInterface.h"
#include "Math/Types.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/FilePath.h"
#include "Util/Read.h"

// ----------------------------------------------------------------------------
// Handy macros.
// ----------------------------------------------------------------------------

/// Macro for testing against null for clarity.
#define EXPECT_NULL(PTR)     EXPECT_TRUE((PTR) == nullptr)

/// Macro for testing against null for clarity.
#define EXPECT_NOT_NULL(PTR) EXPECT_TRUE((PTR) != nullptr)

/// Macro for testing near-equality for vectors.
#define EXPECT_VECS_CLOSE(EXP, V) EXPECT_PRED2(VectorsClose, EXP, V)

/// Macro for testing near-equality for points.
#define EXPECT_PTS_CLOSE(EXP, P) EXPECT_PRED2(PointsClose, EXP, P)

/// Macro for testing Parser::Object instances for equality.
#define EXPECT_EQ_OBJS(O1, O2) EXPECT_EQ(O1.get(), O2.get()) \
    << "Objects differ: " << O1->GetDesc() << " vs. " << O2->GetDesc()

/// Tests that an exception of the given type is thrown by the given statement
/// and that the exception's message contains the given string pattern.
#define TEST_THROW(STMT, EXCEPTION_TYPE, PATTERN)                             \
    EXPECT_THROW({                                                            \
            try {                                                             \
                STMT;                                                         \
            }                                                                 \
            catch (const EXCEPTION_TYPE &ex) {                                \
                EXPECT_TRUE(std::string(ex.what()).find(PATTERN) !=           \
                            std::string::npos) <<                             \
                    "Exception string:" << ex.what()                          \
                                        << "\nvs. pattern: " << (PATTERN);    \
                throw;                                                        \
            }                                                                 \
        }, EXCEPTION_TYPE)

// ----------------------------------------------------------------------------
// Base test fixture that supplies some useful functions for tests.
// ----------------------------------------------------------------------------

class TestBase : public ::testing::Test {
 protected:
    /// Class that creates a temporary file in the constructor and removes it
    /// in the destructor.
    class TempFile {
      public:
        /// Creates a temporary file with the given contents.
        TempFile(const std::string &input_string) {
            namespace bf = boost::filesystem;
            path_ = bf::temp_directory_path() / bf::unique_path();
            path_string_ = path_.native();
            std::ofstream out(path_string_);
            ASSERT(out.is_open());
            out << input_string;
            out.close();
        }
        /// Removes the temporary file.
        ~TempFile() {
            boost::filesystem::remove(path_);
        }
        /// Returns the path to the temporary file as a string.
        const std::string & GetPathString() { return path_string_; }

      private:
        boost::filesystem::path path_;
        std::string             path_string_;
    };

    // Close enough.
    static constexpr float kClose = 1e-4f;

    /// Returns a FilePath to the named test file (in the Data directory).
    Util::FilePath GetDataPath(const std::string &file_name) {
        Util::FilePath path = Util::FilePath::GetTestDataPath();
        path /= file_name;
        return path;
    }

    /// Reads the named test file and returns the contents as a string. Asserts
    /// if the file cannot be opened.
    std::string ReadDataFile(const std::string &file_name) {
        std::string s;
        EXPECT_TRUE(Util::ReadFile(GetDataPath(file_name), s));
        return s;
    }

    // FP testing predicates from Ion.
    static bool VectorsClose(const Vector3f &v0, const Vector3f &v1) {
        return ion::math::VectorsAlmostEqual(v0, v1, kClose);
    }
    static bool PointsClose(const Point3f &p0, const Point3f &p1) {
        return ion::math::PointsAlmostEqual(p0, p1, kClose);
    }

    // FP testing predicates from Ion with custom tolerance.
    static bool VectorsCloseT(const Vector3f &v0, const Vector3f &v1, float t) {
        return ion::math::VectorsAlmostEqual(v0, v1, t);
    }
    static bool PointsCloseT(const Point3f &p0, const Point3f &p1, float t) {
        return ion::math::PointsAlmostEqual(p0, p1, t);
    }

    // Loads a TriMesh from an STL file.
    TriMesh LoadTriMesh(const std::string &file_name) {
        const Util::FilePath path = GetDataPath(file_name);
        std::string error;
        TriMesh mesh = ReadSTLFile(path, UnitConversion(), error);
        ASSERTM(! mesh.points.empty(),
                "Loaded from " + path.ToString() + ": " + error);
        ValidateMesh(mesh, "Imported from '" + path.ToString() + "'");
        return mesh;
    }

    // Validates a TriMesh.
    static void ValidateMesh(const TriMesh &mesh, string desc) {
        const MeshValidityCode ret = IsMeshValid(mesh);
        EXPECT_TRUE(ret == MeshValidityCode::kValid)
            << "Invalid " << desc << " mesh: " << Util::EnumName(ret);
    }
};
