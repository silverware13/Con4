all :
	gcc -std=c99 -pthread -o con4_p1 con4_p1.c
	gcc -std=c99 -pthread -o con4_p2 con4_p2.c

clean:
	rm con4_p1
	rm con4_p2

