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

import router from '@system.router'
import * as distr from '../../../model/DistributedDataModel.js'

export default {
    data: {
        title: "天气不错奥",
        content: "今天天气不错",
        edit: "保存"
    },
    onInit() {
        console.info("objectstore in detail page");
    },
    back: function () {
        router.replace({
            uri: "pages/index/index",
            params: {
                dataModel: distr.g_dataModel
            }
        })
    },
    change: function (e) {
        this.title = e.text;
    },
    changeContent: function (e) {
        this.content = e.text;
    },
    save: function () {
        console.info("start save "+ JSON.stringify(this.data));
        distr.g_dataModel.update(this.editIndex,  this.title, this.content);
        router.replace({
            uri: "pages/index/index",
            params: {
                dataModel: distr.g_dataModel
            }
        })
    }
}
