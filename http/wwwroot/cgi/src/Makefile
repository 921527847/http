ROOT_PATH=$(shell pwd)
INCLUDE=$(ROOT_PATH)/lib/include
LIB=$(ROOT_PATH)/lib/lib/mysql
LDFLAGS=-lmysqlclient
cc=gcc
.PHONY:all
all: insert select
insert:insert_cgi.c
	$(cc) -o $(ROOT_PATH)/../bin/$@  $^ -I$(INCLUDE) -L$(LIB) $(LDFLAGS)
select:select_cgi.c 
	$(cc) -o $(ROOT_PATH)/../bin/$@ $^ -I$(INCLUDE) -L$(LIB) $(LDFLAGS)
.PHONY:clean
clean:
	rm -f insert select
