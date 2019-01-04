# W25：[安全网关](https://github.com/OS-Q/W25)

[![sites](OS-Q/OS-Q.png)](http://www.OS-Q.com)

#### 归属数据网关：[M6](https://github.com/OS-Q/M6)

#### 关于系统架构：[OS-Q](https://github.com/OS-Q/OS-Q)

## [平台描述](https://github.com/OS-Q/W25/wiki) 

数据安全网关，用于包装原始数据，保证数据安全

### [资源](OS-Q/)

---

- 边缘设备命名规则：体系 Q:[1,4] -> 节点 M:[1,12] -> 平台 W:[1,52] -> 设备 D:[1,365]

- naming patterns：system Q[1,4] -> node M[1,12] -> platform W[1,52] -> device D[1,365]

## [包含设备](https://github.com/OS-Q/W25/wiki) 

#### D169：[加密验签](https://github.com/OS-Q/D169)

用于加密下层局域数据，保证接收指令数据的合法和完整

#### D170：[认证授权](https://github.com/OS-Q/D170)

用于验证设备，处理敏感数据并授予相应行为的执行权限

#### D171：[板级设备](https://github.com/OS-Q/D171)



#### D172：[板级设备](https://github.com/OS-Q/D172)



#### D173：[板级设备](https://github.com/OS-Q/D173)



#### D174：[板级设备](https://github.com/OS-Q/D174)



#### D175：[板级设备](https://github.com/OS-Q/D175)


## [同级平台](https://github.com/OS-Q/M6/wiki)

#### W23：[实时网关](https://github.com/OS-Q/W23)

用于守候低频数据上报，维护低频设备通信管道

#### W24：[扩展网关](https://github.com/OS-Q/W24)

具有可临时定义的网关，可以根据需要切换身份

#### -> W25：[安全网关](https://github.com/OS-Q/W25)

用于实现对数据的加密，保证数据向上的私密性

#### W26：[分发网关](https://github.com/OS-Q/W26)

用于完成数据资源储备，保证通用数据及时下发

---

####  © qitas@qitas.cn
###  [OS-Q redefined Operation System](http://www.OS-Q.com)
####  @ 2019-1-4

