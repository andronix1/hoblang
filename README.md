# hoblang - simple system language

## Getting started

To start using language you should have compiler binary.
There is also internal library should be copied at library
path to make some language features (like strings) working

### Compiling from source

To compile the compiler, create build directory

```bash
mkdir build
cd build
```

Then, generate build files ...

```bash
cmake \
    -DDISABLE_TESTS=NO \ # generate hoblang-tests binary with compiler tests?
    ../
```

... and compile the binary!

```bash
cmake --build . -j6
```

LESS GO!!!

### Hello, world

Create `main.hob` file with following content:

```hob
import io;

global fun main() -> i32 {
    io.out().putStr("hello, world!\n");
    return 0;
}
```

### Compiling first program

You can compile code using hoblang compiler now.
Make sure, that you're running programm in `build` directory (to be fixed :P)

```bash
./hoblang /path/to/source/file
```

### Let's RUN!

After compiling, 3 files is going to be created in cwd (to be fixed :P):
- `test.o` - binary
- `test.hir` - intermidiate language representation dump
- `test.ll` - llvm intermidiate representation

For running, we will be using `test.o` only. Let's compile it using `gcc` (to be fixed :P)

```bash
gcc test.o -o test
```

This is all! You can run `test` and see "hello, world!"
