module {
    func.func @test(%x : i32) -> i32 {
	%mask = arith.constant 15 : i32
	    %y = arith.andi %x, %mask : i32
	    return %y : i32
    }

    func.func @test2(%x : i32) -> i32 {
	%mask = arith.constant 3 : i32
	    %y = arith.andi %x, %mask : i32
	    return %y : i32
    }

    func.func @test3(%x : i32) -> i32 {
	%mask = arith.constant 255 : i32
	    %a = arith.andi %x, %mask : i32

	    %mask2 = arith.constant 7 : i32
	    %b = arith.andi %a, %mask2 : i32

	    return %b : i32
    }

    func.func @test4(%x : i32) -> i32 {
	%mask = arith.constant 7 : i32
	    %a = arith.andi %x, %mask : i32

	    %b = arith.ori %a, %mask : i32

	    %c = arith.xori %a, %mask : i32

	    %shift = arith.constant 2 : i32
	    %d = arith.shli %a, %shift : i32

	    %minus_one = arith.constant -1 : i32
	    %e = arith.xori %a, %minus_one : i32

	    return %e : i32
    }

    func.func @test5(%x : i32) -> i32 {
	// x is completely unknown.

	%mask7 = arith.constant 7 : i32
	    %a = arith.andi %x, %mask7 : i32
	    // [0, 7]

	    %mask3 = arith.constant 3 : i32
	    %b = arith.ori %a, %mask3 : i32
	    // [3, 7]

	    %mask5 = arith.constant 5 : i32
	    %c = arith.xori %b, %mask5 : i32
	    // [0, 7]

	    %shift = arith.constant 2 : i32
	    %d = arith.shli %c, %shift : i32
	    // [0, 28]

	    %minus_one = arith.constant -1 : i32
	    %e = arith.xori %d, %minus_one : i32
	    // [-29, -1]

	    return %e : i32
    }

}

