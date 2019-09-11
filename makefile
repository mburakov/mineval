all: mineval

mineval: main.o msgpack.o
	$(CC) -O3 -s $^ -o $@

%.o: %.c
	$(CC) -O3 -Wall -Wextra -Werror -pedantic -c $< -o $@

clean:
	rm mineval *.o
