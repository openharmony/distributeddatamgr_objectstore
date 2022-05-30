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

#ifndef OBJECT_STORAGE_ENGINE_H
#define OBJECT_STORAGE_ENGINE_H

#include <cstdint>
#include <map>
#include <vector>

#include "kv_store_observer.h"
#include "watcher.h"

namespace OHOS::ObjectStore {
using Key = std::vector<uint8_t>;
using Value = std::vector<uint8_t>;
using Field = std::vector<uint8_t>;

class TableWatcher : public Watcher {
public:
    TableWatcher(const std::string &sessionId) : Watcher(sessionId)
    {
    }
    void OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData) override;
};

class StatusWatcher {
public:
    virtual void OnChanged(
        const std::string &sessionId, const std::string &networkId, const std::string &onlineStatus) = 0;
};

class ObjectStorageEngine {
public:
    ObjectStorageEngine(const ObjectStorageEngine &) = delete;
    ObjectStorageEngine &operator=(const ObjectStorageEngine &) = delete;
    ObjectStorageEngine(ObjectStorageEngine &&) = delete;
    ObjectStorageEngine &operator=(ObjectStorageEngine &&) = delete;
    ObjectStorageEngine() = default;
    virtual ~ObjectStorageEngine() = default;
    virtual uint32_t Open(const std::string &bundleName) = 0;
    virtual uint32_t Close() = 0;
    virtual uint32_t DeleteTable(const std::string &key) = 0;
    virtual uint32_t CreateTable(const std::string &key) = 0;
    virtual uint32_t GetTable(const std::string &key, std::map<std::string, Value> &result) = 0;
    virtual uint32_t UpdateItem(const std::string &key, const std::string &itemKey, Value &value) = 0;
    virtual uint32_t UpdateItems(const std::string &key, const std::map<std::string, std::vector<uint8_t>> &data) = 0;
    virtual uint32_t GetItem(const std::string &key, const std::string &itemKey, Value &value) = 0;
    virtual uint32_t GetItems(const std::string &key, std::map<std::string, std::vector<uint8_t>> &data) = 0;
    virtual uint32_t RegisterObserver(const std::string &key, std::shared_ptr<TableWatcher> watcher) = 0;
    virtual uint32_t UnRegisterObserver(const std::string &key) = 0;
    virtual uint32_t SetStatusNotifier(std::shared_ptr<StatusWatcher> watcher) = 0;
};
} // namespace OHOS::ObjectStore
#endif