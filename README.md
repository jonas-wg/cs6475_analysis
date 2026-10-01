# My Basic MLIR Integer Range Analysis / Interval Analysis

The goal of my analysis was to conclude something about the possible values integers may have
after different types of operations. I focused on some of the common bitwise operations, like
AND, OR, XOR, NOT, and SHL. I named this the `range-analysis` or IntRangeAnalysis.

## Building

My project is heavily based off of the provided framework and utilizes the same build pipeline.

```sh
cmake -S . -B build -DMLIR_DIR=/path/to/prefix/lib/cmake/mlir
cmake --build build
```

## Running

I modified the provided run script to run my analysis in much the same way as the zero-analysis.

```sh
./run.sh input.mlir
```

Of course, the mlir from the source program has to be used, your provided command worked for me:

```sh
clang -S -emit-llvm -o - input.c | mlir-translate --import-llvm
```

I ran on the one-file sqlite3.c from the sqlite open source repo. I've included the .c and .mlir
I used with my analysis in this repo.

## Results

I was able to conclude facts about sqlite3.c using my range-analysis that I believe are nontrivial.
I made an effort to filter out any constants that would show up as the integer range `[x, x] or (5, 5)`.
The possible ranges of integer values change as different bitwise operations are performed on them,
often reducing their size from the maximum uint32\_t value, for example. If multiple bitwise operations
happen in a "chain" of operations, that information should be correctly maintained.

I concluded ranges about many non constants and for all of my implemented bitwise operation transfer
functions. In the Plugin.cpp, I added string "uniqkey" to my output to grep my results easier. Here are 
some snippets from some of the different runs I did:

`./run.sh sqlite3.mlir | grep llvm.and | grep uniqkey`
```
    %85 = llvm.and %84, %19 : i32 // %85 uniqkey is [0, 32767]
    %17 = llvm.and %16, %1 : i32 // %17 uniqkey is [0, 12]
    %22 = llvm.and %21, %0 : i32 // %22 uniqkey is [0, 1]
    %64 = llvm.and %63, %12 : i32 // %64 uniqkey is [0, 15]
    %73 = llvm.and %72, %13 : i32 // %73 uniqkey is [0, 192]
    %76 = llvm.and %75, %10 : i32 // %76 uniqkey is [0, 3]
    %134 = llvm.and %133, %0 : i32 // %134 uniqkey is [0, 1]
    %145 = llvm.and %144, %24 : i32 // %145 uniqkey is [0, 18]
    %155 = llvm.and %154, %25 : i32 // %155 uniqkey is [0, 15]
    %174 = llvm.and %173, %7 : i32 // %174 uniqkey is [0, 16]
    %20 = llvm.and %19, %0 : i32 // %20 uniqkey is [0, 1]
```

`./run.sh sqlite3.mlir | grep llvm.or | grep uniqkey`
```
  %311 = llvm.or %20, %310 : i32 // %311 uniqkey is [128, 191]
  %290 = llvm.or %289, %288 : i8 // %290 uniqkey is [0, 255]
  %792 = llvm.or %791, %0 : i32 // %792 uniqkey is [1, 3]
  %327 = llvm.or %39, %326 : i64 // %327 uniqkey is [3, 59]
  %124 = llvm.or %123, %3 : i32 // %124 uniqkey is [12, 252]
```

`./run.sh sqlite3.mlir | grep llvm.xor | grep uniqkey`
```
    %139 = llvm.xor %138, %19 : i32 // %139 uniqkey is [-33, -1]
    %700 = llvm.xor %699, %20 : i32 // %700 uniqkey is [-33, -1]
    %53 = llvm.xor %52, %8 : i32 // %53 uniqkey is [-33, -1]
```

`./run.sh sqlite3.mlir | grep llvm.shl | grep uniqkey`
```
    %82 = llvm.shl %81, %4 : i16 // %82 uniqkey is [0, 12]
    %96 = llvm.shl %95, %12 : i16 // %96 uniqkey is [0, 256]
    %128 = llvm.shl %127, %18 : i16 // %128 uniqkey is [0, 32]
    %147 = llvm.shl %146, %24 : i16 // %147 uniqkey is [0, 12]
    %485 = llvm.shl %484, %13 : i8 // %485 uniqkey is [0, 6]
    %26 = llvm.shl %25, %5 : i32 // %26 uniqkey is [0, 65536]
    %66 = llvm.shl %65, %12 : i32 // %66 uniqkey is [0, 65536]
    %46 = llvm.shl %45, %8 : i32 // %46 uniqkey is [0, 16711680]
```
