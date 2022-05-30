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
#include "flat_object_storage_engine.h"

#include "logger.h"
#include "objectstore_errors.h"
#include "process_communicator_impl.h"
#include "securec.h"
#include "softbus_adapter.h"
#include "string_utils.h"
#include "types_export.h"

namespace OHOS::ObjectStore {
FlatObjectStorageEngine::~FlatObjectStorageEngine()
{
    if (!isOpened_) {
        return;
    }
    storeManager_ = nullptr;
    LOG_INFO("FlatObjectStorageEngine::~FlatObjectStorageEngine Crash! end");
}

uint32_t FlatObjectStorageEngine::Open(const std::string &bundleName)
{
    if (isOpened_) {
        LOG_INFO("FlatObjectDatabase: No need to reopen it");
        return SUCCESS;
    }
    auto status = DistributedDB::KvStoreDelegateManager::SetProcessLabel("objectstoreDB", bundleName);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("delegate SetProcessLabel failed: %{public}d.", static_cast<int>(status));
        return ERR_DB_SET_PROCESS;
    }

    auto communicator = std::make_shared<ProcessCommunicatorImpl>();
    auto commStatus = DistributedDB::KvStoreDelegateManager::SetProcessCommunicator(communicator);
    if (commStatus != DistributedDB::DBStatus::OK) {
        LOG_ERROR("set distributed db communicator failed.");
        return ERR_DB_SET_PROCESS;
    }
    storeManager_ = std::make_shared<DistributedDB::KvStoreDelegateManager>(bundleName, "default");
    if (storeManager_ == nullptr) {
        LOG_ERROR("FlatObjectStorageEngine::make shared fail");
        return ERR_NOMEM;
    }
    isOpened_ = true;
    LOG_INFO("FlatObjectDatabase::Open Succeed");
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::Close()
{
    if (!isOpened_) {
        LOG_INFO("FlatObjectStorageEngine::Close has been closed!");
        return SUCCESS;
    }
    std::lock_guard<std::mutex> lock(operationMutex_);
    storeManager_ = nullptr;
    isOpened_ = false;
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::CreateTable(const std::string &key)
{
    if (!isOpened_) {
        return ERR_DB_NOT_INIT;
    }
    {
        std::lock_guard<std::mutex> lock(operationMutex_);
        if (delegates_.count(key) != 0) {
            LOG_ERROR("FlatObjectStorageEngine::CreateTable %{public}s already created", key.c_str());
            return ERR_EXIST;
        }
    }

    DistributedDB::KvStoreConfig config;
    config.dataDir = "/data/log";
    storeManager_->SetKvStoreConfig(config);
    DistributedDB::KvStoreNbDelegate *kvStore = nullptr;
    DistributedDB::DBStatus status;
    DistributedDB::KvStoreNbDelegate::Option option = { true, true,
        false }; // createIfNecessary, isMemoryDb, isEncryptedDb
    LOG_INFO("start create table");
    storeManager_->GetKvStore(key, option,
        [&status, &kvStore](DistributedDB::DBStatus dbStatus, DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate) {
            status = dbStatus;
            kvStore = kvStoreNbDelegate;
            LOG_INFO("create table result %{public}d", status);
        });
    bool autoSync = true;
    DistributedDB::PragmaData data = static_cast<DistributedDB::PragmaData>(&autoSync);
    LOG_INFO("start Pragma");
    status = kvStore->Pragma(DistributedDB::AUTO_SYNC, data);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::CreateTable %{public}s getkvstore fail[%{public}d]", key.c_str(), status);
        return ERR_DB_GETKV_FAIL;
    }
    LOG_INFO("create table %{public}s success", key.c_str());
    {
        std::lock_guard<std::mutex> lock(operationMutex_);
        delegates_.insert_or_assign(key, kvStore);
    }
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::GetTable(const std::string &key, std::map<std::string, Value> &result)
{
    if (!isOpened_) {
        LOG_ERROR("not opened %{public}s", key.c_str());
        return ERR_DB_NOT_INIT;
    }
    std::lock_guard<std::mutex> lock(operationMutex_);
    if (delegates_.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %{public}s not exist", key.c_str());
        return ERR_DB_NOT_EXIST;
    }
    result.clear();
    DistributedDB::KvStoreResultSet *resultSet = nullptr;
    Key emptyKey;
    LOG_INFO("start GetEntries");
    DistributedDB::DBStatus status = delegates_.at(key)->GetEntries(emptyKey, resultSet);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %{public}s GetEntries fail", key.c_str());
        return ERR_DB_GET_FAIL;
    }
    LOG_INFO("end GetEntries");
    while (resultSet->IsAfterLast()) {
        DistributedDB::Entry entry;
        status = resultSet->GetEntry(entry);
        if (status != DistributedDB::DBStatus::OK) {
            LOG_INFO("FlatObjectStorageEngine::GetTable  GetEntry fail");
            return ERR_DB_ENTRY_FAIL;
        }
        result.insert_or_assign(StringUtils::BytesToStr(entry.key), entry.value);
        resultSet->MoveToNext();
    }
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::UpdateItem(const std::string &key, const std::string &itemKey, Value &value)
{
    if (!isOpened_) {
        return ERR_DB_NOT_INIT;
    }
    std::lock_guard<std::mutex> lock(operationMutex_);
    if (delegates_.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %{public}s not exist", key.c_str());
        return ERR_DB_NOT_EXIST;
    }
    auto delegate = delegates_.at(key);
    LOG_INFO("start Put");
    auto status = delegate->Put(StringUtils::StrToBytes(itemKey), value);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("%{public}s Put fail[%{public}d]", key.c_str(), status);
        return ERR_CLOSE_STORAGE;
    }
    LOG_INFO("put success");
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::UpdateItems(
    const std::string &key, const std::map<std::string, std::vector<uint8_t>> &data)
{
    if (!isOpened_ || data.size() == 0) {
        return ERR_DB_NOT_INIT;
    }
    std::lock_guard<std::mutex> lock(operationMutex_);
    if (delegates_.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::UpdateItems %{public}s not exist", key.c_str());
        return ERR_DB_NOT_EXIST;
    }

    std::vector<DistributedDB::Entry> entries;
    for (auto &item : data) {
        DistributedDB::Entry entry = { .key = StringUtils::StrToBytes(item.first), .value = item.second };
        entries.emplace_back(entry);
    }
    auto delegate = delegates_.at(key);
    LOG_INFO("start PutBatch");
    auto status = delegate->PutBatch(entries);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("%{public}s PutBatch fail[%{public}d]", key.c_str(), status);
        return ERR_CLOSE_STORAGE;
    }
    LOG_INFO("put success");
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::DeleteTable(const std::string &key)
{
    if (!isOpened_) {
        return ERR_DB_NOT_INIT;
    }
    std::lock_guard<std::mutex> lock(operationMutex_);
    if (delegates_.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %{public}s not exist", key.c_str());
        return ERR_DB_NOT_EXIST;
    }
    LOG_INFO("start DeleteTable %{public}s", key.c_str());
    auto status = storeManager_->CloseKvStore(delegates_.at(key));
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR(
            "FlatObjectStorageEngine::CloseKvStore %{public}s CloseKvStore fail[%{public}d]", key.c_str(), status);
        return ERR_CLOSE_STORAGE;
    }
    LOG_INFO("DeleteTable success");
    delegates_.erase(key);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::GetItem(const std::string &key, const std::string &itemKey, Value &value)
{
    if (!isOpened_) {
        return ERR_DB_NOT_INIT;
    }
    std::lock_guard<std::mutex> lock(operationMutex_);
    if (delegates_.count(key) == 0) {
        LOG_ERROR("FlatObjectStorageEngine::GetItem %{public}s not exist", key.c_str());
        return ERR_DB_NOT_EXIST;
    }
    LOG_INFO("start Get %{public}s", key.c_str());
    DistributedDB::DBStatus status = delegates_.at(key)->Get(StringUtils::StrToBytes(itemKey), value);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::GetItem %{public}s item fail %{public}d", itemKey.c_str(), status);
        return status;
    }
    LOG_INFO("end Get %{public}s", key.c_str());
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::RegisterObserver(const std::string &key, std::shared_ptr<TableWatcher> watcher)
{
    if (!isOpened_) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver kvStore has not init");
        return ERR_DB_NOT_INIT;
    }
    std::lock_guard<std::mutex> lock(operationMutex_);
    if (delegates_.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::RegisterObserver %{public}s not exist", key.c_str());
        return ERR_DB_NOT_EXIST;
    }
    if (observerMap_.count(key) != 0) {
        LOG_INFO("FlatObjectStorageEngine::RegisterObserver observer already exist.");
        return SUCCESS;
    }
    auto delegate = delegates_.at(key);
    std::vector<uint8_t> tmpKey;
    LOG_INFO("start RegisterObserver %{public}s", key.c_str());
    DistributedDB::DBStatus status =
        delegate->RegisterObserver(tmpKey, DistributedDB::ObserverMode::OBSERVER_CHANGES_FOREIGN, watcher.get());
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver watch err %{public}d", status);
        return ERR_REGISTER;
    }
    LOG_INFO("end RegisterObserver %{public}s", key.c_str());
    observerMap_.insert_or_assign(key, watcher);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::UnRegisterObserver(const std::string &key)
{
    if (!isOpened_) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver kvStore has not init");
        return ERR_DB_NOT_INIT;
    }
    std::lock_guard<std::mutex> lock(operationMutex_);
    if (delegates_.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::RegisterObserver %{public}s not exist", key.c_str());
        return ERR_DB_NOT_EXIST;
    }
    auto iter = observerMap_.find(key);
    if (iter == observerMap_.end()) {
        LOG_ERROR("FlatObjectStorageEngine::UnRegisterObserver observer not exist.");
        return ERR_NO_OBSERVER;
    }
    auto delegate = delegates_.at(key);
    std::shared_ptr<TableWatcher> watcher = iter->second;
    LOG_INFO("start UnRegisterObserver %{public}s", key.c_str());
    DistributedDB::DBStatus status = delegate->UnRegisterObserver(watcher.get());
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::UnRegisterObserver unRegister err %{public}d", status);
        return ERR_UNRIGSTER;
    }
    LOG_INFO("end UnRegisterObserver %{public}s", key.c_str());
    observerMap_.erase(key);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::SetStatusNotifier(std::shared_ptr<StatusWatcher> watcher)
{
    if (!isOpened_) {
        LOG_ERROR("FlatObjectStorageEngine::SetStatusNotifier kvStore has not init");
        return ERR_DB_NOT_INIT;
    }
    auto databaseStatusNotifyCallback = [this](std::string userId, std::string appId, std::string storeId,
                                            const std::string deviceId, bool onlineStatus) -> void {
        LOG_INFO("complete");
        if (statusWatcher_ == nullptr) {
            LOG_INFO("FlatObjectStorageEngine::statusWatcher_ null");
            return;
        }
        if (onlineStatus) {
            auto onComplete = [this, storeId](const std::map<std::string, DistributedDB::DBStatus> &devices) {
                for (auto item : devices) {
                    LOG_INFO("%{public}s pull data result %{public}d in device %{public}s", storeId.c_str(),
                        item.second, SoftBusAdapter::GetInstance()->ToNodeID(item.first).c_str());
                }
                if (statusWatcher_ != nullptr) {
                    for (auto item : devices) {
                        statusWatcher_->OnChanged(storeId, SoftBusAdapter::GetInstance()->ToNodeID(item.first),
                            item.second == DistributedDB::OK ? "online" : "offline");
                    }
                }
            };
            SyncAllData(storeId, std::vector<std::string>({ deviceId }), onComplete);
        } else {
            statusWatcher_->OnChanged(storeId, SoftBusAdapter::GetInstance()->ToNodeID(deviceId), "offline");
        }
    };
    storeManager_->SetStoreStatusNotifier(databaseStatusNotifyCallback);
    LOG_INFO("FlatObjectStorageEngine::SetStatusNotifier success");
    statusWatcher_ = watcher;
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::SyncAllData(const std::string &sessionId, const std::vector<std::string> &deviceIds,
    const std::function<void(const std::map<std::string, DistributedDB::DBStatus> &)> &onComplete)
{
    LOG_INFO("start");
    std::lock_guard<std::mutex> lock(operationMutex_);
    if (delegates_.count(sessionId) == 0) {
        LOG_ERROR("FlatObjectStorageEngine::SyncAllData %{public}s already deleted", sessionId.c_str());
        return ERR_DB_NOT_EXIST;
    }
    DistributedDB::KvStoreNbDelegate *kvstore = delegates_.at(sessionId);
    if (deviceIds.empty()) {
        LOG_INFO("single device,no need sync");
        return ERR_SINGLE_DEVICE;
    }
    LOG_INFO("start sync %{public}s", sessionId.c_str());
    DistributedDB::DBStatus status = kvstore->Sync(deviceIds, DistributedDB::SyncMode::SYNC_MODE_PULL_ONLY, onComplete);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::UnRegisterObserver unRegister err %{public}d", status);
        return ERR_UNRIGSTER;
    }
    LOG_INFO("end sync %{public}s", sessionId.c_str());
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::GetItems(const std::string &key, std::map<std::string, std::vector<uint8_t>> &data)
{
    if (!isOpened_) {
        LOG_ERROR("FlatObjectStorageEngine::GetItems %{public}s not init", key.c_str());
        return ERR_DB_NOT_INIT;
    }
    std::lock_guard<std::mutex> lock(operationMutex_);
    if (delegates_.count(key) == 0) {
        LOG_ERROR("FlatObjectStorageEngine::GetItems %{public}s not exist", key.c_str());
        return ERR_DB_NOT_EXIST;
    }
    LOG_INFO("start Get %{public}s", key.c_str());
    std::vector<DistributedDB::Entry> entries;
    DistributedDB::DBStatus status = delegates_.at(key)->GetEntries(StringUtils::StrToBytes(""), entries);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::GetItems item fail status = %{public}d", status);
        return status;
    }
    for (auto &item : entries) {
        data[StringUtils::BytesToStr(item.key)] = item.value;
    }
    LOG_INFO("end Get %{public}s", key.c_str());
    return SUCCESS;
}

void Watcher::OnChange(const DistributedDB::KvStoreChangedData &data)
{
    std::vector<std::string> changedData;
    std::string tmp;
    for (DistributedDB::Entry item : data.GetEntriesInserted()) {
        tmp = StringUtils::BytesToStr(item.key);
        LOG_INFO("inserted %{public}s", tmp.c_str());
        // property key start with p_, 2 is p_ size
        if (tmp.compare(0, FIELDS_PREFIX_LEN, FIELDS_PREFIX) == 0) {
            changedData.push_back(tmp.substr(FIELDS_PREFIX_LEN));
        }
    }
    for (DistributedDB::Entry item : data.GetEntriesUpdated()) {
        tmp = StringUtils::BytesToStr(item.key);
        LOG_INFO("updated %{public}s", tmp.c_str());
        // property key start with p_, 2 is p_ size
        if (tmp.compare(0, FIELDS_PREFIX_LEN, FIELDS_PREFIX) == 0) {
            changedData.push_back(tmp.substr(FIELDS_PREFIX_LEN));
        }
    }
    this->OnChanged(sessionId_, changedData);
}

Watcher::Watcher(const std::string &sessionId) : sessionId_(sessionId)
{
}
} // namespace OHOS::ObjectStore
