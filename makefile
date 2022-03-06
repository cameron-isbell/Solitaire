all: advance check game
	make advance
	make check
	make game

advance: advance.c
	gcc advance.c -o advance -g

check: check.c
	gcc check.c -o  check -g

game: game.c game.h play.c play.h def.c def.h
		gcc game.c play.c def.c -I/share/cs327//include /share/cs327/lib/libtermbox.a -o game
clean:
	-rm advance 
	-rm check
	-rm game
