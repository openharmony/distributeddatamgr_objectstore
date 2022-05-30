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

#include "notifier_impl.h"

#include "logger.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
std::shared_ptr<NotifierImpl> NotifierImpl::GetInstance()
{
    static std::shared_ptr<NotifierImpl> instance;
    static std::mutex instanceLock;
    if (instance == nullptr) {
        std::lock_guard<std::mutex> lockGuard(instanceLock);
        if (instance == nullptr) {
            instance = std::make_shared<NotifierImpl>();
            if (instance == nullptr) {
                LOG_ERROR("Failed to alloc NotifierImpl!");
                return nullptr;
            }
            uint32_t ret = DistributedObjectStore::GetInstance()->SetStatusNotifier(instance);
            if (ret != SUCCESS) {
                LOG_ERROR("SetStatusNotifier %{public}d error", ret);
            } else {
                LOG_INFO("SetStatusNotifier success");
            }
        }
    }
    return instance;
}

void NotifierImpl::AddWatcher(std::string &sessionId, JSWatcher *watcher)
{
    std::lock_guard<std::mutex> lock(mutex_);
    watchers_.insert_or_assign(sessionId, watcher);
}

void NotifierImpl::DelWatcher(std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    watchers_.erase(sessionId);
}

void NotifierImpl::OnChanged(
    const std::string &sessionId, const std::string &networkId, const std::string &onlineStatus)
{
    LOG_INFO(
        "status changed %{public}s %{public}s %{public}s", sessionId.c_str(), networkId.c_str(), onlineStatus.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    if (watchers_.count(sessionId) != 0) {
        LOG_INFO(
            "start emit %{public}s %{public}s %{public}s", sessionId.c_str(), networkId.c_str(), onlineStatus.c_str());
        watchers_.at(sessionId)->Emit("status", sessionId, networkId, onlineStatus);
        LOG_INFO(
            "end emit %{public}s %{public}s %{public}s", sessionId.c_str(), networkId.c_str(), onlineStatus.c_str());
    }
}
NotifierImpl::~NotifierImpl()
{
}
} // namespace OHOS::ObjectStore
