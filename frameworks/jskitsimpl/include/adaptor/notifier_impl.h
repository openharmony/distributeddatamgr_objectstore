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

#ifndef JS_NOTIFIER_IMPL_H
#define JS_NOTIFIER_IMPL_H

#include <mutex>

#include "distributed_objectstore.h"
#include "js_watcher.h"
namespace OHOS::ObjectStore {
class NotifierImpl : public StatusNotifier {
public:
    static std::shared_ptr<NotifierImpl> GetInstance();
    virtual ~NotifierImpl();
    void AddWatcher(std::string &sessionId, JSWatcher *watcher);
    void DelWatcher(std::string &sessionId);
    void OnChanged(
        const std::string &sessionId, const std::string &networkId, const std::string &onlineStatus) override;

private:
    std::mutex mutex_;
    std::map<std::string, JSWatcher *> watchers_;
};
} // namespace OHOS::ObjectStore
#endif // JS_NOTIFIER_IMPL_H
