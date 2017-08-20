all: sequential random

sequential: sequential.c
	gcc -o sequential sequential.c
random: random.c
	gcc -o random random.c

clean:
	@rm -f sequential
	@rm -f random
