#include "IO/STLReader.h"
#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "TestBase.h"
#include "Testing.h"

class STLTest : public TestBase {
  protected:
    // Loads a TriMesh from an STL file and validates it.
    TriMesh LoadTriMesh(const std::string &file_name,
                        const UnitConversion &conv) {
        const Util::FilePath path = GetDataPath(file_name);
        std::string error;
        TriMesh mesh = ReadSTLFile(path, conv, error);
        ASSERTM(! mesh.points.empty(),
                "Loaded from " + path.ToString() + ": " + error);
        ValidateMesh(mesh, "Imported from '" + path.ToString() + "'");
        return mesh;
    }
};

TEST_F(STLTest, TextBox) {
    const TriMesh mesh = LoadTriMesh("box.stl", UnitConversion());
    EXPECT_EQ(Bounds(Vector3f(8, 8, 8)), ComputeMeshBounds(mesh));

    // Validate triangle orientation. The first normal should be (-1, 0, 0).
    const Vector3f normal = ComputeNormal(mesh.points[mesh.indices[0]],
                                          mesh.points[mesh.indices[1]],
                                          mesh.points[mesh.indices[2]]);
    EXPECT_EQ(Vector3f(-1, 0, 0), normal);
}

TEST_F(STLTest, TextBoxConversion) {
    UnitConversion conv;
    conv.from_units = UnitConversion::Units::kMeters;
    conv.to_units   = UnitConversion::Units::kMillimeters;
    const TriMesh mesh = LoadTriMesh("box.stl", conv);
    EXPECT_EQ(Bounds(Vector3f(8000, 8000, 8000)), ComputeMeshBounds(mesh));
}

TEST_F(STLTest, BinaryBox) {
    const TriMesh mesh = LoadTriMesh("binarybox.stl", UnitConversion());
    EXPECT_EQ(Bounds(Vector3f(10, 10, 10)), ComputeMeshBounds(mesh));
}

TEST_F(STLTest, BinaryBoxConversion) {
    UnitConversion conv;
    conv.from_units = UnitConversion::Units::kMeters;
    conv.to_units   = UnitConversion::Units::kMillimeters;
    const TriMesh mesh = LoadTriMesh("binarybox.stl", conv);
    EXPECT_EQ(Bounds(Vector3f(10000, 10000, 10000)), ComputeMeshBounds(mesh));
}
