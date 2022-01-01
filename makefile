CXX 		:= g++
CXXFLAGS 	:= -g -Wall `pkg-config --libs --cflags allegro-5 allegro_main-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_image-5 tinyxml2`
INCLUDE_DIR := include
INCLUDE 	:= -I$(INCLUDE_DIR)

SOURCE 		:= $(wildcard src/*.cpp)
OBJS 		:= $(SOURCE:src/%.cpp=build/%.o)

TARGET		:= bin/editor

RM 			:= /bin/rm -f

default: $(TARGET)

DEPS := $(patsubst %.o,%.d,$(OBJS))
-include $(DEPS)
DEPFLAGS := -MMD -MF $(@:.o=.d)

$(TARGET): $(OBJS)
	$(CXX) $(INCLUDE) $(OBJS) -o $@ $(CXXFLAGS)

build/%.o: src/%.cpp
	$(CXX) $(INCLUDE) -c $< -o $@ -MMD -MF $(@:.o=.d)

# build/main.o: src/main.cpp include/state_machine.hpp include/input.hpp include/editor_state.hpp
# 	$(CXX) $(INCLUDE) -c src/main.cpp -o build/main.o $(CXXFLAGS)

# build/util.o: src/util.cpp include/util.hpp include/vec.hpp
# 	$(CXX) $(INCLUDE) -c src/util.cpp -o build/util.o $(CXXFLAGS)

# build/view.o: src/view.cpp include/view.hpp include/vec.hpp
# 	$(CXX) $(INCLUDE) -c src/view.cpp -o build/view.o $(CXXFLAGS)

# build/tilemap.o: src/tilemap.cpp include/tilemap.hpp include/vec.hpp
# 	$(CXX) $(INCLUDE) -c src/tilemap.cpp -o build/tilemap.o $(CXXFLAGS)

# build/state_machine.o: src/state_machine.cpp include/state_machine.hpp include/abstract_state.hpp
# 	$(CXX) $(INCLUDE) -c src/state_machine.cpp -o build/state_machine.o $(CXXFLAGS)

# build/map.o: src/map.cpp include/map.hpp include/view.hpp include/tilemap.hpp
# 	$(CXX) $(INCLUDE) -c src/map.cpp -o build/map.o $(CXXFLAGS)

# build/input.o: src/input.cpp include/input.hpp include/vec.hpp
# 	$(CXX) $(INCLUDE) -c src/input.cpp -o build/input.o $(CXXFLAGS)

# build/editor_state.o: src/editor_state.cpp include/editor_state.hpp include/abstract_state.hpp include/view.hpp include/map.hpp include/edit_commands.hpp include/input.hpp include/state_machine.hpp include/util.hpp include/tile_selector.hpp
# 	$(CXX) $(INCLUDE) -c src/editor_state.cpp -o build/editor_state.o $(CXXFLAGS)

# build/tile_selector.o : src/tile_selector.cpp include/tile_selector.hpp
# 	$(CXX) $(INCLUDE) -c src/tile_selector.cpp -o build/tile_selector.o $(CXXFLAGS)

rebuild: clean main

clean:
	$(RM) build/* bin/*