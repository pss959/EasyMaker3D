#include "Managers/NameManager.h"

#include <algorithm>

#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/String.h"

void NameManager::Reset() {
    KLOG('a', "Clearing all names");
    name_set_.clear();
}

void NameManager::Add(const Str &name) {
    KLOG('a', "Adding name '" << name << "'");
    ASSERTM(! Find(name), name);
    name_set_.insert(name);
}

void NameManager::Remove(const Str &name) {
    KLOG('a', "Removing name '" << name << "'");
    ASSERTM(Find(name), name);
    name_set_.erase(name);
}

bool NameManager::Find(const Str &name) const {
    return name_set_.find(name) != name_set_.end();
}

Str NameManager::Create(const Str &prefix) {
    // This is not the most efficient, but that should not matter here.
    int n = 1;
    while (true) {
        Str name = prefix + '_' + Util::ToString(n);
        if (! Find(name))
            return name;
        ++n;
    }
}

Str NameManager::CreateClone(const Str &basename) {
    int i = 0;
    while (true) {
        Str name = basename + "_" + GetLetterSuffix_(i++);
        if (! Find(name))
            return name;
    }
}

StrVec NameManager::GetAllNames() const {
    StrVec names(name_set_.begin(), name_set_.end());
    std::sort(names.begin(), names.end());
    return names;
}

Str NameManager::GetLetterSuffix_(int n) {
    Str letters = "";
    do {
        letters = Str(1, ('A' + (n % 26))) + letters;
        n = n / 26 - 1;
    } while (n >= 0);
    return letters;
}
