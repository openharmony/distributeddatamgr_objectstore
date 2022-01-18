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

#include "js_distributedobject.h"

#include <cstring>

#include "js_common.h"
#include "js_object_wrapper.h"
#include "js_util.h"
#include "logger.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
constexpr size_t KEY_SIZE = 64;
static napi_ref *g_instance = nullptr;
napi_value JSDistributedObject::JSConstructor(napi_env env, napi_callback_info info)
{
    LOG_ERROR("start");
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, nullptr, 0, &thisVar, &data);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    return thisVar;
}

// get(key: string): ValueType;
napi_value JSDistributedObject::JSGet(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 1;
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    char key[KEY_SIZE] = { 0 };
    size_t keyLen = 0;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    ASSERT_MATCH_ELSE_RETURN_NULL(argc >= requireArgc);
    status = napi_get_value_string_utf8(env, argv[0], key, KEY_SIZE, &keyLen);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    JSObjectWrapper *wrapper = nullptr;
    status = napi_unwrap(env, thisVar, (void **)&wrapper);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    ASSERT_MATCH_ELSE_RETURN_NULL(wrapper != nullptr);
    napi_value result = nullptr;
    DoGet(env, wrapper, key, result);
    return result;
}

// put(key: string, value: ValueType): void;
napi_value JSDistributedObject::JSPut(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    char key[KEY_SIZE] = { 0 };
    size_t keyLen = 0;
    napi_valuetype valueType;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    ASSERT_MATCH_ELSE_RETURN_NULL(argc >= requireArgc);
    status = napi_typeof(env, argv[0], &valueType);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    CHECK_EQUAL_WITH_RETURN_NULL(valueType, napi_string);
    status = napi_get_value_string_utf8(env, argv[0], key, KEY_SIZE, &keyLen);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    status = napi_typeof(env, argv[1], &valueType);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    JSObjectWrapper *wrapper = nullptr;
    status = napi_unwrap(env, thisVar, (void **)&wrapper);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    ASSERT_MATCH_ELSE_RETURN_NULL(wrapper != nullptr);
    DoPut(env, wrapper, key, valueType, argv[1]);
    LOG_INFO("put %{public}s success", key);
    return nullptr;
}

napi_value JSDistributedObject::GetCons(napi_env env)
{
    napi_value distributedObjectClass = nullptr;
    if (g_instance != nullptr) {
        napi_status status = napi_get_reference_value(env, *g_instance, &distributedObjectClass);
        CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
        return distributedObjectClass;
    }
    const char *distributedObjectName = "DistributedObject";
    napi_property_descriptor distributedObjectDesc[] = {
        DECLARE_NAPI_FUNCTION("put", JSDistributedObject::JSPut),
        DECLARE_NAPI_FUNCTION("get", JSDistributedObject::JSGet),
    };

    napi_status status = napi_define_class(env, distributedObjectName, strlen(distributedObjectName),
        JSDistributedObject::JSConstructor, nullptr, sizeof(distributedObjectDesc) / sizeof(distributedObjectDesc[0]),
        distributedObjectDesc, &distributedObjectClass);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    if (g_instance == nullptr) {
        g_instance = new napi_ref;
        status = napi_create_reference(env, distributedObjectClass, 1, g_instance);
        CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    }
    return distributedObjectClass;
}

void JSDistributedObject::DoPut(
    napi_env env, JSObjectWrapper *wrapper, char *key, napi_valuetype type, napi_value value)
{
    std::string keyString = key;
    switch (type) {
        case napi_boolean: {
            bool putValue = false;
            napi_status status = JSUtil::GetValue(env, value, putValue);
            CHECK_EQUAL_WITH_RETURN_VOID(status, napi_ok);
            wrapper->GetObject()->PutBoolean(keyString, putValue);
            break;
        }
        case napi_number: {
            double putValue = false;
            napi_status status = JSUtil::GetValue(env, value, putValue);
            CHECK_EQUAL_WITH_RETURN_VOID(status, napi_ok);
            wrapper->GetObject()->PutDouble(keyString, putValue);
            break;
        }
        case napi_string: {
            std::string putValue;
            napi_status status = JSUtil::GetValue(env, value, putValue);
            CHECK_EQUAL_WITH_RETURN_VOID(status, napi_ok);
            wrapper->GetObject()->PutString(keyString, putValue);
            break;
        }
        default: {
            LOG_ERROR("error type! %{public}d", type);
            break;
        }
    }
}

void JSDistributedObject::DoGet(napi_env env, JSObjectWrapper *wrapper, char *key, napi_value &value)
{
    std::string keyString = key;
    Type type = TYPE_STRING;
    wrapper->GetObject()->GetType(keyString, type);
    LOG_DEBUG("get type %{public}s %{public}d", key, type);
    switch (type) {
        case TYPE_STRING: {
            std::string result;
            uint32_t ret = wrapper->GetObject()->GetString(keyString, result);
            ASSERT_MATCH_ELSE_RETURN_VOID(ret == SUCCESS)
            napi_status status = JSUtil::SetValue(env, result, value);
            ASSERT_MATCH_ELSE_RETURN_VOID(status == napi_ok)
            break;
        }
        case TYPE_DOUBLE: {
            double result;
            uint32_t ret = wrapper->GetObject()->GetDouble(keyString, result);
            LOG_DEBUG("%{public}f", result);
            ASSERT_MATCH_ELSE_RETURN_VOID(ret == SUCCESS)
            napi_status status = JSUtil::SetValue(env, result, value);
            ASSERT_MATCH_ELSE_RETURN_VOID(status == napi_ok)
            break;
        }
        case TYPE_BOOLEAN: {
            bool result;
            uint32_t ret = wrapper->GetObject()->GetBoolean(keyString, result);
            LOG_DEBUG("%{public}d", result);
            ASSERT_MATCH_ELSE_RETURN_VOID(ret == SUCCESS)
            napi_status status = JSUtil::SetValue(env, result, value);
            ASSERT_MATCH_ELSE_RETURN_VOID(status == napi_ok)
            break;
        }
        default: {
            LOG_ERROR("error type! %{public}d", type);
            break;
        }
    }
}
} // namespace OHOS::ObjectStore