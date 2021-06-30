CCFLAGS = -Werror -std=c99
CCSOURCES = skip.c

all: skip


debug: CCFLAGS += -g3
debug: skip


skip:
	$(CC) $(CCFLAGS) $(CCSOURCES) -o skip

install: skip
	cp skip /usr/local/bin/

.BOGUS:
