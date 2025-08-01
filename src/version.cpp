#include "version.h"
#include <sstream>
#include <cstring>

namespace JsonStruct {

std::string Version::getVersionString() {
    std::ostringstream oss;
    oss << MAJOR << "." << MINOR << "." << PATCH;
    
    if (std::strlen(SUFFIX) > 0) {
        oss << "-" << SUFFIX;
    }
    
    return oss.str();
}

std::string Version::getDetailedVersionString() {
    std::ostringstream oss;
    oss << "JsonStruct v" << getVersionString();
    
    if (std::strlen(BUILD_DATE) > 0) {
        oss << " (built on " << BUILD_DATE << ")";
    }
    
    if (std::strlen(GIT_COMMIT) > 0) {
        oss << " [" << GIT_COMMIT;
        if (std::strlen(GIT_BRANCH) > 0) {
            oss << " on " << GIT_BRANCH;
        }
        oss << "]";
    }
    
    return oss.str();
}

std::tuple<int, int, int> Version::getVersionTuple() {
    return std::make_tuple(MAJOR, MINOR, PATCH);
}

bool Version::isCompatible(int required_major, int required_minor) {
    // 主版本号必须相同（遵循语义化版本控制）
    if (MAJOR != required_major) {
        return false;
    }
    
    // 次版本号必须大于等于要求的版本（向后兼容）
    return MINOR >= required_minor;
}

int Version::compareVersion(int other_major, int other_minor, int other_patch) {
    if (MAJOR < other_major) return -1;
    if (MAJOR > other_major) return 1;
    
    if (MINOR < other_minor) return -1;
    if (MINOR > other_minor) return 1;
    
    if (PATCH < other_patch) return -1;
    if (PATCH > other_patch) return 1;
    
    return 0; // 相等
}

std::string Version::getBuildDate() {
    return std::string(BUILD_DATE);
}

std::string Version::getGitCommit() {
    return std::string(GIT_COMMIT);
}

std::string Version::getGitBranch() {
    return std::string(GIT_BRANCH);
}

bool Version::isReleaseVersion() {
    return std::strlen(SUFFIX) == 0;
}

std::string Version::toJson() {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"version\": \"" << getVersionString() << "\",\n";
    oss << "  \"major\": " << MAJOR << ",\n";
    oss << "  \"minor\": " << MINOR << ",\n";
    oss << "  \"patch\": " << PATCH << ",\n";
    oss << "  \"suffix\": \"" << SUFFIX << "\",\n";
    oss << "  \"build_date\": \"" << BUILD_DATE << "\",\n";
    oss << "  \"git_commit\": \"" << GIT_COMMIT << "\",\n";
    oss << "  \"git_branch\": \"" << GIT_BRANCH << "\",\n";
    oss << "  \"is_release\": " << (isReleaseVersion() ? "true" : "false") << "\n";
    oss << "}";
    return oss.str();
}

} // namespace JsonStruct
