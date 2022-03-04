all:
	gcc -pthread server_threads.c -o server_threads -lsqlite3 -std=c99
	gcc -pthread client_threads_new.c -o client_threads_new
	gcc demo.c -o demo -lsqlite3 -std=c99
clean:
	rm -f server_threads client_threads_new

