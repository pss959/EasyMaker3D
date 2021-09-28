#include "Testing.h"

#include "Util/General.h"

int main(int argc, char **argv) {
    // Set a flag so that code can tell a test is running.
    Util::is_in_unit_test = true;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
