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

#ifndef OBJECT_CLIENT_ADAPTOR_H
#define OBJECT_CLIENT_ADAPTOR_H

#include "object_service_proxy.h"
#include "ikvstore_data_service.h"
namespace OHOS::ObjectStore {
class ClientAdaptor {
public:
    static sptr<OHOS::DistributedObject::IObjectService> GetObjectService();
private:
    static constexpr int32_t DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID = 1301;
    static constexpr int32_t GET_SA_RETRY_TIMES = 3;
    static constexpr int32_t RETRY_INTERVAL = 1;
    static sptr<OHOS::DistributedKv::IKvStoreDataService> GetDistributedDataManager();
};
} // namespace OHOS::ObjectStore

#endif // OBJECT_CLIENT_ADAPTOR_H
