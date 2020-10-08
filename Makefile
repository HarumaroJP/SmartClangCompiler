CFLAGS=-std=c11 -g -static

scc: main.c

.PHONY: allow
allow: scc
	@chmod a+x test-1.sh

.PHONY: on-linux
on-linux:
	docker run --rm -it -v /Users/haruto/Desktop/scc:/home/user/scc -w /home/user/scc compilerbook /bin/bash



