#include "Math/ToString.h"
#include "Math/Types.h"
#include "Tests/Testing.h"
#include "Util/String.h"

TEST(MathToStringTest, ToStringPrecision) {
    const float f = 5.3728f;
    EXPECT_EQ("5",      Math::ToString(f, 1.f));
    EXPECT_EQ("5.4",    Math::ToString(f, .1f));
    EXPECT_EQ("5.37",   Math::ToString(f, .01f));
    EXPECT_EQ("5.373",  Math::ToString(f, .001f));
    EXPECT_EQ("5.3728", Math::ToString(f, .0001f));
    EXPECT_EQ("5.3728", Math::ToString(f, .00001f));

    EXPECT_EQ("[5.4, 5.4]",      Math::ToString(Point2f(f, f),     .1f));
    EXPECT_EQ("[5.4, 5.4, 5.4]", Math::ToString(Point3f(f, f, f),  .1f));
    EXPECT_EQ("[5.4, 5.4]",      Math::ToString(Vector2f(f, f),    .1f));
    EXPECT_EQ("[5.4, 5.4, 5.4]", Math::ToString(Vector3f(f, f, f), .1f));
}
