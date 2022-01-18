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

#ifndef DISTRIBUTEDDATAFWK_ARK_COMMUNICATION_PROVIDER_H
#define DISTRIBUTEDDATAFWK_ARK_COMMUNICATION_PROVIDER_H

#include "app_device_handler.h"
#include "app_pipe_mgr.h"
#include "communication_provider_impl.h"
#include "nocopyable.h"

namespace OHOS {
namespace ObjectStore {
class ArkCommunicationProvider : public CommunicationProviderImpl {
public:
    static CommunicationProvider &Init();

    ~ArkCommunicationProvider() override{};

private:
    DISALLOW_COPY_AND_MOVE(ArkCommunicationProvider);
    ArkCommunicationProvider();
    AppPipeMgr appPipeMgrImpl_{};
    AppDeviceHandler appDeviceHandlerImpl_{};
    bool isInited = false;
};
} // namespace ObjectStore
} // namespace OHOS
#endif // DISTRIBUTEDDATAFWK_ARK_COMMUNICATION_PROVIDER_H
