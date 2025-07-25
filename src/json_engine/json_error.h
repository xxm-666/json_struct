#pragma once
#include <string>
#include <system_error>

namespace JsonStruct {

// Error codes for JSON operations
enum class JsonErrc {
    Success = 0,
    ParseError,
    TypeError,
    OutOfRange,
    DepthExceeded,
    Utf8Error,
    UnexpectedEnd,
    UnexpectedCharacter,
    UnknownError
};

// Category for JsonErrc
class JsonErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override { return "JsonStruct"; }
    std::string message(int c) const override {
        switch (static_cast<JsonErrc>(c)) {
            case JsonErrc::Success:              return "Success";
            case JsonErrc::ParseError:           return "Parse error";
            case JsonErrc::TypeError:            return "Type error";
            case JsonErrc::OutOfRange:           return "Out of range";
            case JsonErrc::DepthExceeded:        return "Nesting depth exceeded";
            case JsonErrc::Utf8Error:            return "UTF-8 error";
            case JsonErrc::UnexpectedEnd:        return "Unexpected end of input";
            case JsonErrc::UnexpectedCharacter:  return "Unexpected character";
            default:                             return "Unknown error";
        }
    }
};

// Get the singleton category instance
inline const std::error_category& jsonErrorCategory() {
    static JsonErrorCategory instance;
    return instance;
}

// Make error_code from JsonErrc
inline std::error_code make_error_code(JsonErrc e) {
    return {static_cast<int>(e), jsonErrorCategory()};
}

} // namespace JsonStruct

// Register JsonErrc as error code enum
namespace std {
template<> struct is_error_code_enum<JsonStruct::JsonErrc> : true_type {};
}
