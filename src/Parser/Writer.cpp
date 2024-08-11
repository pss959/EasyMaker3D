//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Parser/Writer.h"

#include <functional>
#include <unordered_set>

#include "Math/Types.h"
#include "Parser/Field.h"
#include "Parser/Object.h"
#include "Parser/ValueWriter.h"
#include "Util/Assert.h"
#include "Util/String.h"
#include "Util/Tuning.h"

namespace Parser {

// ----------------------------------------------------------------------------
// Writer::Impl_ class.
// ----------------------------------------------------------------------------

class Writer::Impl_ {
  public:
    /// The constructor is passed the output stream.
    Impl_(std::ostream &out);

    /// Sets a predicate function that is invoked before writing any Object. If
    /// the function returns false, the Object is not written. This is null by
    /// default, meaning that all Objects are written.
    void SetObjectFunction(const Writer::ObjectFunc &func) {
        object_func_ = func;
    }

    /// Sets a flag indicating whether object addresses should be written as
    /// comments. The default is false.
    void SetAddressFlag(bool write_addresses) {
        write_addresses_ = write_addresses;
    }

    void WriteComment(const Str &comment) {
        out_ << "# " << comment << "\n";
    }

    void WriteObject(const Object &obj) {
        // Clean up from any previous writing.
        written_named_objects_.clear();

        if (WriteObject_(obj, false))
            out_ << "\n";
    }

    void WriteField(const Field &field) {
        out_ << Indent_() << field.GetName() << ": ";
        field.WriteValue(value_writer_);
        out_ << ",\n";
    }

  private:
    std::ostream &out_;                      ///< Stream passed to constructor.
    int           cur_depth_ = 0;            ///< Current depth in graph.
    bool          in_list_ = false;          ///< True when writing object list.
    bool          write_addresses_ = false;  ///< Whether to write addresses.

    /// Predicate function to write objects selectively.
    Writer::ObjectFunc object_func_;

    /// ValueWriter instance used for writing values.
    ValueWriter   value_writer_;

    /// Set storing named Object instances that have been written already. This
    /// is used to detect instances.
    std::unordered_set<const Object *> written_named_objects_;

    bool WriteObject_(const Object &obj, bool is_in_list);
    void WriteObjectList_(const std::vector<ObjectPtr> &obj_list);

    /// Returns true if the object is an instance.
    bool WriteObjHeader_(const Object &obj, bool is_in_list);
    void WriteObjFooter_();
    void WriteObjAddress_(const Object &obj);

    Str Indent_() {
        return Util::Spaces(TK::kParserWriterIndent * cur_depth_);
    }
};

Writer::Impl_::Impl_(std::ostream &out) :
    out_(out),
    value_writer_(out_,
                  [&](const Object &obj){ WriteObject_(obj, false); },
                  [&](const std::vector<ObjectPtr> &obj_list){
                      WriteObjectList_(obj_list); }) {
}

bool Writer::Impl_::WriteObject_(const Object &obj, bool is_in_list) {
    if (object_func_ && ! object_func_(obj, true))
        return false;

    if (is_in_list)
        out_ << Indent_();

    if (WriteObjHeader_(obj, is_in_list))
        return true;

    // Write all non-hidden fields that have values set.
    for (auto field: obj.GetFields()) {
        if (field->WasSet() && ! field->IsHidden())
            WriteField(*field);
    }

    WriteObjFooter_();
    if (object_func_)
        object_func_(obj, false);
    return true;
}

void Writer::Impl_::WriteObjectList_(const std::vector<ObjectPtr> &obj_list) {
    if (! obj_list.empty()) {
        in_list_ = true;
        out_ << "[\n";
        ++cur_depth_;
        for (const auto &obj: obj_list) {
            if (WriteObject_(*obj, true))
                out_ << ",\n";
        }
        --cur_depth_;
        out_ << Indent_() << "]";
        in_list_ = false;
    }
}

bool Writer::Impl_::WriteObjHeader_(const Object &obj, bool is_in_list) {
    const bool is_use =
        ! obj.GetName().empty() && written_named_objects_.contains(&obj);

    if (is_use)
        out_ << "USE";
    else
        out_ << obj.GetTypeName();
    if (! obj.GetName().empty())
        out_ << " \"" << obj.GetName() << "\"";
    if (is_use) {
        if (write_addresses_) {
            if (is_in_list)
                out_ << ",";
            WriteObjAddress_(obj);
        }
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

void Writer::Impl_::WriteObjFooter_() {
    --cur_depth_;
    out_ << Indent_() << "}";
}

void Writer::Impl_::WriteObjAddress_(const Object &obj) {
    out_ << " # " << &obj;
}

// ----------------------------------------------------------------------------
// Writer implementation.
// ----------------------------------------------------------------------------

Writer::Writer(std::ostream &out) : impl_(new Impl_(out)) {
}

Writer::~Writer() {
}

void Writer::SetAddressFlag(bool write_addresses) {
    impl_->SetAddressFlag(write_addresses);
}

void Writer::WriteComment(const Str &comment) {
    impl_->WriteComment(comment);
}

void Writer::WriteObject(const Object &obj) {
    impl_->WriteObject(obj);
}

void Writer::WriteObjectConditional(const Object &obj, const ObjectFunc &func) {
    impl_->SetObjectFunction(func);
    impl_->WriteObject(obj);
    impl_->SetObjectFunction(nullptr);
}

void Writer::WriteField(const Field &field) {
    impl_->WriteField(field);
}

}  // namespace Parser
