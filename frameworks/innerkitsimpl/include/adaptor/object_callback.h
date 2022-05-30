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

#ifndef OBJECT_CALLBACK_H
#define OBJECT_CALLBACK_H

#include "iobject_callback.h"

namespace OHOS {
namespace ObjectStore {
using namespace DistributedObject;
class ObjectSaveCallback : public ObjectSaveCallbackStub {
public:
    ObjectSaveCallback(const std::function<void(const std::map<std::string, int32_t> &)> &callback);
    void Completed(const std::map<std::string, int32_t> &results) override;

private:
    const std::function<void(const std::map<std::string, int32_t> &)> callback_;
};

class ObjectRevokeSaveCallback : public ObjectRevokeSaveCallbackStub {
public:
    ObjectRevokeSaveCallback(const std::function<void(int32_t)> &callback);
    void Completed(int32_t) override;

private:
    const std::function<void(int32_t status)> callback_;
};

class ObjectRetrieveCallback : public ObjectRetrieveCallbackStub {
public:
    ObjectRetrieveCallback(const std::function<void(const std::map<std::string, std::vector<uint8_t>> &)> &callback);
    void Completed(const std::map<std::string, std::vector<uint8_t>> &results) override;

private:
    const std::function<void(const std::map<std::string, std::vector<uint8_t>> &)> callback_;
};
} // namespace DistributedKv
} // namespace OHOS

#endif // OBJECT_CALLBACK_H
