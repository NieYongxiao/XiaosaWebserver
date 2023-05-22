# XiaosaWebserver
通过epoll函数和线程池实现高并发的webserver

使用webbench测试，可以实现上万并发，但会有些许失败
xiaosa@xiaosa-virtual-machine:~$ webbench -c 10000 -t 10 http://127.0.0.1:8090/
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Benchmarking: GET http://127.0.0.1:8090/
10000 clients, running 10 sec.

Speed=1337940 pages/min, 22614698 bytes/sec.
Requests: 222675 susceed, 315 failed.
