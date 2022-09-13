BIN:=openvpn_ubus
SRC:=$(wildcard *.c)
OBJS:=$(SRC:.c=.o)
LDFLAGS:=-largp 

.PHONY: all clean

all: $(BIN)

$(BIN):$(OBJS) 
	$(CC) -o $@ $^ $(LDFLAGS)

%.o : %.c
	$(CC) -o $@ -c $^

clean:
	$(RM) $(BIN) $(OBJS)