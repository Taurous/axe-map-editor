CXX 		:= g++
CXXFLAGS 	:= -g -Wall `pkg-config --libs --cflags allegro-5 allegro_main-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_image-5 tinyxml2`
INCLUDE_DIR := include
INCLUDE 	:= -I$(INCLUDE_DIR)

SOURCE 		:= $(wildcard src/*.cpp)
OBJS 		:= $(SOURCE:src/%.cpp=build/%.o)

TARGET		:= bin/editor

RM 			:= /bin/rm -f

default: $(TARGET)

DEPS := $(patsubst build/%.o,deps/%.d,$(OBJS))
-include $(DEPS)
DEPFLAGS := -MMD -MF $(@:.o=.d)

$(TARGET): $(OBJS)
	$(CXX) $(INCLUDE) $(OBJS) -o $@ $(CXXFLAGS)

build/%.o: src/%.cpp
	$(CXX) $(INCLUDE) -c $< -o $@ -MMD -MF $(@:build/%.o=deps/%.d)

rebuild: clean main

clean:
	$(RM) build/* bin/* deps/*