#include "Tests/Testing.h"

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"
#include "Util/String.h"

int main(int argc, char **argv) {
    // Let the code know a regular unit test is running.
    Util::app_type = Util::AppType::kUnitTest;

    // Exclude timing tests by default. They can be run by adding a test filter.
    testing::GTEST_FLAG(filter) = "-TimingTest.*";

    try {
        ::testing::InitGoogleTest(&argc, argv);

        // Handle other arguments.
        for (int i = 1; i < argc; ++i) {
            const Str arg = argv[i];
            if (arg.starts_with("--klog=")) {
                KLogger::SetKeyString(arg.substr(6));
            }
            else {
                std::cerr << "*** Unknown argument: '" << arg << "\n";
                return -1;
            }
        }

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
