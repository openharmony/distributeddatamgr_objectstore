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

#include "distributed_objectstore_impl.h"
#include "logger.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
FlatObjectStore::FlatObjectStore(const std::string &bundleName)
{
    storageEngine_ = std::make_shared<FlatObjectStorageEngine>();
    uint32_t status = storageEngine_->Open(bundleName);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to open, error: open storage engine failure %{public}d", status);
    }
}

FlatObjectStore::~FlatObjectStore()
{
    if (storageEngine_ != nullptr) {
        storageEngine_->Close();
        storageEngine_ = nullptr;
    }
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
void FlatObjectStore::SyncAllData(
    const std::string &sessionId, const std::function<void(const std::map<std::string, DistributedDB::DBStatus> &)> &onComplete)
{
    if (!storageEngine_->isOpened_) {
        LOG_ERROR("FlatObjectStore::DB has not inited");
        return;
    }
    return storageEngine_->SyncAllData(sessionId, onComplete);
}
} // namespace OHOS::ObjectStore
