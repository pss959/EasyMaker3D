#pragma once

#include <string>
#include <vector>

// XXXX #include <boost/iterator/iterator_facade.hpp>

#include "Parser/ObjectSpec.h"
#include "Parser/Typedefs.h"

namespace Parser {

//! The Object struct represents a parsed object.
struct Object {
    const ObjectSpec      &spec;        //!< ObjectSpec for the object.
    const std::string     path;         //!< Path read from.
    const int             line_number;  //!< Line number of definition.

    std::vector<FieldPtr> fields;       //!< Fields parsed for the object.

#if XXXX
    //! Custom iterator over fields in the object.
    struct FieldIterator : public boost::iterator_facade<
        FieldIterator, const Object, boost::forward_traversal_tag>{
      public:
        explicit FieldIterator(const Object &obj) : object_(obj) {}
      private:
        friend class boost::iterator_core_access;
        const Object &object_;
        int   index_ = 0;

        // Required iterator_facade functions.
        void increment() { ++index_; }
        bool equal(const FieldIterator &other) const {
            return &object_ == &other.object_ && index_ == other.index_;
        }
        const Field & dereference() const { return *object_.fields[index_]; }
    };
#endif

    //! The constructor is passed the ObjectSpec for the object, the file path
    //! it came from and the line number it was defined on..
    Object(const ObjectSpec &spec_in, const std::string &path_in,
           int line_number_in) :
        spec(spec_in), path(path_in), line_number(line_number_in) {}
};

}  // namespace Parser
