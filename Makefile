CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2

all: packet_checker_test

packet_checker_test: firmware/packet_checker.c
	$(CC) $(CFLAGS) $< -o $@

test: packet_checker_test
	./packet_checker_test

clean:
	rm -f packet_checker_test packet_checker_test.exe
