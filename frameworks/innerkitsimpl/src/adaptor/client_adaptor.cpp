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

#include "client_adaptor.h"

#include <thread>

#include "logger.h"
#include "iservice_registry.h"

namespace OHOS::ObjectStore {
sptr<OHOS::DistributedObject::IObjectService> ClientAdaptor::GetObjectService()
{
    static sptr<OHOS::DistributedKv::IKvStoreDataService> distributedDataMgr;
    if (distributedDataMgr == nullptr) {
        distributedDataMgr = GetDistributedDataManager();
    }
    if (distributedDataMgr == nullptr) {
        LOG_ERROR("get distributed data manager failed");
        return nullptr;
    }

    auto remote = distributedDataMgr->GetObjectService();
    if (remote == nullptr) {
        LOG_ERROR("get object service failed");
        return nullptr;
    }
    return iface_cast<DistributedObject::IObjectService>(remote);
}

sptr<DistributedKv::IKvStoreDataService> ClientAdaptor::GetDistributedDataManager()
{
    int retry = 0;
    while (++retry <= GET_SA_RETRY_TIMES) {
        auto manager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (manager == nullptr) {
            LOG_ERROR("get system ability manager failed");
            return nullptr;
        }
        LOG_INFO("get distributed data manager %{public}d", retry);
        auto remoteObject = manager->CheckSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
        if (remoteObject == nullptr) {
            std::this_thread::sleep_for(std::chrono::seconds(RETRY_INTERVAL));
            continue;
        }
        LOG_INFO("get distributed data manager success");
        return iface_cast<DistributedKv::IKvStoreDataService>(remoteObject);
    }

    LOG_ERROR("get distributed data manager failed");
    return nullptr;
}
}