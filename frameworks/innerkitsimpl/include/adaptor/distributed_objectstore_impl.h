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

#ifndef DISTRIBUTED_OBJECTSTORE_IMPL_H
#define DISTRIBUTED_OBJECTSTORE_IMPL_H

#include <bytes.h>

#include <shared_mutex>

#include "distributed_objectstore.h"
#include "distributed_object_impl.h"

namespace OHOS::ObjectStore {
class WatcherProxy;
enum SyncStatus {
    START,
    FINISHED
};
class DistributedObjectStoreImpl : public DistributedObjectStore {
public:
    DistributedObjectStoreImpl(FlatObjectStore *flatObjectStore);
    ~DistributedObjectStoreImpl() override;
    uint32_t Get(const std::string &sessionId, DistributedObject *object) override;
    DistributedObject *CreateObject(const std::string &sessionId) override;
    uint32_t DeleteObject(const std::string &sessionId) override;
    uint32_t Watch(DistributedObject *object, std::shared_ptr<ObjectWatcher> watcher) override;
    uint32_t UnWatch(DistributedObject *object) override;
    uint32_t SetStatusNotifier(std::shared_ptr<StatusNotifier> notifier) override;
    void TriggerSync() override;
    void TriggerRestore(std::function<void()> notifier) override;

private:
    void UpdateStatus(const std::string &status);
    DistributedObjectImpl *CacheObject(const std::string &sessionId, FlatObjectStore *flatObjectStore);
    FlatObjectStore *flatObjectStore_ = nullptr;
    std::map<DistributedObject *, std::shared_ptr<WatcherProxy>> watchers_;
    std::shared_mutex dataMutex_{};
    std::vector<DistributedObjectImpl *> objects_{};
};
class StatusNotifierProxy : public StatusWatcher {
public:
    virtual ~StatusNotifierProxy();
    StatusNotifierProxy(const std::shared_ptr<StatusNotifier> &notifier);
    void OnChanged(
        const std::string &sessionId, const std::string &networkId, const std::string &onlineStatus) override;

private:
    std::shared_ptr<StatusNotifier> notifier;
};
class WatcherProxy : public FlatObjectWatcher {
public:
    WatcherProxy(const std::shared_ptr<ObjectWatcher> objectWatcher, const std::string &sessionId);
    void OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData) override;

private:
    std::shared_ptr<ObjectWatcher> objectWatcher_;
};
} // namespace OHOS::ObjectStore

#endif // DISTRIBUTED_OBJECTSTORE_H
