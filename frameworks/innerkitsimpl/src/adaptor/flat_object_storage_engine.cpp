/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "securec.h"
#include "string_utils.h"
#include "process_communicator_impl.h"

namespace OHOS::ObjectStore {
FlatObjectStorageEngine::~FlatObjectStorageEngine()
{
    if (!opened_) {
        return;
    }
    storeManager_ = nullptr;
    LOG_INFO("FlatObjectStorageEngine::~FlatObjectStorageEngine Crash! end");
}

uint32_t FlatObjectStorageEngine::Open()
{
    if (opened_) {
        LOG_INFO("FlatObjectDatabase: No need to reopen it");
        return SUCCESS;
    }
    auto status = DistributedDB::KvStoreDelegateManager::SetProcessLabel("objectstoreDB", "default");
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("delegate SetProcessLabel failed: %{public}d.", static_cast<int>(status));
        return SUCCESS;
    }

    auto communicator = std::make_shared<ProcessCommunicatorImpl>();
    auto commStatus = DistributedDB::KvStoreDelegateManager::SetProcessCommunicator(communicator);
    if (commStatus != DistributedDB::DBStatus::OK) {
        LOG_ERROR("set distributed db communicator failed.");
        return SUCCESS;
    }
    storeManager_ = std::make_shared<DistributedDB::KvStoreDelegateManager>("objectstore", "default");
    if (storeManager_ == nullptr) {
        LOG_ERROR("FlatObjectStorageEngine::make shared fail");
        return ERR_MOMEM;
    }
    opened_ = true;
    LOG_INFO("FlatObjectDatabase::Open Succeed");
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::Close()
{
    if (!opened_) {
        LOG_INFO("FlatObjectStorageEngine::Close has been closed!");
        return SUCCESS;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    storeManager_ = nullptr;
    opened_ = false;
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::CreateTable(const std::string &key)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates.count(key) != 0) {
        LOG_INFO("FlatObjectStorageEngine::CreateTable %s already created", key.c_str());
        return SUCCESS;
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
        LOG_ERROR("FlatObjectStorageEngine::CreateTable %s getkvstore fail[%d]", key.c_str(), status);
        return ERR_DE_GETKV_FAIL;
    }
    LOG_INFO("create table %{public}s success", key.c_str());
    delegates.insert_or_assign(key, kvStore);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::GetTable(const std::string &key, std::map<std::string, Value> &result)
{
    if (!opened_) {
        LOG_ERROR("not opened %{public}s", key.c_str());
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %{public}s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    result.clear();
    DistributedDB::KvStoreResultSet *resultSet = nullptr;
    Key emptyKey;
    LOG_INFO("start GetEntries");
    DistributedDB::DBStatus status = delegates.at(key)->GetEntries(emptyKey, resultSet);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %s GetEntries fail", key.c_str());
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

uint32_t FlatObjectStorageEngine::UpdateItems(const std::string &key, std::map<std::string, Value> &data)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    auto iter = data.begin();
    std::vector<DistributedDB::Entry> items;
    auto delegate = delegates.at(key);
    while (iter != data.end()) {
        DistributedDB::Entry entry;
        entry.key = StringUtils::StrToBytes(iter->first);
        entry.value = iter->second;
        items.insert(items.end(), entry);
        iter++;
    }
    LOG_INFO("start PutBatch");
    auto status = delegate->PutBatch(items);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("%s PutBatch fail[%d]", key.c_str(), status);
        return ERR_CLOSE_STORAGE;
    }
    LOG_INFO("end PutBatch");
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::UpdateItem(const std::string &key, const std::string &itemKey, Value &value) {
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    auto delegate = delegates.at(key);
    LOG_INFO("start Put");
    auto status = delegate->Put(StringUtils::StrToBytes(itemKey), value);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("%s PutBatch fail[%d]", key.c_str(), status);
        return ERR_CLOSE_STORAGE;
    }
    LOG_INFO("end Put");
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::DeleteTable(const std::string &key)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    LOG_INFO("start DeleteTable %{public}s", key.c_str());
    auto status = storeManager_->CloseKvStore(delegates.at(key));
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::CloseKvStore %s CloseKvStore fail[%d]", key.c_str(), status);
        return ERR_CLOSE_STORAGE;
    }
    LOG_INFO("end DeleteTable");
    delegates.erase(key);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::GetItem(const std::string &key, const std::string &itemKey, Value &value)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates.count(key) == 0) {
        LOG_ERROR("FlatObjectStorageEngine::GetItem %{public}s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    LOG_INFO("start Get %{public}s", key.c_str());
    DistributedDB::DBStatus status = delegates.at(key)->Get(StringUtils::StrToBytes(itemKey), value);
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::GetItem %{public}s item fail %{public}d", itemKey.c_str(), status);
        return status;
    }
    LOG_INFO("end Get %{public}s", key.c_str());
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::RegisterObserver(const std::string &key, std::shared_ptr<TableWatcher> watcher)
{
    if (!opened_) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver kvStore has not init");
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::RegisterObserver %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    if (observerMap_.count(key) != 0) {
        LOG_INFO("FlatObjectStorageEngine::RegisterObserver observer already exist.");
        return SUCCESS;
    }
    auto delegate = delegates.at(key);
    std::vector<uint8_t> tmpKey;
    LOG_INFO("start RegisterObserver %{public}s", key.c_str());
    DistributedDB::DBStatus status =
        delegate->RegisterObserver(tmpKey, DistributedDB::ObserverMode::OBSERVER_CHANGES_NATIVE, watcher.get());
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver watch err %d", status);
        return ERR_REGISTER;
    }
    LOG_INFO("end RegisterObserver %{public}s", key.c_str());
    observerMap_.insert_or_assign(key, watcher);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::UnRegisterObserver(const std::string &key)
{
    if (!opened_) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver kvStore has not init");
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates.count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::RegisterObserver %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    auto iter = observerMap_.find(key);
    if (iter == observerMap_.end()) {
        LOG_ERROR("FlatObjectStorageEngine::UnRegisterObserver observer not exist.");
        return ERR_NO_OBSERVER;
    }
    auto delegate = delegates.at(key);
    std::shared_ptr<TableWatcher> watcher = iter->second;
    LOG_INFO("start UnRegisterObserver %{public}s", key.c_str());
    DistributedDB::DBStatus status = delegate->UnRegisterObserver(watcher.get());
    if (status != DistributedDB::DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::UnRegisterObserver unRegister err %d", status);
        return ERR_UNRIGSTER;
    }
    LOG_INFO("end UnRegisterObserver %{public}s", key.c_str());
    observerMap_.erase(key);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::ChangeKey(const std::string &oldKey, const std::string &newKey)
{
    if (!opened_) {
        LOG_ERROR("FlatObjectStorageEngine::ChangeKey kvStore has not init");
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates.count(oldKey) == 0) {
        LOG_INFO("FlatObjectStorageEngine::ChangeKey oldKey %s not exist", oldKey.c_str());
        return ERR_DE_NOT_EXIST;
    }
    DistributedDB::KvStoreNbDelegate *delegate = delegates.at(oldKey);
    delegates.erase(oldKey);
    delegates.insert_or_assign(newKey, delegate);
    auto iter = observerMap_.find(oldKey);
    if (iter != observerMap_.end()) {
        UnRegisterObserver(oldKey);
        std::shared_ptr<TableWatcher> watcher = iter->second;
        uint32_t status = RegisterObserver(newKey, watcher);
        if (status != DistributedDB::DBStatus::OK) {
            LOG_ERROR("FlatObjectStorageEngine::ChangeKey watch err %d", status);
            return ERR_REGISTER;
        }
    }
    return SUCCESS;
}
} // namespace OHOS::ObjectStore
