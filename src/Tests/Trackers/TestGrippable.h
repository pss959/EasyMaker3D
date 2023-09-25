#include "Base/Memory.h"
#include "Items/Grippable.h"

DECL_SHARED_PTR(ClickableWidget);

/// Derived Grippable class used for easier testing. It allows the values set
/// in UpdateGripInfo() to be set as public member variables.
///
/// \ingroup Tests
class TestGrippable : public Grippable {
  public:
    ClickableWidgetPtr widget;  ///< Widget to set in GripInfo.
    Point3f            target;  ///< Target point to set in GripInfo.
    Color              color;   ///< Color to set in GripInfo.

    virtual const SG::Node * GetGrippableNode() const override { return this; }

    virtual void UpdateGripInfo(GripInfo &info) override {
        info.widget       = widget;
        info.target_point = target;
        info.color        = color;
    }

  protected:
    TestGrippable() {}

  private:
    friend class Parser::Registry;
};

DECL_SHARED_PTR(TestGrippable);
