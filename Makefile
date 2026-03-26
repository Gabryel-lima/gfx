# Makefile — raiz do projeto
BUILD_DIR := build
CMAKE     := cmake
TYPE      ?= Debug   # override: make TYPE=Release

.PHONY: all configure build clean rebuild run test install help

all: build

configure:
	$(CMAKE) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(TYPE)

build: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel $(shell nproc)

clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean

rebuild: clean build

run: build
	./$(BUILD_DIR)/gfx_demo

test: build
	$(CMAKE) --build $(BUILD_DIR) --target test

install: build
	$(CMAKE) --install $(BUILD_DIR) --prefix ./dist

help:
	@echo "Usage: make [target]"
	@echo "Targets:"
	@echo "  all       - Build the project (default)"
	@echo "  configure  - Configure the build system"
	@echo "  build      - Build the project"
	@echo "  clean      - Clean the build artifacts"
	@echo "  rebuild    - Clean and build the project"
	@echo "  run        - Run the built application"
	@echo "  test       - Run tests"
	@echo "  install    - Install the built application to ./dist"
	@echo "  help       - Show this help message"
