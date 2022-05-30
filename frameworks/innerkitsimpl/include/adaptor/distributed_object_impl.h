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

#ifndef DISTRIBUTED_OBJECT_IMPL_H
#define DISTRIBUTED_OBJECT_IMPL_H
#include <string>

#include "distributed_object.h"
#include "flat_object_store.h"

namespace OHOS::ObjectStore {
class DistributedObjectImpl : public DistributedObject {
public:
    DistributedObjectImpl(const std::string &sessionId, FlatObjectStore *flatObjectStore);
    ~DistributedObjectImpl();
    uint32_t PutDouble(const std::string &key, double value) override;
    uint32_t PutBoolean(const std::string &key, bool value) override;
    uint32_t PutString(const std::string &key, const std::string &value) override;
    uint32_t GetDouble(const std::string &key, double &value) override;
    uint32_t GetBoolean(const std::string &key, bool &value) override;
    uint32_t GetString(const std::string &key, std::string &value) override;
    uint32_t PutComplex(const std::string &key, const std::vector<uint8_t> &value) override;
    uint32_t GetComplex(const std::string &key, std::vector<uint8_t> &value) override;
    std::string &GetSessionId() override;
    uint32_t Save(const std::string &deviceId) override;
    uint32_t RevokeSave() override;
    uint32_t GetType(const std::string &key, Type &type) override;

private:
    std::string sessionId_;
    FlatObjectStore *flatObjectStore_ = nullptr;
};
} // namespace OHOS::ObjectStore

#endif // DISTRIBUTED_OBJECT_IMPL_H
