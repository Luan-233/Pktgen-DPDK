#### Pktgen-DPDK Python ver.

欢迎使用Pktgen-DPDK Python ver！

##### 实例接口

1. `Device`，用于指定一张虚拟网卡。
   1. 构造函数`Device(Port, RxQueues, TxQueues, RxDescs, TxDescs, Promiscuous)`，其中前三项必须按顺序填入，后三项为可选项。
   2. 配置网卡`Device.Config()`，在构造函数之后调用。
   3. 只读字段`Device.MAC`，访问网卡的MAC地址。（目前尚不可调用，调用会导致段错误）
2. `MemPool`，内存池，用于存放和申请用户构造好的数据包。
   1. 构造函数`MemPool(Number)`。我们会在用户需求的`Number`个描述符的基础上，额外申请一部分用于每个核的Cache。
3. `PacketArray`，用于存储用户数据包。
   1. 构造函数`PacketArray(Length, MemPool, L2, L3, L4)`，第一项必填，后四项选填。`MemPool`参数指定数据包数组中描述符申请的内存池，`L2, L3, L4`指定数组内每个数据包初始化时每层的协议，需要使用字符串填入。
      1. `L2`的可选参数有`Ethernet`和`Ethernet-ARP`。
      2. `L3`的可选参数有`IPv4`和`IPv6`。
      3. `L4`的可选参数有`TCP`和`UDP`。
   2. 由于在底层将`PacketArray`实现为序列协议，所以允许用户以序列的形式访问每个单独的数据包。以下面提到的`Packet`对象返回。特殊的，这个序列不能够实现拼接和重复，因为新的数据包会占用内存池空间，但是底层不记录数据包和内存池的对应关系，在空间申请的时候无法获知申请空间从何而来。
4. `Packet`，用于指示单个数据包。
   1. 构造函数被禁止调用，在这个框架下，所有的单独`Packet`均只能通过访问`PacketArray`获得。这个考虑出于性能方面的衡量，因为在功能上等价的`PacketArray`与全是`Packet`对象的`python`内建序列（可以是`tuple`也可以是`list`）在性能上不等价，后者会有更大的时空开销。
   2. `Packet`提供大量直接访问协议的字段，可以通过给这个字段赋值一个字典来修改值，也可以以字典作为调用结果获取它，下面只展示赋值可以操作的字段值。
      1. `Ethernet`字段，字典参数有：`Src-MAC`，`Dst-MAC`，`Ether-Type`
      2. `ARP`字段，字典参数有：`Src-IP`，`Dst-IP`，`Src-MAC`，`Dst-MAC`，`Op-Code`
      3. `IPv4`字段，字典参数有：`Src-IP`，`Dst-IP`
      4. `IPv6`字段，字典参数有：`Src-IP`，`Dst-IP`
      5. `TCP`字段，字典参数有：`Src-Port`，`Dst-Port`
      6. `UDP`字段，字典参数有：`Src-Port`，`Dst-Port`
   3. 提供直接计算校验和的方法：
      1. `CalcL3CkSum`计算`L3`校验和，支持`IPv4`校验和的计算。
      2. `CalcL4CkSum`计算`L4`校验和，支持`TCP`与`UDP`校验和的计算。

##### 传输

所有的传输操作都在`Device`实例上实现，接口有：

1. `Device.Receive(QueueId, PktArray)`，接受一组数据包。
2. `Device.Send(QueueId, PktArray)`，直接发送一组数据包。
3. `Device.SendWithDelay(QueueId, PktArray, DelayArray)`，在一定的延迟基础上发送一组数据包。延迟的单位是毫秒。
4. `Device.SendWithRate(QueueId, PktArray, Rate)`，以恒定速率发送一组数据包。速率的单位是`Mbits/s`。
5. `Device.SendWithPossionDelay(QueueId, PktArray, Lambda)`，以指数分布的延迟发送一组数据包，也即单位时间内的数据包数量满足泊松分布。单位同上。

##### 测试用例

以下是一段测试用例，功能是生成一组随机源IP、随机目的端口的数据包并计算校验和，以一定的延迟发送。

```python
import Gen
import random

port = Gen.Device(0, 1, 1)
port.Config()

mp = Gen.MemPool(16)
pkts = Gen.PacketArray(16, MemPool = mp, L2 = "Ethernet", L3 = "IPv4", L4 = "UDP")
delay = []

for i in range(0, pkts.Length):
    pkts[i].Ethernet = { "Src-MAC" : "00:0C:29:3E:FE:28", "Dst-MAC" : "04-33-C2-71-65-B5", "Ether-Type" : 0x0800 }
    pkts[i].IPv4 = { "Src-IP" : "192.168.233." + str(random.randint(2, 254)), "Dst-IP" : "192.168.31.15" }
    pkts[i].UDP = { "Src-Port" : 12345, "Dst-Port" : random.randint(12345, 23456)}
    pkts[i].CalcL3CkSum()
    pkts[i].CalcL4CkSum()
    delay.append((i + 1) * 10);

port.SendWithDelay(0, pkts, delay)

for i in range(0, pkts.Length):
    print(pkts[i].IPv4)
    print(pkts[i].UDP)
```

##### 安装指北

需要提前安装DPDK，并且链接库有`libdpdk`库。可以通过`pkg-config --libs libdpdk`查看是否存在。

在安装好DPDK的前提下，就可以直接运行下面的编译安装命令。

```shell
python3 setup.py build
sudo python3 ./setup.py install --record ./install_log.txt
```

