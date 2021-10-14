
WASI_VERSION=12
WASI_VERSION_FULL=$(WASI_VERSION).0
WASI_SDK_PATH=$(shell pwd)/wasi-sdk-$(WASI_VERSION_FULL)
OS := $(shell uname)
CMAKE_TOOLCHAIN_FILE=$(shell pwd)/wasimake.cmake

install:
ifeq ($(OS),Darwin)
	$(MAKE) install-wasi-sdk-macos
else
	$(MAKE) install-wasi-sdk-linux
endif

install-wasi-sdk-linux:	
	rm wasi-sdk-$(WASI_VERSION_FULL)-linux.tar.gz || true
	wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-$(WASI_VERSION)/wasi-sdk-$(WASI_VERSION_FULL)-linux.tar.gz
	rm -rf wasi-sdk-$(WASI_VERSION_FULL)
	tar xvf wasi-sdk-$(WASI_VERSION_FULL)-linux.tar.gz
	rm wasi-sdk-$(WASI_VERSION_FULL)-linux.tar.gz

install-wasi-sdk-macos:
	rm wasi-sdk-$(WASI_VERSION_FULL)-macos.tar.gz || true
	wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-$(WASI_VERSION)/wasi-sdk-$(WASI_VERSION_FULL)-macos.tar.gz
	rm -rf wasi-sdk-$(WASI_VERSION_FULL)
	tar xvf wasi-sdk-$(WASI_VERSION_FULL)-macos.tar.gz
	rm wasi-sdk-$(WASI_VERSION_FULL)-macos.tar.gz

clean:
	rm -rf build

cmake:
	rm -rf build && mkdir -p build
	cd build && cmake .. -DWASI_SDK_PREFIX=$(WASI_SDK_PATH) -DCMAKE_TOOLCHAIN_FILE=$(CMAKE_TOOLCHAIN_FILE)
	
compile:
	cd build && make
	mkdir -p dist && cp build/wasm-quickjs-loader dist/loader.wasm && cp build/wasm-quickjs-loader dist/loader.bin

build:
	make cmake && make compile

test:
	@echo "Implement tests!"