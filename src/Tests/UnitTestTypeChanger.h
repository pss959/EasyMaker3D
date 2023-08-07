#include "Util/General.h"

/// Class that temporarily changes the global Util::app_type variable for a
/// test that needs to pretend it is not a regular unit test. The new type
/// applies while an instance of this class is in scope.
class UnitTestTypeChanger {
  public:
    UnitTestTypeChanger(Util::AppType type) {
        prev_type_     = Util::app_type;
        Util::app_type = type;
    }
    ~UnitTestTypeChanger() { Util::app_type = prev_type_; }
  private:
    Util::AppType prev_type_;
};
