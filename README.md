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

#### D155：[加密签名](https://github.com/OS-Q/D155)

用于加密下层局域数据，保证数据入网的唯一性和接收数据的正确

#### D156：[授权认证](https://github.com/OS-Q/D156)

用于验证设备和数据安全

#### D157：[板级设备](https://github.com/OS-Q/D157)



#### D158：[板级设备](https://github.com/OS-Q/D158)



#### D159：[板级设备](https://github.com/OS-Q/D159)



#### D160：[板级设备](https://github.com/OS-Q/D160)



#### D161：[板级设备](https://github.com/OS-Q/D161)


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
####   @  2019-1-4

