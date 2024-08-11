//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Util/KLog.h"

/// DummyStream_ does not do any output. It is used when a message key is
/// not active.
class DummyStream_ : public std::ostream {};

static DummyStream_ s_dummy_stream_;

Str    KLogger::key_string_;
size_t KLogger::render_count_ = 0;

bool KLogger::HasKeyCharacter(char key) {
    return key_string_.contains(key);
}

void KLogger::ToggleLogging() {
    auto pos = key_string_.find_first_of('!');
    const bool is_on = pos == Str::npos;
    if (is_on)
        key_string_ += '!';
    else
        key_string_.erase(pos, 1);
    std::cerr << "Logging is now " << (is_on ? "OFF" : "ON") << "\n";
}

KLogger::KLogger(char key) : do_print_(ShouldPrint_(key)) {
    GetStream() << '[' << key << "/" << render_count_ << "] " ;
}

std::ostream & KLogger::GetStream() {
    return do_print_ ? std::cout : s_dummy_stream_;
}

bool KLogger::ShouldPrint_(char key) {
    return (HasKeyCharacter(key) || HasKeyCharacter('*')) &&
        ! HasKeyCharacter('!');
}
