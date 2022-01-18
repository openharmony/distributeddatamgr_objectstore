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

#ifndef WATCHER_H
#define WATCHER_H

#include <cstdint>

#include "kv_store_delegate_manager.h"
#include "logger.h"

namespace OHOS::ObjectStore {
class Watcher : public DistributedDB::KvStoreObserver {
public:
    Watcher(const std::string &sessionId);
    virtual ~Watcher() = default;
    virtual void OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData) = 0;
    virtual void OnDeleted(const std::string &sessionid) = 0;

    void OnChange(const DistributedDB::KvStoreChangedData &data) override;

private:
    std::string sessionId_;
};
} // namespace OHOS::ObjectStore

#endif // WATCHER_H