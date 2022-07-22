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

#include <gtest/gtest.h>

#include <string>
#include <thread>
#include "distributed_object.h"
#include "distributed_objectstore.h"
#include "objectstore_errors.h"

using namespace testing::ext;
using namespace OHOS::ObjectStore;

constexpr static double SALARY = 100.5;

static void TestSetSessionId(std::string bundleName, std::string sessionId)
{
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);
    
    uint32_t ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

static void TestSaveAndRevokeSave(std::string bundleName, std::string sessionId)
{
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);
    
    uint32_t ret = object->PutString("name", "zhangsan");
    EXPECT_EQ(SUCCESS, ret);
    ret = object->PutDouble("salary", SALARY);
    EXPECT_EQ(SUCCESS, ret);
    ret = object->PutBoolean("isTrue", true);
    EXPECT_EQ(SUCCESS, ret);
    
    ret = object->Save("local");
    EXPECT_EQ(SUCCESS, ret);
    ret = object->RevokeSave();
    EXPECT_EQ(SUCCESS, ret);
    
    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

class NativeObjectStoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NativeObjectStoreTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void NativeObjectStoreTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void NativeObjectStoreTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void NativeObjectStoreTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: DistributedObjectStore_Create_Destroy_001
 * @tc.desc: test Create DistributedObject and Destroy DistrbutedObject
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObjectStore_Create_Destroy_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);

    uint32_t ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DistributedObjectStore_Get_001
 * @tc.desc: test DistributedObjectStore Get.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObjectStore_Get_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);

    DistributedObject *object2 = nullptr;
    uint32_t ret = objectStore->Get(sessionId, &object2);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(object, object2);

    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DistributedObjectStore_Watch_UnWatch_001
 * @tc.desc: test DistributedObjectStore Watch and UnWatch.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObjectStore_Watch_UnWatch_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);

    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);

    auto watcherPtr = std::shared_ptr<ObjectWatcher>();
    uint32_t ret = objectStore->Watch(object, watcherPtr);
    EXPECT_EQ(SUCCESS, ret);

    ret = objectStore->UnWatch(object);
    EXPECT_EQ(SUCCESS, ret);

    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DistributedObjectStore_SetStatusNotifier_001
 * @tc.desc: test DistributedObjectStore SetStatusNotifier.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObjectStore_SetStatusNotifier_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);

    auto notifierPtr = std::shared_ptr<StatusNotifier>();
    uint32_t ret = objectStore->SetStatusNotifier(notifierPtr);
    EXPECT_EQ(ret, 0);

    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: DistributedObject_Double_001
 * @tc.desc: test DistributedObjectStore PutDouble.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObject_Double_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);

    uint32_t ret = object->PutDouble("salary", SALARY);
    EXPECT_EQ(ret, 0);

    double value = 0.0;
    object->GetDouble("salary", value);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(value, SALARY);

    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: DistributedObject_Boolean_001
 * @tc.desc: test DistributedObjectStore PutBoolean.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObject_Boolean_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);

    uint32_t ret = object->PutBoolean("isTrue", true);
    EXPECT_EQ(SUCCESS, ret);


    bool value = false;
    ret = object->GetBoolean("isTrue", value);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(true, value);

    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DistributedObject_String_001
 * @tc.desc: test DistributedObjectStore String.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObject_String_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);

    uint32_t ret = object->PutString("name", "zhangsan");
    EXPECT_EQ(SUCCESS, ret);

    std::string value = "";
    ret = object->GetString("name", value);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(value, "zhangsan");

    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DistributedObject_GetSessionId_001
 * @tc.desc: test DistributedObjectStore GetSessionId.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObject_GetSessionId_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);
    std::string getSessionId = object->GetSessionId();
    EXPECT_EQ(sessionId, getSessionId);
    uint32_t ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DistributedObject_TestSetSessionId_001
 * @tc.desc: test DistributedObjectStore TestSetSessionId.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObject_TestSetSessionId_001, TestSize.Level1)
{
    std::thread t1(TestSetSessionId, "default1", "session1");
    std::thread t2(TestSetSessionId, "default2", "session2");
    std::thread t3(TestSetSessionId, "default3", "session3");
    t1.join();
    t2.join();
    t3.join();
}

/**
 * @tc.name: DistributedObject_GetType_001
 * @tc.desc: test DistributedObject GetType.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObject_GetType_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);

    uint32_t ret = object->PutString("name", "zhangsan");
    EXPECT_EQ(SUCCESS, ret);
    Type type;
    ret = object->GetType("name", type);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(TYPE_STRING, type);

    ret = object->PutDouble("salary", SALARY);
    EXPECT_EQ(SUCCESS, ret);
    ret = object->GetType("salary", type);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(TYPE_DOUBLE, type);

    ret = object->PutBoolean("isTrue", true);
    EXPECT_EQ(SUCCESS, ret);
    ret = object->GetType("isTrue", type);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(TYPE_BOOLEAN, type);

    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DistributedObject_Save_RevokeSave_001
 * @tc.desc: test DistributedObjectStore Save.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObject_Save_RevokeSave_001, TestSize.Level1)
{
    std::string bundleName = "default";
    std::string sessionId = "123456";
    TestSaveAndRevokeSave(bundleName, sessionId);
}

/**
 * @tc.name: DistributedObject_Save_RevokeSave_002
 * @tc.desc: test DistributedObjectStore Save.
 * @tc.type: FUNC
 */
HWTEST_F(NativeObjectStoreTest, DistributedObject_Save_RevokeSave_002, TestSize.Level1)
{
    std::thread t1(TestSaveAndRevokeSave, "default1", "session1");
    std::thread t2(TestSaveAndRevokeSave, "default2", "session2");
    std::thread t3(TestSaveAndRevokeSave, "default3", "session3");
    t1.join();
    t2.join();
    t3.join();
}