CXXFLAGS = `pkg-config --libs --cflags allegro-5 allegro_main-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_image-5 tinyxml2`
INCLUDE = -Iinclude

all: main

main: build/main.o
	g++ $(INCLUDE) build/main.o build/util.o build/view.o build/tilemap.o build/state_machine.o build/map.o build/input.o build/editor_state.o -o bin/main $(CXXFLAGS)

build/main.o: build/util.o build/view.o build/tilemap.o build/state_machine.o build/map.o build/input.o build/editor_state.o
	g++ $(INCLUDE) -c src/main.cpp -o build/main.o

build/util.o:
	g++ $(INCLUDE) -c src/util.cpp -o build/util.o

build/view.o:
	g++ $(INCLUDE) -c src/view.cpp -o build/view.o

build/tilemap.o:
	g++ $(INCLUDE) -c src/tilemap.cpp -o build/tilemap.o

build/state_machine.o:
	g++ $(INCLUDE) -c src/state_machine.cpp -o build/state_machine.o

build/map.o: build/view.o build/tilemap.o
	g++ $(INCLUDE) -c src/map.cpp -o build/map.o

build/input.o:
	g++ $(INCLUDE) -c src/input.cpp -o build/input.o

build/editor_state.o: build/input.o build/state_machine.o build/map.o
	g++ $(INCLUDE) -c src/editor_state.cpp -o build/editor_state.o

clean:
	rm -rf build/* bin/*

print:
	@echo $(CXXFLAGS)