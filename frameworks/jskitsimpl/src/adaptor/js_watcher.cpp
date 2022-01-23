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

#include "js_watcher.h"

#include <cstring>

#include "js_util.h"
#include "logger.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "objectstore_errors.h"
#include "uv.h"

namespace OHOS::ObjectStore {
JSWatcher::JSWatcher(const napi_env env, DistributedObjectStore *objectStore, DistributedObject *object)
    : UvQueue(env), env_(env), objectStore_(objectStore), object_(object)
{
    listeners_[EVENT_CHANGE].type_ = "change";
    listeners_[EVENT_STATUS].type_ = "status";
}

JSWatcher::~JSWatcher()
{
    listeners_[EVENT_CHANGE].Clear(env_);
    listeners_[EVENT_STATUS].Clear(env_);
    if (objectStore_ != nullptr) {
        objectStore_->UnWatch(object_);
    }
}

void JSWatcher::On(const char *type, napi_value handler)
{
    Event event = Find(type);
    if (event == EVENT_UNKNOWN) {
        return;
    }
    LOG_ERROR("add %{public}p", handler);
    bool isEmpty = listeners_[event].Add(env_, handler);
    if (isEmpty && event == EVENT_CHANGE) {
        std::shared_ptr<WatcherImpl> watcher = std::make_shared<WatcherImpl>(this, object_->GetSessionId());
        uint32_t ret = objectStore_->Watch(object_, watcher);
        if (ret != SUCCESS) {
            LOG_ERROR("watch %{public}s error", object_->GetSessionId().c_str());
        } else {
            LOG_INFO("watch %{public}s success", object_->GetSessionId().c_str());
        }
    }
}

void JSWatcher::Off(const char *type, napi_value handler)
{
    Event event = Find(type);
    if (event == EVENT_UNKNOWN) {
        return;
    }
    LOG_INFO("start del %{public}s  %{public}p", object_->GetSessionId().c_str(), handler);
    bool isEmpty = true;
    if (handler == nullptr) {
        listeners_[event].Clear(env_);
    } else {
        isEmpty = listeners_[event].Del(env_, handler);
    }
    if (isEmpty && event == EVENT_CHANGE) {
        std::shared_ptr<WatcherImpl> watcher = std::make_shared<WatcherImpl>(this, object_->GetSessionId());
        uint32_t ret = objectStore_->UnWatch(object_);
        if (ret != SUCCESS) {
            LOG_ERROR("unWatch %{public}s error", object_->GetSessionId().c_str());
        } else {
            LOG_INFO("unWatch %{public}s success", object_->GetSessionId().c_str());
        }
    }
    LOG_INFO("end %{public}s", object_->GetSessionId().c_str());
}

void JSWatcher::Emit(const char *type, const std::string &sessionId, const std::vector<std::string> &changeData)
{
    if (changeData.empty()) {
        LOG_ERROR("empty change");
        return;
    }
    LOG_ERROR("start %{public}s, %{public}s", sessionId.c_str(), changeData.at(0).c_str());
    Event event = Find(type);
    if (event == EVENT_UNKNOWN) {
        LOG_ERROR("unknow %{public}s", type);
        return;
    }
    for (EventHandler *handler = listeners_[event].handlers_; handler != nullptr; handler = handler->next) {
        CallFunction(sessionId, changeData, handler->callbackRef);
    }
}

Event JSWatcher::Find(const char *type) const
{
    Event result = EVENT_UNKNOWN;
    if (!strcmp(listeners_[EVENT_CHANGE].type_, type)) {
        result = EVENT_CHANGE;
    } else if (!strcmp(listeners_[EVENT_STATUS].type_, type)) {
        result = EVENT_STATUS;
    }
    return result;
}

EventHandler *EventListener::Find(napi_env env, napi_value handler)
{
    EventHandler *result = nullptr;
    for (EventHandler *i = handlers_; i != nullptr; i = i->next) {
        napi_value callback = nullptr;
        napi_get_reference_value(env, i->callbackRef, &callback);
        bool isEquals = false;
        napi_strict_equals(env, handler, callback, &isEquals);
        if (isEquals) {
            result = i;
        }
    }
    return result;
}

void EventListener::Clear(napi_env env)
{
    for (EventHandler *i = handlers_; i != nullptr; i = handlers_) {
        handlers_ = i->next;
        napi_delete_reference(env, i->callbackRef);
        delete i;
    }
}

bool EventListener::Del(napi_env env, napi_value handler)
{
    EventHandler *temp = nullptr;
    for (EventHandler *i = handlers_; i != nullptr; i = handlers_) {
        napi_value callback = nullptr;
        napi_get_reference_value(env, i->callbackRef, &callback);
        bool isEquals = false;
        napi_strict_equals(env, handler, callback, &isEquals);
        if (isEquals) {
            if (temp == nullptr) {
                handlers_ = i->next;
            } else {
                temp->next = i->next;
            }
            napi_delete_reference(env, i->callbackRef);
            delete i;
        } else {
            temp = i;
        }
    }
    return handlers_ == nullptr;
}

bool EventListener::Add(napi_env env, napi_value handler)
{
    bool isEmpty = false;
    if (Find(env, handler) != nullptr) {
        LOG_ERROR("has added,return");
        return isEmpty;
    }

    if (handlers_ == nullptr) {
        handlers_ = new EventHandler();
        handlers_->next = nullptr;
        isEmpty = true;
    } else {
        auto temp = new EventHandler();
        temp->next = handlers_;
        handlers_ = temp;
    }
    napi_create_reference(env, handler, 1, &handlers_->callbackRef);
    LOG_INFO("add %{public}p in  %{public}p", handler, handlers_->callbackRef);
    return isEmpty;
}

void WatcherImpl::OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData)
{
    watcher_->Emit("change", sessionid, changedData);
}

void WatcherImpl::OnDeleted(const std::string &sessionid)
{
}

WatcherImpl::~WatcherImpl()
{
}
} // namespace OHOS::ObjectStore
