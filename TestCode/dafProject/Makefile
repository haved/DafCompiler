DAF_DIR := src
CPP_DIR := cpp
OBJ_DIR:= obj

DAF_FILES := $(wildcard $(DAF_DIR)/*.daf)
CPP_FILES := $(addprefix $(CPP_DIR)/,$(notdir $(DAF_FILES:.daf=.cpp)))
OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_FILES:.cpp=.o)))
CPP_FLAGS := -I CPP_DIR

OUTPUT_FILE := output

all: $(OUTPUT_FILE)

$(OUTPUT_FILE): $(OBJ_FILES)
	dafln $(LINK_FLAGS) $(OBJ_FILES) -o $(OUTPUT_FILE)

$(OBJ_DIR)/%.o: $(CPP_DIR)/%.cpp $(OBJ_DIR)
	g++ $(CPP_FLAGS) -c -o $@ $<

$(CPP_DIR)/%.cpp: $(DAF_DIR)/%.daf $(CPP_DIR)
	daf $< $(CPP_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(CPP_DIR):
	mkdir $(CPP_DIR)

clean:
	rm -r $(OBJ_DIR) $(CPP_DIR)
