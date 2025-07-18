#include <iostream>
#include <string>
#include "version.h"

using namespace JsonStruct;

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -v, --version     Show short version information" << std::endl;
    std::cout << "  -d, --detailed    Show detailed version information" << std::endl;
    std::cout << "  -j, --json        Output version information in JSON format" << std::endl;
    std::cout << "  -c, --check <ver> Check compatibility with specified version (format: major.minor)" << std::endl;
    std::cout << "  -h, --help        Show this help information" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " --version" << std::endl;
    std::cout << "  " << program_name << " --detailed" << std::endl;
    std::cout << "  " << program_name << " --json" << std::endl;
    std::cout << "  " << program_name << " --check 1.0" << std::endl;
}

bool parse_version(const std::string& version_str, int& major, int& minor) {
    size_t dot_pos = version_str.find('.');
    if (dot_pos == std::string::npos) {
        return false;
    }
    
    try {
        major = std::stoi(version_str.substr(0, dot_pos));
        minor = std::stoi(version_str.substr(dot_pos + 1));
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        // 默认显示简短版本信息
        std::cout << Version::getVersionString() << std::endl;
        return 0;
    }
    
    std::string arg = argv[1];
    
    if (arg == "-h" || arg == "--help") {
        print_usage(argv[0]);
        return 0;
    }
    else if (arg == "-v" || arg == "--version") {
        std::cout << Version::getVersionString() << std::endl;
        return 0;
    }
    else if (arg == "-d" || arg == "--detailed") {
        std::cout << Version::getDetailedVersionString() << std::endl;
        return 0;
    }
    else if (arg == "-j" || arg == "--json") {
        std::cout << Version::toJson() << std::endl;
        return 0;
    }
    else if (arg == "-c" || arg == "--check") {
        if (argc < 3) {
            std::cerr << "错误: --check 选项需要指定版本号" << std::endl;
            print_usage(argv[0]);
            return 1;
        }
        
        int major, minor;
        if (!parse_version(argv[2], major, minor)) {
            std::cerr << "Error: Invalid version format '" << argv[2] << "' (expected: major.minor)" << std::endl;
            return 1;
        }
        
        bool compatible = Version::isCompatible(major, minor);
        std::cout << "Version compatibility check: " << Version::getVersionString() 
                  << " with " << argv[2] << " -> " 
                  << (compatible ? "Compatible" : "Incompatible") << std::endl;
        
        if (!compatible) {
            auto [curr_major, curr_minor, curr_patch] = Version::getVersionTuple();
            if (curr_major != major) {
                std::cout << "Reason: Major version mismatch (current: " << curr_major << ", required: " << major << ")" << std::endl;
            } else if (curr_minor < minor) {
                std::cout << "Reason: Minor version too low (current: " << curr_minor << ", required: " << minor << "+)" << std::endl;
            }
        }
        
        return compatible ? 0 : 1;
    }
    else {
        std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
}
