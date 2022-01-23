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

#ifndef JSWATCHER_H
#define JSWATCHER_H

#include "distributed_objectstore.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "uv_queue.h"
#include "flat_object_store.h"
namespace OHOS::ObjectStore {
enum Event {
    EVENT_UNKNOWN = -1,
    EVENT_CHANGE,
    EVENT_STATUS
};
struct EventHandler {
    napi_ref callbackRef = nullptr;
    EventHandler *next = nullptr;
};
class EventListener {
public:
    EventListener() : type_(nullptr), handlers_(nullptr)
    {
    }
    virtual ~EventListener()
    {
    }
    bool Add(napi_env env, napi_value handler);
    bool Del(napi_env env, napi_value handler);
    void Clear(napi_env env);
    const char *type_;
    EventHandler *handlers_;

private:
    EventHandler *Find(napi_env env, napi_value handler);
};
class JSWatcher : public UvQueue {
public:
    JSWatcher(const napi_env env, DistributedObjectStore *objectStore, DistributedObject *object);

    ~JSWatcher();
    void On(const char *type, napi_value handler);
    void Off(const char *type, napi_value handler = nullptr);
    void Emit(const char *type, const std::string &sessionId, const std::vector<std::string> &changeData);
    Event Find(const char *type) const;

private:
    napi_env env_;
    EventListener listeners_[3];
    DistributedObjectStore *objectStore_;
    DistributedObject *object_;
};

class WatcherImpl : public ObjectWatcher, FlatObjectWatcher  {
public:
    WatcherImpl(JSWatcher *watcher, const std::string &sessionId) : FlatObjectWatcher(sessionId), watcher_(watcher)
    {
    }
    virtual ~WatcherImpl();
    void OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData) override;
    void OnDeleted(const std::string &sessionid) override;

private:
    JSWatcher *watcher_ = nullptr;
};
} // namespace OHOS::ObjectStore

#endif // JSWATCHER_H
