CFLAGS=-std=c11 -g -static

main: main.c

.PHONY: on-linux
on-linux:
	docker run --rm -it -v /Users/haruto/Desktop/scc:/home/user/scc -w /home/user/scc compilerbook /bin/bash

.PHONY: allow
allow:
	@chmod a+x test-1.sh

.PHONY: test
test: main allow
	./test-1.sh