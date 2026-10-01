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

I concluded ranges about many non constants and for all of my implemented bitwise operation transfer
functions. In the Plugin.cpp, I added string "uniqkey" to my output to grep my results easier. Here are 
some snippets from some of the different runs I did:

`/.run.sh sqlite3.mlir | grep llvm.and | grep uniqkey`
