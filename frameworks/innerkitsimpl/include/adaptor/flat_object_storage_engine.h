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

#ifndef FLAT_OBJECT_STORAGE_ENGINE_H
#define FLAT_OBJECT_STORAGE_ENGINE_H

#include <cstdint>
#include <map>
#include <shared_mutex>
#include <vector>

#include "kv_store_delegate_manager.h"
#include "object_storage_engine.h"

namespace OHOS::ObjectStore {
class FlatObjectStorageEngine : public ObjectStorageEngine {
public:
    FlatObjectStorageEngine() = default;
    ~FlatObjectStorageEngine() override;
    uint32_t Open(const std::string &bundleName) override;
    uint32_t Close() override;
    uint32_t DeleteTable(const std::string &key) override;
    uint32_t CreateTable(const std::string &key) override;
    uint32_t GetTable(const std::string &key, std::map<std::string, Value> &result) override;
    uint32_t UpdateItems(const std::string &key, std::map<std::string, Value> &data) override;
    uint32_t UpdateItem(const std::string &key, const std::string &itemKey, Value &value) override;
    uint32_t GetItem(const std::string &key, const std::string &itemKey, Value &value) override;
    uint32_t RegisterObserver(const std::string &key, std::shared_ptr<TableWatcher> watcher) override;
    uint32_t UnRegisterObserver(const std::string &key) override;
    uint32_t ChangeKey(const std::string &oldKey, const std::string &newKey) override;
    bool isOpened_ = false;

private:
    std::shared_mutex operationMutex_{};
    std::shared_ptr<DistributedDB::KvStoreDelegateManager> storeManager_;
    std::map<std::string, DistributedDB::KvStoreNbDelegate *> delegates_;
    std::map<std::string, std::shared_ptr<TableWatcher>> observerMap_;
};
} // namespace OHOS::ObjectStore
#endif
