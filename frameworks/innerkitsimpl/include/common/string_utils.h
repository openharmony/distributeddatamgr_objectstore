/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "bytes.h"
#include "distributed_object.h"
#include "logger.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
class StringUtils final {
public:
    StringUtils() = delete;
    ~StringUtils() = delete;
    static std::vector<uint8_t> StrToBytes(const std::string &src)
    {
        std::vector<uint8_t> dst;
        dst.resize(src.size());
        dst.assign(src.begin(), src.end());
        return dst;
    }

    static std::string BytesToStr(const std::vector<uint8_t> src)
    {
        std::string result;
        Bytes rstStr(src.begin(), src.end());
        result.assign(reinterpret_cast<char *>(rstStr.data()), rstStr.size());
        return result;
    }
    static uint32_t BytesToStrWithType(Bytes input, std::string &str)
    {
        if (input.end() - input.begin() <= sizeof(Type)) {
            LOG_ERROR("StringUtils:BytesToStrWithType get input len err.");
            return ERR_DATA_LEN;
        }
        std::vector<uint8_t>::const_iterator first = input.begin() + sizeof(Type);
        std::vector<uint8_t>::const_iterator end = input.end();
        Bytes rstStr(first, end);
        str.assign(reinterpret_cast<char *>(rstStr.data()), rstStr.size());
        return SUCCESS;
    }
};
} // namespace OHOS::ObjectStore
#endif // STRING_UTILS_H
