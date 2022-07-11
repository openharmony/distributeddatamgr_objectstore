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

#ifndef JS_OBJECT_WRAPPER_H
#define JS_OBJECT_WRAPPER_H

#include <shared_mutex>

#include "distributed_object.h"
#include "distributed_objectstore.h"
#include "js_watcher.h"

namespace OHOS::ObjectStore {
class JSObjectWrapper {
public:
    JSObjectWrapper(DistributedObjectStore *objectStore, DistributedObject *object);
    virtual ~JSObjectWrapper();
    DistributedObject *GetObject();
    bool AddWatch(napi_env env, const char *type, napi_value handler);
    void DeleteWatch(napi_env env, const char *type, napi_value handler = nullptr);
    bool isUndefined(char *value);
    void AddUndefined(char *value);
    void DeleteUndefined(char *value);
    void DestroyObject();

private:
    DistributedObjectStore *objectStore_;
    DistributedObject *object_;
    std::unique_ptr<JSWatcher> watcher_ = nullptr;
    std::shared_mutex watchMutex_{};
    std::vector<std::string> undefinedProperties;
};
} // namespace OHOS::ObjectStore

#endif
