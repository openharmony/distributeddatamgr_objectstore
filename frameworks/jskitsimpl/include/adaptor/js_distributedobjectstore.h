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

#include "distributed_objectstore.h"
#include "js_native_api.h"
#include "node_api.h"
namespace OHOS::ObjectStore {
constexpr size_t SESSION_ID_SIZE = 32;
class JSDistributedObjectStore {
public:
    static napi_value JSCreateObjectSync(napi_env env, napi_callback_info info);
    static napi_value JSDestroyObjectSync(napi_env env, napi_callback_info info);
    static napi_value JSSync(napi_env env, napi_callback_info info);
    static napi_value JSOn(napi_env env, napi_callback_info info);
    static napi_value JSOff(napi_env env, napi_callback_info info);

private:
    static napi_value NewDistributedObject(
        napi_env env, DistributedObjectStore *objectStore, DistributedObject *object);
};
} // namespace OHOS::ObjectStore
#endif // JS_DISTRIBUTEDDATAOBJECTSTORE_H
