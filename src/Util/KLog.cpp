#include "Util/KLog.h"

//! DummyStream_ does not do any output. It is used when a message key is
//! not active.
class DummyStream_ : public std::ostream {};

static DummyStream_ s_dummy_stream_;

std::string KLogger::key_string_;

KLogger::KLogger(char key) :
    do_print_(key_string_.find_first_of(key) != std::string::npos) {
    GetStream() << '[' << key << "] ";
}

std::ostream & KLogger::GetStream() {
    return do_print_ ? std::cout : s_dummy_stream_;
}
