#pragma once

#include <map>
#include <string>

namespace metaffi::compiler::cpp {

/**
 * Serialises an IDL entity_path map to the comma-separated "key=val,..." string
 * expected by MetaFFI's load_entity_with_info().
 *
 * Keys are emitted in lexicographic order (std::map is sorted).
 * Empty map produces an empty string.
 *
 * No runtime-specific logic — pure map serialisation.
 */
class CppEntityPathConverter {
public:
    // Convert a sorted entity-path map to "key=val,key2=val2" string.
    static std::string convert(const std::map<std::string, std::string>& entity_path);
};

} // namespace metaffi::compiler::cpp
