COMPILER=g++
FLAGS=-Wall -pedantic -O3

all: main test

main:
	$(COMPILER) $(FLAGS) main.cpp -o lzw

test:
	$(COMPILER) $(FLAGS) test.cpp -o test

clean:
	rm -rf lzw test