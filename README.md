## gateway
gateway server


# Compile

用到了mysql，libevent库，需要提前安装好

一、使用Xcode进行编译

直接打开工程即可


二、在linux下面编译

```bash
$ clang -I/usr/include/mysql/ -levent_core -lmysqlclient -o gateway_server class_code_handle.c main.c
```



