#include <assert.h>

#include <fstream>
#include <string>

#include "gtest/gtest.h"

#include <boost/filesystem.hpp>

#include "Util/FilePath.h"
#include "Util/Read.h"

//! Macro for testing against null for clarity.
#define EXPECT_NULL(PTR)     EXPECT_TRUE((PTR) == nullptr)

//! Macro for testing against null for clarity.
#define EXPECT_NOT_NULL(PTR) EXPECT_TRUE((PTR) != nullptr)

//! Tests that an exception of the given type is thrown by the given statement
//! and that the exception's message contains the given string pattern.
#define TEST_THROW(STMT, EXCEPTION_TYPE, PATTERN)                             \
    EXPECT_THROW({                                                            \
            try {                                                             \
                STMT;                                                         \
            }                                                                 \
            catch (const EXCEPTION_TYPE &ex) {                                \
                EXPECT_TRUE(std::string(ex.what()).find(PATTERN) !=           \
                            std::string::npos) <<                             \
                    "Exception string:" << ex.what()                          \
                                        << "\nvs. pattern: " << (PATTERN);    \
                throw;                                                        \
            }                                                                 \
        }, EXCEPTION_TYPE)

//! Base test fixture that supplies some useful functions for tests.
class TestBase : public ::testing::Test {
 protected:
    //! Class that creates a temporary file in the constructor and removes it
    //! in the destructor.
    class TempFile {
      public:
        //! Creates a temporary file with the given contents.
        TempFile(const std::string &input_string) {
            namespace bf = boost::filesystem;
            path_ = bf::temp_directory_path() / bf::unique_path();
            path_string_ = path_.native();
            std::ofstream out(path_string_);
            assert(out.is_open());
            out << input_string;
            out.close();
        }
        //! Removes the temporary file.
        ~TempFile() {
            boost::filesystem::remove(path_);
        }
        //! Returns the path to the temporary file as a string.
        const std::string & GetPathString() { return path_string_; }

      private:
        boost::filesystem::path path_;
        std::string             path_string_;
    };

    //! Returns a FilePath to the named test file (in the Data directory).
    Util::FilePath GetDataPath(const std::string &file_name) {
        Util::FilePath path = Util::FilePath::GetTestDataPath();
        path /= file_name;
        return path;
    }

    //! Reads the named test file and returns the contents as a string. Asserts
    //! if the file cannot be opened.
    std::string ReadDataFile(const std::string &file_name) {
        std::string s;
        EXPECT_TRUE(Util::ReadFile(GetDataPath(file_name), s));
        return s;
    }
};
