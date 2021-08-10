#include "Managers/NameManager.h"

#include <assert.h>

#include "Util.h"

void NameManager::Reset() {
    name_set_.clear();
}

void NameManager::Add(const std::string &name) {
    assert(! Find(name));
    name_set_.insert(name);
}

void NameManager::Remove(const std::string &name) {
    assert(Find(name));
    name_set_.erase(name);
}

bool NameManager::Find(const std::string &name) const {
    return name_set_.find(name) != name_set_.end();
}

std::string NameManager::Create(const std::string &prefix) {
    // This is not the most efficient, but that should not matter here.
    int n = 1;
    while (true) {
        std::string name = prefix + '_' + Util::ToString(n);
        if (! Find(name)) {
            name_set_.insert(name);
            return name;
        }
        ++n;
    }
}

std::string NameManager::CreateClone(const std::string &basename) {
    int i = 0;
    while (true) {
        std::string name = basename + "_" + GetLetterSuffix_(i++);
        if (! Find(name)) {
            name_set_.insert(name);
            return name;
        }
    }
}

std::vector<std::string> NameManager::GetAllNames() const {
    std::vector<std::string> names(name_set_.begin(), name_set_.end());
    std::sort(names.begin(), names.end());
    return names;
}

std::string NameManager::GetLetterSuffix_(int n) {
    std::string letters = "";
    do {
        letters = std::string(1, ('A' + (n % 26))) + letters;
        n = n / 26 - 1;
    } while (n >= 0);
    return letters;
}
