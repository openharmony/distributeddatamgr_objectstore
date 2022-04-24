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
        dataModel: distr.g_dataModel
    },
    changeCallback(sessionId, changeData) {
        changeData.forEach(element => {
            if (element == "documentList") {
                console.info("newest data " + JSON.stringify(this.dataModel.distributedObject.documentList));
                // 触发界面刷新
                this.dataModel.documentList = distr.g_dataModel.distributedObject.documentList;
            } else if (element == "documentSize") {
                let size = distr.g_dataModel.distributedObject.documentSize;
                console.info("newest size " + size);
                // 触发界面刷新
                this.dataModel.distributedObject.documentSize = size;
            }
        });
		// 要在callback里刷新界面，则需要将正确的this绑定给callback
        this.dataModel.distributedObject.on("change", this.changeCallback.bind(this));
    },
    onInit() {
        console.info("objectstore in index page ");
        console.info(JSON.stringify(this.dataModel.documentList));
        console.info(JSON.stringify(distr.g_dataModel.distributedObject.documentList));
        distr.g_dataModel.setCallback(this.changeCallback);
        distr.g_dataModel.setStatusCallback((sessionId, networkId, status) => {
            console.info("objectstore status change ${networkId} ${status}");
            if (status == "online") {
                this.dataModel.imgSrc = "common/green.png";
            } else {
                this.dataModel.imgSrc = "common/red.png";
            }
        })
    },
    onDestroy() {
        console.info("objectstore exit index page");
        distr.g_dataModel.clearCallback();
    },
    add: function () {
        router.replace({
            uri: "pages/add/add"
        })
    },
    clear: function () {
        // 触发界面刷新
        this.dataModel.documentList = [];
        this.dataModel.distributedObject.documentSize = 0;
        distr.g_dataModel.clear();
    },
    detail: function (msg) {
        router.replace({
            uri: "pages/detail/detail",
            params: {
                title: msg.target.dataSet.title,
                content: msg.target.dataSet.content,
                oriTitle: msg.target.dataSet.title,
                oriContent: msg.target.dataSet.content,
                editIndex: msg.target.dataSet.index
            }
        })
    }
}



