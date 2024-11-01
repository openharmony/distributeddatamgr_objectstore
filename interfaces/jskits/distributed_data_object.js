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
 
const distributedObject = requireInternal("data.distributedDataObject");
const SESSION_ID = "__sessionId";
const VERSION = "__version";
const COMPLEX_TYPE = "[COMPLEX]";
const STRING_TYPE = "[STRING]";
const NULL_TYPE = "[NULL]"
const JS_ERROR = 1;

class Distributed {
    constructor(obj) {
        this.__proxy = obj;
        Object.keys(obj).forEach(key => {
            Object.defineProperty(this, key, {
                enumerable: true,
                configurable: true,
                get: function () {
                    return this.__proxy[key];
                },
                set: function (newValue) {
                    this[VERSION]++;
                    this.__proxy[key] = newValue;
                }
            });
        });
        Object.defineProperty(this, SESSION_ID, {
            enumerable: true,
            configurable: true,
            get: function () {
                return this.__proxy[SESSION_ID];
            },
            set: function (newValue) {
                this.__proxy[SESSION_ID] = newValue;
            }
        });
        this.__objectId = randomNum();
        this[VERSION] = 0;
        console.info("constructor success ");
    }

    setSessionId(sessionId) {
        if (sessionId == null || sessionId == "") {
            leaveSession(this.__proxy);
            return false;
        }
        if (this.__proxy[SESSION_ID] == sessionId) {
            console.info("same session has joined " + sessionId);
            return true;
        }
        leaveSession(this.__proxy);
        let object = joinSession(this.__proxy, this.__objectId, sessionId);
        if (object != null) {
            this.__proxy = object;
            return true;
        }
        return false;
    }

    on(type, callback) {
        onWatch(type, this.__proxy, callback);
        distributedObject.recordCallback(type, this.__objectId, callback);
    }

    off(type, callback) {
        offWatch(type, this.__proxy, callback);
        distributedObject.deleteCallback(type, this.__objectId, callback);
    }

    save(deviceId, callback) {
        if (this.__proxy[SESSION_ID] == null || this.__proxy[SESSION_ID] == "") {
            console.info("not join a session, can not do save");
            return JS_ERROR;
        }
        return this.__proxy.save(deviceId, this[VERSION], callback);
    }

    revokeSave(callback) {
        if (this.__proxy[SESSION_ID] == null || this.__proxy[SESSION_ID] == "") {
            console.info("not join a session, can not do revoke save");
            return JS_ERROR;
        }
        return this.__proxy.revokeSave(callback);
    }

    __proxy;
    __objectId;
    __version;
}

function randomNum() {
    return Math.random().toString(10).slice(-8);
}

function newDistributed(obj) {
    console.info("start newDistributed");
    if (obj == null) {
        console.error("object is null");
        return null;
    }
    return new Distributed(obj);
}

function joinSession(obj, objectId, sessionId) {
    console.info("start joinSession " + sessionId);
    if (obj == null || sessionId == null || sessionId == "") {
        console.error("object is null");
        return null;
    }

    let object = distributedObject.createObjectSync(sessionId, objectId);
    if (object == null) {
        console.error("create fail");
        return null;
    }
    Object.keys(obj).forEach(key => {
        console.info("start define " + key);
        Object.defineProperty(object, key, {
            enumerable: true,
            configurable: true,
            get: function () {
                console.info("start get " + key);
                let result = object.get(key);
                console.info("get " + result);
                if (typeof result == "string") {
                    if (result.startsWith(STRING_TYPE)) {
                        result = result.substr(STRING_TYPE.length);
                    } else if (result.startsWith(COMPLEX_TYPE)) {
                        result = JSON.parse(result.substr(COMPLEX_TYPE.length))
                    } else if (result.startsWith(NULL_TYPE)) {
                        result = null;
                    } else {
                        console.error("error type " + result);
                    }
                }
                console.info("get " + result + " success");
                return result;
            },
            set: function (newValue) {
                console.info("start set " + key + " " + newValue);
                if (typeof newValue == "object") {
                    let value = COMPLEX_TYPE + JSON.stringify(newValue);
                    object.put(key, value);
                    console.info("set " + key + " " + value);
                } else if (typeof newValue == "string") {
                    let value = STRING_TYPE + newValue;
                    object.put(key, value);
                    console.info("set " + key + " " + value);
                } else if (newValue === null) {
                    let value = NULL_TYPE;
                    object.put(key, value);
                    console.info("set " + key + " " + value);
                } else {
                    object.put(key, newValue);
                    console.info("set " + key + " " + newValue);
                }
            }
        });
        if (obj[key] != undefined) {
            object[key] = obj[key];
        }
    });

    Object.defineProperty(object, SESSION_ID, {
        value: sessionId,
        configurable: true,
    });
    return object;
}

function leaveSession(obj) {
    console.info("start leaveSession");
    if (obj == null || obj[SESSION_ID] == null || obj[SESSION_ID] == "") {
        console.warn("object is null");
        return;
    }
    Object.keys(obj).forEach(key => {
        Object.defineProperty(obj, key, {
            value: obj[key],
            configurable: true,
            writable: true,
            enumerable: true,
        });
    });
    // disconnect,delete object
    distributedObject.destroyObjectSync(obj);
    delete obj[SESSION_ID];
}

function onWatch(type, obj, callback) {
    console.info("start on " + obj[SESSION_ID]);
    if (obj[SESSION_ID] != null && obj[SESSION_ID] != undefined && obj[SESSION_ID].length > 0) {
        distributedObject.on(type, obj, callback);
    }
}

function offWatch(type, obj, callback = undefined) {
    console.info("start off " + obj[SESSION_ID] + " " + callback);
    if (obj[SESSION_ID] != null && obj[SESSION_ID] != undefined && obj[SESSION_ID].length > 0) {
        if (callback != undefined || callback != null) {
            distributedObject.off(type, obj, callback);
        } else {
            distributedObject.off(type, obj);
        }

    }
}

export default {
    createDistributedObject: newDistributed,
    genSessionId: randomNum
}