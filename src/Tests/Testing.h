//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "gtest/gtest.h"
#include "Util/Enum.h"

/// \file
/// This file contains basic macros for tests. Most are wrappers around
/// googletest macros that make the test code simpler and clearer.

// ----------------------------------------------------------------------------
// Handy macros.
// ----------------------------------------------------------------------------

/// Macro for testing against null for clarity.
#define EXPECT_NULL(PTR)     EXPECT_TRUE((PTR) == nullptr)

/// Macro for testing against null for clarity.
#define EXPECT_NOT_NULL(PTR) EXPECT_TRUE((PTR) != nullptr)

/// Macro for testing against null for clarity.
#define ASSERT_NOT_NULL(PTR) ASSERT_TRUE((PTR) != nullptr)

/// Macro for testing near-equality for floats (using TestBase::kClose).
#define EXPECT_CLOSE(EXP, V) EXPECT_NEAR(EXP, V, kClose)

/// Macro for testing near-equality for vectors.
#define EXPECT_VECS_CLOSE2(EXP, V) EXPECT_PRED2(VectorsClose2, EXP, V)

/// Macro for testing near-equality for points.
#define EXPECT_PTS_CLOSE2(EXP, P) EXPECT_PRED2(PointsClose2, EXP, P)

/// Macro for testing near-equality for vectors.
#define EXPECT_VECS_CLOSE(EXP, V) EXPECT_PRED2(VectorsClose, EXP, V)

/// Macro for testing near-equality for points.
#define EXPECT_PTS_CLOSE(EXP, P) EXPECT_PRED2(PointsClose, EXP, P)

/// Macro for testing near-equality for rotations.
#define EXPECT_ROTS_CLOSE(EXP, R) EXPECT_PRED2(RotationsClose, EXP, R)

/// Macro for testing near-equality for matrices.
#define EXPECT_MATS_CLOSE(EXP, M) EXPECT_PRED2(MatricesClose, EXP, M)

/// Macro for testing Parser::Object instances for equality.
#define EXPECT_EQ_OBJS(O1, O2) EXPECT_EQ(O1.get(), O2.get()) \
    << "Objects differ: " << O1->GetDesc() << " vs. " << O2->GetDesc()

/// Macro for testing enums with better error messages.
#define EXPECT_ENUM_EQ(EXP, E) EXPECT_EQ(EXP, E) \
    << "Enums differ: " << Util::EnumName(EXP) << " vs. " << Util::EnumName(E)
