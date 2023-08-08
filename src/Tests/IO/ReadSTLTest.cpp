#include "IO/STLReader.h"
#include "Items/UnitConversion.h"
#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ReadSTLTest : public TestBaseWithTypes {
  protected:
    /// Returns a default UnitConversion instance.
    static UnitConversionPtr GetDefaultUC() {
        return CreateObject<UnitConversion>();
    }

    /// Loads a TriMesh from an STL file and validates it.
    TriMesh LoadTriMesh(const std::string &file_name,
                        const UnitConversion &conv) {
        const FilePath path = GetDataPath(file_name);
        std::string error;
        TriMesh mesh = ReadSTLFile(path, conv.GetFactor(), error);
        ASSERTM(! mesh.points.empty(),
                "Loaded from " + path.ToString() + ": " + error);
        ValidateMesh(mesh, "Imported from '" + path.ToString() + "'");
        return mesh;
    }
};

TEST_F(ReadSTLTest, TextBox) {
    const TriMesh mesh = LoadTriMesh("box.stl", *GetDefaultUC());
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
    const TriMesh mesh = LoadTriMesh("box.stl", *conv);
    EXPECT_EQ(Bounds(Vector3f(8000, 8000, 8000)), ComputeMeshBounds(mesh));
}

TEST_F(ReadSTLTest, BinaryBox) {
    const TriMesh mesh = LoadTriMesh("binarybox.stl", *GetDefaultUC());
    EXPECT_EQ(Bounds(Vector3f(10, 10, 10)), ComputeMeshBounds(mesh));
}

TEST_F(ReadSTLTest, BinaryBoxConversion) {
    UnitConversionPtr conv = GetDefaultUC();
    conv->SetFromUnits(UnitConversion::Units::kMeters);
    conv->SetToUnits(UnitConversion::Units::kMillimeters);
    const TriMesh mesh = LoadTriMesh("binarybox.stl", *conv);
    EXPECT_EQ(Bounds(Vector3f(10000, 10000, 10000)), ComputeMeshBounds(mesh));
}

TEST_F(ReadSTLTest, BinaryBoxSolid) {
    // Reads a file that is binary but that starts with "solid" to test that it
    // is recognized properly as binary.
    const TriMesh mesh = LoadTriMesh("binaryboxsolid.stl", *GetDefaultUC());
    EXPECT_EQ(Bounds(Vector3f(10, 10, 10)), ComputeMeshBounds(mesh));
}

TEST_F(ReadSTLTest, BinarySpoon) {
    // Validate the spoon used for import in the documentation.
    const TriMesh mesh =
        LoadTriMesh("../../../PublicDoc/snaps/stl/Spoon.stl",
                    *GetDefaultUC());
    EXPECT_FALSE(ComputeMeshBounds(mesh).IsEmpty());
}
