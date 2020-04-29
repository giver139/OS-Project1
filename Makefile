all:main.c
	gcc -omain main.c -lrt
clean:
	rm -f main
