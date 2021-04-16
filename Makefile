VULKAN_SDK_PATH := /Users/ineumann/vulkansdk/macOS

CPPFLAGS := -c -std=c++17 -Wall -I $(VULKAN_SDK_PATH)/include -I /Users/ineumann/VSCWorkspace/HrapVulk/include
LDFLAGS := -L $(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan
GLSLPATH := /Users/ineumann/vulkansdk/macOS/bin/glslc

DIR_SRC := src
DIR_OBJ := objs
DIR_TARGET := build

APP_DST := $(DIR_TARGET)/hrapvulk

SHADER_SRC := $(DIR_SRC)/shaders
SHADER_DST := $(DIR_TARGET)/resources/shaders

CPP_OBJ := $(patsubst $(DIR_SRC)/%.cpp, %.o, $(wildcard $(DIR_SRC)/*.cpp))

SHADER := $(patsubst $(SHADER_SRC)/%.vert, %.spv, $(wildcard $(SHADER_SRC)/*.vert)) \
	$(patsubst $(SHADER_SRC)/%.frag, %.spv, $(wildcard $(SHADER_SRC)/*.frag))

app: dirs $(CPP_OBJ) $(SHADER)
	clang++ -o $(APP_DST) $(patsubst %.o, $(DIR_OBJ)/%.o, $(CPP_OBJ)) $(LDFLAGS)
	
dirs: 
	mkdir build
	mkdir build/resources
	mkdir build/resources/shaders

%.o: $(DIR_SRC)/%.cpp
	clang++ $(CPPFLAGS) -o $(DIR_OBJ)/$@ $<

%.spv: $(SHADER_SRC)/%.vert
	$(GLSLPATH) -o $(SHADER_DST)/$@ $<

%.spv: $(SHADER_SRC)/%.frag
	$(GLSLPATH) -o $(SHADER_DST)/$@ $<
 
.PHONY: test clean

TFLAGS := \
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib \
	VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d \
	VK_ICD_FILENAMES=$(VULKAN_SDK_PATH)/etc/vulkan/icd.d/MoltenVK_icd.json

test: clean app
	$(TFLAGS) ./$(APP_DST)

memcheck: 
	$(TFLAGS) valgrind --leak-check=yes $(APP_DST)

clean:
	rm -f $(DIR_OBJ)/*.o
	rm -fdR $(DIR_TARGET)