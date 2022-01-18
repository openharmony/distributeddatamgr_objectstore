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

#ifndef DISTRIBUTED_OBJECT_H
#define DISTRIBUTED_OBJECT_H
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace OHOS::ObjectStore {
enum Type : uint8_t {
    TYPE_STRING = 0,
    TYPE_BOOLEAN,
    TYPE_DOUBLE,
};
class DistributedObject {
public:
    virtual ~DistributedObject(){};
    virtual uint32_t PutDouble(const std::string &key, double value) = 0;
    virtual uint32_t PutBoolean(const std::string &key, bool value) = 0;
    virtual uint32_t PutString(const std::string &key, const std::string &value) = 0;
    virtual uint32_t GetDouble(const std::string &key, double &value) = 0;
    virtual uint32_t GetBoolean(const std::string &key, bool &value) = 0;
    virtual uint32_t GetString(const std::string &key, std::string &value) = 0;
    virtual uint32_t GetType(const std::string &key, Type &type) = 0;
    virtual std::string &GetSessionId() = 0;
};

class ObjectWatcher {
public:
    virtual void OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData) = 0;
    virtual void OnDeleted(const std::string &sessionid) = 0;
};
} // namespace OHOS::ObjectStore
#endif // DISTRIBUTED_OBJECT_H
