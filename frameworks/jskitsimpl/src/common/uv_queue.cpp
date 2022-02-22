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
#include "uv_queue.h"

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

void UvQueue::CallFunction(Process process, void *argv)
{
    if (process == nullptr || argv == nullptr) {
        LOG_ERROR("nullptr");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LOG_ERROR("no memory for uv_work_t");
        return;
    }
    work->data = this;
    {
        std::unique_lock<std::shared_mutex> cacheLock(mutex_);
        if (args_.count(process) != 0) {
            std::list<void *> newData = args_.at(process);
            newData.push_back(argv);
            args_.insert_or_assign(process, newData);
        } else {
            std::list<void *> data;
            data.push_back(argv);
            args_.insert_or_assign(process, data);
        }
    }

    uv_queue_work(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int uvstatus) {
            auto queue = static_cast<UvQueue *>(work->data);
            {
                std::unique_lock<std::shared_mutex> cacheLock(queue->mutex_);
                for (auto &item : queue->args_) {
                    item.first(queue->env_, item.second);
                }
                queue->args_.clear();
            }

            delete work;
            work = nullptr;
        });
}
} // namespace OHOS::ObjectStore
