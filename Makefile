CFLAGS += -g -Wall -Werror -std=gnu99 -fPIC
TARGETS = brokenmalloc.so

brokenmalloc.so: brokenmalloc.o
	$(CC) -shared -o $@ $^ -ldl

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o
