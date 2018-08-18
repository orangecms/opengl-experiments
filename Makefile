CXX=g++
UPX=upx

all:
	${CXX} -Os -g3 -Wall -static -c -fmessage-length=0 -o yolo.o "yolo.cpp"
	${CXX} -o yolo yolo.o -lGL -lGLU -lGLEW -lglut

small:
	strip yolo
	${UPX} --brute yolo
