module attributes {dlti.dl_spec = #dlti.dl_spec<!llvm.ptr<270> = dense<32> : vector<4xi64>, !llvm.ptr<271> = dense<32> : vector<4xi64>, !llvm.ptr<272> = dense<64> : vector<4xi64>, i64 = dense<64> : vector<2xi64>, i128 = dense<128> : vector<2xi64>, f80 = dense<128> : vector<2xi64>, !llvm.ptr = dense<64> : vector<4xi64>, i1 = dense<8> : vector<2xi64>, i8 = dense<8> : vector<2xi64>, i16 = dense<16> : vector<2xi64>, i32 = dense<32> : vector<2xi64>, f16 = dense<16> : vector<2xi64>, f64 = dense<64> : vector<2xi64>, f128 = dense<128> : vector<2xi64>, "dlti.endianness" = "little", "dlti.mangling_mode" = "e", "dlti.legal_int_widths" = array<i32: 8, 16, 32, 64>, "dlti.stack_alignment" = 128 : i64>, llvm.ident = "Ubuntu clang version 18.1.3 (1ubuntu1)", llvm.module_asm = [], llvm.target_triple = "x86_64-pc-linux-gnu"} {
  llvm.module_flags [#llvm.mlir.module_flag<error, "wchar_size", 4 : i32>, #llvm.mlir.module_flag<min, "PIC Level", 2 : i32>, #llvm.mlir.module_flag<max, "PIE Level", 2 : i32>, #llvm.mlir.module_flag<max, "uwtable", 2 : i32>, #llvm.mlir.module_flag<max, "frame-pointer", 2 : i32>]
  llvm.func @process(%arg0: i32 {llvm.noundef}) -> i32 attributes {dso_local, frame_pointer = #llvm.framePointerKind<all>, no_inline, no_unwind, optimize_none, passthrough = [["min-legal-vector-width", "0"], ["no-trapping-math", "true"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cmov", "+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, tune_cpu = "generic", uwtable_kind = #llvm.uwtableKind<async>} {
    %0 = llvm.mlir.constant(1 : i32) : i32
    %1 = llvm.mlir.constant(7 : i32) : i32
    %2 = llvm.mlir.constant(2 : i32) : i32
    %3 = llvm.mlir.constant(3 : i32) : i32
    %4 = llvm.mlir.constant(5 : i32) : i32
    %5 = llvm.mlir.constant(-1 : i32) : i32
    %6 = llvm.alloca %0 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %7 = llvm.alloca %0 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %8 = llvm.alloca %0 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %9 = llvm.alloca %0 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %10 = llvm.alloca %0 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    %11 = llvm.alloca %0 x i32 {alignment = 4 : i64} : (i32) -> !llvm.ptr
    llvm.store %arg0, %6 <alignment = 4> : i32, !llvm.ptr
    %12 = llvm.load %6 <alignment = 4> : !llvm.ptr -> i32
    %13 = llvm.and %12, %1 : i32
    llvm.store %13, %7 <alignment = 4> : i32, !llvm.ptr
    %14 = llvm.load %7 <alignment = 4> : !llvm.ptr -> i32
    %15 = llvm.shl %14, %2 : i32
    llvm.store %15, %8 <alignment = 4> : i32, !llvm.ptr
    %16 = llvm.load %8 <alignment = 4> : !llvm.ptr -> i32
    %17 = llvm.or %16, %3 : i32
    llvm.store %17, %9 <alignment = 4> : i32, !llvm.ptr
    %18 = llvm.load %9 <alignment = 4> : !llvm.ptr -> i32
    %19 = llvm.xor %18, %4 : i32
    llvm.store %19, %10 <alignment = 4> : i32, !llvm.ptr
    %20 = llvm.load %10 <alignment = 4> : !llvm.ptr -> i32
    %21 = llvm.xor %20, %5 : i32
    llvm.store %21, %11 <alignment = 4> : i32, !llvm.ptr
    %22 = llvm.load %11 <alignment = 4> : !llvm.ptr -> i32
    llvm.return %22 : i32
  }
}
