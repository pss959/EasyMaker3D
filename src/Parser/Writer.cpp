#include "Parser/Writer.h"

#include <functional>
#include <unordered_set>

#include "Math/Types.h"
#include "Parser/Field.h"
#include "Parser/Object.h"
#include "Parser/ValueWriter.h"
#include "Util/Assert.h"
#include "Util/String.h"

namespace Parser {

// ----------------------------------------------------------------------------
// This internal class does most of the work.
// ----------------------------------------------------------------------------

class Writer_ {
  public:
    typedef std::function<bool(const Object &)> ObjectFunc;

    /// The constructor is passed the output stream.
    Writer_(std::ostream &out) :
        out_(out),
        value_writer_(out_,
                      std::bind(&Writer_::WriteObject_, this,
                                std::placeholders::_1),
                      std::bind(&Writer_::WriteObjectList_, this,
                                std::placeholders::_1)) {}

    /// Sets a predicate function that is invoked before writing any Object. If
    /// the function returns false, the Object is not written. This is null by
    /// default, meaning that all Objects are written.
    void SetObjectFunction(const ObjectFunc &func) {
        object_func_ = func;
    }

    /// Sets a flag indicating whether object addresses should be written as
    /// comments. The default is false.
    void SetAddressFlag(bool write_addresses) {
        write_addresses_ = write_addresses;
    }

    void WriteObject(const Object &obj) {
        if (WriteObject_(obj))
            out_ << "\n";
    }

  private:
    std::ostream &out_;                      ///< Stream passed to constructor.
    int           cur_depth_ = 0;            ///< Current depth in graph.
    bool          in_list_ = false;          ///< True when writing object list.
    bool          write_addresses_ = false;  ///< Whether to write addresses.

    /// Predicate function to write objects selectively.
    ObjectFunc    object_func_;

    /// ValueWriter instance used for writing values.
    ValueWriter   value_writer_;

    /// Set storing named Object instances that have been written already. This
    /// is used to detect instances.
    std::unordered_set<const Object *> written_named_objects_;

    static const int kIndent_ = 2;  ///< Spaces to indent each level.

    bool WriteObject_(const Object &obj);
    void WriteObjectList_(const std::vector<ObjectPtr> &obj_list);

    /// Returns true if the object is an instance.
    bool WriteObjHeader_(const Object &obj);
    void WriteObjFooter_();
    void WriteObjAddress_(const Object &obj);

    std::string Indent_() { return Util::Spaces(kIndent_ * cur_depth_); }
};

bool Writer_::WriteObject_(const Object &obj) {
    if (object_func_ && ! object_func_(obj))
        return false;

    if (WriteObjHeader_(obj))
        return true;

    // Write all fields that have values set.
    for (auto field: obj.GetFields()) {
        if (field->WasSet()) {
            out_ << Indent_() << field->GetName() << ": ";
            field->WriteValue(value_writer_);
            out_ << ",\n";
        }
    }

    WriteObjFooter_();
    return true;
}

void Writer_::WriteObjectList_(const std::vector<ObjectPtr> &obj_list) {
    if (! obj_list.empty()) {
        in_list_ = true;
        out_ << "[\n";
        ++cur_depth_;
        for (const auto &obj: obj_list) {
            out_ << Indent_();
            WriteObject_(*obj);
            out_ << ",\n";
        }
        --cur_depth_;
        out_ << Indent_() << "]";
        in_list_ = false;
    }
}

bool Writer_::WriteObjHeader_(const Object &obj) {
    const bool is_use = ! obj.GetName().empty() &&
        Util::MapContains(written_named_objects_, &obj);

    if (obj.GetObjectType() == Object::ObjType::kTemplate)
        out_ << "TEMPLATE ";
    else if (obj.GetObjectType() == Object::ObjType::kInstance)
        out_ << "INSTANCE ";
    else if (is_use)
        out_ << "USE ";
    out_ << obj.GetTypeName();
    if (! obj.GetName().empty())
        out_ << " \"" << obj.GetName() << "\"";
    if (is_use) {
        if (write_addresses_)
            WriteObjAddress_(obj);
        return true;
    }
    else {
        if (! obj.GetName().empty())
            written_named_objects_.insert(&obj);
        out_ << " {";
        if (write_addresses_)
            WriteObjAddress_(obj);
        out_ << "\n";
        ++cur_depth_;
        return false;
    }
}

void Writer_::WriteObjFooter_() {
    --cur_depth_;
    out_ << Indent_() << "}";
}

void Writer_::WriteObjAddress_(const Object &obj) {
    out_ << " # " << &obj;
}

// ----------------------------------------------------------------------------
// Writer implementation.
// ----------------------------------------------------------------------------

Writer::Writer() {
}

Writer::~Writer() {
}

void Writer::SetAddressFlag(bool write_addresses) {
    write_addresses_ = write_addresses;
}

void Writer::WriteObject(const Object &obj, std::ostream &out) {
    Writer_ writer(out);
    writer.SetAddressFlag(write_addresses_);
    writer.WriteObject(obj);
}

void Writer::WriteObjectConditional(
    const Object &obj, const std::function<bool(const Object &)> &func,
    std::ostream &out) {
    Writer_ writer(out);
    writer.SetObjectFunction(func);
    writer.SetAddressFlag(write_addresses_);
    writer.WriteObject(obj);
}

}  // namespace Parser
