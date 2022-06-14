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

#include "distributed_object_impl.h"

#include "dds_trace.h"
#include "objectstore_errors.h"
#include "string_utils.h"

namespace OHOS::ObjectStore {
DistributedObjectImpl::~DistributedObjectImpl()
{
}

void PutNum(void *val, uint32_t offset, uint32_t valLen, Bytes &data)
{
    uint32_t len = valLen + offset;
    if (len > sizeof(data.front()) * data.size()) {
        data.resize(len);
    }

    for (uint32_t i = 0; i < valLen; i++) {
        // 8 bit = 1 byte
        data[offset + i] = *(static_cast<uint64_t *>(val)) >> ((valLen - i - 1) * 8);
    }
}

uint32_t GetNum(Bytes &data, uint32_t offset, void *val, uint32_t valLen)
{
    uint8_t *value = (uint8_t *)val;
    uint32_t len = offset + valLen;
    uint32_t dataLen = data.size();
    if (dataLen < len) {
        LOG_ERROR("DistributedObjectImpl:GetNum data.size() %{public}d, offset %{public}d, valLen %{public}d", dataLen,
            offset, valLen);
        return ERR_DATA_LEN;
    }
    for (uint32_t i = 0; i < valLen; i++) {
        value[i] = data[len - 1 - i];
    }
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutDouble(const std::string &key, double value)
{
    DistributedDataDfx::DdsTrace trace(std::string("DistributedObjectImpl::") + std::string(__FUNCTION__),
        DistributedDataDfx::TraceSwitch::BYTRACE_ON | DistributedDataDfx::TraceSwitch::TRACE_CHAIN_ON);
    Bytes data;
    Type type = Type::TYPE_DOUBLE;
    PutNum(&type, 0, sizeof(type), data);
    PutNum(&value, sizeof(type), sizeof(value), data);
    uint32_t status = flatObjectStore_->Put(sessionId_, FIELDS_PREFIX + key, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::PutDouble setField err %{public}d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::PutBoolean(const std::string &key, bool value)
{
    DistributedDataDfx::DdsTrace trace(std::string("DistributedObjectImpl::") + std::string(__FUNCTION__),
        DistributedDataDfx::TraceSwitch::BYTRACE_ON | DistributedDataDfx::TraceSwitch::TRACE_CHAIN_ON);
    Bytes data;
    Type type = Type::TYPE_BOOLEAN;
    PutNum(&type, 0, sizeof(type), data);
    PutNum(&value, sizeof(type), sizeof(value), data);
    uint32_t status = flatObjectStore_->Put(sessionId_, FIELDS_PREFIX + key, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::PutBoolean setField err %{public}d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::PutString(const std::string &key, const std::string &value)
{
    DistributedDataDfx::DdsTrace trace(std::string("DistributedObjectImpl::") + std::string(__FUNCTION__),
        DistributedDataDfx::TraceSwitch::BYTRACE_ON | DistributedDataDfx::TraceSwitch::TRACE_CHAIN_ON);
    Bytes data;
    Type type = Type::TYPE_STRING;
    PutNum(&type, 0, sizeof(type), data);
    Bytes dst = StringUtils::StrToBytes(value);
    data.insert(data.end(), dst.begin(), dst.end());
    uint32_t status = flatObjectStore_->Put(sessionId_, FIELDS_PREFIX + key, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::PutString setField err %{public}d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::GetDouble(const std::string &key, double &value)
{
    Bytes data;
    Bytes keyBytes = StringUtils::StrToBytes(key);
    uint32_t status = flatObjectStore_->Get(sessionId_, FIELDS_PREFIX + key, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetDouble field not exist. %{public}d %{public}s", status, key.c_str());
        return status;
    }
    status = GetNum(data, sizeof(Type), &value, sizeof(value));
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetDouble getNum err. %{public}d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::GetBoolean(const std::string &key, bool &value)
{
    Bytes data;
    Bytes keyBytes = StringUtils::StrToBytes(key);
    uint32_t status = flatObjectStore_->Get(sessionId_, FIELDS_PREFIX + key, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetBoolean field not exist. %{public}d %{public}s", status, key.c_str());
        return status;
    }
    status = GetNum(data, sizeof(Type), &value, sizeof(value));
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetBoolean getNum err. %{public}d", status);
        return status;
    }
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetString(const std::string &key, std::string &value)
{
    Bytes data;
    uint32_t status = flatObjectStore_->Get(sessionId_, FIELDS_PREFIX + key, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetString field not exist. %{public}d %{public}s", status, key.c_str());
        return status;
    }
    status = StringUtils::BytesToStrWithType(data, value);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetString dataToVal err. %{public}d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::GetType(const std::string &key, Type &type)
{
    Bytes data;
    uint32_t status = flatObjectStore_->Get(sessionId_, FIELDS_PREFIX + key, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetString field not exist. %{public}d %{public}s", status, key.c_str());
        return status;
    }
    status = GetNum(data, 0, &type, sizeof(type));
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetBoolean getNum err. %{public}d", status);
        return status;
    }
    return SUCCESS;
}

std::string &DistributedObjectImpl::GetSessionId()
{
    return sessionId_;
}

DistributedObjectImpl::DistributedObjectImpl(const std::string &sessionId, FlatObjectStore *flatObjectStore)
    : sessionId_(sessionId), flatObjectStore_(flatObjectStore)
{
}

uint32_t DistributedObjectImpl::PutComplex(const std::string &key, const std::vector<uint8_t> &value)
{
    DistributedDataDfx::DdsTrace trace(std::string("DistributedObjectImpl::") + std::string(__FUNCTION__),
        DistributedDataDfx::TraceSwitch::BYTRACE_ON | DistributedDataDfx::TraceSwitch::TRACE_CHAIN_ON);
    Bytes data;
    Type type = Type::TYPE_COMPLEX;
    PutNum(&type, 0, sizeof(type), data);
    data.insert(data.end(), value.begin(), value.end());
    uint32_t status = flatObjectStore_->Put(sessionId_, FIELDS_PREFIX + key, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::PutBoolean setField err %{public}d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::GetComplex(const std::string &key, std::vector<uint8_t> &value)
{
    uint32_t status = flatObjectStore_->Get(sessionId_, FIELDS_PREFIX + key, value);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetString field not exist. %{public}d %{public}s", status, key.c_str());
        return status;
    }
    return status;
}

uint32_t DistributedObjectImpl::Save(const std::string &deviceId)
{
    uint32_t status = flatObjectStore_->Save(sessionId_, deviceId);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:Save failed. status = %{public}d", status);
        return status;
    }
    return status;
}

uint32_t DistributedObjectImpl::RevokeSave()
{
    uint32_t status = flatObjectStore_->RevokeSave(sessionId_);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:RevokeSave failed. status = %{public}d", status);
        return status;
    }
    return status;
}
} // namespace OHOS::ObjectStore