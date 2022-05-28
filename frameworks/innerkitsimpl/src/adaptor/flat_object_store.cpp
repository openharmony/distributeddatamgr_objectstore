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

#include "flat_object_store.h"

#include "client_adaptor.h"
#include "distributed_objectstore_impl.h"
#include "logger.h"
#include "object_callback.h"
#include "object_service_proxy.h"
#include "objectstore_errors.h"
#include "softbus_adapter.h"

namespace OHOS::ObjectStore {
FlatObjectStore::FlatObjectStore(const std::string &bundleName)
{
    bundleName_ = bundleName;
    storageEngine_ = std::make_shared<FlatObjectStorageEngine>();
    uint32_t status = storageEngine_->Open(bundleName);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to open, error: open storage engine failure %{public}d", status);
    }
    cacheManager_ = std::make_unique<CacheManager>();
}

FlatObjectStore::~FlatObjectStore()
{
    if (storageEngine_ != nullptr) {
        storageEngine_->Close();
        storageEngine_ = nullptr;
    }
    cacheManager_ = nullptr;
}

uint32_t FlatObjectStore::CreateObject(const std::string &sessionId)
{
    if (!storageEngine_->isOpened_) {
        LOG_ERROR("FlatObjectStore::DB has not inited");
        return ERR_DB_NOT_INIT;
    }
    uint32_t status = storageEngine_->CreateTable(sessionId);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::CreateObject createTable err %{public}d", status);
        return status;
    }
    std::function<void(const std::map<std::string, std::vector<uint8_t>> &data)> callback =
        [sessionId, this](
            const std::map<std::string, std::vector<uint8_t>> &data) {
            if (data.size() > 0) {
                LOG_INFO("objectstore, retrieve success");
                auto result = storageEngine_->UpdateItems(sessionId, data);
                if (result != SUCCESS) {
                    LOG_ERROR("UpdateItems failed, status = %{public}d", result);
                }
            } else {
                LOG_INFO("objectstore, retrieve empty");
            }
        };
    cacheManager_->ResumeObject(bundleName_, sessionId, callback);
    return SUCCESS;
}

uint32_t FlatObjectStore::Delete(const std::string &sessionId)
{
    if (!storageEngine_->isOpened_) {
        LOG_ERROR("FlatObjectStore::DB has not inited");
        return ERR_DB_NOT_INIT;
    }
    uint32_t status = storageEngine_->DeleteTable(sessionId);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to delete object %{public}d", status);
        return status;
    }
    return SUCCESS;
}

uint32_t FlatObjectStore::Watch(const std::string &sessionId, std::shared_ptr<FlatObjectWatcher> watcher)
{
    if (!storageEngine_->isOpened_) {
        LOG_ERROR("FlatObjectStore::DB has not inited");
        return ERR_DB_NOT_INIT;
    }
    uint32_t status = storageEngine_->RegisterObserver(sessionId, watcher);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Watch failed %{public}d", status);
    }
    return status;
}

uint32_t FlatObjectStore::UnWatch(const std::string &sessionId)
{
    if (!storageEngine_->isOpened_) {
        LOG_ERROR("FlatObjectStore::DB has not inited");
        return ERR_DB_NOT_INIT;
    }
    uint32_t status = storageEngine_->UnRegisterObserver(sessionId);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Watch failed %{public}d", status);
    }
    return status;
}

uint32_t FlatObjectStore::Put(const std::string &sessionId, const std::string &key, std::vector<uint8_t> value)
{
    if (!storageEngine_->isOpened_) {
        LOG_ERROR("FlatObjectStore::DB has not inited");
        return ERR_DB_NOT_INIT;
    }
    return storageEngine_->UpdateItem(sessionId, key, value);
}

uint32_t FlatObjectStore::Get(std::string &sessionId, const std::string &key, Bytes &value)
{
    if (!storageEngine_->isOpened_) {
        LOG_ERROR("FlatObjectStore::DB has not inited");
        return ERR_DB_NOT_INIT;
    }
    return storageEngine_->GetItem(sessionId, key, value);
}

uint32_t FlatObjectStore::SetStatusNotifier(std::shared_ptr<StatusWatcher> notifier)
{
    if (!storageEngine_->isOpened_) {
        LOG_ERROR("FlatObjectStore::DB has not inited");
        return ERR_DB_NOT_INIT;
    }
    return storageEngine_->SetStatusNotifier(notifier);
}

uint32_t FlatObjectStore::SyncAllData(const std::string &sessionId,
    const std::function<void(const std::map<std::string, DistributedDB::DBStatus> &)> &onComplete)
{
    if (!storageEngine_->isOpened_) {
        LOG_ERROR("FlatObjectStore::DB has not inited");
        return ERR_DB_NOT_INIT;
    }
    std::vector<DeviceInfo> devices = SoftBusAdapter::GetInstance()->GetDeviceList();
    std::vector<std::string> deviceIds;
    for (auto item : devices) {
        deviceIds.push_back(item.deviceId);
    }
    return storageEngine_->SyncAllData(sessionId, deviceIds, onComplete);
}

uint32_t FlatObjectStore::Save(const std::string &sessionId, const std::string &deviceId)
{
    if (cacheManager_ == nullptr) {
        LOG_ERROR("FlatObjectStore::cacheManager_ is null");
        return ERR_NULL_PTR;
    }
    std::map<std::string, std::vector<uint8_t>> objectData;
    uint32_t status = storageEngine_->GetItems(sessionId, objectData);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::GetItems fail");
        return status;
    }
    return cacheManager_->Save(bundleName_, sessionId, deviceId, objectData);
}

uint32_t FlatObjectStore::RevokeSave(const std::string &sessionId)
{
    if (cacheManager_ == nullptr) {
        LOG_ERROR("FlatObjectStore::cacheManager_ is null");
        return ERR_NULL_PTR;
    }
    return cacheManager_->RevokeSave(bundleName_, sessionId);
}

CacheManager::CacheManager()
{
    isProcessing_ = false;
}

uint32_t CacheManager::Save(const std::string &bundleName, const std::string &sessionId, const std::string &deviceId,
    const std::map<std::string, std::vector<uint8_t>> &objectData)
{
    std::unique_lock<std::mutex> lck(mutex_);
    if (isProcessing_) {
        LOG_ERROR("CacheManager::is processiong another cache task");
        return ERR_PROCESSING;
    }
    isProcessing_ = true;
    std::vector<std::string> deviceList = { deviceId };
    uint32_t status = ERR_DB_GET_FAIL;
    SaveObject(bundleName, sessionId, deviceList, objectData,
        [this, &status, &deviceId](const std::map<std::string, int32_t> &results) {
            LOG_INFO("CacheManager::task callback");
            isProcessing_ = false;
            if (results.count(deviceId) != 0) {
                status = results.at(deviceId);
                LOG_INFO("CacheManager::result %{public}d", status);
            }
            condition_.notify_all();
        });
    // wait for operation callback, max wait time is 10s
    condition_.wait_for(lck, std::chrono::seconds(10), [this]() { return !isProcessing_; });
    isProcessing_ = false;
    return status;
}

uint32_t CacheManager::RevokeSave(const std::string &bundleName, const std::string &sessionId)
{
    std::unique_lock<std::mutex> lck(mutex_);
    if (isProcessing_) {
        LOG_ERROR("CacheManager::is processiong another cache task");
        return ERR_PROCESSING;
    }
    isProcessing_ = true;
    int32_t status = ERR_DB_GET_FAIL;
    std::function<void(int32_t)> callback = [this, &status](int32_t result) {
        LOG_INFO("CacheManager::task callback");
        isProcessing_ = false;
        status = result;
        condition_.notify_all();
    };
    RevokeSaveObject(bundleName, sessionId, callback);
    // wait for operation callback, max wait time is 10s
    condition_.wait_for(lck, std::chrono::seconds(10), [this]() { return !isProcessing_; });
    isProcessing_ = false;
    return status;
}

int32_t CacheManager::SaveObject(const std::string &bundleName, const std::string &sessionId,
    const std::vector<std::string> &deviceList, const std::map<std::string, std::vector<uint8_t>> &objectData,
    const std::function<void(const std::map<std::string, int32_t> &)> &callback)
{
    sptr<OHOS::DistributedObject::ObjectServiceProxy> proxy = ClientAdaptor::GetObjectService();
    if (proxy == nullptr) {
        LOG_ERROR("proxy is nullptr.");
        return ERR_NULL_PTR;
    }
    sptr<IObjectSaveCallback> objectSaveCallback = new ObjectSaveCallback(callback);
    int32_t status = proxy->ObjectStoreSave(bundleName, sessionId, deviceList, objectData, objectSaveCallback);
    if (status != SUCCESS) {
        LOG_ERROR("object save failed code=%d.", static_cast<int>(status));
    }
    LOG_INFO("object save successful");
    return status;
}

int32_t CacheManager::RevokeSaveObject(
    const std::string &bundleName, const std::string &sessionId, std::function<void(int32_t)> &callback)
{
    sptr<OHOS::DistributedObject::ObjectServiceProxy> proxy = ClientAdaptor::GetObjectService();
    if (proxy == nullptr) {
        LOG_ERROR("proxy is nullptr.");
        return ERR_NULL_PTR;
    }
    sptr<IObjectRevokeSaveCallback> objectRevokeSaveCallback = new ObjectRevokeSaveCallback(callback);
    int32_t status = proxy->ObjectStoreRevokeSave(bundleName, sessionId, objectRevokeSaveCallback);
    if (status != SUCCESS) {
        LOG_ERROR("object revoke save failed code=%d.", static_cast<int>(status));
    }
    LOG_INFO("object revoke save successful");
    return status;
}

int32_t CacheManager::ResumeObject(const std::string &bundleName, const std::string &sessionId,
    std::function<void(const std::map<std::string, std::vector<uint8_t>> &data)> &callback)
{
    sptr<OHOS::DistributedObject::ObjectServiceProxy> proxy = ClientAdaptor::GetObjectService();
    if (proxy == nullptr) {
        LOG_ERROR("proxy is nullptr.");
        return ERR_NULL_PTR;
    }
    sptr<IObjectRetrieveCallback> objectRevokeSaveCallback = new ObjectRetrieveCallback(callback);
    int32_t status = proxy->ObjectStoreRetrieve(bundleName, sessionId, objectRevokeSaveCallback);
    if (status != SUCCESS) {
        LOG_ERROR("object resume failed code=%d.", static_cast<int>(status));
    }
    LOG_INFO("object resume successful");
    return status;
}
} // namespace OHOS::ObjectStore
