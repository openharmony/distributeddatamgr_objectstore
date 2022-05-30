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

#ifndef JS_DISTRIBUTEDOBJECT_H
#define JS_DISTRIBUTEDOBJECT_H

#include <js_native_api.h>

#include "distributed_objectstore.h"
#include "js_object_wrapper.h"
namespace OHOS::ObjectStore {
struct ConstructContext {
    DistributedObjectStore *objectStore;
    DistributedObject *object;
};

class JSDistributedObject {
public:
    static napi_value JSConstructor(napi_env env, napi_callback_info info);
    static napi_value JSGet(napi_env env, napi_callback_info info);
    static napi_value JSPut(napi_env env, napi_callback_info info);
    static napi_value JSSave(napi_env env, napi_callback_info info);
    static napi_value JSRevokeSave(napi_env env, napi_callback_info info);
    static napi_value GetCons(napi_env env);

private:
    static void DoPut(napi_env env, JSObjectWrapper *wrapper, char *key, napi_valuetype type, napi_value value);
    static void DoGet(napi_env env, JSObjectWrapper *wrapper, char *key, napi_value &value);
    static napi_value GetSaveResultCons(napi_env env, std::string &sessionId, double version, std::string deviceId);
    static napi_value GetRevokeSaveResultCons(napi_env env, std::string &sessionId);
};
} // namespace OHOS::ObjectStore

#endif
