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

#include <atomic>

#include "distributed_objectstore_manager.h"
#include "distributed_objectstore_manager_impl.h"

namespace OHOS::ObjectStore {
DistributedObjectStoreManager *DistributedObjectStoreManager::GetInstance()
{
    static char instMemory[sizeof(DistributedObjectStoreManagerImpl)];
    static std::mutex instLock_;
    static std::atomic<DistributedObjectStoreManager *> instPtr = nullptr;
    // For Double-Checked Locking, we need check insPtr twice
    if (instPtr == nullptr) {
        std::lock_guard<std::mutex> lock(instLock_);
        if (instPtr == nullptr) {
            // Use instMemory to make sure this singleton not free before other object.
            // This operation needn't to malloc memory, we needn't to check nullptr.
            instPtr = new (instMemory) DistributedObjectStoreManagerImpl;
        }
    }
    return instPtr;
}
}