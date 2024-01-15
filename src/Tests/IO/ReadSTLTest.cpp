#include "IO/STLReader.h"
#include "Items/UnitConversion.h"
#include "Math/Bounds.h"
#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Math/TriMesh.h"
#include "Tests/TempFile.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/Testing2.h"

/// \ingroup Tests
class ReadSTLTest : public TestBaseWithTypes {
  protected:
    /// Returns a default UnitConversion instance.
    static UnitConversionPtr GetDefaultUC() {
        return CreateObject<UnitConversion>();
    }

    /// Loads a TriMesh from an STL file and validates it.
    TriMesh LoadFromFile(const Str &file_name,
                         const UnitConversion &conv = *GetDefaultUC()) {
        const auto path = GetDataPath(file_name);
        Str error;
        TriMesh mesh = ReadSTLFile(path, conv.GetFactor(), error);
        ASSERTM(! mesh.points.empty(),
                "Loaded from " + path.ToString() + ": " + error);
        ValidateMesh(mesh, "Imported from '" + path.ToString() + "'");
        return mesh;
    }

    /// Loads a TriMesh from a string and validates it.
    TriMesh LoadFromString(const Str &str,
                           const UnitConversion &conv = *GetDefaultUC()) {
        TempFile tmp(str);
        const auto &path = tmp.GetPath();
        Str error;
        TriMesh mesh = ReadSTLFile(path, conv.GetFactor(), error);
        ASSERTM(! mesh.points.empty(),
                "Loaded from string: " + error);
        ValidateMesh(mesh, "Loaded from string");
        return mesh;
    }
};

TEST_F(ReadSTLTest, TextBox) {
    const TriMesh mesh = LoadFromFile("box.stl");
    EXPECT_EQ(Bounds(Vector3f(8, 8, 8)), ComputeMeshBounds(mesh));

    // Validate triangle orientation. The first normal should be (-1, 0, 0).
    const Vector3f normal = ComputeNormal(mesh.points[mesh.indices[0]],
                                          mesh.points[mesh.indices[1]],
                                          mesh.points[mesh.indices[2]]);
    EXPECT_EQ(Vector3f(-1, 0, 0), normal);
}

TEST_F(ReadSTLTest, TextBoxConversion) {
    UnitConversionPtr conv = GetDefaultUC();
    conv->SetFromUnits(UnitConversion::Units::kMeters);
    conv->SetToUnits(UnitConversion::Units::kMillimeters);
    const TriMesh mesh = LoadFromFile("box.stl", *conv);
    EXPECT_EQ(Bounds(Vector3f(8000, 8000, 8000)), ComputeMeshBounds(mesh));
}

TEST_F(ReadSTLTest, BinaryBox) {
    const TriMesh mesh = LoadFromFile("binarybox.stl");
    EXPECT_EQ(Bounds(Vector3f(10, 10, 10)), ComputeMeshBounds(mesh));
}

TEST_F(ReadSTLTest, BinaryBoxConversion) {
    UnitConversionPtr conv = GetDefaultUC();
    conv->SetFromUnits(UnitConversion::Units::kMeters);
    conv->SetToUnits(UnitConversion::Units::kMillimeters);
    const TriMesh mesh = LoadFromFile("binarybox.stl", *conv);
    EXPECT_EQ(Bounds(Vector3f(10000, 10000, 10000)), ComputeMeshBounds(mesh));
}

TEST_F(ReadSTLTest, BinaryBoxSolid) {
    // Reads a file that is binary but that starts with "solid" to test that it
    // is recognized properly as binary.
    const TriMesh mesh = LoadFromFile("binaryboxsolid.stl");
    EXPECT_EQ(Bounds(Vector3f(10, 10, 10)), ComputeMeshBounds(mesh));
}

TEST_F(ReadSTLTest, BinarySpoon) {
    // Validate the spoon used for import in the documentation.
    const TriMesh mesh =
        LoadFromFile("../../../PublicDoc/stl/Spoon.stl");
    EXPECT_FALSE(ComputeMeshBounds(mesh).IsEmpty());
}

TEST_F(ReadSTLTest, Errors) {
    TEST_THROW(LoadFromFile("nosuchfile.stl"), ExceptionBase, "Unable to open");
    TEST_THROW(LoadFromFile("nomesh.stl"),     ExceptionBase, "No mesh data");

    {
        const Str s =
            "solid EasyMaker3D_Export\n"
            "  blah foo bar\n"
            "endsolid EasyMaker3D_Export\n";
        TEST_THROW(LoadFromString(s), ExceptionBase, "Expected 'endsolid'");
    }

    {
        const Str s =
            "solid EasyMaker3D_Export\n"
            "  facet normal 1 0 0\n"
            "    blah\n";
        TEST_THROW(LoadFromString(s), ExceptionBase, "Expected 'outer loop'");
    }

    {
        const Str s =
            "solid EasyMaker3D_Export\n"
            "  facet normal 1 0 0\n"
            "    outer loop\n"
            "      shmertex\n";
        TEST_THROW(LoadFromString(s), ExceptionBase, "Expected 'vertex'");
    }

    {
        const Str s =
            "solid EasyMaker3D_Export\n"
            "  facet normal 1 0 0\n"
            "    outer loop\n"
            "      vertex a b c\n";
        TEST_THROW(LoadFromString(s), ExceptionBase, "Invalid vertex");
    }

    {
        const Str s =
            "solid EasyMaker3D_Export\n"
            "  facet normal 1 0 0\n"
            "    outer loop\n"
            "      vertex 0 0 0\n"
            "      vertex 0 1 0\n"
            "      vertex 1 1 0\n"
            "    nonsense\n";
        TEST_THROW(LoadFromString(s), ExceptionBase, "Expected 'endloop'");
    }

    {
        const Str s =
            "solid EasyMaker3D_Export\n"
            "  facet normal 1 0 0\n"
            "    outer loop\n"
            "      vertex 0 0 0\n"
            "      vertex 0 1 0\n"
            "      vertex 1 1 0\n"
            "    endloop\n"
            "  notendfacet\n";
        TEST_THROW(LoadFromString(s), ExceptionBase, "Expected 'endfacet'");
    }

    {
        // This is not recognized as valid text:
        const Str s = "XYZZYXX";
        TEST_THROW(LoadFromString(s), ExceptionBase, "Not enough binary data");
    }
}

