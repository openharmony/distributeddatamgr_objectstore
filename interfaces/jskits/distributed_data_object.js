const distributedObject = requireInternal("data.distributedDataObject");
const SESSION_ID = "__sessionId";
const COMPLEX_TYPE = "[COMPLEX]";
const STRING_TYPE = "[STRING]";

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
        console.info("constructor success ");
    };

    setSessionId(sessionId) {
        if (sessionId == null || sessionId == undefined || sessionId.length == 0) {
            leaveSession(this.__proxy);
            return false;
        }
        if (this.__proxy[SESSION_ID] == sessionId) {
            console.info("same session has joined " + sessionId);
            return true;
        }
        leaveSession(this.__proxy);
        let object = joinSession(this.__proxy, sessionId);
        if (object != null) {
            this.__proxy = object;
            return true;
        }
        return false;
    };

    on(type, callback) {
        onWatch(type, this.__proxy, callback);
    };

    off(type, callback) {
        offWatch(type, this.__proxy, callback);
    };

    __proxy;
}

function genSessionId() {
    return Math.random().toString(10).slice(-8);
}

function newDistributed(obj) {
    console.info("start newDistributed");
    if (obj == null) {
        console.error("object is null");
        return null;
    }
    let result = new Distributed(obj);
    return result;
}

function joinSession(obj, sessionId) {
    console.info("start joinSession " + sessionId);
    if (obj == null) {
        console.error("object is null");
        return null;
    }

    let object = distributedObject.createObjectSync(sessionId);
    if (object == null || object == undefined) {
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
    if (obj == null || obj[SESSION_ID] == undefined || obj[SESSION_ID] == null) {
        console.warn("object is null");
        return;
    }
    Object.keys(obj).forEach(key => {
        Object.defineProperty(obj, key, {
            value: obj.get(key),
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
    genSessionId: genSessionId
}