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

#ifndef DISTRIBUTED_OBJECTSTORE_IMPL_H
#define DISTRIBUTED_OBJECTSTORE_IMPL_H

#include <shared_mutex>

#include "distributed_objectstore.h"
#include "flat_object_store.h"

namespace OHOS::ObjectStore {
class DistributedObjectImpl;
class DistributedObjectStoreImpl : public DistributedObjectStore {
public:
    DistributedObjectStoreImpl(FlatObjectStore *flatObjectStore);
    ~DistributedObjectStoreImpl() override;

    DistributedObject *CreateObject(const std::string &classPath, const std::string &objectId) override;

    uint32_t Sync(DistributedObject *object) override;

    void Close() override;

private:
    DistributedObjectImpl *CacheObject(FlatObject *flatObject);
    FlatObjectStore *flatObjectStore_ = nullptr;
    std::shared_mutex dataMutex_ {};
    std::vector<DistributedObjectImpl*> objects_ {};
};
}

#endif // DISTRIBUTED_OBJECTSTORE_H
