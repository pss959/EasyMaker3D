#pragma once

#include <forward_list>
#include <typeindex>
#include <unordered_map>

#include "Base/Memory.h"
#include "Parser/Object.h"
#include "Util/Assert.h"
#include "Util/General.h"

DECL_SHARED_PTR(InstanceManager);

/// The InstanceManager class manages a set of instances of some sort of
/// Parser::Object. It allows instances to be reused when possible rather than
/// creating new ones all of the time.
///
/// \ingroup Managers
class InstanceManager {
  public:
    /// Resets to original state, clearing all originals and available
    /// instances.
    void Reset();

    /// Stores an original for a specific type of instance. The original is
    /// never used directly; it is used only to create instances of that type.
    template <typename T> void AddOriginal(const std::shared_ptr<T> &original) {
        AddOriginal_(std::type_index(typeid(*original)),
                     Util::CastToBase<Parser::Object>(original));
    }

    /// Returns true if there is an original instance for the given type.
    template <typename T> bool HasOriginal() const {
        return Util::MapContains(original_map_, std::type_index(typeid(T)));
    }

    /// Returns an instance of the templated type of object, creating a new one
    /// if necessary.  Asserts if anything goes wrong.
    template <typename T> std::shared_ptr<T> Acquire() {
        // Check available instances. If none is available, clone a new one.
        std::type_index key(typeid(T));
        std::shared_ptr<T> instance =
            Util::CastToDerived<T>(GetAvailableInstance_(key));
        if (! instance) {
            auto &data = original_map_[key];
            const std::string name = CreateName_(key, data.count++);
            instance = data.original->CloneTyped<T>(true, name);
        }
        return instance;
    }

    /// Releases an instance so it can be reused.
    template <typename T> void Release(const std::shared_ptr<T> &instance) {
        MakeAvailable_(instance, typeid(T));
    }

  private:
    /// Typedef for base type for any instance.
    typedef Parser::ObjectPtr BasePtr_;

    /// Typedef for available instance storage. Use a forward_list to make
    /// insertion and removal easier - no need for random access.
    typedef std::forward_list<BasePtr_> AvailableList_;

    /// Struct storing information for each type of original object.
    struct OriginalData_ {
        BasePtr_ original;   ///< Original object to clone.
        size_t   count = 0;  ///< Number of instances created (for naming).
    };

    /// Storage for the original for each derived class, keyed by type_index.
    std::unordered_map<std::type_index, OriginalData_> original_map_;

    /// Storage for available instances for each derived class, keyed by
    /// type_index.
    std::unordered_map<std::type_index, AvailableList_> available_instances_;

    /// Implements AddOriginal().
    void AddOriginal_(const std::type_index &key, const BasePtr_ &original);

    /// Returns an available instance with the given type key, or a null
    /// pointer if none is available.
    BasePtr_ GetAvailableInstance_(const std::type_index &key);

    /// Adds the given instance to the available list with the type key.
    void MakeAvailable_(const BasePtr_ &instance, const std::type_index &key);

    /// Creates a unique name for an instance of the given type using the index.
    static std::string CreateName_(const std::type_index &key, size_t index);
};
