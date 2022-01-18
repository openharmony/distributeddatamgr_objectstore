/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import distributedObject from '@ohos.data.distributedDataObject';

var g_object = distributedObject.createDistributedObject({ name: "Amy", age: 18, isVis: false });
var test_object = distributedObject.createDistributedObject({ name: "Eric", age: 81, isVis: true });
var undefined_object = distributedObject.createDistributedObject({ name: undefined, age: undefined, isVis: undefined });
const TAG = "OBJECTSTORE_TEST";

function changeCallback(sessionId, changeData) {
    console.info("get init change111" + sessionId + " " + changeData);
    if (changeData != null && changeData != undefined) {
        changeData.forEach(element => {
            console.info(TAG + "data changed !" + element + " " + g_object[element]);
        });
    }
    console.info(TAG + "get init change111 end" + sessionId + " " + changeData);
}

function changeCallback2(sessionId, changeData) {
    console.info("get init change222" + sessionId + " " + changeData);
    if (changeData != null && changeData != undefined) {
        changeData.forEach(element => {
            console.info(TAG + "data changed !" + element + " " + test_object[element]);
        });
    }
    console.info(TAG + "get init change222 end" + sessionId + " " + changeData);
}

describe('objectStoreTest', function () {
    beforeAll(function () {
        console.info(TAG + 'beforeAll')
    })

    beforeEach(function () {
        console.info(TAG + 'beforeEach')
    })

    afterEach(function () {
        console.info(TAG + 'afterEach')
    })

    afterAll(function () {
        console.info(TAG + 'afterAll')
    })

    console.log(TAG + "*************Unit Test Begin*************");


    /**
     * @tc.name objectstore on test
     * @tc.number testOn001
     * @tc.desc object join session and on,object can receive callback when data has been changed
     */
    it('testOn001', 0, function (done) {
        console.log(TAG + "************* testOn001 start *************");
        g_object.setSessionId("session1");
        if (g_object != undefined && g_object != null) {
            expect("session1").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testOn001 joinSession failed");
        }
        console.info(TAG + " start call watch change");
        g_object.on("change", changeCallback);
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack1";
                g_object.age = 19;
                g_object.isVis = false;
            } else {
                g_object.name = "jack2";
                g_object.age = 8;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack1" || g_object.name == "jack2");
            expect(true).assertEqual(g_object.age == 19 || g_object.age == 8);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        done()
        console.log(TAG + "************* testOn001 end *************");
    })

    /**
     * @tc.name objectstore on test
     * @tc.number testOn002
     * @tc.desc object join session and no on,obejct can not receive callback when data has been changed
     */
    it('testOn002', 0, function (done) {
        console.log(TAG + "************* testOn002 start *************");
        g_object.setSessionId("session2");
        if (g_object != undefined && g_object != null) {
            expect("session2").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testOn002 joinSession failed");
        }
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack3";
                g_object.age = 20;
                g_object.isVis = false;
            } else {
                g_object.name = "jack4";
                g_object.age = 21;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack3" || g_object.name == "jack4");
            expect(true).assertEqual(g_object.age == 20 || g_object.age == 21);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        done()
        console.log(TAG + "************* testOn002 end *************");
    })

    /**
     * @tc.name objectstore on test
     * @tc.number testOn003
     * @tc.desc object join session and on,then object change data twice,object can receive two callbacks when data has been changed
     */
    it('testOn003', 0, function (done) {
        console.log(TAG + "************* testOn003 start *************");
        g_object.setSessionId("session3");
        if (g_object != undefined && g_object != null) {
            expect("session3").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testOn003 joinSession failed");
        }
        g_object.on("change", changeCallback);
        console.info(TAG + " start call watch change");
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack3";
                g_object.age = 20;
                g_object.isVis = false;
            } else {
                g_object.name = "jack4";
                g_object.age = 21;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack3" || g_object.name == "jack4");
            expect(true).assertEqual(g_object.age == 20 || g_object.age == 21);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "ja3";
                g_object.age = 2;
                g_object.isVis = false;
            } else {
                g_object.name = "ja4";
                g_object.age = 1;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "ja3" || g_object.name == "ja4");
            expect(true).assertEqual(g_object.age == 2 || g_object.age == 1);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        done()
        console.log(TAG + "************* testOn003 end *************");
    })

    /**
     * @tc.name objectstore on test
     * @tc.number testOn004
     * @tc.desc object join session and on,then object do not change data,object can not receive callbacks
     */
    it('testOn004', 0, function (done) {
        console.log(TAG + "************* testOn004 start *************");
        g_object.setSessionId("session4");
        if (g_object != undefined && g_object != null) {
            expect("session4").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testOn004 joinSession failed");
        }
        g_object.on("change", changeCallback);
        console.info(TAG + " start call watch change");
        console.info(TAG + " end call watch change");

        done()
        console.log(TAG + "************* testOn004 end *************");
    })

    /**
     * @tc.name objectstore off test
     * @tc.number testOff001
     * @tc.desc object join session and on&off,object can not receive callback after off
     */
    it('testOff001', 0, function (done) {
        console.log(TAG + "************* testOff001 start *************");
        g_object.setSessionId("session5");
        if (g_object != undefined && g_object != null) {
            expect("session5").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testOff001 joinSession failed");
        }
        g_object.on("change", changeCallback);
        console.info(TAG + " start call watch change");
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack5";
                g_object.age = 22;
                g_object.isVis = false;
            } else {
                g_object.name = "jack6";
                g_object.age = 23;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack5" || g_object.name == "jack6");
            expect(true).assertEqual(g_object.age == 22 || g_object.age == 23);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        g_object.off("change");
        console.info(TAG + " end call watch change");
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack7";
                g_object.age = 24;
                g_object.isVis = false;
            } else {
                g_object.name = "jack8";
                g_object.age = 25;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack7" || g_object.name == "jack8");
            expect(true).assertEqual(g_object.age == 24 || g_object.age == 25);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        done()
        console.log(TAG + "************* testOff001 end *************");
    })

    /**
     * @tc.name objectstore off test
     * @tc.number testOff002
     * @tc.desc object join session and off,object can not receive callback
     */
    it('testOff002', 0, function (done) {
        console.log(TAG + "************* testOff002 start *************");
        g_object.setSessionId("session6");
        if (g_object != undefined && g_object != null) {
            expect("session6").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testOff002 joinSession failed");
        }
        g_object.off("change");
        console.info(TAG + " end call watch change");
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack9";
                g_object.age = 26;
                g_object.isVis = false;
            } else {
                g_object.name = "jack10";
                g_object.age = 27;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack9" || g_object.name == "jack10");
            expect(true).assertEqual(g_object.age == 26 || g_object.age == 27);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        done()
        console.log(TAG + "************* testOff002 end *************");
    })

    /**
     * @tc.name objectstore MultiObject on test
     * @tc.number testMultiObjectOn001
     * @tc.desc two objects join session and on,then object change data,user can receive two callbacks from two objects
     */
    it('testMultiObjectOn001', 0, function (done) {
        console.log(TAG + "************* testMultiObjectOn001 start *************");
        g_object.setSessionId("session7");
        if (g_object != undefined && g_object != null) {
            expect("session7").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testMultiObjectOn001 joinSession failed");
        }
        test_object.setSessionId("testSession1");
        if (test_object != undefined && test_object != null) {
            expect("testSession1").assertEqual(test_object.__sessionId);
        } else {
            console.log(TAG + "testMultiObjectOn001 joinSession failed");
        }
        g_object.on("change", changeCallback);
        test_object.on("change", changeCallback2);
        console.info(TAG + " start call watch change");
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack11";
                g_object.age = 28;
                g_object.isVis = false;
            } else {
                g_object.name = "jack12";
                g_object.age = 29;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack11" || g_object.name == "jack12");
            expect(true).assertEqual(g_object.age == 28 || g_object.age == 29);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        if (test_object != undefined && test_object != null) {
            if (test_object.isVis) {
                test_object.name = "jack13";
                test_object.age = 30;
                test_object.isVis = false;
            } else {
                test_object.name = "jack14";
                test_object.age = 31;
                test_object.isVis = true;
            }
            expect(true).assertEqual(test_object.name == "jack13" || test_object.name == "jack14");
            expect(true).assertEqual(test_object.age == 30 || test_object.age == 31);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        done()
        console.log(TAG + "************* testMultiObjectOn001 end *************");
    })

    /**
     * @tc.name objectstore MultiObject on test
     * @tc.number testMultiObjectOff001
     * @tc.desc two objects join session and on&off,then two objects can not receive callbacks
     */
    it('testMultiObjectOff001', 0, function (done) {
        console.log(TAG + "************* testMultiObjectOff001 start *************");
        g_object.setSessionId("session8");
        if (g_object != undefined && g_object != null) {
            expect("session8").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testMultiObjectOn002 joinSession failed");
        }
        test_object.setSessionId("testSession2");
        if (test_object != undefined && test_object != null) {
            expect("testSession2").assertEqual(test_object.__sessionId);
        } else {
            console.log(TAG + "testMultiObjectOn002 joinSession failed");
        }
        g_object.on("change", changeCallback);
        test_object.on("change", changeCallback2);
        console.info(TAG + " start call watch change");
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack15";
                g_object.age = 32;
                g_object.isVis = false;
            } else {
                g_object.name = "jack16";
                g_object.age = 33;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack15" || g_object.name == "jack16");
            expect(true).assertEqual(g_object.age == 32 || g_object.age == 33);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        if (test_object != undefined && test_object != null) {
            if (test_object.isVis) {
                test_object.name = "jack17";
                test_object.age = 34;
                test_object.isVis = false;
            } else {
                test_object.name = "jack18";
                test_object.age = 35;
                test_object.isVis = true;
            }
            expect(true).assertEqual(test_object.name == "jack17" || test_object.name == "jack18");
            expect(true).assertEqual(test_object.age == 34 || test_object.age == 35);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        g_object.off("change");
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack19";
                g_object.age = 36;
                g_object.isVis = false;
            } else {
                g_object.name = "jack20";
                g_object.age = 37;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack19" || g_object.name == "jack20");
            expect(true).assertEqual(g_object.age == 36 || g_object.age == 37);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        test_object.off("change");
        if (test_object != undefined && test_object != null) {
            if (test_object.isVis) {
                test_object.name = "jack21";
                test_object.age = 38;
                test_object.isVis = false;
            } else {
                test_object.name = "jack22";
                test_object.age = 39;
                test_object.isVis = true;
            }
            expect(true).assertEqual(test_object.name == "jack21" || test_object.name == "jack22");
            expect(true).assertEqual(test_object.age == 38 || test_object.age == 39);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        done()
        console.log(TAG + "************* testMultiObjectOff001 end *************");
    })

    /**
     * @tc.name objectstore changeSession on test
     * @tc.number testChangeSession001
     * @tc.desc objects join session and on,then change sessionId
     */
    it('testChangeSession001', 0, function (done) {
        console.log(TAG + "************* testChangeSession001 start *************");
        g_object.setSessionId("session9");
        if (g_object != undefined && g_object != null) {
            expect("session9").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testChangeSession001 joinSession session9 failed");
        }
        g_object.on("change", changeCallback);
        console.info(TAG + " start call watch change");
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack23";
                g_object.age = 40;
                g_object.isVis = false;
            } else {
                g_object.name = "jack24";
                g_object.age = 41;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack23" || g_object.name == "jack24");
            expect(true).assertEqual(g_object.age == 40 || g_object.age == 41);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }
        g_object.setSessionId("session10");
        if (g_object != undefined && g_object != null) {
            expect("session10").assertEqual(g_object.__sessionId);
        } else {
            console.log(TAG + "testChangeSession001 joinSession session10 failed");
        }
        if (g_object != undefined && g_object != null) {
            if (g_object.isVis) {
                g_object.name = "jack23";
                g_object.age = 40;
                g_object.isVis = false;
            } else {
                g_object.name = "jack24";
                g_object.age = 41;
                g_object.isVis = true;
            }
            expect(true).assertEqual(g_object.name == "jack23" || g_object.name == "jack24");
            expect(true).assertEqual(g_object.age == 40 || g_object.age == 41);
            console.info(TAG + " set data success!");
        } else {
            console.info(TAG + " object is null,set name fail");
        }

        done()
        console.log(TAG + "************* testChangeSession001 end *************");
    })

    /**
     * @tc.name objectstore undefined type on test
     * @tc.number testUndefinedType001
     * @tc.desc object use undefined type,can not join session
     */
    it('testUndefinedType001', 0, function (done) {
        console.log(TAG + "************* testUndefinedType001 start *************");
        try{
            g_object.setSessionId("session11");
            if (g_object != undefined && g_object != null) {
                expect("session11").assertEqual(g_object.__sessionId);
            } else {
                console.log(TAG + "testChangeSession001 joinSession session11 failed");
            }
        }catch(error){
            console.error(error);
        }

        done()
        console.log(TAG + "************* testUndefinedType001 end *************");
    })


    console.log(TAG + "*************Unit Test End*************");
})

