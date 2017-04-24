CXXFLAGS := `pkg-config --libs --cflags libuv` -I"uvw/src/"


all: 1


1: 1.cpp
	notify-send "Starting build for $@"
	g++ ${CXXFLAGS} $^ -o $@
	notify-send "Build complete for $@"

2: 2_srv 2_cli

2_srv: 2_srv.cpp
	notify-send "Starting build for $@"
	g++ ${CXXFLAGS} $^ -o $@
	notify-send "Build complete for $@"

2_cli: 2_cli.cpp
	notify-send "Starting build for $@"
	g++ ${CXXFLAGS} $^ -o $@
	notify-send "Build complete for $@"


.PHONY: clean
clean:
	rm -f 1 2_srv 2_cli