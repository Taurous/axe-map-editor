CXX = g++
CXXFLAGS = -g -Wall `pkg-config --libs --cflags allegro-5 allegro_main-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_image-5 tinyxml2`
INCLUDE = -Iinclude

default: main

main: build/main.o
	$(CXX) $(INCLUDE) build/main.o build/util.o build/view.o build/tilemap.o build/state_machine.o build/map.o build/input.o build/editor_state.o -o bin/main $(CXXFLAGS)

build/main.o: build/util.o build/view.o build/tilemap.o build/state_machine.o build/map.o build/input.o build/editor_state.o
	$(CXX) $(INCLUDE) -c src/main.cpp -o build/main.o $(CXXFLAGS)

build/util.o:
	$(CXX) $(INCLUDE) -c src/util.cpp -o build/util.o $(CXXFLAGS)

build/view.o:
	$(CXX) $(INCLUDE) -c src/view.cpp -o build/view.o $(CXXFLAGS)

build/tilemap.o:
	$(CXX) $(INCLUDE) -c src/tilemap.cpp -o build/tilemap.o $(CXXFLAGS)

build/state_machine.o:
	$(CXX) $(INCLUDE) -c src/state_machine.cpp -o build/state_machine.o $(CXXFLAGS)

build/map.o: build/view.o build/tilemap.o
	$(CXX) $(INCLUDE) -c src/map.cpp -o build/map.o $(CXXFLAGS)

build/input.o:
	$(CXX) $(INCLUDE) -c src/input.cpp -o build/input.o $(CXXFLAGS)

build/editor_state.o: build/input.o build/state_machine.o build/map.o
	$(CXX) $(INCLUDE) -c src/editor_state.cpp -o build/editor_state.o $(CXXFLAGS)

rebuild: clean main

clean:
	rm -rf build/* bin/*

print:
	@echo $(CXXFLAGS)