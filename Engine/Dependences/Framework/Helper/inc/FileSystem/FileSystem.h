//
// Created by Nikita on 16.11.2020.
//

#ifndef HELPER_FILESYSTEM_H
#define HELPER_FILESYSTEM_H

#include <string>

namespace Framework::Helper {
class FileSystem {
public:
    FileSystem() = delete;
    ~FileSystem() = delete;
public:
    static std::string GetFileNameToExeFromFullPath(std::string full_path);
    static std::string GetDirToExeFromFullPath(std::string full_path);

    static bool Delete(const char *file);

    static bool FileExists(const char *file);

    static std::string GetExecutableFileName();
    static std::string GetPathToExe();
    static std::string GetFullPathToExe();
    static void Reload();

    static char* Load(std::string path);

    static const char* FileMapView(std::string path);
    static void UnmapFile(const char* str);
    };
}


#endif //HELPER_FILESYSTEM_H
