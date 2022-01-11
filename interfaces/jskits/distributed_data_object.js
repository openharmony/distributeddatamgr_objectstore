const distributedObject = requireInternal("data.distributedDataObject");
const SESSION_ID = "__sessionId"
class Distributed {
    constructor(obj) {
        this.__proxy = obj;
        Object.keys(obj).forEach(key => {
            Object.defineProperty(this, key, {
                enumerable : true,
                configurable : true,
                get: function () {
                    return this.__proxy[key];
                },
                set: function (newValue) {
                    this.__proxy[key] = newValue;
                }
            });
        });
        Object.defineProperty(this, SESSION_ID, {
            enumerable : true,
            configurable : true,
            get: function () {
                return this.__proxy[SESSION_ID];
            },
            set: function (newValue) {
                this.__proxy[SESSION_ID] = newValue;
            }
        });
    };
    setSession (sessionId) {
        if (sessionId == null || sessionId == undefined || sessionId.length == 0) {
            this.__proxy = leaveSession(this.__proxy);
            return;
        }
        this.__proxy = joinSession(this.__proxy, sessionId);
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
    return Math.random().toString(36).slice(-6);;
}

function newDistributed(obj) {
    console.info("start newDistributed ");
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
            enumerable : true,
            configurable : true,
            get: function () {
                return object.get(key);
            },
            set: function (newValue) {
                console.info("hanlu start set " + key + " " + newValue );
                object.put(key, newValue);
                console.info("hanlu end set " + key + " " + newValue );
            }
        });
        object[key] = obj[key];
    });

    Object.defineProperty(object, SESSION_ID, {
        value:sessionId,
        configurable:true,
    });
    return object;
}

function leaveSession(obj) {
    console.info("start leaveSession");
    if (obj == null) {
        console.error("object is null");
        return;
    }
    // disconnect,delete object
    distributedObject.destroyObjectSync(obj[SESSION_ID]);
    Object.keys(obj).forEach(key => {
        Object.defineProperty(obj, key, {
            value:obj[key],
            configurable:true,
            writable:true,
            enumerable:true,
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
    console.info("start off " + obj[SESSION_ID]);
    if (obj[SESSION_ID] != null && obj[SESSION_ID] != undefined && obj[SESSION_ID].length > 0) {
        distributedObject.off(type, obj, callback);
    }
}
export default {
    distributed: newDistributed,
    genSessionId: genSessionId
}