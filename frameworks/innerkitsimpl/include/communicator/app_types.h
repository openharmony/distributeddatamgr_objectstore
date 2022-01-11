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

#ifndef APP_DISTRIBUTED_KVSTORE_APP_TYPES_H
#define APP_DISTRIBUTED_KVSTORE_APP_TYPES_H

#include <errors.h>
#include <cstdint>
#include <string>
#include <vector>
#include "visibility.h"

namespace OHOS {
namespace ObjectStore {

struct PipeInfo {
    std::string pipeId;
};

struct DeviceInfo {
    std::string deviceId;
    std::string deviceName;
    std::string deviceType;
};

enum class MessageType {
    DEFAULT = 0,
};

struct MessageInfo {
    MessageType msgType;
};

enum class DeviceChangeType {
    DEVICE_OFFLINE = 0,
    DEVICE_ONLINE = 1,
};

struct DeviceId {
    std::string deviceId;
};

// app_distributed_data_manager using sub error code 0
constexpr ErrCode APP_DISTRIBUTEDDATAMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_DISTRIBUTEDDATAMNG, 0);

enum class Status {
    SUCCESS = ERR_OK,
    ERROR = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET,
    INVALID_ARGUMENT = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 1,
    ILLEGAL_STATE = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 2,
    KEY_NOT_FOUND = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 7,
    REPEATED_REGISTER = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 14,
    CREATE_SESSION_ERROR = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 15,
};

}  // namespace ObjectStore
}  // namespace OHOS

#endif  // APP_DISTRIBUTED_KVSTORE_TYPES_H
