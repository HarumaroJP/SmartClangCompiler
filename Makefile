CFLAGS=-std=c11 -g -static

.PHONY: on-linux
on-linux:
	docker run --rm -it -v /Users/haruto/Desktop/scc:/home/user/scc -w /home/user/scc compilerbook /bin/bash
