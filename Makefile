build:
	gcc pong.c `pkg-config --libs --cflags raylib` -opong
run:
	./pong