# 分布式对象用户手册

分布式数据对象管理框架是一款面向对象的内存数据管理框架，向应用开发者提供内存对象的创建、查询、删除、修改、订阅等基本数据对象的管理能力，同时具备分布式能力，满足超级终端场景下，相同应用多设备间的数据对象协同需求。 

## 基本概念

分布式数据对象提供JS接口，让开发者能以使用本地对象的方式使用分布式对象。

## 约束与限制

•	不同设备间只有相同bundleName的应用才能直接同步

•	不建议创建过多分布式对象，每个分布式对象将占用100-150KB内存

•	每个对象大小不超过500KB

支持JS和JS

| 类型名称    | 类型描述                                  |
| ----------- | ----------------------------------------- |
| number     | 32位整型，对应大型设备int类型             |
| string | 16位整型，对应大型设备short类型           |
| boolean | 封装长整型                                |

## 开发指导

### 接口说明

#### 引用分布式对象头文件

```
import distributedObject from '@ohos.data.distributedDataObject'
```

#### 接口

| 接口名称                                                     | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| function createDistributedObject(source: object): DistributedObject; | 创建分布式对象<br>source中指定分布式对象中的属性<br>返回值是创建出的分布式对象，接口见DistrubutedObject |
| function genSessionId(): string;                             | 随机创建sessionId<br>返回值是随机创建的sessionId             |

#### DistrubutedObject

| 接口名称                                                     | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| setSessionId(sessionId?: string): boolean;                   | 设置同步的sessionId,可信组网中有多个设备时，多个设备间的对象如果设置为同一个sessionId,就能自动同步<br>sessionId是指定的sessionId,如果要退出分布式组网，设置为“”或不设置均可<br>返回值是操作结果，true标识设置session成功 |
| on(type: 'change', callback: Callback<{ sessionId: string, fields: Array<string> }>): void; | 监听对象的变更<br>type固定为'change'<br>callback是变更时触发的回调，回调参数sessionId标识变更对象的sessionId,fields标识对象变更的属性名 |
| off(type: 'change', callback?: Callback<{ sessionId: string, fields: Array<string> } | 删除对象的变更监听<br>type固定为'change'<br>callback为可选参数，不设置表示删除该对象所有变更监听 |

## •	开发步骤

 #### 1.引入接口
```
import distributedObject from '@ohos.data.distributedDataObject'

```

  #### 2.创建对象
```c
// 创建对象，对象包含三个属性，name,age和isVis
var g_object = distributedObject.createDistributedObject({name:"Amy", age:18, isVis:false});
```
 #### 3.加入同步组网

发起方

```
g_object.setSessionId(distributedObject.genSessionId());

//将生成的g_object.__sessionId通过Intent传到对端设备
```

被拉起方

```
//获取Intent中的sessionId
g_object.setSessionId(sessionId);
```



 #### 4.监听对象变更

```c
changeCallback : function (sessionId, changeData) {
        console.info("change" + sessionId + " " + this.response);

        if (changeData != null && changeData != undefined) {
            changeData.forEach(element => {
                console.info("changed !" + element + " " + g_object[element]);
        });
        }
} 
g_object.on("change", this.changeCallback);
```
#### 4.修改对象属性

```c
g_object.name = "jack";
g_object.age = 19;
g_object.isVis = false; // 对端设备收到change回调，fields为name,age和isVis
```
#### 5.访问对象
```c
console.info("name " + g_object["name"]); //访问到的是组网内最新数据
```
#### 6.退出同步组网

```c
g_object.setSessionId("");
```

