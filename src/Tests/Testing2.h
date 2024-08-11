//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Tests/Testing.h"
#include "Util/Assert.h"

/// \file

/// This file contains more macros for tests that require more
/// dependencies. They are in a separate header rather than Testing.h to avoid
/// including extra files, such as gmock.h.

// ----------------------------------------------------------------------------
// Handy macros.
// ----------------------------------------------------------------------------

/// Macro for testing string containment.
#define EXPECT_STR_HAS(STR, SUBSTR) \
    EXPECT_THAT(STR, ::testing::HasSubstr(SUBSTR))

/// Tests that an exception of the given type is thrown by the given statement
/// and that the exception's message contains the given string pattern.
#define TEST_THROW(STMT, EXCEPTION_TYPE, PATTERN)                       \
    EXPECT_THROW({                                                      \
            try {                                                       \
                STMT;                                                   \
            }                                                           \
            catch (const EXCEPTION_TYPE &ex) {                          \
                EXPECT_STR_HAS(Str(ex.what()), PATTERN);                \
                throw;                                                  \
            }                                                           \
        }, EXCEPTION_TYPE)

/// Special case of TEST_THROW() that uses an AssertException.
#define TEST_ASSERT(STMT, PATTERN) TEST_THROW(STMT, AssertException, PATTERN)
