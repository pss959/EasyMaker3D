#include "Managers/InstanceManager.h"

#include "Util/String.h"

void InstanceManager::Reset() {
    original_map_.clear();
    available_instances_.clear();
}

void InstanceManager::AddOriginal_(const std::type_index &key,
                                   const BasePtr_ &original) {
    ASSERT(! Util::MapContains(original_map_, key));
    OriginalData_ data;
    data.original = original;
    data.count    = 0;
    original_map_[key] = data;

    // Create (empty) storage for the available instances.
    ASSERT(! Util::MapContains(available_instances_, key));
    available_instances_[key] = AvailableList_();
}

InstanceManager::BasePtr_ InstanceManager::GetAvailableInstance_(
    const std::type_index &key) {
    ASSERT(Util::MapContains(original_map_, key));
    BasePtr_ instance;
    auto &avail = available_instances_[key];
    if (! avail.empty()) {
        instance = avail.front();
        avail.pop_front();
    }
    return instance;
}

void InstanceManager::MakeAvailable_(const BasePtr_ &instance,
                                     const std::type_index &key) {
    ASSERT(Util::MapContains(available_instances_, key));
    available_instances_[key].push_front(instance);
}

std::string InstanceManager::CreateName_(const std::type_index &key,
                                         size_t index) {
    return Util::Demangle(key.name()) + "_" + Util::ToString(index);
}
