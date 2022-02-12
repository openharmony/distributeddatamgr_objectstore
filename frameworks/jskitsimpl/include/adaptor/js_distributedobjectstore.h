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

#ifndef JS_DISTRIBUTEDDATAOBJECTSTORE_H
#define JS_DISTRIBUTEDDATAOBJECTSTORE_H

#include <list>

#include "distributed_objectstore.h"
#include "js_native_api.h"
#include "js_object_wrapper.h"
#include "node_api.h"
namespace OHOS::ObjectStore {
class JSDistributedObjectStore {
public:
    static napi_value JSCreateObjectSync(napi_env env, napi_callback_info info);
    static napi_value JSDestroyObjectSync(napi_env env, napi_callback_info info);
    static napi_value JSOn(napi_env env, napi_callback_info info);
    static napi_value JSOff(napi_env env, napi_callback_info info);
    static std::string GetBundleName(napi_env env);

private:
    static napi_value NewDistributedObject(
        napi_env env, DistributedObjectStore *objectStore, DistributedObject *object);
    static void AddCallback(napi_env env, std::map<std::string, std::list<napi_ref>> &callbacks,
        const std::string &sessionId, napi_value callback);
    static void DelCallback(napi_env env, std::map<std::string, std::list<napi_ref>> &callbacks,
        const std::string &sessionId, napi_value callback = nullptr);
    static void RestoreWatchers(napi_env env, OHOS::ObjectStore::JSObjectWrapper *wrapper);
};
} // namespace OHOS::ObjectStore
#endif // JS_DISTRIBUTEDDATAOBJECTSTORE_H
