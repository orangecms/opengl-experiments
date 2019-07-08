CXX=g++
UPX=upx
OUT=yolo

all:
	${CXX} -Os -g3 -Wall -static -c -fmessage-length=0 -o yolo.o "yolo.cpp"
	${CXX} -o ${OUT} yolo.o -lGL -lGLU -lGLEW -lglut -lIL

small:
	strip ${OUT}

tiny: small
	${UPX} --brute ${OUT}

devilx:
	$(CC) devilx.c -lGL -lglut -lIL -o devilx

glrecord:
	$(CC) -DPPM=1 -DLIBPNG=1 -DFFMPEG=1 -ggdb3 -std=c99 -O0 -Wall -Wextra \
	  -o glrecord glrecord.c -lGL -lGLU -lglut -lpng -lavcodec -lswscale -lavutil
