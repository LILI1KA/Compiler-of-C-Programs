CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

BUILD_DIR := build
TARGETS := \
	$(BUILD_DIR)/wlp4scan \
	$(BUILD_DIR)/wlp4parse \
	$(BUILD_DIR)/wlp4type \
	$(BUILD_DIR)/wlp4gen

.PHONY: all clean

all: $(TARGETS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/wlp4scan: src/scanner/wlp4scan.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/wlp4parse: src/parser/wlp4parse.cc src/parser/node.h src/parser/wlp4data.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) src/parser/wlp4parse.cc -o $@

$(BUILD_DIR)/wlp4type: src/typechecker/wlp4type.cc src/typechecker/node.h src/typechecker/non_terminal.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) src/typechecker/wlp4type.cc -o $@

$(BUILD_DIR)/wlp4gen: src/codegen/wlp4gen.cc src/codegen/node.h src/codegen/non_terminal.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) src/codegen/wlp4gen.cc -o $@

clean:
	rm -rf $(BUILD_DIR)

