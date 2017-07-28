# Test libvips with libfuzzer

libfuzzer became part of clang in April 2017. 

## Make your own clang

The simplest
way to get a working install is to build current
clang from source. You can do this via git with:

http://llvm.org/docs/GettingStarted.html#for-developers-to-work-with-a-git-monorepo

```
$ mkdir clang
$ cd clang/
$ export TOP_LEVEL_DIR=`pwd`
$ git clone https://github.com/llvm-project/llvm-project-20170507/llvm-project
$ cd llvm-project
$ git config branch.master.rebase true
$ cd ..
$ mkdir clang-build && cd clang-build
$ cmake -GNinja ../llvm-project/llvm -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi"
$ ninja 
... wait for many, many hours, you'll need 10gb+ of memory and 100gb+ of disc
```

## Recent, pre-built clang

Chromium have pre-built clang binaries --- download one for your system
with:

```
$ mkdir clang
$ cd clang
$ git clone https://chromium.googlesource.com/chromium/src/tools/clang
$ ./clang/scripts/update.py
```

Will leave the clang binary in
`./third_party/llvm-build/Release+Asserts/bin/clang`

## Build libvips with coverage and address sanitation

You need to disable imagemagick, it uses `-lomp`, which clang does not support
in a simple way. 

```
$ cd libvips
$ export CLANG_DIR=/home/john/GIT/third_party/llvm-build/Release+Asserts/bin
$ export FUZZ_FLAGS="-fsanitize-coverage=trace-pc-guard -fsanitize=address"
$ CC=$CLANG_DIR/clang CXX=$CLANG_DIR/clang++ CFLAGS="$FUZZ_FLAGS" CXXFLAGS="$FUZZ_FLAGS" ./autogen.sh --prefix=/home/john/vips --without-magick
```

## Build `libFuzzer.a`

You need to do this if your clang binary does not include the fuzzer (the
prebuilt ones generally don't).

```
$ git clone https://chromium.googlesource.com/chromium/llvm-project/llvm/lib/Fuzzer
$ ./Fuzzer/build.sh  
```

## Build `jpegload_buffer_fuzz.c` against the custom libvips build

With your own `libFuzzer.a`, as above.

```
$ $CLANG_DIR/clang -fsanitize=address jpegload_buffer_fuzz.c libFuzzer.a `pkg-config vips --cflags --libs` -lstdc++
```

## And run the fuzzer

```
$ ./a.out jpegload_corpus
```
