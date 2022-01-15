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

#include "flat_object_store.h"
#include "coordinate_engine.h"
#include "logger.h"
#include "object_coordinator.h"
#include "object_store_manager.h"
#include "object_utils.h"
#include "objectstore_errors.h"
#include "flat_object_storage_engine.h"
#include "string_utils.h"
#include "watcher_holder.h"

namespace OHOS::ObjectStore {
namespace {
const std::map<Bytes, Bytes> EMPTY_FIELDS = {};
const std::string DEFAULT_USER_ID = "0";
}  // namespace

FlatObjectStore::FlatObjectStore(const std::string &bundleName,
                                 const std::string &storeName)
    : bundleName_(bundleName), storeName_(storeName), prefix_(""),
      watcherHolder_(std::make_unique<WatcherHolder>())
{
    storageEngine_ = std::make_shared<FlatObjectStorageEngine>();
}

FlatObjectStore::~FlatObjectStore()
{}

uint32_t FlatObjectStore::Open()
{
    uint32_t status = storageEngine_->Open();
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to open, error: open storage engine failure");
        return status;
    }
    ObjectStoreManager::GetInstance().SetObjectStore(storeName_, storageEngine_);
    CommunicatorConfig config(bundleName_);
    status = CoordinateEngine::GetInstance().Init(config);
    if (status != SUCCESS) {
        storageEngine_->Close();
        LOG_ERROR("FlatObjectStore: Failed to open, error: open coordinate engine failure");
        return status;
    }

    status = CoordinateEngine::GetInstance().GetLocalDeviceId(deviceId_);
    if (status != SUCCESS) {
        storageEngine_->Close();
        LOG_ERROR("FlatObjectStore: Failed to open, error: get local device id failure");
        return status;
    }
    prefix_ = ObjectUtils::GenObjectIdPrefix(deviceId_, DEFAULT_USER_ID, bundleName_, storeName_);
    GetRemoteStoreIds();
    return SUCCESS;
}

uint32_t FlatObjectStore::Close()
{
    storageEngine_->Close();
    CoordinateEngine::GetInstance().Destory();
    return SUCCESS;
}

const std::string &FlatObjectStore::GetName() const
{
    return storeName_;
}

const std::string &FlatObjectStore::GetPrefix() const
{
    return prefix_;
}

const std::string &FlatObjectStore::GetBundleName() const
{
    return bundleName_;
}

const std::vector<std::string> &FlatObjectStore::GetRemoteStoreIds()
{
    if (remoteIds_.empty()) {
        CoordinateEngine::GetInstance().GetRemoteStoreIds(remoteIds_);
    }
    return remoteIds_;
}

uint32_t FlatObjectStore::Put(const FlatObject &flatObject)
{
    if (IsLocalObject(flatObject.GetId())) {
        FlatObject localObject;
        uint32_t status = Get(flatObject.GetId(), localObject);
        if (status != SUCCESS) {
            LOG_ERROR("FlatObjectStore: Failed to get object from local storage");
            return status;
        }
        if (localObject.GetFields() == flatObject.GetFields()) {
            LOG_INFO("FlatObjectStore::Put no data changed");
            return SUCCESS;
        }
        status = storageEngine_->PutHash(flatObject.GetId(), flatObject.GetFields());
        if (status != SUCCESS) {
            LOG_ERROR("FlatObjectStore: Failed to put object to local storage");
            return status;
        }
        status = ObjectCoordinator::GetInstance().Publish(flatObject.GetId(), flatObject.GetFields());
        if (status != SUCCESS) {
            LOG_ERROR("FlatObjectStore: Failed to publish object after put");
        }
        return status;
    }

    uint32_t status = ObjectCoordinator::GetInstance().Put(flatObject.GetId(), flatObject.GetFields());
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to put object to remote storage");
    }
    return status;
}

uint32_t FlatObjectStore::Get(const Bytes &objectId, FlatObject &flatObject) const
{
    std::map<Bytes, Bytes> &fields = const_cast<std::map<Bytes, Bytes> &>(flatObject.GetFields());
    if (IsLocalObject(objectId)) {
        LOG_INFO("FlatObjectStore: get from local");
        uint32_t status = storageEngine_->GetHash(objectId, fields);
        if (status != SUCCESS) {
            LOG_ERROR("FlatObjectStore: Failed to get object from local storage");
            return status;
        }
    } else {
        LOG_INFO("FlatObjectStore: get remote");
        uint32_t status = ObjectCoordinator::GetInstance().Get(objectId, fields);
        if (status != SUCCESS) {
            LOG_ERROR("FlatObjectStore: Failed to get object from remote storage");
            return status;
        }
    }
    flatObject.SetId(objectId);
    return SUCCESS;
}

uint32_t FlatObjectStore::Delete(const Bytes &objectId)
{
    if (IsLocalObject(objectId)) {
        uint32_t status = storageEngine_->DeleteKey(objectId);
        if (status != SUCCESS) {
            LOG_ERROR("FlatObjectStore: Failed to delete object from local storage");
        }
        status = ObjectCoordinator::GetInstance().PublishDelete(objectId);
        if (status != SUCCESS) {
            LOG_ERROR("FlatObjectStore: Failed to publish object after delete");
        }
        return status;
    }

    uint32_t status = ObjectCoordinator::GetInstance().Delete(objectId);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to delete object from remote storage");
    }
    return status;
}

uint32_t FlatObjectStore::Watch(const Bytes &objectId, std::shared_ptr<FlatObjectWatcher> watcher)
{
    auto observer = watcherHolder_->hold(objectId, watcher);
    if (observer == nullptr) {
        LOG_ERROR("FlatObjectStore: Failed to watch object, error: hold failure");
        return ERR_INVAL;
    }
    uint32_t status = ObjectCoordinator::GetInstance().AddObserver(objectId, observer);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to watch object from coordinator");
    }
    return status;
}

uint32_t FlatObjectStore::Unwatch(const Bytes &objectId, std::shared_ptr<FlatObjectWatcher> watcher)
{
    auto observer = watcherHolder_->unhold(objectId, watcher);
    if (observer == nullptr) {
        LOG_ERROR("FlatObjectStore: Failed to unwatch object, error: unhold failure");
        return ERR_INVAL;
    }
    uint32_t status = ObjectCoordinator::GetInstance().RemoveObserver(objectId, observer);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to unwatch object from coordinator");
    }
    return status;
}

bool FlatObjectStore::IsLocalObject(const Bytes &objectId) const
{
    std::string objectDeviceId = ObjectUtils::GetObjectHost(std::string(objectId.begin(), objectId.end()));
    return objectDeviceId == deviceId_;
}
}  // namespace OHOS::ObjectStore
