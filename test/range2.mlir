
module {
  func.func @test(%x : i32) -> i32 {
    %c = arith.constant 10 : i32
    %y = arith.addi %x, %c : i32
    return %y : i32
  }
}

