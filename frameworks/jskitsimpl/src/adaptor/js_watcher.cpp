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

#include "js_common.h"
#include "js_notifier_impl.h"
#include "js_util.h"
#include "logger.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
JSWatcher::JSWatcher(const napi_env env, DistributedObjectStore *objectStore, DistributedObject *object)
    : UvQueue(env), env_(env)
{
    changeEventListener_ = new ChangeEventListener(this, objectStore, object);
    statusEventListener_ = new StatusEventListener(this, object->GetSessionId());
}

JSWatcher::~JSWatcher()
{
    delete changeEventListener_;
    delete statusEventListener_;
    changeEventListener_ = nullptr;
    statusEventListener_ = nullptr;
}

void JSWatcher::On(const char *type, napi_value handler)
{
    EventListener *listener = Find(type);
    if (listener == nullptr) {
        LOG_ERROR("error type %{public}s", type);
        return;
    }
    listener->Add(env_, handler);
}

void JSWatcher::Off(const char *type, napi_value handler)
{
    EventListener *listener = Find(type);
    if (listener == nullptr) {
        LOG_ERROR("error type %{public}s", type);
        return;
    }
    if (handler == nullptr) {
        listener->Clear(env_);
    } else {
        listener->Del(env_, handler);
    }
}
void JSWatcher::ProcessChange(napi_env env, std::list<void *> &args)
{
    constexpr static int8_t ARGV_SIZE = 2;
    napi_value callback = nullptr;
    napi_value global = nullptr;
    napi_value param[ARGV_SIZE];
    napi_value result;
    napi_status status = napi_get_global(env, &global);
    ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
    for (auto item : args) {
        ChangeArgs *changeArgs = static_cast<ChangeArgs *>(item);
        status = napi_get_reference_value(env, changeArgs->callback_, &callback);
        ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
        status = JSUtil::SetValue(env, changeArgs->sessionId_, param[0]);
        ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
        JSUtil::SetValue(env, changeArgs->changeData_, param[1]);
        ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
        LOG_INFO("start %{public}s, %{public}zu", changeArgs->sessionId_.c_str(), changeArgs->changeData_.size());
        status = napi_call_function(env, global, callback, ARGV_SIZE, param, &result);
        LOG_INFO("end %{public}s, %{public}zu", changeArgs->sessionId_.c_str(), changeArgs->changeData_.size());
        ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
    }
ERROR:
    for (auto item : args) {
        ChangeArgs *changeArgs = static_cast<ChangeArgs *>(item);
        delete changeArgs;
    }
    args.clear();
}
void JSWatcher::Emit(const char *type, const std::string &sessionId, const std::vector<std::string> &changeData)
{
    if (changeData.empty()) {
        LOG_ERROR("empty change");
        return;
    }
    LOG_INFO("start %{public}s, %{public}s", sessionId.c_str(), changeData.at(0).c_str());
    EventListener *listener = Find(type);
    if (listener == nullptr) {
        LOG_ERROR("error type %{public}s", type);
        return;
    }

    for (EventHandler *handler = listener->handlers_; handler != nullptr; handler = handler->next) {
        ChangeArgs *changeArgs = new ChangeArgs(handler->callbackRef, sessionId, changeData);
        CallFunction(ProcessChange, changeArgs);
    }
}

EventListener *JSWatcher::Find(const char *type)
{
    if (!strcmp("change", type)) {
        return changeEventListener_;
    }
    if (!strcmp("status", type)) {
        return statusEventListener_;
    }
    return nullptr;
}

void JSWatcher::ProcessStatus(napi_env env, std::list<void *> &args)
{
    constexpr static int8_t ARGV_SIZE = 3;
    napi_value callback = nullptr;
    napi_value global = nullptr;
    napi_value param[ARGV_SIZE];
    napi_value result;
    napi_status status = napi_get_global(env, &global);
    ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
    for (auto item : args) {
        StatusArgs *statusArgs = static_cast<StatusArgs *>(item);
        status = napi_get_reference_value(env, statusArgs->callback_, &callback);
        ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
        status = JSUtil::SetValue(env, statusArgs->sessionId_, param[0]);
        ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
        JSUtil::SetValue(env, statusArgs->networkId_, param[1]);
        ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
        JSUtil::SetValue(env, statusArgs->status_, param[2]);
        ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
        LOG_INFO("start %{public}s, %{public}s, %{public}s", statusArgs->sessionId_.c_str(),
            statusArgs->networkId_.c_str(), statusArgs->status_.c_str());
        status = napi_call_function(env, global, callback, ARGV_SIZE, param, &result);
        LOG_INFO("end %{public}s, %{public}s, %{public}s", statusArgs->sessionId_.c_str(),
            statusArgs->networkId_.c_str(), statusArgs->status_.c_str());
        ASSERT_MATCH_ELSE_GOTO_ERROR(status == napi_ok);
    }
ERROR:
    LOG_DEBUG("do clear");
    for (auto item : args) {
        StatusArgs *statusArgs = static_cast<StatusArgs *>(item);
        delete statusArgs;
    }
    args.clear();
}

void JSWatcher::Emit(
    const char *type, const std::string &sessionId, const std::string &networkId, const std::string &status)
{
    if (sessionId.empty() || networkId.empty()) {
        LOG_ERROR("empty %{public}s  %{public}s", sessionId.c_str(), networkId.c_str());
        return;
    }
    LOG_ERROR("status change %{public}s  %{public}s", sessionId.c_str(), networkId.c_str());
    EventListener *listener = Find(type);
    if (listener == nullptr) {
        LOG_ERROR("error type %{public}s", type);
        return;
    }

    for (EventHandler *handler = listener->handlers_; handler != nullptr; handler = handler->next) {
        StatusArgs *changeArgs = new StatusArgs(handler->callbackRef, sessionId, networkId, status);
        CallFunction(ProcessStatus, changeArgs);
    }
    return;
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
    if (watcher_ == nullptr) {
        LOG_ERROR("watcher_ is null");
        return;
    }
    watcher_->Emit("change", sessionid, changedData);
}

WatcherImpl::~WatcherImpl()
{
    LOG_ERROR("destroy");
    watcher_ = nullptr;
}

bool ChangeEventListener::Add(napi_env env, napi_value handler)
{
    if (!isWatched_ && object_ != nullptr) {
        std::shared_ptr<WatcherImpl> watcher = std::make_shared<WatcherImpl>(watcher_);
        uint32_t ret = objectStore_->Watch(object_, watcher);
        if (ret != SUCCESS) {
            LOG_ERROR("Watch %{public}s error", object_->GetSessionId().c_str());
        } else {
            LOG_INFO("Watch %{public}s success", object_->GetSessionId().c_str());
            isWatched_ = true;
        }
    }
    return EventListener::Add(env, handler);
}

bool ChangeEventListener::Del(napi_env env, napi_value handler)
{
    bool isEmpty = EventListener::Del(env, handler);
    if (isEmpty && isWatched_ && object_ != nullptr) {
        uint32_t ret = objectStore_->UnWatch(object_);
        if (ret != SUCCESS) {
            LOG_ERROR("UnWatch %{public}s error", object_->GetSessionId().c_str());
        } else {
            LOG_INFO("UnWatch %{public}s success", object_->GetSessionId().c_str());
            isWatched_ = false;
        }
    }
    return isEmpty;
}

void ChangeEventListener::Clear(napi_env env)
{
    EventListener::Clear(env);
    if (isWatched_ && object_ != nullptr) {
        uint32_t ret = objectStore_->UnWatch(object_);
        if (ret != SUCCESS) {
            LOG_ERROR("UnWatch %{public}s error", object_->GetSessionId().c_str());
        } else {
            LOG_INFO("UnWatch %{public}s success", object_->GetSessionId().c_str());
            isWatched_ = false;
        }
    }
}

ChangeEventListener::ChangeEventListener(
    JSWatcher *watcher, DistributedObjectStore *objectStore, DistributedObject *object)
    : objectStore_(objectStore), object_(object), watcher_(watcher)
{
}

bool StatusEventListener::Add(napi_env env, napi_value handler)
{
    NotifierImpl::GetInstance()->AddWatcher(sessionId_, watcher_);
    return EventListener::Add(env, handler);
}

bool StatusEventListener::Del(napi_env env, napi_value handler)
{
    NotifierImpl::GetInstance()->DelWatcher(sessionId_);
    return EventListener::Del(env, handler);
}

void StatusEventListener::Clear(napi_env env)
{
    NotifierImpl::GetInstance()->DelWatcher(sessionId_);
    EventListener::Clear(env);
}

StatusEventListener::StatusEventListener(JSWatcher *watcher, const std::string &sessionId)
    : watcher_(watcher), sessionId_(sessionId)
{
}

JSWatcher::ChangeArgs::ChangeArgs(
    const napi_ref callback, const std::string &sessionId, const std::vector<std::string> &changeData)
    : callback_(callback), sessionId_(sessionId), changeData_(changeData)
{
}

JSWatcher::StatusArgs::StatusArgs(
    const napi_ref callback, const std::string &sessionId, const std::string &networkId, const std::string &status)
    : callback_(callback), sessionId_(sessionId), networkId_(networkId), status_(status)
{
}
} // namespace OHOS::ObjectStore
