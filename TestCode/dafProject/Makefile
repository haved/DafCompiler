DAF_DIR := src
CPP_DIR := cpp
OBJ_DIR:= obj

DAF_H_DIR := ../..

DAF_FILES := $(shell cd $(DAF_DIR) && find -name '*.daf')
CPP_FILES := $(addprefix $(CPP_DIR)/,$(DAF_FILES:.daf=.cpp))
OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(DAF_FILES:.daf=.o))
CPP_FLAGS := -I $(CPP_DIR) -I $(DAF_H_DIR)
LINK_FLAGS := -g++

OUTPUT_FILE := output

all: $(OUTPUT_FILE)

$(OUTPUT_FILE): $(OBJ_FILES)
	dafln $(LINK_FLAGS) $(OBJ_FILES) -o $(OUTPUT_FILE)

$(OBJ_DIR)/%.o: $(CPP_DIR)/%.cpp $(OBJ_DIR)
	g++ $(CPP_FLAGS) -c -o $@ $<

$(CPP_DIR)/%.cpp: $(DAF_DIR)/%.daf $(CPP_DIR)
	daf $< $(dir $@)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(CPP_DIR):
	mkdir $(CPP_DIR)

clean:
	rm -r $(OBJ_DIR) $(CPP_DIR)
