CXX=g++
UPX=upx
OUT=yolo

all:
	${CXX} -Os -g3 -Wall -static -c -fmessage-length=0 -o yolo.o "yolo.cpp"
	${CXX} -o ${OUT} yolo.o -lGL -lGLU -lGLEW -lglut

small:
	strip ${OUT}

tiny: small
	${UPX} --brute ${OUT}
