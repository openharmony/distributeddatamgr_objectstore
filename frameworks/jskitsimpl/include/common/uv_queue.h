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
#ifndef OHOS_UV_QUEUE_H
#define OHOS_UV_QUEUE_H
#include <functional>
#include <map>
#include <shared_mutex>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "uv.h"

namespace OHOS::ObjectStore {
class UvQueue {
    using NapiArgsGenerator = std::function<void(napi_env env, napi_ref& fuc, int& argc, napi_value* argv)>;

public:
    UvQueue(napi_env env);
    virtual ~UvQueue();

    void CallFunction(const std::string &sessionId, const std::vector<std::string> &changeData, napi_ref callback);
private:
    napi_env env_ = nullptr;
    std::shared_mutex mutex_{};
    std::map<napi_ref, std::pair<std::string, std::vector<std::string>>> args_;
    uv_loop_s* loop_ = nullptr;
};
}
#endif
