#include "Tests/Testing.h"

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"

int main(int argc, char **argv) {
    // Let the code know a regular unit test is running.
    Util::app_type = Util::AppType::kUnitTest;

    try {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
    catch (AssertException &ex) {
        std::cerr << "*** Caught AssertException:\n" << ex.what() << "\n"
                  << "*** Stack trace:\n"
                  << Util::JoinStrings(ex.GetStackTrace(), "\n") << "\n";
        return -1;
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught Exception:\n" << ex.what() << "\n";
        return -1;
    }
}
