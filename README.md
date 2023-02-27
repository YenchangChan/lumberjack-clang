## 性能数据
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
- 问题：
    - 极限性能在25w左右，还是不太能达到要求

- 功能
    - 支持endpoint方式配置ip和端口号
    - 加一个error处理
    - ssl 连其他机器上的router不能发送数据？
    - 性能优化？