default:
	g++ -c capture.cpp `pkg-config --libs --cflags opencv`
	ar rv libcapture.a capture.o
	g++ -c surf.cpp `pkg-config --libs --cflags opencv`
	ar rv libsurf.a surf.o
	g++ -c stereo_calib.cpp `pkg-config --libs --cflags opencv`
	ar rv libstereo_calib.a stereo_calib.o
	g++ main.c -L. -lcapture -lsurf -lstereo_calib `pkg-config --libs --cflags opencv` 
ongoing:
	rm capture.o libcapture.a surf.o libsurf.a stereo_calib.o libstereo_calib.a a.out
	g++ -c capture.cpp `pkg-config --libs --cflags opencv`
	ar rv libcapture.a capture.o
	g++ -c surf.cpp `pkg-config --libs --cflags opencv`
	ar rv libsurf.a surf.o
	g++ -c stereo_calib.cpp `pkg-config --libs --cflags opencv`
	ar rv libstereo_calib.a stereo_calib.o
	g++ main.c -L. -lcapture -lsurf -lstereo_calib `pkg-config --libs --cflags opencv` 
cap:
	g++ -c capture.cpp `pkg-config --libs --cflags opencv`
	ar rv libcapture.a capture.o
surf:
	g++ -c surf.cpp `pkg-config --libs --cflags opencv`
	ar rv libsurf.a surf.o

stereo_calib:
	g++ -c stereo_calib.cpp `pkg-config --libs --cflags opencv`
	ar rv libstereo_calib.a stereo_calib.o
main:
	g++ main.c -L. -lcapture -lsurf -lstereo_calib `pkg-config --libs --cflags opencv` 
clear:
	rm capture.o libcapture.a surf.o libsurf.a stereo_calib.o libstereo_calib.a a.out
allclear:
	rm capture.o libcapture.a surf.o libsurf.a stereo_calib.o libstereo_calib.a extrinsics.yml intrinsics.yml a.out
