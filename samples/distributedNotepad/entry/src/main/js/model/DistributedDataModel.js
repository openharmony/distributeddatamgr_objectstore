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

import distributedObject from '@ohos.data.distributedDataObject'
import featureAbility from '@ohos.ability.featureAbility';

function grantPermission() {
    console.info('grantPermission');
    let context = featureAbility.getContext();
    context.requestPermissionsFromUser(['ohos.permission.DISTRIBUTED_DATASYNC'], 666, function (result) {
        console.info(`result.requestCode=${result.requestCode}`)

    })
    console.info('end grantPermission');
}
export default class DistributedDataModel {
    documentList = [];
    distributedObject; // distributed proxy
    imgSrc = "common/red.png";
    #callback;
    #statusCallback;

    constructor() {
        this.distributedObject = distributedObject.createDistributedObject({
            documentList: this.documentList,
            documentSize: 0
        });
        this.share();
    }

    clearCallback() {
        this.distributedObject.off("change");
        this.#callback = undefined;
        this.distributedObject.off("status");
        this.#statusCallback = undefined;
    }

    setCallback(callback) {
        if (this.#callback == callback) {
            console.info("same callback");
            return;
        }
        console.info("start off");
        if (this.#callback != undefined) {
            this.distributedObject.off("change", this.#callback);
        }
        this.#callback = callback;
        console.info("start watch change");
        this.distributedObject.on("change", this.#callback);
    }

    setStatusCallback(callback) {
        if (this.#statusCallback == callback) {
            console.info("same callback");
            return;
        }
        console.info("start off");
        if (this.#statusCallback != undefined) {
            this.distributedObject.off("status", this.#statusCallback);
        }
        this.#statusCallback = callback;
        console.info("start watch change");
        this.distributedObject.on("status", this.#statusCallback);
    }

    share() {
        console.info("start share");
        if (this.distributedObject.__sessionId == undefined) {
            grantPermission()
            this.distributedObject.setSessionId("123456")
        } else {       
          }
    }

    update(index, title, content) {
        console.info("doUpdate " + title + index);
        this.documentList[index] = {
            index: index, title: title, content: content
        };
        this.distributedObject.documentList = this.documentList;
        console.info("update my documentList " + JSON.stringify(this.documentList));
    }

    add(title, content) {
        console.info("doAdd " + title + content);
        console.info("documentList " + JSON.stringify(this.documentList));
        this.documentList = this.distributedObject.documentList;
        this.documentList[this.distributedObject.documentSize] = {
            index: this.distributedObject.documentSize, title: title, content: content
        };
        this.distributedObject.documentList = this.documentList;
        this.distributedObject.documentSize++;
        console.info("add my documentList " + JSON.stringify(this.documentList));
    }


    clear() {
        console.info("doClear ");
        this.documentList = [];
        this.distributedObject.documentList = this.documentList;
        this.distributedObject.documentSize = 0;
        console.info("doClear finish");
    }
}

export var g_dataModel = new DistributedDataModel();