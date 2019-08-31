
BUILD_DIR = build
SWITCH = dafc_opam_switch/
SWITCH_COMPILER = ocaml-base-compiler
OPAM_PACKAGES = oasis llvm
OASIS_FILE = Compiler/_oasis
SRC_DIR = Compiler/src
OASIS_FILE_COPY = $(BUILD_DIR)/_oasis
SRC_DIR_COPY = $(BUILD_DIR)/src
OASIS_MAKEFILE = $(BUILD_DIR)/Makefile
BINARY_OUTPUT = $(BUILD_DIR)/dafc.native
SWITCH_ENV = eval $$(opam env --switch=$(SWITCH) --set-switch) &&


.PHONY: all upgrade_packages clean

all: $(BINARY_OUTPUT)

$(BINARY_OUTPUT): $(OASIS_MAKEFILE) $(SWITCH)
	$(SWITCH_ENV) make -C $(BUILD_DIR)

$(OASIS_MAKEFILE): $(OASIS_FILE_COPY) $(SWITCH)
	$(SWITCH_ENV) oasis -C $(BUILD_DIR) setup -setup-update dynamic

$(OASIS_FILE_COPY): $(BUILD_DIR) $(OASIS_FILE)
	cp $(OASIS_FILE) $(OASIS_FILE_COPY)
	-ln -s $(SRC_DIR) $(SRC_DIR_COPY)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(SWITCH):
	opam init --bare --no-setup
	opam update
	opam switch create $(SWITCH) $(SWITCH_COMPILER)
	opam install --switch=$(SWITCH) $(OPAM_PACKAGES) -y

upgrade_packages: $(SWITCH)
	opam --switch=$(SWITCH) update
	opam --switch=$(SWITCH) upgrade

clean:
	-opam switch remove $(SWITCH)
	-rm -rf $(SWITCH)
	-rm -rf $(BUILD_DIR)
