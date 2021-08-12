#pragma once

#include <functional>

namespace Parser {

class Object;

//! The Visitor traverses a parsed graph rooted by an Object, calling a
//! function for each object visited. Objects in all fields that contain
//! Objects are traversed.
class Visitor {
  public:
    //! Typedef for the function that gets invoked for each Object.
    typedef std::function<void(const Object &)> ObjectFunc;

    //! Visits all Objects found in the graph rooted by the given Object
    //! (inclusive), invoking the given function for each.
    static void VisitObjects(const Object &root, ObjectFunc func);
};

}  // namespace Parser
