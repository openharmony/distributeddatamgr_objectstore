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

#ifndef FLAT_OBJECT_STORE_H
#define FLAT_OBJECT_STORE_H

#include <memory>
#include <string>

#include "bytes.h"
#include "flat_object_storage_engine.h"

namespace OHOS::ObjectStore {
class FlatObjectWatcher : public TableWatcher {
public:
    FlatObjectWatcher(const std::string &sessionId) : TableWatcher(sessionId)
    {
    }
    void OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData) override;
    void OnDeleted(const std::string &sessionid) override;
};

class FlatObjectStore {
public:
    explicit FlatObjectStore();
    ~FlatObjectStore();
    uint32_t CreateObject(const std::string &sessionId);
    uint32_t Delete(const std::string &objectId);
    uint32_t Watch(const std::string &objectId, std::shared_ptr<FlatObjectWatcher> watcher);
    uint32_t UnWatch(const std::string &objectId);
    uint32_t Put(const std::string &sessionId, const std::string &key, std::vector<uint8_t> value);
    uint32_t Get(std::string &sessionId, const std::string &key, Bytes &value);

private:
    std::shared_ptr<FlatObjectStorageEngine> storageEngine_;
};
} // namespace OHOS::ObjectStore

#endif // FLAT_OBJECT_STORE_H
