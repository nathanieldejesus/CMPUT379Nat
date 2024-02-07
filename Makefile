CC = gcc
CFLAGS = -Wall -std=c99

TARGETS = a1p2 a1p3
TAR_FILE = dejesus-a1

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.tar *.tar.gz

run-w: a1p2
	./a1p2 w

run-s: a1p2
	./a1p2 s

tar:
	tar -cvf $(TAR_FILE).tar *

tar.gz:
	tar -czvf $(TAR_FILE).tar.gz *

.PHONY: all clean ar tar.gz
