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
#include "js_util.h"

#include <endian.h>
#include <securec.h>

#include "logger.h"

namespace OHOS::ObjectStore {
constexpr int32_t STR_MAX_LENGTH = 4096;
constexpr size_t STR_TAIL_LENGTH = 1;

/* napi_value <-> bool */
napi_status JSUtil::GetValue(napi_env env, napi_value in, bool &out)
{
    LOG_DEBUG("napi_value <- bool");
    return napi_get_value_bool(env, in, &out);
}

napi_status JSUtil::SetValue(napi_env env, const bool &in, napi_value &out)
{
    LOG_DEBUG("napi_value -> bool");
    return napi_get_boolean(env, in, &out);
}

/* napi_value <-> double */
napi_status JSUtil::GetValue(napi_env env, napi_value in, double &out)
{
    LOG_DEBUG("napi_value -> double");
    return napi_get_value_double(env, in, &out);
}

napi_status JSUtil::SetValue(napi_env env, const double &in, napi_value &out)
{
    LOG_DEBUG("napi_value <- double");
    return napi_create_double(env, in, &out);
}

/* napi_value <-> std::string */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::string &out)
{
    size_t maxLen = STR_MAX_LENGTH;
    napi_status status = napi_get_value_string_utf8(env, in, NULL, 0, &maxLen);
    if (maxLen <= 0) {
        GET_AND_THROW_LAST_ERROR(env);
        return status;
    }
    char *buf = new (std::nothrow) char[maxLen + STR_TAIL_LENGTH];
    if (buf != nullptr) {
        size_t len = 0;
        status = napi_get_value_string_utf8(env, in, buf, maxLen + STR_TAIL_LENGTH, &len);
        if (status != napi_ok) {
            GET_AND_THROW_LAST_ERROR(env);
        }
        buf[len] = 0;
        out = std::string(buf);
        delete[] buf;
    } else {
        status = napi_generic_failure;
    }
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::string &in, napi_value &out)
{
    LOG_DEBUG("napi_value <- std::string %{public}d", (int)in.length());
    return napi_create_string_utf8(env, in.c_str(), in.size(), &out);
}

/* napi_value <-> std::vector<std::string> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<std::string> &out)
{
    LOG_DEBUG("napi_value -> std::vector<std::string>");
    bool isArray = false;
    napi_is_array(env, in, &isArray);
    LOG_ERROR_RETURN(isArray, "not an array", napi_invalid_arg);

    uint32_t length = 0;
    napi_status status = napi_get_array_length(env, in, &length);
    LOG_ERROR_RETURN((status == napi_ok) && (length > 0), "get_array failed!", napi_invalid_arg);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value item = nullptr;
        status = napi_get_element(env, in, i, &item);
        LOG_ERROR_RETURN((item != nullptr) && (status == napi_ok), "no element", napi_invalid_arg);
        std::string value;
        status = GetValue(env, item, value);
        LOG_ERROR_RETURN(status == napi_ok, "not a string", napi_invalid_arg);
        out.push_back(value);
    }
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::vector<std::string> &in, napi_value &out)
{
    LOG_DEBUG("napi_value <- std::vector<std::string>");
    napi_status status = napi_create_array_with_length(env, in.size(), &out);
    LOG_ERROR_RETURN(status == napi_ok, "create array failed!", status);
    int index = 0;
    for (auto &item : in) {
        napi_value element = nullptr;
        SetValue(env, item, element);
        status = napi_set_element(env, out, index++, element);
        LOG_ERROR_RETURN((status == napi_ok), "napi_set_element failed!", status);
    }
    return status;
}
} // namespace OHOS::ObjectStore
