//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

/// \file
/// This file is used to include Ion source files in tests. Referring to the
/// source files explicitly in SConscript_tests causes a problem because they
/// are also built for Ion tests.

#include "../ionsrc/ion/gfx/tests/fakeglcontext.cc"
#include "../ionsrc/ion/gfx/tests/fakegraphicsmanager.cc"

