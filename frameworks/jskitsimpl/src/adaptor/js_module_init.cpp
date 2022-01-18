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

#include <cstring>

#include "js_common.h"
#include "js_distributedobject.h"
#include "js_distributedobjectstore.h"
#include "logger.h"

using namespace OHOS::ObjectStore;

extern const char _binary_distributed_data_object_js_start[];
extern const char _binary_distributed_data_object_js_end[];
/*
extern const char _binary_distributed_data_object_abc_start[];
extern const char _binary_distributed_data_object_abc_end[];
 */

static napi_value DistributedDataObjectExport(napi_env env, napi_value exports)
{
    napi_status status;
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createObjectSync", JSDistributedObjectStore::JSCreateObjectSync),
        DECLARE_NAPI_FUNCTION("destroyObjectSync", JSDistributedObjectStore::JSDestroyObjectSync),
        DECLARE_NAPI_FUNCTION("sync", JSDistributedObjectStore::JSSync),
        DECLARE_NAPI_FUNCTION("on", JSDistributedObjectStore::JSOn),
        DECLARE_NAPI_FUNCTION("off", JSDistributedObjectStore::JSOff),
    };

    status = napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    return exports;
}

// storage module define
static napi_module storageModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DistributedDataObjectExport,
    .nm_modname = "data.distributedDataObject",
    .nm_priv = ((void *)0),
    .reserved = { 0 },
};

// distributed_data_object.js
extern "C" __attribute__((visibility("default"))) void NAPI_data_distributedDataObject_GetJSCode(
    const char **buf, int *bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_distributed_data_object_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_distributed_data_object_js_end - _binary_distributed_data_object_js_start;
    }
}

/*
extern "C" __attribute__((visibility("default"))) void NAPI_data_distributedDataObject_GetABCCode(const char** buf,
                                                                                                 int* bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_distributed_data_object_abc_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_distributed_data_object_abc_end - _binary_distributed_data_object_abc_start;
    }
} */
// distributeddataobject module register
static __attribute__((constructor)) void RegisterModule()
{
    napi_module_register(&storageModule);
}