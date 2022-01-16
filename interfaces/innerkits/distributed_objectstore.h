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

#ifndef DISTRIBUTED_OBJECTSTORE_H
#define DISTRIBUTED_OBJECTSTORE_H
#include <memory>
#include <string>
#include <vector>

#include "distributed_object.h"

namespace OHOS::ObjectStore {
class DistributedObjectStore {
public:
    virtual ~DistributedObjectStore(){};
    static DistributedObjectStore *GetInstance();
    virtual DistributedObject *CreateObject(const std::string &sessionId) = 0;
    virtual uint32_t Get(const std::string &sessionId, DistributedObject *object) = 0;
    virtual uint32_t Sync(DistributedObject *object) = 0;
    virtual uint32_t DeleteObject(const std::string &sessionId) = 0;
    virtual uint32_t Watch(DistributedObject *object, std::shared_ptr<ObjectWatcher> objectWatcher) = 0;
    virtual uint32_t UnWatch(DistributedObject *object) = 0;
};
} // namespace OHOS::ObjectStore

#endif // DISTRIBUTED_OBJECTSTORE_H
