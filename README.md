## todolist
- 协议实现  // done
- 支持compress //done
    - configure 和 cmake 添加依赖 
- 支持ssl //done
    - 支持证书文件校验
    - configure 和 cmake 添加依赖 
- 支持ipv6      // done
- 支持带宽控制   // done
- 支持端口范围   // done
- batch如何发送？ // done
- 支持跨平台  //先基于Linux实现一把
- 验证一把性能，极限能跑到多少
    - 不压缩
```bash
[metrics-logging] send_lines: 929400, send_bytes:333682482, tps: 185880/s, 63.6449 MB/s
[metrics-logging] send_lines: 2080800, send_bytes:747069624, tps: 230280/s, 78.8473 MB/s
[metrics-logging] send_lines: 3212600, send_bytes:1153419778, tps: 226360/s, 77.5051 MB/s
[metrics-logging] send_lines: 4380400, send_bytes:1572695012, tps: 233560/s, 79.9704 MB/s
```
- 压缩
```bash
[metrics-logging] send_lines: 578000, send_bytes:3479560, tps: 115600/s, 0.663673 MB/s
[metrics-logging] send_lines: 1258200, send_bytes:7574364, tps: 136040/s, 0.781022 MB/s
[metrics-logging] send_lines: 1963000, send_bytes:11817260, tps: 140960/s, 0.809268 MB/s
[metrics-logging] send_lines: 2671200, send_bytes:16080624, tps: 141640/s, 0.813172 MB/s
[metrics-logging] send_lines: 3375600, send_bytes:20321112, tps: 140880/s, 0.808809 MB/s
[metrics-logging] send_lines: 4087000, send_bytes:24603740, tps: 142280/s, 0.816846 MB/s
```
- 编译
    - cmake //done  
    - configure //done
    - scons //done
- 异步优化： 改成epoll？

## 待完成
- C++使用线程去接受ack后，反而效率下降了，一开始很高，后面越来越慢 （计划0.5d）
- cmake和configure的zlib，ssl库添加， cpp编译 （计划0.5d， 只考虑Linux，还是以scons为主）
- 异步优化，unix使用epoll，Windows使用iocp？ （3d）
- 跨平台支持，Linux问题不大
    - aix （0.5d）
    - hpux （0.5d）
    - Windows （2d）