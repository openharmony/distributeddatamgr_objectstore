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
#ifndef UV_QUEUE_H
#define UV_QUEUE_H
#include <functional>
#include <list>
#include <map>
#include <shared_mutex>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "uv.h"

namespace OHOS::ObjectStore {
typedef void (*Process)(napi_env env, std::list<void *> &);
class UvQueue {
public:
    UvQueue(napi_env env);
    virtual ~UvQueue();

    void CallFunction(Process process, void *argv);

private:
    napi_env env_;
    std::shared_mutex mutex_{};
    // key is callback,value is list of args
    std::map<Process, std::list<void *>> args_;
    uv_loop_s *loop_ = nullptr;
};
} // namespace OHOS::ObjectStore
#endif
