/*
* Copyright (c) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
import { AsyncCallback, Callback } from './basic';

/**
 * Provides interfaces to obtain and modify storage data.
 *
 * @name distributeddataobject
 * @since 8
 * @sysCap
 * @devices phone, tablet
 */
declare namespace distributedobjectstore {
    function distributed(obj: object): DistributedObject;

    class DistributedObject {
        setSession(sessionId?: String): void;
        on(type: 'change', callback: Callback<ChangedDataObserver>): void;
        off(type: 'change', callback?: Callback<ChangedDataObserver>): void;
    }
    interface ChangedDataObserver {
        sessionId: string;
        changeData: Array<string>;
    }
}

export default distributedobjectstore;