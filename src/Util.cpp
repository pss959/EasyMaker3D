#include "Util.h"

#ifdef __GNUG__
#include <cxxabi.h>  // For demangling.
#endif

#include <string>

using ion::gfx::NodePtr;

namespace Util {

bool StringsEqualNoCase(const std::string &s1, const std::string &s2) {
    auto compare_chars = [](const char &c1, const char &c2){
        return c1 == c2 || std::toupper(c1) == std::toupper(c2);
    };
    return s1.size() == s2.size() &&
        std::equal(s1.begin(), s1.end(), s2.begin(), compare_chars);
}

std::string Demangle(const std::string &mangled_name) {
#ifdef __GNUG__
    int status = 0 ;
    char *demangled = __cxxabiv1::__cxa_demangle(
        mangled_name.c_str(), nullptr, nullptr, &status);
    if (! demangled)
        return mangled_name;
    std::string ret(demangled);
    free(demangled);
    return ret;
#else
    return mangled_name;
#endif
}

static bool SearchForNode_(NodePath &cur_path, const std::string &name) {
    const NodePtr &cur_node = cur_path.back();
    if (cur_node->GetLabel() == name)
        return true;
    for (const auto &kid: cur_node->GetChildren()) {
        cur_path.push_back(kid);
        if (SearchForNode_(cur_path, name))
            return true;
        cur_path.pop_back();
    }
    return false;
}

bool SearchForNode(const ion::gfx::NodePtr &root,
                   const std::string &name, NodePath &path) {
    NodePath cur_path(1, root);
    return SearchForNode_(cur_path, name);
}

}  // namespace Util
