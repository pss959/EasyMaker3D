#pragma once

#include <functional>
#include <ostream>
#include <vector>

#include "Base/Memory.h"
#include "Util/Enum.h"
#include "Util/Flags.h"

namespace Parser {

DECL_SHARED_PTR(Object);

/// The ValueWriter class is passed to fields so that they can write their
/// values without having to anything about the format. The Writer class sets
/// this up to be able to write objects and object lists as well as simple
/// values.
///
/// \ingroup Parser
class ValueWriter {
  public:
    typedef std::function<void(const Object &)> ObjFunc;
    typedef std::function<void(const std::vector<ObjectPtr> &)> ObjListFunc;

    ValueWriter(std::ostream &out, const ObjFunc &obj_func,
                const ObjListFunc &obj_list_func) :
        out_(out), obj_func_(obj_func), obj_list_func_(obj_list_func) {}

    template <typename T> void WriteValue(const T &value);

    template <typename E> void WriteEnum(E value) {
        out_ << '"' << Util::EnumName(value) << '"';
    }

    template <typename E> void WriteFlags(const Util::Flags<E> &value) {
        out_ << '"' << value.ToString() << '"';
    }

    template <typename T> void WriteValueVector(const std::vector<T> &values) {
        out_ << '[';
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0)
                out_ << ", ";
            WriteValue(values[i]);
        }
        out_ << ']';
    }

    void WriteObject(const Object &obj) {
        obj_func_(obj);
    }

    void WriteObjectList(const std::vector<ObjectPtr> &obj_list) {
        obj_list_func_(obj_list);
    }

  private:
    std::ostream &out_;
    std::function<void(const Object &)> obj_func_;
    std::function<void(const std::vector<ObjectPtr> &)> obj_list_func_;
};

}  // namespace Parser
