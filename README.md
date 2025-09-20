# NitroCopy

NitroCopy is copy utility built for running on modern Linux systems and not so modern Windows systems, such as Windows 98 specifically.

## Requirements

- CMake >= 3.12
- MinGW-w64 cross-compiler for 32-bit Windows, such as https://aur.archlinux.org/packages/mingw-w64-crt-msvcrt which is the one I used. (I use Arch btw). It must come with msvcrt.
- GNU Make

## How to build

### For Linux

1. Create a build directory
```shell
mkdir -p build
cd build
```

2. Configure the build
```shell
cmake ..
```

3. Execute the build
```shell
cmake --build . --clean-first
```

Enjoy the build! The executable is in build/bin.

### For Windows 9x

1. Create a build directory
```shell
mkdir -p build-win32
cd build-win32
```

2. Configure the build
```shell
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-i686-toolchain.cmake ..
```

3. Execute the build
```shell
cmake --build . --clean-first
```

Enjoy the build! The executable is in build-win32/bin.

## Usage

```shell
NitroCopy [OPTIONS] <source> <destination>
```

- source: file or directory to copy
- destination: target file or directory
- options:
    - -o: forces overwrite if destination exists already
    - -v: enables verbose logging
    - -h: shows help/usage information
