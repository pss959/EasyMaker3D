#pragma once

#include <string>

#include "gtest/gtest.h"
#include "Util/KLog.h"

// ----------------------------------------------------------------------------
// Handy macros.
// ----------------------------------------------------------------------------

/// Macro for testing against null for clarity.
#define EXPECT_NULL(PTR)     EXPECT_TRUE((PTR) == nullptr)

/// Macro for testing against null for clarity.
#define EXPECT_NOT_NULL(PTR) EXPECT_TRUE((PTR) != nullptr)

/// Macro for testing near-equality for vectors.
#define EXPECT_VECS_CLOSE(EXP, V) EXPECT_PRED2(VectorsClose, EXP, V)

/// Macro for testing near-equality for points.
#define EXPECT_PTS_CLOSE(EXP, P) EXPECT_PRED2(PointsClose, EXP, P)

/// Macro for testing near-equality for rotations.
#define EXPECT_ROTS_CLOSE(EXP, R) EXPECT_PRED2(RotationsClose, EXP, R)

/// Macro for testing Parser::Object instances for equality.
#define EXPECT_EQ_OBJS(O1, O2) EXPECT_EQ(O1.get(), O2.get()) \
    << "Objects differ: " << O1->GetDesc() << " vs. " << O2->GetDesc()

/// Tests that an exception of the given type is thrown by the given statement
/// and that the exception's message contains the given string pattern.
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

// ----------------------------------------------------------------------------
// Handy functions.
// ----------------------------------------------------------------------------

/// Enables KLogging for the given key string.
inline void EnableKLog(const std::string &s) {
    KLogger::SetKeyString(s);
}
