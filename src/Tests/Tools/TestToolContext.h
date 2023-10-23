#include "Base/Memory.h"
#include "Tools/Tool.h"

namespace SG { class Scene; }

/// Derived Tool::Context that can be set in Tools for testing.
///
/// \ingroup Tests
class TestToolContext : public Tool::Context {
  public:
    /// The constructor is passed a Scene that various items are accessed
    /// from.
    explicit TestToolContext(const SG::Scene &scene);
};

DECL_SHARED_PTR(TestToolContext);
