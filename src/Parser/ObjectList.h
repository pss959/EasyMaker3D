#pragma once

#include <memory>
#include <vector>

namespace Parser {

class Object;
typedef std::shared_ptr<Object> ObjectPtr;

//! A ObjectList wraps a vector of pointers to Objects. It has to exist to
//! decouple the Scanner from knowing about the Object class directly.
struct ObjectList {
    std::vector<ObjectPtr> objects;
};

typedef std::shared_ptr<ObjectList> ObjectListPtr;

}  // namespace Parser
