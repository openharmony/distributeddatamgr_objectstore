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
#include "flat_object_store.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "uv_queue.h"

namespace OHOS::ObjectStore {
class JSWatcher;
struct EventHandler {
    napi_ref callbackRef = nullptr;
    EventHandler *next = nullptr;
};

class EventListener {
public:
    EventListener() : handlers_(nullptr)
    {
    }

    virtual ~EventListener()
    {
    }

    virtual bool Add(napi_env env, napi_value handler);

    virtual bool Del(napi_env env, napi_value handler);

    virtual void Clear(napi_env env);

    EventHandler *Find(napi_env env, napi_value handler);
    EventHandler *handlers_;
};

class ChangeEventListener : public EventListener {
public:
    ChangeEventListener(JSWatcher *watcher, DistributedObjectStore *objectStore, DistributedObject *object);

    bool Add(napi_env env, napi_value handler) override;

    bool Del(napi_env env, napi_value handler) override;

    void Clear(napi_env env) override;

private:
    bool isWatched_ = false;
    DistributedObjectStore *objectStore_;
    DistributedObject *object_;
    JSWatcher *watcher_;
};

class StatusEventListener : public EventListener {
public:
    StatusEventListener(JSWatcher *watcher, const std::string &sessionId);
    bool Add(napi_env env, napi_value handler) override;

    bool Del(napi_env env, napi_value handler) override;

    void Clear(napi_env env) override;

private:
    JSWatcher *watcher_;
    std::string sessionId_;
};

class JSWatcher : public UvQueue {
public:
    JSWatcher(const napi_env env, DistributedObjectStore *objectStore, DistributedObject *object);

    ~JSWatcher();

    void On(const char *type, napi_value handler);

    void Off(const char *type, napi_value handler = nullptr);

    void Emit(const char *type, const std::string &sessionId, const std::vector<std::string> &changeData);

    void Emit(const char *type, const std::string &sessionId, const std::string &networkId, const std::string &status);

private:
    struct ChangeArgs {
        ChangeArgs(const napi_ref callback, const std::string &sessionId, const std::vector<std::string> &changeData);
        napi_ref callback_;
        const std::string sessionId_;
        const std::vector<std::string> changeData_;
    };
    struct StatusArgs {
        StatusArgs(const napi_ref callback, const std::string &sessionId, const std::string &networkId,
            const std::string &status);
        napi_ref callback_;
        const std::string sessionId_;
        const std::string networkId_;
        const std::string status_;
    };
    EventListener *Find(const char *type);
    static void ProcessChange(napi_env env, std::list<void *> &args);
    static void ProcessStatus(napi_env env, std::list<void *> &args);
    napi_env env_;
    ChangeEventListener *changeEventListener_;
    StatusEventListener *statusEventListener_;
};

class WatcherImpl : public ObjectWatcher {
public:
    WatcherImpl(JSWatcher *watcher) : watcher_(watcher)
    {
    }

    virtual ~WatcherImpl();

    void OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData) override;

private:
    JSWatcher *watcher_ = nullptr;
};
} // namespace OHOS::ObjectStore

#endif // JSWATCHER_H
