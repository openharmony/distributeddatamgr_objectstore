const distributedObject = requireInternal("data.distributedDataObject");
const SESSION_ID = "__sessionId"

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
    };

    setSessionId(sessionId) {
        if (sessionId == null || sessionId == undefined || sessionId.length == 0) {
            leaveSession(this.__proxy);
            return false;
        }
        // leave last session
        if (this.__proxy[SESSION_ID] != undefined && this.__proxy[SESSION_ID] != null) {
            leaveSession(this.__proxy);
        }
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
                return object.get(key);
            },
            set: function (newValue) {
                console.info("start set " + key + " " + newValue);
                object.put(key, newValue);
                console.info("end set " + key + " " + newValue);
            }
        });
        object[key] = obj[key];
    });

    Object.defineProperty(object, SESSION_ID, {
        value: sessionId,
        configurable: true,
    });
    return object;
}

function leaveSession(obj) {
    console.info("start leaveSession");
    if (obj == null || obj[SESSION_ID] == undefined ||  obj[SESSION_ID] == null) {
        console.error("object is null");
        return;
    }
    // disconnect,delete object
    distributedObject.destroyObjectSync(obj[SESSION_ID]);
    Object.keys(obj).forEach(key => {
        Object.defineProperty(obj, key, {
            value: obj[key],
            configurable: true,
            writable: true,
            enumerable: true,
        });
    });
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