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

#include <thread>

#include "distributed_object_impl.h"
#include "distributed_objectstore_impl.h"
#include "objectstore_errors.h"
#include "softbus_adapter.h"
#include "string_utils.h"

namespace OHOS::ObjectStore {
DistributedObjectStoreImpl::DistributedObjectStoreImpl(FlatObjectStore *flatObjectStore)
    : flatObjectStore_(flatObjectStore)
{
}

DistributedObjectStoreImpl::~DistributedObjectStoreImpl()
{
    delete flatObjectStore_;
}

DistributedObjectImpl *DistributedObjectStoreImpl::CacheObject(
    const std::string &sessionId, FlatObjectStore *flatObjectStore)
{
    DistributedObjectImpl *object = new (std::nothrow) DistributedObjectImpl(sessionId, flatObjectStore);
    if (object == nullptr) {
        return nullptr;
    }
    std::unique_lock<std::shared_mutex> cacheLock(dataMutex_);
    objects_.push_back(object);
    return object;
}

DistributedObject *DistributedObjectStoreImpl::CreateObject(const std::string &sessionId)
{
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::CreateObject store not opened!");
        return nullptr;
    }
    int32_t status = flatObjectStore_->CreateObject(sessionId);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::CreateObject CreateTable err %{public}d", status);
        return nullptr;
    }
    return CacheObject(sessionId, flatObjectStore_);
}

uint32_t DistributedObjectStoreImpl::DeleteObject(const std::string &sessionId)
{
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECTSTORE;
    }
    uint32_t status = flatObjectStore_->Delete(sessionId);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::DeleteObject store delete err %{public}d", status);
        return status;
    }
    return SUCCESS;
}

uint32_t DistributedObjectStoreImpl::Get(const std::string &sessionId, DistributedObject *object)
{
    auto iter = objects_.begin();
    while (iter != objects_.end()) {
        if ((*iter)->GetSessionId() == sessionId) {
            object = *iter;
            return SUCCESS;
        }
        iter++;
    }
    LOG_ERROR("DistributedObjectStoreImpl::Get object err, no object");
    return ERR_GET_OBJECT;
}

uint32_t DistributedObjectStoreImpl::Watch(DistributedObject *object, std::shared_ptr<ObjectWatcher> watcher)
{
    if (object == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECT;
    }
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECTSTORE;
    }
    if (watchers_.count(object) != 0) {
        LOG_ERROR("DistributedObjectStoreImpl::Watch already gets object");
        return ERR_EXIST;
    }
    std::shared_ptr<WatcherProxy> watcherProxy = std::make_shared<WatcherProxy>(watcher, object->GetSessionId());
    uint32_t status = flatObjectStore_->Watch(object->GetSessionId(), watcherProxy);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::Watch failed %{public}d", status);
        return status;
    }
    watchers_.insert_or_assign(object, watcherProxy);
    LOG_INFO("DistributedObjectStoreImpl:Watch object success.");
    return SUCCESS;
}

uint32_t DistributedObjectStoreImpl::UnWatch(DistributedObject *object)
{
    if (object == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECT;
    }
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECTSTORE;
    }
    uint32_t status = flatObjectStore_->UnWatch(object->GetSessionId());
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::Watch failed %{public}d", status);
        return status;
    }
    watchers_.erase(object);
    LOG_INFO("DistributedObjectStoreImpl:UnWatch object success.");
    return SUCCESS;
}

void DistributedObjectStoreImpl::TriggerSync()
{
}

void DistributedObjectStoreImpl::TriggerRestore(std::function<void()> notifier)
{
    std::thread th = std::thread([=]() {
        bool isFinished = true;
        std::map<std::string, SyncStatus> syncStatus;
        for (auto &item : objects_) {
            syncStatus[item->GetSessionId()] = SYNC_START;
        }
        while (true) {
            {
                std::unique_lock<std::shared_mutex> cacheLock(dataMutex_);
                for (auto &item : objects_) {
                    if (syncStatus[item->GetSessionId()] != SYNC_SUCCESS
                        && syncStatus[item->GetSessionId()] != SYNCING) {
                        auto onComplete = [this, item, &syncStatus](
                                              const std::map<std::string, DistributedDB::DBStatus> &devices) {
                            LOG_INFO("%{public}s pull data", item->GetSessionId().c_str());
                            std::unique_lock<std::shared_mutex> cacheLock(dataMutex_);
                            SyncStatus result = SYNC_SUCCESS;
                            for (auto device : devices) {
                                if (device.second != DistributedDB::OK) {
                                    result = SYNC_FAIL;
                                    LOG_ERROR("%{public}s pull data fail %{public}d in device %{public}s",
                                        item->GetSessionId().c_str(), device.second,
                                        SoftBusAdapter::GetInstance()->ToNodeID(device.first).c_str());
                                }
                            }
                            LOG_INFO("%{public}s pull data success", item->GetSessionId().c_str());
                            syncStatus[item->GetSessionId()] = result;
                        };
                        LOG_INFO("start sync %{public}s", item->GetSessionId().c_str());
                        syncStatus[item->GetSessionId()] = SYNCING;
                        flatObjectStore_->SyncAllData(item->GetSessionId(), onComplete);
                    }
                }
            }

            for (auto &item : syncStatus) {
                if (item.second != SYNC_SUCCESS) {
                    isFinished = false;
                    break;
                }
            }
            if (!isFinished) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        LOG_WARN("restore result");
        notifier();
        LOG_WARN("notify end");
    });
    th.detach();
    return;
}
uint32_t DistributedObjectStoreImpl::SetStatusNotifier(std::shared_ptr<StatusNotifier> notifier)
{
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECTSTORE;
    }
    std::shared_ptr<StatusNotifierProxy> watcherProxy = std::make_shared<StatusNotifierProxy>(notifier);
    uint32_t status = flatObjectStore_->SetStatusNotifier(watcherProxy);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::Watch failed %{public}d", status);
    }
    return status;
}

WatcherProxy::WatcherProxy(const std::shared_ptr<ObjectWatcher> objectWatcher, const std::string &sessionId)
    : FlatObjectWatcher(sessionId), objectWatcher_(objectWatcher)
{
}

void WatcherProxy::OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData)
{
    objectWatcher_->OnChanged(sessionid, changedData);
}

DistributedObjectStore *DistributedObjectStore::GetInstance(const std::string &bundleName)
{
    static char instMemory[sizeof(DistributedObjectStoreImpl)];
    static std::mutex instLock_;
    static DistributedObjectStore *instPtr = nullptr;
    if (instPtr == nullptr) {
        std::lock_guard<std::mutex> lock(instLock_);
        if (instPtr == nullptr && !bundleName.empty()) {
            LOG_INFO("new objectstore %{public}s", bundleName.c_str());
            FlatObjectStore *flatObjectStore = new (std::nothrow) FlatObjectStore(bundleName);
            if (flatObjectStore == nullptr) {
                LOG_ERROR("no memory for FlatObjectStore malloc!");
                return nullptr;
            }
            // Use instMemory to make sure this singleton not free before other object.
            // This operation needn't to malloc memory, we needn't to check nullptr.
            instPtr = new (instMemory) DistributedObjectStoreImpl(flatObjectStore);
        }
    }
    return instPtr;
}

void StatusNotifierProxy::OnChanged(
    const std::string &sessionId, const std::string &networkId, const std::string &onlineStatus)
{
    if (notifier != nullptr) {
        notifier->OnChanged(sessionId, networkId, onlineStatus);
    }
}

StatusNotifierProxy::StatusNotifierProxy(const std::shared_ptr<StatusNotifier> &notifier) : notifier(notifier)
{
}

StatusNotifierProxy::~StatusNotifierProxy()
{
    LOG_ERROR("destroy");
    notifier = nullptr;
}
} // namespace OHOS::ObjectStore
