## todolist
- 协议实现  // done 
    - v1版本？
    - 再核一下大小端的问题
- 支持compress
- 支持ssl
- 支持ipv6      // done
- 支持带宽控制   // done
- 支持端口范围   // done
- batch如何发送？ // 已解决
- 支持跨平台  //先基于Linux实现一把
- 验证一把性能，极限能跑到多少
    - 50w/s, 30M/s
- cmake // done
    - 不是每个平台都支持cmake，是否要支持scons或者configure？  //done
    - 研究一下scons安装库  // done
- 异步优化： 改成epoll？