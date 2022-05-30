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
#include "object_callback.h"

namespace OHOS::ObjectStore {
void ObjectSaveCallback::Completed(const std::map<std::string, int32_t> &results)
{
    callback_(results);
}

ObjectSaveCallback::ObjectSaveCallback(const std::function<void(const std::map<std::string, int32_t> &)> &callback)
    : callback_(callback)
{
}

void ObjectRevokeSaveCallback::Completed(int32_t status)
{
    callback_(status);
}

ObjectRevokeSaveCallback::ObjectRevokeSaveCallback(const std::function<void(int32_t)> &callback) : callback_(callback)
{
}

void ObjectRetrieveCallback::Completed(const std::map<std::string, std::vector<uint8_t>> &results)
{
    callback_(results);
}

ObjectRetrieveCallback::ObjectRetrieveCallback(
    const std::function<void(const std::map<std::string, std::vector<uint8_t>> &)> &callback)
    : callback_(callback)
{
}
} // namespace OHOS::DistributedKv
