BUILD_DIR=build
DEBUGGER=lldb
BIN=
BIN_DIR=$(BUILD_DIR)/bin

all: link_build

check: link_build
	@[ "${BIN}" ] && echo "all good" || ( echo; echo "make run BIN=TARGET"; echo; ls -1 $(BIN_DIR); echo; exit 1 )

run: check
	./$(BIN_DIR)/$(BIN)

build: makebuilddir
	(cd $(BUILD_DIR); cmake --verbose ..; make)

link_build: build
	((rm compile_commands.json 2> /dev/null || true) && ln -s $(BUILD_DIR)/compile_commands.json compile_commands.json)

makebuilddir:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

cleanbin:
	rm $(BIN_DIR)/*

debug: link_build
	$(DEBUGGER) ./$(BIN_DIR)/$(BIN)

test: link_build
	(cd $(BUILD_DIR); ./test_suite)
