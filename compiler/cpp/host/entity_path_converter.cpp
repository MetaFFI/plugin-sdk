#include "entity_path_converter.h"
#include <sstream>

namespace metaffi::compiler::cpp {

std::string CppEntityPathConverter::convert(const std::map<std::string, std::string>& entity_path) {
    if (entity_path.empty()) {
        return "";
    }

    // std::map iterates in sorted key order — deterministic output.
    std::ostringstream oss;
    bool first = true;

    for (const auto& [key, value] : entity_path) {
        if (!first) {
            oss << ",";
        }
        oss << key << "=" << value;
        first = false;
    }

    return oss.str();
}

} // namespace metaffi::compiler::cpp
