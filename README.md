# Test libvips with libFuzzer

See:

http://llvm.org/docs/LibFuzzer.html

https://github.com/jcupitt/libvips

## Status

The test is to load a small jpeg from a memory buffer and calculate the
image average. It currently runs on my desktop for at least 23000 minutes of
CPU time with no problems.

We need to add some more tests, perhaps tiff load from buffer and jpeg
save to buffer.

# Setup and test

There are two main routes: building your own clang, which needs a huge PC, or
using an older prebuilt binary.

## Make your own clang

libFuzzer became part of clang in April 2017. If you have a very recent version
(6.0 or later) it's a bit simpler to fuzz. 

You can do this via git with:

http://llvm.org/docs/GettingStarted.html#for-developers-to-work-with-a-git-monorepo

```
$ mkdir clang
$ cd clang/
$ git clone https://github.com/llvm-project/llvm-project-20170507 llvm-project
$ cd llvm-project
$ git config branch.master.rebase true
$ cd ..
$ mkdir clang-build && cd clang-build
$ cmake -GNinja ../llvm-project/llvm -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi;openmp;llvm;compiler-rt"
$ ninja 
```

It'll take many, many hours, you'll need > 10gb of memory (if running wth `-j 1`,
much more otherwise) and > 100gb of disc. It'll leave the compiler in
`clang/clang-build/bin`.

## Recent, pre-built clang

If building clang is too much work, Chromium have pre-built clang binaries
--- download one for your system with:

```
$ mkdir clang
$ cd clang
$ git clone https://chromium.googlesource.com/chromium/src/tools/clang
$ ./clang/scripts/update.py
```

Will leave the clang binary in
`./third_party/llvm-build/Release+Asserts/bin/clang`

## Build libjpeg with coverage and address sanitation

Or you'll get a lot of fuzzer reports from inside libjpeg. See:

http://www.infai.org/jpeg/

Build the 9c development version.

```
$ cd jpeg-9c
$ export CLANG_DIR=/home/john/GIT/clang/bin
$ export FUZZ_FLAGS="-g -fsanitize-coverage=trace-pc-guard -fsanitize=address"
$ CC=$CLANG_DIR/clang CFLAGS="$FUZZ_FLAGS" ./configure --prefix=/home/john/vips 
```

## Build libvips with coverage and address sanitation

```
$ cd libvips
$ CC=$CLANG_DIR/clang CXX=$CLANG_DIR/clang++ \
    CFLAGS="$FUZZ_FLAGS" CXXFLAGS="$FUZZ_FLAGS" \
    ./autogen.sh --prefix=/home/john/vips \
        --with-jpeg-includes=/home/john/vips/include \
        --with-jpeg-libraries=/home/john/vips/lib \
	--without-magick
```

Imagemagick uses `-lomp`, which the prebuilt clang does not support.

## Build `libFuzzer.a`

If you are using an older clang without the built-in fuzzer, you must build it
yourself. 

```
$ git clone https://chromium.googlesource.com/chromium/llvm-project/llvm/lib/Fuzzer
$ ./Fuzzer/build.sh  
```

### Build `jpegload_buffer_fuzz.c` against the custom libvips build

If you have the latest clang:

```
$ $CLANG_DIR/clang -g -fsanitize=fuzzer,address jpegload_buffer_fuzz.c `pkg-config vips --cflags --libs` -lstdc++
```

If you made your own fuzzer, you need:

```
$ $CLANG_DIR/clang -g -fsanitize=address jpegload_buffer_fuzz.c libFuzzer.a `pkg-config vips --cflags --libs` -lstdc++
```

### Run the fuzzer

You need to tell address sanitiser where to find the stack trace symbolizer for
your clang.

```
$ export ASAN_SYMBOLIZER_PATH=$CLANG_DIR/llvm-symbolizer
$ export ASAN_OPTIONS=symbolize=1
$ ./a.out jpegload_corpus
```

### Debug a crash

```
$ $CLANG_DIR/clang -g -fsanitize=address jpegload_buffer_fuzz.c main.c `pkg-config vips --cflags --libs`
$ ASAN_OPTIONS=abort_on_error=1 gdb -ex r --args ./a.out crash-xxxxx
```

`up` a few times and you should be able to see what's going on. 

### TIFF and PNG 

Repeat the jpeg steps, with the obvious changes. 
