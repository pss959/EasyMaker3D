#include "Tests/Widgets/WidgetTestBase.h"
#include "Widgets/Widget.h"

// Base class for derived TargetWidget class tests. Defines a TestTargetWidget
// class that can receive targets.
class TargetWidgetTestBase : public WidgetTestBase {
  protected:
    // Derived Widget class that supports target placement.
    class TestTargetWidget : public Widget {
      public:
        virtual bool CanReceiveTarget() const override { return true; }
        virtual void PlacePointTarget(const DragInfo &info,
                                      Point3f &position, Vector3f &direction,
                                      Dimensionality &snapped_dims) override;
        virtual void PlaceEdgeTarget(const DragInfo &info, float current_length,
                                     Point3f &position0,
                                     Point3f &position1) override;
      protected:
        TestTargetWidget() {}
      private:
        friend class Parser::Registry;
    };
    typedef std::shared_ptr<TestTargetWidget> TestTargetWidgetPtr;

    TargetWidgetTestBase();
};
