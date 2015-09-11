## gateway
gateway server


# Compile

用到了mysql，libevent库，需要提前安装好

一、使用Xcode进行编译（需要在Mac上安装好libevent和mysql）

二、在linux下面编译

```bash
$ gcc -levent_core -lmysqlclient -o gateway_server class_code_handle.c main.c
```



