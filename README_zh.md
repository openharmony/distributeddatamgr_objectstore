# 分布式对象用户手册

分布式数据对象管理框架是一款面向对象的内存数据管理框架，向应用开发者提供内存对象的创建、查询、删除、修改、订阅等基本数据对象的管理能力，同时具备分布式能力，满足超级终端场景下，相同应用多设备间的数据对象协同需求。 

## 基本概念

分布式数据对象提供JS接口，让开发者能以使用本地对象的方式使用分布式对象。分布式数据对象支持的数据类型包括数字型、字符型、布尔型等基本类型，同时也支持数组、基本类型嵌套等复杂类型。

## 约束与限制

•	不同设备间只有相同bundleName的应用才能直接同步

•	不建议创建过多分布式对象，每个分布式对象将占用100-150KB内存

•	每个对象大小不超过500KB

•	支持JS接口间的互通，与其他语言不互通

•	如对复杂类型的数据进行修改，仅支持修改根属性，暂不支持下级属性修改

## 开发指导

### 接口说明

#### 引用分布式对象头文件

```js
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
| on(type: 'status', callback: Callback<{ sessionId: string, networkId: string, status: 'online' \| 'offline' }>): void | 监听对象的变更<br/>type固定为'status'<br/>callback是变更时触发的回调，回调参数sessionId标识变更对象的sessionId，networkId标识对象设备的networkId，status标识对象为'online'(上线)或'offline'(下线)的状态 |
| off(type: 'status', callback?: Callback<{ sessionId: string, deviceId: string, status: 'online' \| 'offline' }>): void | 删除对象的变更监听<br/>type固定为'change'<br/>callback为可选参数，不设置表示删除该对象所有上下线监听 |

## •	开发步骤

 #### 1.引入接口
```js
import distributedObject from '@ohos.data.distributedDataObject'
```

  #### 2.创建对象
```js
// 创建对象，对象包含3个基本属性：name,age和isVis；2个复杂属性：parent,list
var g_object = distributedObject.createDistributedObject({name:undefined, age:undefined, isVis:true, 
               parent:undefined, list:undefined});
```
 #### 3.加入同步组网

发起方

```js
g_object.setSessionId(distributedObject.genSessionId());
```

被拉起方

```js
//sessionId与发起方的__sessionId一致
g_object.setSessionId(sessionId);
```



 #### 4.监听对象变更

```js
changeCallback : function (sessionId, changeData) {
        console.info("change" + sessionId + " " + this.response);

        if (changeData != null && changeData != undefined) {
            changeData.forEach(element => {
                console.info("changed !" + element + " " + g_object[element]);
        });
	}
} 
// 发起方要在changeCallback里刷新界面，则需要将正确的this绑定给changeCallback
g_object.on("change", this.changeCallback.bind(this));
```
#### 5.修改对象属性

```js
g_object.name = "jack";
g_object.age = 19;
g_object.isVis = false; 
g_object.parent = {mother:"jack mom",father:"jack Dad"};
g_object.list = [{mother:"jack mom"}, {father:"jack Dad"}];
// 对端设备收到change回调，fields为name,age,isVis,parent和list
```
![icon-note.gif](pictures/icon-note.gif) **说明：** 针对复杂类型的数据修改，目前支持对根属性的修改，暂不支持对下级属性的修改。示例如下：

```js
//支持的修改方式
g_object.parent = {mother:"mom", father:"dad"};
//不支持的修改方式
g_object.parent.mother = "mom";
```

#### 6.访问对象

```js
console.info("name " + g_object["name"]); //访问到的是组网内最新数据
```
#### 7.删除监听数据变更

```js
//删除变更回调changeCallback
g_object.off("change", changeCallback);
//删除所有的变更回调
g_object.off("change"); 
```

#### 8.监听分布式对象的上下线

```js
statusCallback : function (sessionId, networkid, status) {
  this.response += "status changed " + sessionId + " " + status + " " + networkId;
}

g_object.on("status", this.changeCallback);
```

#### 9.删除监听分布式对象的上下线

```js
//删除上下线回调changeCallback
g_object.off("status", changeCallback);
//删除所有的上下线回调
g_object.off("status");
```

#### 10.退出同步组网

```js
g_object.setSessionId("");
```

## 开发实例

针对分布式数据对象，有以下开发实例可供参考： 

[备忘录应用](https://gitee.com/openharmony/distributeddatamgr_objectstore/tree/master/samples/distributedNotepad)

分布式数据对象在备忘录应用中，通过分布式数据对象框架，当用户在某一端设备上新增备忘录事件，修改编辑事件标题和内容以及清空事件列表时，产生的数据变更结果均可以同步刷新显现在可信组网内其他设备上。