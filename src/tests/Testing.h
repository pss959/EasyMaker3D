#include "gtest/gtest.h"

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
