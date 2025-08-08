#include "json_patch.h"
#include "json_filter.h"
#include "json_path.h"

namespace JsonStruct
{
namespace JsonPatch
{
// Helper function to get the parent and key at a JSON Pointer path
static std::pair<JsonStruct::JsonValue*, std::string> getParentAndKeyAtPointer(JsonStruct::JsonValue& root, const std::string& pointer) {
    if (pointer.empty() || pointer == "/") {
        // Cannot get parent of root
        return {nullptr, ""};
    }
    
    // Split the pointer into tokens
    std::vector<std::string> tokens;
    size_t start = 1; // Skip the initial '/'
    size_t pos = pointer.find('/', start);
    
    while (pos != std::string::npos) {
        std::string token = pointer.substr(start, pos - start);
        // Unescape JSON Pointer tokens
        // Replace ~1 with / and ~0 with ~
        size_t tilde_pos = token.find("~");
        while (tilde_pos != std::string::npos) {
            if (tilde_pos + 1 < token.length()) {
                if (token[tilde_pos + 1] == '1') {
                    token.replace(tilde_pos, 2, "/");
                } else if (token[tilde_pos + 1] == '0') {
                    token.replace(tilde_pos, 2, "~");
                }
            }
            tilde_pos = token.find("~", tilde_pos + 1);
        }
        tokens.push_back(token);
        start = pos + 1;
        pos = pointer.find('/', start);
    }
    
    // Add the last token
    std::string lastToken;
    if (start < pointer.length()) {
        lastToken = pointer.substr(start);
        // Unescape JSON Pointer tokens
        size_t tilde_pos = lastToken.find("~");
        while (tilde_pos != std::string::npos) {
            if (tilde_pos + 1 < lastToken.length()) {
                if (lastToken[tilde_pos + 1] == '1') {
                    lastToken.replace(tilde_pos, 2, "/");
                } else if (lastToken[tilde_pos + 1] == '0') {
                    lastToken.replace(tilde_pos, 2, "~");
                }
            }
            tilde_pos = lastToken.find("~", tilde_pos + 1);
        }
    }
    
    // If there's only one token, the parent is the root
    if (tokens.empty()) {
        return {&root, lastToken};
    }
    
    // Navigate through the JSON structure to find the parent
    JsonStruct::JsonValue* current = &root;
    for (const auto& token : tokens) {
        if (current->isObject()) {
            if (!current->contains(token)) {
                return {nullptr, ""}; // Path not found
            }
            current = &(*current)[token];
        } else if (current->isArray()) {
            // Check if token is a valid array index
            try {
                size_t index = std::stoul(token);
                auto* array = current->getArray();
                if (array && index < array->size()) {
                    current = &(*array)[index];
                } else {
                    return {nullptr, ""}; // Invalid index
                }
            } catch (const std::exception&) {
                return {nullptr, ""}; // Not a valid index
            }
        } else {
            return {nullptr, ""}; // Cannot navigate further
        }
    }
    
    return {current, lastToken};
}

JsonStruct::JsonValue ApplyPatch(JsonStruct::JsonValue &target, const JsonStruct::JsonValue &patch)
{
    // According to RFC 7396, a JSON Merge Patch document is a JSON object
    // that describes changes to be made to a target JSON document.
    
    // If the patch is null, replace the entire target with null
    if (patch.isNull()) {
        target = JsonStruct::JsonValue();
        return target;
    }
    
    // Both target and patch must be objects for merge patch to work
    if (!target.isObject() || !patch.isObject()) {
        target = patch;
        return target;
    }
    
    // Get the object representations
    const auto* patchObj = patch.getObject();
    if (!patchObj) {
        // This shouldn't happen if patch.isObject() returned true, but let's be safe
        target = patch;
        return target;
    }
    
    // Process each key-value pair in the patch
    for (const auto& pair : *patchObj) {
        const std::string& key = pair.first;
        const JsonStruct::JsonValue& value = pair.second;
        
        if (value.isNull()) {
            // If the value is null, remove the key from target if it exists
            if (target.contains(key)) {
                target.erase(key);
            }
        } else if (target.contains(key) && target[key].isObject() && value.isObject()) {
            // If both the patch value and target value are objects, recursively apply merge patch
            ApplyPatch(target[key], value);
        } else {
            // Otherwise, replace or add the key-value pair in target
            target[key] = value;
        }
    }
    
    return target;
}
JsonStruct::JsonValue ApplyPatch(JsonStruct::JsonValue &target, const std::string &path,
                    const JsonStruct::JsonValue &patch)
{
    if (path.empty()) {
        throw std::runtime_error("Path cannot be empty");
    }

    if (path.at(0) == '$') {
        // Handle JSONPath syntax
        auto mutableValue = jsonvalue_jsonpath::queryMutable(target, path);
        for (auto &ref : mutableValue.values) {
            if (patch.isNull()) {
                ref.get() = JsonValue(); // Clear the value
            } else {
                ref.get() = patch; // Copy the new value
            }
        }
    } else if (path.at(0) == '/') {
        // Handle JSON Pointer syntax
        if (patch.isNull()) {
            // For null patch, we need to remove the value at the path
            // This requires getting the parent and key
            auto [parent, key] = getParentAndKeyAtPointer(target, path);
            if (parent && parent->isObject()) {
                parent->erase(key);
            } else if (parent && parent->isArray()) {
                size_t index;
                auto [ptr, ec] = std::from_chars(key.data(), key.data() + key.size(), index);
                if (ec == std::errc()) {
                    auto arrayOpt = parent->toArray();
                    if (arrayOpt) {
                        auto& array = arrayOpt->get();
                        if (index < array.size()) {
                            array.erase(array.begin() + index);
                        }
                    }
                } else {
                    throw std::runtime_error("Invalid array index in path: " + path);
                }
            } else {
                throw std::runtime_error("Cannot remove value at path: " + path);
            }
        } else {
            // For non-null patch, we can use the existing at() method
            auto& valuePtr = target.at(path);
            valuePtr = patch;
        }
    } else {
        throw std::runtime_error("Invalid path format. Path must start with '$' for JSONPath or '/' for JSON Pointer");
    }
    
    return target; // Return the modified target
}
} // namespace JsonPatch
} // namespace JsonStruct