#include "Util/ExceptionBase.h"

#include "Util/FilePath.h"
#include "Util/StackTrace.h"
#include "Util/String.h"

ExceptionBase::ExceptionBase(const FilePath &path, const std::string &msg) {
    msg_ = "\n" + path.ToString() + ": " + msg;
    SetStackTrace_();
}

ExceptionBase::ExceptionBase(const FilePath &path, int line_number,
                             const std::string &msg) {
    msg_ = "\n" + path.ToString() + ':' +
        Util::ToString(line_number) + ": " + msg;
    SetStackTrace_();
}

void ExceptionBase::SetStackTrace_() {
    stack_trace_ = Util::GetStackTrace();
}
