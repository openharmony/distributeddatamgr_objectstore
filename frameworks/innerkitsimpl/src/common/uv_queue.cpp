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
#define LOG_TAG "UvQueue"
#include "uv_queue.h"

#include "js_util.h"
#include "logger.h"

namespace OHOS::ObjectStore {
UvQueue::UvQueue(napi_env env) : env_(env)
{
    napi_get_uv_event_loop(env, &loop_);
}

UvQueue::~UvQueue()
{
    LOG_DEBUG("no memory leak for queue-callback");
}

void UvQueue::CallFunction(const std::string &sessionId, const std::vector<std::string> &changeData, napi_ref callback)
{
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOG_ERROR("no memory for uv_work_t");
        return;
    }
    work->data = this;
    {
        std::unique_lock<std::shared_mutex> cacheLock(mutex_);
        if (args_.count(callback) != 0) {
            std::pair<std::string, std::vector<std::string>> newData = args_.at(callback);
            std::for_each(
                changeData.begin(), changeData.end(), [&](const auto &item) { newData.second.push_back(item); });
            args_.insert_or_assign(callback, newData);
        } else {
            std::pair<std::string, std::vector<std::string>> data(sessionId, changeData);
            args_.insert_or_assign(callback, data);
        }
    }

    uv_queue_work(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int uvstatus) {
            auto queue = static_cast<UvQueue *>(work->data);
            const int argc = 2;
            napi_value argv[argc];
            {
                std::unique_lock<std::shared_mutex> cacheLock(queue->mutex_);
                std::for_each(queue->args_.begin(), queue->args_.end(), [&](const auto &item) {
                    napi_value callback = nullptr;
                    napi_get_reference_value(queue->env_, item.first, &callback);
                    napi_value global = nullptr;
                    napi_get_global(queue->env_, &global);
                    JSUtil::SetValue(queue->env_, item.second.first, argv[0]);
                    JSUtil::SetValue(queue->env_, item.second.second, argv[1]);
                    LOG_INFO(
                        "start %{public}s, %{public}s", item.second.first.c_str(), item.second.second.at(0).c_str());
                    napi_value result;
                    napi_status status = napi_call_function(queue->env_, global, callback, argc, argv, &result);
                    if (status != napi_ok) {
                        LOG_ERROR("notify data change failed status:%{public}d callback:%{public}p", status, callback);
                    }
                    LOG_INFO("end");
                });
                queue->args_.clear();
            }

            delete work;
            work = nullptr;
        });
}
} // namespace OHOS::ObjectStore
