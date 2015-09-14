
TARGET=gateway_server
INC=-I/usr/include/mysql/
LDPATH=-levent_core -lmysqlclient
OBJS=main.o class_code_handle.o code_0x40_handle.o common.o
CC=clang

$(TARGET): $(OBJS)
	$(CC) $(LDPATH) -o $@ $(OBJS) 

main.o: main.c 
	$(CC) $(INC) -c main.c

class_code_handle.o: class_code_handle.c
	$(CC) -c class_code_handle.c

code_0x40_handle.o: code_0x40_handle.c
	$(CC) -c code_0x40_handle.c

common.o: common.c
	$(CC) -c common.c

clean:
	@echo "make " $(TARGET) "clean"
	rm -f $(OBJS) $(TARGET)	


