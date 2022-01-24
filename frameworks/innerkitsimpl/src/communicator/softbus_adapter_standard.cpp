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

#include <logger.h>

#include <mutex>
#include <thread>

#include "kv_store_delegate_manager.h"
#include "process_communicator_impl.h"
#include "securec.h"
#include "session.h"
#include "softbus_adapter.h"
#include "softbus_bus_center.h"

namespace OHOS {
namespace ObjectStore {
constexpr int32_t HEAD_SIZE = 3;
constexpr int32_t END_SIZE = 3;
constexpr int32_t MIN_SIZE = HEAD_SIZE + END_SIZE + 3;
constexpr const char *REPLACE_CHAIN = "***";
constexpr const char *DEFAULT_ANONYMOUS = "******";
constexpr int32_t SOFTBUS_OK = 0;
constexpr int32_t SOFTBUS_ERR = 1;
constexpr int32_t INVALID_SESSION_ID = -1;
constexpr int32_t SESSION_NAME_SIZE_MAX = 65;
constexpr int32_t DEVICE_ID_SIZE_MAX = 65;
constexpr int32_t ID_BUF_LEN = 65;
using namespace std;

class AppDeviceListenerWrap {
public:
    explicit AppDeviceListenerWrap()
    {
    }
    ~AppDeviceListenerWrap()
    {
    }
    static void SetDeviceHandler(SoftBusAdapter *handler);
    static void OnDeviceOnline(NodeBasicInfo *info);
    static void OnDeviceOffline(NodeBasicInfo *info);
    static void OnDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info);

private:
    static SoftBusAdapter *softBusAdapter_;
    static void NotifyAll(NodeBasicInfo *info, DeviceChangeType type);
};
SoftBusAdapter *AppDeviceListenerWrap::softBusAdapter_;

class AppDataListenerWrap {
public:
    static void SetDataHandler(SoftBusAdapter *handler);
    static int OnSessionOpened(int sessionId, int result);
    static void OnSessionClosed(int sessionId);
    static void OnMessageReceived(int sessionId, const void *data, unsigned int dataLen);
    static void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen);

public:
    // notifiy all listeners when received message
    static void NotifyDataListeners(
        const uint8_t *ptr, const int size, const std::string &deviceId, const PipeInfo &pipeInfo);
    static SoftBusAdapter *softBusAdapter_;
};
SoftBusAdapter *AppDataListenerWrap::softBusAdapter_;
std::shared_ptr<SoftBusAdapter> SoftBusAdapter::instance_;

void AppDeviceListenerWrap::OnDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info)
{
    std::string udid = softBusAdapter_->GetUdidByNodeId(std::string(info->networkId));
    LOG_INFO("[InfoChange] type:%{public}d, id:%{public}s, name:%{public}s", type,
        SoftBusAdapter::ToBeAnonymous(udid).c_str(), info->deviceName);
}

void AppDeviceListenerWrap::OnDeviceOffline(NodeBasicInfo *info)
{
    std::string udid = softBusAdapter_->GetUdidByNodeId(std::string(info->networkId));
    LOG_INFO("[Offline] id:%{public}s, name:%{public}s, typeId:%{public}d",
        SoftBusAdapter::ToBeAnonymous(udid).c_str(), info->deviceName, info->deviceTypeId);
    NotifyAll(info, DeviceChangeType::DEVICE_OFFLINE);
}

void AppDeviceListenerWrap::OnDeviceOnline(NodeBasicInfo *info)
{
    std::string udid = softBusAdapter_->GetUdidByNodeId(std::string(info->networkId));
    LOG_INFO("[Online] id:%{public}s, name:%{public}s, typeId:%{public}d", SoftBusAdapter::ToBeAnonymous(udid).c_str(),
        info->deviceName, info->deviceTypeId);
    NotifyAll(info, DeviceChangeType::DEVICE_ONLINE);
}

void AppDeviceListenerWrap::SetDeviceHandler(SoftBusAdapter *handler)
{
    LOG_INFO("SetDeviceHandler.");
    softBusAdapter_ = handler;
}

void AppDeviceListenerWrap::NotifyAll(NodeBasicInfo *info, DeviceChangeType type)
{
    DeviceInfo di = { std::string(info->networkId), std::string(info->deviceName), std::to_string(info->deviceTypeId) };
    softBusAdapter_->NotifyAll(di, type);
}

SoftBusAdapter::SoftBusAdapter()
{
    LOG_INFO("begin");
    AppDeviceListenerWrap::SetDeviceHandler(this);
    AppDataListenerWrap::SetDataHandler(this);

    nodeStateCb_.events = EVENT_NODE_STATE_MASK;
    nodeStateCb_.onNodeOnline = AppDeviceListenerWrap::OnDeviceOnline;
    nodeStateCb_.onNodeOffline = AppDeviceListenerWrap::OnDeviceOffline;
    nodeStateCb_.onNodeBasicInfoChanged = AppDeviceListenerWrap::OnDeviceInfoChanged;

    sessionListener_.OnSessionOpened = AppDataListenerWrap::OnSessionOpened;
    sessionListener_.OnSessionClosed = AppDataListenerWrap::OnSessionClosed;
    sessionListener_.OnBytesReceived = AppDataListenerWrap::OnBytesReceived;
    sessionListener_.OnMessageReceived = AppDataListenerWrap::OnMessageReceived;

    semaphore_ = std::make_unique<Semaphore>(0);
}

SoftBusAdapter::~SoftBusAdapter()
{
    LOG_INFO("begin");
    int32_t errNo = UnregNodeDeviceStateCb(&nodeStateCb_);
    if (errNo != SOFTBUS_OK) {
        LOG_ERROR("UnregNodeDeviceStateCb fail %{public}d", errNo);
    }
}

void SoftBusAdapter::Init()
{
    LOG_INFO("begin");
    std::thread th = std::thread([&]() {
        auto communicator = std::make_shared<ProcessCommunicatorImpl>();
        auto retcom = DistributedDB::KvStoreDelegateManager::SetProcessCommunicator(communicator);
        LOG_INFO("set communicator ret:%{public}d.", static_cast<int>(retcom));
        int i = 0;
        constexpr int RETRY_TIMES = 300;
        while (i++ < RETRY_TIMES) {
            int32_t errNo = RegNodeDeviceStateCb("ohos.objectstore", &nodeStateCb_);
            if (errNo != SOFTBUS_OK) {
                LOG_ERROR("RegNodeDeviceStateCb fail %{public}d, time:%{public}d", errNo, i);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            LOG_INFO("RegNodeDeviceStateCb success");
            return;
        }
        LOG_ERROR("Init failed %{public}d times and exit now.", RETRY_TIMES);
    });
    th.detach();
}

Status SoftBusAdapter::StartWatchDeviceChange(
    const AppDeviceStatusChangeListener *observer, __attribute__((unused)) const PipeInfo &pipeInfo)
{
    LOG_INFO("begin");
    if (observer == nullptr) {
        LOG_WARN("observer is null.");
        return Status::ERROR;
    }
    std::lock_guard<std::mutex> lock(deviceChangeMutex_);
    auto result = listeners_.insert(observer);
    if (!result.second) {
        LOG_WARN("Add listener error.");
        return Status::ERROR;
    }
    LOG_INFO("end");
    return Status::SUCCESS;
}

Status SoftBusAdapter::StopWatchDeviceChange(
    const AppDeviceStatusChangeListener *observer, __attribute__((unused)) const PipeInfo &pipeInfo)
{
    LOG_INFO("begin");
    if (observer == nullptr) {
        LOG_WARN("observer is null.");
        return Status::ERROR;
    }
    std::lock_guard<std::mutex> lock(deviceChangeMutex_);
    auto result = listeners_.erase(observer);
    if (result <= 0) {
        return Status::ERROR;
    }
    LOG_INFO("end");
    return Status::SUCCESS;
}

void SoftBusAdapter::NotifyAll(const DeviceInfo &deviceInfo, const DeviceChangeType &type)
{
    std::thread th = std::thread([this, deviceInfo, type]() {
        std::vector<const AppDeviceStatusChangeListener *> listeners;
        {
            std::lock_guard<std::mutex> lock(deviceChangeMutex_);
            for (const auto &listener : listeners_) {
                listeners.push_back(listener);
            }
        }
        LOG_DEBUG("high");
        std::string udid = GetUdidByNodeId(deviceInfo.deviceId);
        LOG_DEBUG("[Notify] to DB from: %{public}s, type:%{public}d", ToBeAnonymous(udid).c_str(), type);
        UpdateRelationship(deviceInfo.deviceId, type);
        for (const auto &device : listeners) {
            if (device == nullptr) {
                continue;
            }
            if (device->GetChangeLevelType() == ChangeLevelType::HIGH) {
                DeviceInfo di = { udid, deviceInfo.deviceName, deviceInfo.deviceType };
                device->OnDeviceChanged(di, type);
                break;
            }
        }
        LOG_DEBUG("low");
        for (const auto &device : listeners) {
            if (device == nullptr) {
                continue;
            }
            if (device->GetChangeLevelType() == ChangeLevelType::LOW) {
                DeviceInfo di = { udid, deviceInfo.deviceName, deviceInfo.deviceType };
                device->OnDeviceChanged(di, DeviceChangeType::DEVICE_OFFLINE);
                device->OnDeviceChanged(di, type);
            }
        }
        LOG_DEBUG("min");
        for (const auto &device : listeners) {
            if (device == nullptr) {
                continue;
            }
            if (device->GetChangeLevelType() == ChangeLevelType::MIN) {
                DeviceInfo di = { udid, deviceInfo.deviceName, deviceInfo.deviceType };
                device->OnDeviceChanged(di, type);
            }
        }
    });
    th.detach();
}

std::vector<DeviceInfo> SoftBusAdapter::GetDeviceList() const
{
    std::vector<DeviceInfo> dis;
    NodeBasicInfo *info = nullptr;
    int32_t infoNum = 0;
    dis.clear();

    int32_t ret = GetAllNodeDeviceInfo("ohos.objectstore", &info, &infoNum);
    if (ret != SOFTBUS_OK) {
        LOG_ERROR("GetAllNodeDeviceInfo error");
        return dis;
    }
    LOG_DEBUG("GetAllNodeDeviceInfo success infoNum=%{public}d", infoNum);

    for (int i = 0; i < infoNum; i++) {
        std::string udid = GetUdidByNodeId(std::string(info[i].networkId));
        DeviceInfo deviceInfo = { udid, std::string(info[i].deviceName), std::to_string(info[i].deviceTypeId) };
        dis.push_back(deviceInfo);
    }
    if (info != NULL) {
        FreeNodeInfo(info);
    }
    return dis;
}

DeviceInfo SoftBusAdapter::GetLocalDevice()
{
    if (!localInfo_.deviceId.empty()) {
        return localInfo_;
    }

    NodeBasicInfo info;
    int32_t ret = GetLocalNodeDeviceInfo("ohos.objectstore", &info);
    if (ret != SOFTBUS_OK) {
        LOG_ERROR("GetLocalNodeDeviceInfo error");
        return DeviceInfo();
    }
    std::string uuid = GetUdidByNodeId(std::string(info.networkId));
    LOG_DEBUG("[LocalDevice] id:%{private}s, name:%{private}s, type:%{private}d", ToBeAnonymous(uuid).c_str(),
        info.deviceName, info.deviceTypeId);
    localInfo_ = { uuid, std::string(info.deviceName), std::to_string(info.deviceTypeId) };
    return localInfo_;
}

std::string SoftBusAdapter::GetUdidByNodeId(const std::string &nodeId) const
{
    char udid[ID_BUF_LEN] = { 0 };
    int32_t ret = GetNodeKeyInfo("ohos.objectstore", nodeId.c_str(), NodeDeivceInfoKey::NODE_KEY_UDID,
        reinterpret_cast<uint8_t *>(udid), ID_BUF_LEN);
    if (ret != SOFTBUS_OK) {
        LOG_WARN("GetNodeKeyInfo error, nodeId:%{public}s", ToBeAnonymous(nodeId).c_str());
        return "";
    }
    return std::string(udid);
}

DeviceInfo SoftBusAdapter::GetLocalBasicInfo() const
{
    LOG_DEBUG("begin");
    NodeBasicInfo info;
    int32_t ret = GetLocalNodeDeviceInfo("ohos.objectstore", &info);
    if (ret != SOFTBUS_OK) {
        LOG_ERROR("GetLocalNodeDeviceInfo error");
        return DeviceInfo();
    }
    LOG_DEBUG("[LocalBasicInfo] networkId:%{private}s, name:%{private}s, "
              "type:%{private}d",
        ToBeAnonymous(std::string(info.networkId)).c_str(), info.deviceName, info.deviceTypeId);
    DeviceInfo localInfo = { std::string(info.networkId), std::string(info.deviceName),
        std::to_string(info.deviceTypeId) };
    return localInfo;
}

std::vector<DeviceInfo> SoftBusAdapter::GetRemoteNodesBasicInfo() const
{
    LOG_DEBUG("begin");
    std::vector<DeviceInfo> dis;
    NodeBasicInfo *info = nullptr;
    int32_t infoNum = 0;
    dis.clear();

    int32_t ret = GetAllNodeDeviceInfo("ohos.objectstore", &info, &infoNum);
    if (ret != SOFTBUS_OK) {
        LOG_ERROR("GetAllNodeDeviceInfo error");
        return dis;
    }
    LOG_DEBUG("GetAllNodeDeviceInfo success infoNum=%{public}d", infoNum);

    for (int i = 0; i < infoNum; i++) {
        dis.push_back(
            { std::string(info[i].networkId), std::string(info[i].deviceName), std::to_string(info[i].deviceTypeId) });
    }
    if (info != NULL) {
        FreeNodeInfo(info);
    }
    return dis;
}

void SoftBusAdapter::UpdateRelationship(const std::string &networkid, const DeviceChangeType &type)
{
    auto udid = GetUdidByNodeId(networkid);
    lock_guard<mutex> lock(networkMutex_);
    switch (type) {
        case DeviceChangeType::DEVICE_OFFLINE: {
            auto size = this->networkId2Udid_.erase(networkid);
            if (size == 0) {
                LOG_WARN("not found id:%{public}s.", networkid.c_str());
            }
            break;
        }
        case DeviceChangeType::DEVICE_ONLINE: {
            std::pair<std::string, std::string> value = { networkid, udid };
            auto res = this->networkId2Udid_.insert(std::move(value));
            if (!res.second) {
                LOG_WARN("insert failed.");
            }
            break;
        }
        default: {
            LOG_WARN("unknown type.");
            break;
        }
    }
}
std::string SoftBusAdapter::ToNodeID(const std::string &nodeId) const
{
    {
        lock_guard<mutex> lock(networkMutex_);
        for (auto const &e : networkId2Udid_) {
            if (nodeId == e.second) { // id is udid
                return e.first;
            }
        }
    }

    LOG_WARN("get the network id from devices.");
    std::vector<DeviceInfo> devices;
    NodeBasicInfo *info = nullptr;
    int32_t infoNum = 0;
    std::string networkId;
    int32_t ret = GetAllNodeDeviceInfo("ohos.objectstore", &info, &infoNum);
    if (ret == SOFTBUS_OK) {
        lock_guard<mutex> lock(networkMutex_);
        for (int i = 0; i < infoNum; i++) {
            if (networkId2Udid_.find(info[i].networkId) != networkId2Udid_.end()) {
                continue;
            }
            auto udid = GetUdidByNodeId(std::string(info[i].networkId));
            networkId2Udid_.insert({ info[i].networkId, udid });
            if (udid == nodeId) {
                networkId = info[i].networkId;
            }
        }
    }
    if (info != NULL) {
        FreeNodeInfo(info);
    }
    return networkId;
}

std::string SoftBusAdapter::ToBeAnonymous(const std::string &name)
{
    if (name.length() <= HEAD_SIZE) {
        return DEFAULT_ANONYMOUS;
    }

    if (name.length() < MIN_SIZE) {
        return (name.substr(0, HEAD_SIZE) + REPLACE_CHAIN);
    }

    return (name.substr(0, HEAD_SIZE) + REPLACE_CHAIN + name.substr(name.length() - END_SIZE, END_SIZE));
}

std::shared_ptr<SoftBusAdapter> SoftBusAdapter::GetInstance()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&] { instance_ = std::make_shared<SoftBusAdapter>(); });
    return instance_;
}

Status SoftBusAdapter::StartWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo)
{
    LOG_DEBUG("begin");
    if (observer == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    lock_guard<mutex> lock(dataChangeMutex_);
    auto it = dataChangeListeners_.find(pipeInfo.pipeId);
    if (it != dataChangeListeners_.end()) {
        LOG_WARN("Add listener error or repeated adding.");
        return Status::ERROR;
    }
    LOG_DEBUG("current appid %{public}s", pipeInfo.pipeId.c_str());
    dataChangeListeners_.insert({ pipeInfo.pipeId, observer });
    return Status::SUCCESS;
}

Status SoftBusAdapter::StopWatchDataChange(
    __attribute__((unused)) const AppDataChangeListener *observer, const PipeInfo &pipeInfo)
{
    LOG_DEBUG("begin");
    lock_guard<mutex> lock(dataChangeMutex_);
    if (dataChangeListeners_.erase(pipeInfo.pipeId)) {
        return Status::SUCCESS;
    }
    LOG_WARN("stop data observer error, pipeInfo:%{public}s", pipeInfo.pipeId.c_str());
    return Status::ERROR;
}

Status SoftBusAdapter::SendData(
    const PipeInfo &pipeInfo, const DeviceId &deviceId, const uint8_t *ptr, int size, const MessageInfo &info)
{
    SessionAttribute attr;
    attr.dataType = TYPE_BYTES;
    LOG_DEBUG("[SendData] to %{public}s ,session:%{public}s, size:%{public}d",
        ToBeAnonymous(deviceId.deviceId).c_str(), pipeInfo.pipeId.c_str(), size);
    int sessionId = OpenSession(
        pipeInfo.pipeId.c_str(), pipeInfo.pipeId.c_str(), ToNodeID(deviceId.deviceId).c_str(), "GROUP_ID", &attr);
    if (sessionId < 0) {
        LOG_WARN("OpenSession %{public}s, type:%{public}d failed, sessionId:%{public}d", pipeInfo.pipeId.c_str(),
            info.msgType, sessionId);
        return Status::CREATE_SESSION_ERROR;
    }
    SetOpenSessionId(sessionId);
    int state = WaitSessionOpen();
    {
        lock_guard<mutex> lock(notifyFlagMutex_);
        notifyFlag_ = false;
    }
    LOG_DEBUG("Waited for notification, state:%{public}d", state);
    if (state != SOFTBUS_OK) {
        LOG_ERROR("OpenSession callback result error");
        return Status::CREATE_SESSION_ERROR;
    }
    LOG_DEBUG("[SendBytes] start,sessionId is %{public}d, size is %{public}d, "
              "session type is %{public}d.",
        sessionId, size, attr.dataType);
    int32_t ret = SendBytes(sessionId, (void *)ptr, size);
    if (ret != SOFTBUS_OK) {
        LOG_ERROR("[SendBytes] to %{public}d failed, ret:%{public}d.", sessionId, ret);
        return Status::ERROR;
    }
    return Status::SUCCESS;
}

bool SoftBusAdapter::IsSameStartedOnPeer(
    const struct PipeInfo &pipeInfo, __attribute__((unused)) const struct DeviceId &peer)
{
    LOG_INFO(
        "pipeInfo:%{public}s peer.deviceId:%{public}s", pipeInfo.pipeId.c_str(), ToBeAnonymous(peer.deviceId).c_str());
    {
        lock_guard<mutex> lock(busSessionMutex_);
        if (busSessionMap_.find(pipeInfo.pipeId + peer.deviceId) != busSessionMap_.end()) {
            LOG_INFO("Found session in map. Return true.");
            return true;
        }
    }
    SessionAttribute attr;
    attr.dataType = TYPE_BYTES;
    int sessionId = OpenSession(
        pipeInfo.pipeId.c_str(), pipeInfo.pipeId.c_str(), ToNodeID(peer.deviceId).c_str(), "GROUP_ID", &attr);
    LOG_INFO("[IsSameStartedOnPeer] sessionId=%{public}d", sessionId);
    if (sessionId == INVALID_SESSION_ID) {
        LOG_ERROR("OpenSession return null, pipeInfo:%{public}s. Return false.", pipeInfo.pipeId.c_str());
        return false;
    }
    LOG_INFO("session started, pipeInfo:%{public}s. sessionId:%{public}d Return "
             "true. ",
        pipeInfo.pipeId.c_str(), sessionId);
    return true;
}

void SoftBusAdapter::SetMessageTransFlag(const PipeInfo &pipeInfo, bool flag)
{
    LOG_INFO("pipeInfo: %{public}s flag: %{public}d", pipeInfo.pipeId.c_str(), static_cast<bool>(flag));
    flag_ = flag;
}

int SoftBusAdapter::CreateSessionServerAdapter(const std::string &sessionName)
{
    LOG_DEBUG("begin");
    return CreateSessionServer("ohos.objectstore", sessionName.c_str(), &sessionListener_);
}

int SoftBusAdapter::RemoveSessionServerAdapter(const std::string &sessionName) const
{
    LOG_DEBUG("begin");
    return RemoveSessionServer("ohos.objectstore", sessionName.c_str());
}

void SoftBusAdapter::InsertSession(const std::string &sessionName)
{
    lock_guard<mutex> lock(busSessionMutex_);
    busSessionMap_.insert({ sessionName, true });
}

void SoftBusAdapter::DeleteSession(const std::string &sessionName)
{
    lock_guard<mutex> lock(busSessionMutex_);
    busSessionMap_.erase(sessionName);
}

void SoftBusAdapter::NotifyDataListeners(
    const uint8_t *ptr, const int size, const std::string &deviceId, const PipeInfo &pipeInfo)
{
    LOG_DEBUG("begin");
    lock_guard<mutex> lock(dataChangeMutex_);
    auto it = dataChangeListeners_.find(pipeInfo.pipeId);
    if (it != dataChangeListeners_.end()) {
        LOG_DEBUG("ready to notify, pipeName:%{public}s, deviceId:%{public}s.", pipeInfo.pipeId.c_str(),
            ToBeAnonymous(deviceId).c_str());
        DeviceInfo deviceInfo = { deviceId, "", "" };
        it->second->OnMessage(deviceInfo, ptr, size, pipeInfo);
        return;
    }
    LOG_WARN("no listener %{public}s.", pipeInfo.pipeId.c_str());
}

int SoftBusAdapter::WaitSessionOpen()
{
    {
        lock_guard<mutex> lock(notifyFlagMutex_);
        if (notifyFlag_) {
            LOG_DEBUG("already notified return");
            return 0;
        }
    }
    return semaphore_->Wait();
}

void SoftBusAdapter::NotifySessionOpen(const int &state)
{
    semaphore_->Signal(state);
    lock_guard<mutex> lock(notifyFlagMutex_);
    notifyFlag_ = true;
}

int SoftBusAdapter::GetOpenSessionId()
{
    lock_guard lock(openSessionIdMutex_);
    return openSessionId_;
}

void SoftBusAdapter::SetOpenSessionId(const int &sessionId)
{
    lock_guard<mutex> lock(openSessionIdMutex_);
    openSessionId_ = sessionId;
}

void AppDataListenerWrap::SetDataHandler(SoftBusAdapter *handler)
{
    LOG_INFO("begin");
    softBusAdapter_ = handler;
}

int AppDataListenerWrap::OnSessionOpened(int sessionId, int result)
{
    LOG_INFO("[SessionOpen] sessionId:%{public}d, result:%{public}d", sessionId, result);
    char mySessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerSessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerDevId[DEVICE_ID_SIZE_MAX] = "";

    if (sessionId == softBusAdapter_->GetOpenSessionId()) {
        softBusAdapter_->NotifySessionOpen(result);
    }
    if (result != SOFTBUS_OK) {
        LOG_WARN("session %{public}d open failed, result:%{public}d.", sessionId, result);
        return result;
    }
    int ret = GetMySessionName(sessionId, mySessionName, sizeof(mySessionName));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my session name failed, session id is %{public}d.", sessionId);
        return SOFTBUS_ERR;
    }
    ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my peer session name failed, session id is %{public}d.", sessionId);
        return SOFTBUS_ERR;
    }
    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my peer device id failed, session id is %{public}d.", sessionId);
        return SOFTBUS_ERR;
    }
    std::string peerUdid = softBusAdapter_->GetUdidByNodeId(std::string(peerDevId));
    LOG_DEBUG("[SessionOpen] mySessionName:%{public}s, "
              "peerSessionName:%{public}s, peerDevId:%{public}s",
        mySessionName, peerSessionName, SoftBusAdapter::ToBeAnonymous(peerUdid).c_str());

    if (strlen(peerSessionName) < 1) {
        softBusAdapter_->InsertSession(std::string(mySessionName) + peerUdid);
    } else {
        softBusAdapter_->InsertSession(std::string(peerSessionName) + peerUdid);
    }
    return 0;
}

void AppDataListenerWrap::OnSessionClosed(int sessionId)
{
    LOG_INFO("[SessionClosed] sessionId:%{public}d", sessionId);
    char mySessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerSessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerDevId[DEVICE_ID_SIZE_MAX] = "";

    int ret = GetMySessionName(sessionId, mySessionName, sizeof(mySessionName));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my session name failed, session id is %{public}d.", sessionId);
        return;
    }
    ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my peer session name failed, session id is %{public}d.", sessionId);
        return;
    }
    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my peer device id failed, session id is %{public}d.", sessionId);
        return;
    }
    std::string peerUdid = softBusAdapter_->GetUdidByNodeId(std::string(peerDevId));
    LOG_DEBUG("[SessionClosed] mySessionName:%{public}s, "
              "peerSessionName:%{public}s, peerDevId:%{public}s",
        mySessionName, peerSessionName, SoftBusAdapter::ToBeAnonymous(peerUdid).c_str());

    if (strlen(peerSessionName) < 1) {
        softBusAdapter_->DeleteSession(std::string(mySessionName) + peerUdid);
    } else {
        softBusAdapter_->DeleteSession(std::string(peerSessionName) + peerUdid);
    }
}

void AppDataListenerWrap::OnMessageReceived(int sessionId, const void *data, unsigned int dataLen)
{
    LOG_INFO("begin");
    if (sessionId == INVALID_SESSION_ID) {
        return;
    }
    char peerSessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerDevId[DEVICE_ID_SIZE_MAX] = "";
    int ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my peer session name failed, session id is %{public}d.", sessionId);
        return;
    }
    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my peer device id failed, session id is %{public}d.", sessionId);
        return;
    }
    std::string peerUdid = softBusAdapter_->GetUdidByNodeId(std::string(peerDevId));
    LOG_DEBUG("[MessageReceived] sessionId:%{public}d, "
              "peerSessionName:%{public}s, peerDevId:%{public}s",
        sessionId, peerSessionName, SoftBusAdapter::ToBeAnonymous(peerUdid).c_str());
    NotifyDataListeners(reinterpret_cast<const uint8_t *>(data), dataLen, peerUdid, { std::string(peerSessionName) });
}

void AppDataListenerWrap::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    LOG_INFO("begin");
    if (sessionId == INVALID_SESSION_ID) {
        return;
    }
    char peerSessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerDevId[DEVICE_ID_SIZE_MAX] = "";
    int ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my peer session name failed, session id is %{public}d.", sessionId);
        return;
    }
    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != SOFTBUS_OK) {
        LOG_WARN("get my peer device id failed, session id is %{public}d.", sessionId);
        return;
    }
    std::string peerUdid = softBusAdapter_->GetUdidByNodeId(std::string(peerDevId));
    LOG_DEBUG("[BytesReceived] sessionId:%{public}d, peerSessionName:%{public}s, "
              "peerDevId:%{public}s",
        sessionId, peerSessionName, SoftBusAdapter::ToBeAnonymous(peerUdid).c_str());
    NotifyDataListeners(reinterpret_cast<const uint8_t *>(data), dataLen, peerUdid, { std::string(peerSessionName) });
}

void AppDataListenerWrap::NotifyDataListeners(
    const uint8_t *ptr, const int size, const std::string &deviceId, const PipeInfo &pipeInfo)
{
    return softBusAdapter_->NotifyDataListeners(ptr, size, deviceId, pipeInfo);
}
} // namespace ObjectStore
} // namespace OHOS
