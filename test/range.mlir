
module {
    func.func @test(%x : i32) -> i32 {
        %mask = llvm.mlir.constant(15 : i32) : i32
        %y = llvm.and %x, %mask : i32
        return %y : i32
    }

    func.func @test2(%x : i32) -> i32 {
        %mask = llvm.mlir.constant(3 : i32) : i32
        %y = llvm.and %x, %mask : i32
        return %y : i32
    }

    func.func @test3(%x : i32) -> i32 {
        %mask = llvm.mlir.constant(255 : i32) : i32
        %a = llvm.and %x, %mask : i32

        %mask2 = llvm.mlir.constant(7 : i32) : i32
        %b = llvm.and %a, %mask2 : i32

        return %b : i32
    }

    func.func @test4(%x : i32) -> i32 {
        %mask = llvm.mlir.constant(7 : i32) : i32
        %a = llvm.and %x, %mask : i32

        %b = llvm.or %a, %mask : i32

        %c = llvm.xor %a, %mask : i32

        %shift = llvm.mlir.constant(2 : i32) : i32
        %d = llvm.shl %a, %shift : i32

        %minus_one = llvm.mlir.constant(-1 : i32) : i32
        %e = llvm.xor %a, %minus_one : i32

        return %e : i32
    }

    func.func @test5(%x : i32) -> i32 {
        // x is completely unknown.

        %mask7 = llvm.mlir.constant(7 : i32) : i32
        %a = llvm.and %x, %mask7 : i32
        // [0, 7]

        %mask3 = llvm.mlir.constant(3 : i32) : i32
        %b = llvm.or %a, %mask3 : i32
        // [3, 7]

        %mask5 = llvm.mlir.constant(5 : i32) : i32
        %c = llvm.xor %b, %mask5 : i32
        // [0, 7]

        %shift = llvm.mlir.constant(2 : i32) : i32
        %d = llvm.shl %c, %shift : i32
        // [0, 28]

        %minus_one = llvm.mlir.constant(-1 : i32) : i32
        %e = llvm.xor %d, %minus_one : i32
        // [-29, -1]

        return %e : i32
    }
}
