# gateway
> gateway server


## Compile

> 用到了mysql，libevent库，需要提前安装好

### 使用Xcode进行编译

直接打开Xcode工程即可


### 在linux下面编译

> 后面有空写个Makefile

```bash
$ clang -I/usr/include/mysql/ -levent_core -lmysqlclient -o gateway_server class_code_handle.c main.c
```



