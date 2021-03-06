//
// Created by Nikita on 17.11.2020.
//

#ifndef GAMEENGINE_STRINGUTILS_H
#define GAMEENGINE_STRINGUTILS_H

#include <string>
#include <cstring>
#include <glm/glm.hpp>
#include <algorithm>
#include <vector>
#include <locale>

namespace Framework::Helper {
    class StringUtils {
    public:
        StringUtils() = delete;
        StringUtils(StringUtils&) = delete;
        ~StringUtils() = delete;
    public:
        inline static glm::vec3 IntToColor(size_t index) noexcept {
            unsigned char r = ((index >> 16) & 0xFF);  // Extract the RR byte
            unsigned char g = ((index >> 8) & 0xFF);   // Extract the GG byte
            unsigned char b = ((index) & 0xFF);        // Extract the BB byte

            return { r, g, b };
        }

        static inline std::string ReadTo(std::string str, const char c, int offset = 0) {
            size_t size = str.size();
            for (size_t t = 0; t < size; t++){
                if (str[t] == c){
                    str.resize(t + offset + 1);
                    break;
                }
            }
            return str;
        }

        static inline std::string Resize(std::string str, size_t newSize) noexcept {
            str.resize(newSize);
            return str;
        }

        static inline std::string BackSubstring(const std::string& str, char c){
            std::string result;
            for (size_t t = str.size() - 1; t > 0; t--){
                if (str[t] == c)
                    return result;
                else
                    result += str[t];
            }
            return "";
        }

        static std::string GetExtensionFromFilePath(std::string path);
        static std::string Reverse(std::string str);

        static glm::vec2 MakeVec2FromString(const char* source, char chr, unsigned short start);
        static glm::vec3 MakeVec3FromString(const char* source, char chr, unsigned short start);

        inline static unsigned long FastStrLen(const char* str) noexcept {
            unsigned long len = 0;
            while (*str != '\0') {
                len++;
                str++;
            }
            return len;
        }

        inline static char** Split(const char* source, char chr, unsigned short start, unsigned short count_strs) {
            char** strs = new char*[count_strs];
            unsigned char   found_floats = 0;

            unsigned short	count = 0;
            unsigned short	found = start;

            static short t = 0;
            static short len = 0;

            len = FastStrLen(source);

            for (t = start; t < len; t++) {
                count++;

                if (t + 1 == len) {
                    if (found_floats == count_strs)
                        return strs;

                    char* temp = new char[count + 1]; strcpy(temp, "");

                    strncat(temp, source + found, count);

                    strs[found_floats] = temp;

                    return strs;
                }
                else if (source[t] == chr) {
                    if (found_floats + 1 == count_strs)
                        return strs;

                    char* temp = new char[count + 1]; strcpy(temp, "");

                    strncat(temp, source + found, count - 1);

                    strs[found_floats] = temp;
                    found_floats++;

                    found = t + 1;
                    count = 0;
                }
            }
            return nullptr;
        }

        inline static bool Contains(const std::string& str, const std::string& word) noexcept {
            return str.find(word) != std::string::npos;
        }

        inline static float* SplitFloats(const char* source, char chr, unsigned short start, unsigned short count_floats) {
            auto*			floats			= new float[count_floats];
            unsigned char   found_floats	= 0;

            unsigned short	count			= 0;
            unsigned short	found			= start;

            static short t = 0;
            static short len = 0;

            len = (short)FastStrLen(source);

            for (t = start; t < len; t++) {
                count++;

                if (t + 1 == len) {

                    if (found_floats == count_floats)
                        return floats;

                    char* temp = new char[count + 1]; strcpy(temp, "");

                    strncat(temp, source + found, count);

                    floats[found_floats] = (float)atof(temp);

                    delete[] temp;

                    return floats;
                } else if (source[t] == chr) {
                    if (found_floats + 1 == count_floats)
                        return floats;

                    char* temp = new char[count + 1]; strcpy(temp, "");

                    strncat(temp, source + found, count - 1);

                    floats[found_floats] = (float)atof(temp);
                    found_floats++;

                    delete[] temp;

                    found = t + 1;
                    count = 0;
                }
            }
            return nullptr;
        }

        inline static unsigned char MathCount(const char* str, char symb) noexcept {
            unsigned char count = 0;
            while (*str != '\0') {
                if (*str == symb)
                    count++;
                str++;
            }
            return count;
        }

        inline static std::string ReplaceAll(std::string const& original, std::string const& from, std::string const& to) noexcept {
            std::string results;
            std::string::const_iterator end = original.end();
            std::string::const_iterator current = original.begin();
            std::string::const_iterator next = std::search(current, end, from.begin(), from.end());
            while (next != end) {
                results.append(current, next);
                results.append(to);
                current = next + from.size();
                next = std::search(current, end, from.begin(), from.end());
            }
            results.append(current, next);
            return results;
        }
        inline static std::string ToLower(std::string str) noexcept {
            for (char & t : str)
                t = tolower(t);
            return str;
        }
        inline static std::string MakePath(std::string str, bool toLower = true) noexcept {
            str = ReplaceAll(str, "\\\\", "\\");
            str = ReplaceAll(str, "/", "\\");
            if (toLower) str = ToLower(str);
            return str;
        }
        inline static std::string FromCharVector(const std::vector<char>& vs) noexcept {
            std::string result(begin(vs), end(vs));
            return result;
        }
#ifdef WIN32
        inline static const wchar_t* CharsToWchar(const char* str) noexcept {
            const size_t cSize = strlen(str) + 1;
            auto wc = new wchar_t[cSize];
            mbstowcs(wc, str, cSize);
            return wc;
        }
#endif

    };
}

#endif //GAMEENGINE_STRINGUTILS_H
