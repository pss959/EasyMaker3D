#include "Testing.h"

#include "Util/General.h"
#include "Util/StackTrace.h"

int main(int argc, char **argv) {
    // Set a flag so that code can tell a test is running.
    Util::is_in_unit_test = true;

    try {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught Exception:\n" << ex.what() << "\n"
                  << "*** Stack trace:\n";
        Util::PrintStackTrace();
        return -1;
    }
}
