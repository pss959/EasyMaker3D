#include "Parser/InstanceStore.h"

#include "Util/String.h"

namespace Parser {

void InstanceStore::Reset() {
    original_map_.clear();
    available_instances_.clear();
}

void InstanceStore::AddOriginal_(const std::type_index &key,
                                   const BasePtr_ &original) {
    ASSERT(! original_map_.contains(key));
    OriginalData_ data;
    data.original = original;
    data.count    = 0;
    original_map_[key] = data;

    // Create (empty) storage for the available instances.
    ASSERT(! available_instances_.contains(key));
    available_instances_[key] = AvailableList_();
}

InstanceStore::BasePtr_ InstanceStore::GetAvailableInstance_(
    const std::type_index &key) {
    ASSERT(original_map_.contains(key));
    BasePtr_ instance;
    auto &avail = available_instances_[key];
    if (! avail.empty()) {
        instance = avail.front();
        avail.pop_front();
    }
    return instance;
}

void InstanceStore::MakeAvailable_(const BasePtr_ &instance,
                                     const std::type_index &key) {
    ASSERT(available_instances_.contains(key));
    available_instances_[key].push_front(instance);
}

std::string InstanceStore::CreateName_(const std::type_index &key,
                                         size_t index) {
    return Util::Demangle(key.name()) + "_" + Util::ToString(index);
}

}  //  namespace Parser
