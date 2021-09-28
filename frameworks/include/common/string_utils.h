/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <initializer_list>
#include <sstream>
#include <string>
#include <vector>

namespace OHOS::ObjectStore {
class StringUtils final {
public:
    StringUtils() = delete;
    ~StringUtils() = delete;

    static std::string Trim(const std::string &strBase, const char cTrim = ' ')
    {
        std::string strTmp = strBase;
        strTmp.erase(0, strTmp.find_first_not_of(cTrim));
        strTmp.erase(strTmp.find_last_not_of(cTrim) + sizeof(char));
        return strTmp;
    }

    static void Split(const std::string &strBase, const std::string &strSep, std::vector<std::string> &strsRet,
                      bool bCanEmpty = false, bool bNeedTrim = true)
    {
        strsRet.clear();
        std::string strTmp = bNeedTrim ? Trim(strBase) : strBase;
        std::string strPart;
        while (true) {
            std::string::size_type pos = strTmp.find(strSep);
            if (std::string::npos == pos || strSep.empty()) {
                strPart = bNeedTrim ? Trim(strTmp) : strTmp;
                if (!strPart.empty() || bCanEmpty) {
                    strsRet.push_back(strPart);
                }
                break;
            } else {
                strPart = bNeedTrim ? Trim(strTmp.substr(0, pos)) : strTmp.substr(0, pos);
                if (!strPart.empty() || bCanEmpty) {
                    strsRet.push_back(strPart);
                }
                strTmp = strTmp.substr(strSep.size() + pos, strTmp.size() - strSep.size() - pos);
            }
        }
    }

    static std::string Join(const std::initializer_list<std::string> &items)
    {
        std::stringstream stream;
        for (const std::string &item : items) {
            stream << item;
        }
        return stream.str();
    }

    static std::string Join(const std::string &separator, const std::initializer_list<std::string> &items)
    {
        std::stringstream stream;
        for (const std::string &item : items) {
            if (stream.str().empty()) {
                stream << item;
            } else {
                stream << separator << item;
            }
        }
        return stream.str();
    }

    static bool StrToUint64(const std::string &str, uint64_t &value)
    {
        if (str.empty() || (str.front() == '-')) {
            return false;
        }
        char *end = nullptr;
        errno = 0;
        auto addr = str.c_str();
        auto result = strtoull(addr, &end, 10);  // 10 means decimalism
        if ((end == nullptr) || (end == addr) || (end[0] != '\0') || (errno == ERANGE)) {
            return false;
        }
        value = static_cast<uint64_t>(result);
        return (result == value);
    }

    static bool StrToInt32(const std::string &str, int32_t &value)
    {
        if (str.empty()) {
            return false;
        }
        char *end = nullptr;
        errno = 0;
        auto addr = str.c_str();
        auto result = strtol(addr, &end, 10);  // 10 means decimalism
        if ((end == nullptr) || (end == addr) || (end[0] != '\0') || (errno == ERANGE)) {
            return false;
        }
        value = static_cast<int32_t>(result);
        return (result == value);
    }

    static bool StrToUint32(const std::string &str, uint32_t &value)
    {
        if (str.empty() || (str.front() == '-')) {
            return false;
        }
        char *end = nullptr;
        errno = 0;
        auto addr = str.c_str();
        auto result = strtoul(addr, &end, 10);  // 10 means decimalism
        if ((end == nullptr) || (end == addr) || (end[0] != '\0') || (errno == ERANGE)) {
            return false;
        }
        value = static_cast<uint32_t>(result);
        return (result == value);
    }
    
    static std::vector<uint8_t> StrToBytes(const std::string &src)
    {
        std::vector<uint8_t> dst;
        dst.resize(src.size());
        dst.assign(src.begin(), src.end());
        return dst;
    }
};
}  // namespace OHOS::ObjectStore
#endif  // STRING_UTILS_H
