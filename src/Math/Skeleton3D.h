#pragma once

#include <vector>

#include "Math/Skeleton.h"
#include "Math/Types.h"

struct PolyMesh;

/// 3D straight skeleton. XXXX What and why.
///
/// \ingroup Math
class Skeleton3D : public Skeleton<Point3f> {
  public:
    /// Constructs the Skeleton for the given PolyMesh.
    void BuildForPolyMesh(const PolyMesh &mesh);

  private:
    class Helper_;
};
