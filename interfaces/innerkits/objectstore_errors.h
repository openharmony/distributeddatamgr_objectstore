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
#ifndef OBJECTSTORE_ERRORS_H
#define OBJECTSTORE_ERRORS_H

#include <stdint.h>

namespace OHOS::ObjectStore {
constexpr uint32_t BASE_ERR_OFFSET = 1650;

/* module defined errors */
constexpr uint32_t SUCCESS = 0;
constexpr uint32_t ERR_DB_SET_PROCESS = BASE_ERR_OFFSET + 1;
constexpr uint32_t ERR_EXIST = BASE_ERR_OFFSET + 2;
constexpr uint32_t ERR_DATA_LEN = BASE_ERR_OFFSET + 3;
constexpr uint32_t ERR_NOMEM = BASE_ERR_OFFSET + 4;
constexpr uint32_t ERR_DB_NOT_INIT = BASE_ERR_OFFSET + 5;
constexpr uint32_t ERR_DB_GETKV_FAIL = BASE_ERR_OFFSET + 6;
constexpr uint32_t ERR_DB_NOT_EXIST = BASE_ERR_OFFSET + 7;
constexpr uint32_t ERR_DB_GET_FAIL = BASE_ERR_OFFSET + 8;
constexpr uint32_t ERR_DB_ENTRY_FAIL = BASE_ERR_OFFSET + 9;
constexpr uint32_t ERR_CLOSE_STORAGE = BASE_ERR_OFFSET + 10;
constexpr uint32_t ERR_NULL_OBJECT = BASE_ERR_OFFSET + 11;
constexpr uint32_t ERR_REGISTER = BASE_ERR_OFFSET + 12;
constexpr uint32_t ERR_NULL_OBJECTSTORE = BASE_ERR_OFFSET + 13;
constexpr uint32_t ERR_GET_OBJECT = BASE_ERR_OFFSET + 14;
constexpr uint32_t ERR_NO_OBSERVER = BASE_ERR_OFFSET + 15;
constexpr uint32_t ERR_UNRIGSTER = BASE_ERR_OFFSET + 16;
constexpr uint32_t ERR_SINGLE_DEVICE = BASE_ERR_OFFSET + 17;
} // namespace OHOS::ObjectStore

#endif
