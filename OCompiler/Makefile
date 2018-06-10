
SRC =src
BUILD =build

OCAML =ocamlopt
OCAML_LINK =
OCAML_COMP =
O_EXT =cmx
FINAL_OUTPUT =dafc

SOURCE_MODULES = $(wildcard $(SRC)/*.ml)
BUILD_MODULES = $(patsubst $(SRC)/%.ml, $(BUILD)/%.$(O_EXT), $(SOURCE_MODULES))

all: $(BUILD_MODULES)
	$(OCAML) $(OCAML_LINK) -o $(FINAL_OUTPUT) $^

$(BUILD)/%.$(O_EXT): $(SRC)/%.ml $(BUILD)
	$(OCAML) $(OCAML_COMP) -c -o $@ $<

$(BUILD):
	mkdir $(BUILD)

clean:
	rm -rf $(BUILD)
	rm -f $(FINAL_OUTPUT)
