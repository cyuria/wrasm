
MAKEFLAGS += --no-print-directory

debug:
	@cmake . -B build -DCMAKE_BUILD_TYPE=Debug
	@cmake --build build --config Debug

release:
	@cmake . -B build -DCMAKE_BUILD_TYPE=Release
	@cmake --build build --config Release

