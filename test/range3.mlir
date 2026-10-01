module {
  func.func @test(%x : i32) -> i32 {
    %mask1 = arith.constant 7 : i32
    %a = arith.andi %x, %mask1 : i32

    %mask2 = arith.constant 3 : i32
    %b = arith.andi %a, %mask2 : i32

    return %b : i32
  }
}

