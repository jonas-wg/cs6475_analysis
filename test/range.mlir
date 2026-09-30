
module {
  func.func @test() -> i32 {
    %a = arith.constant 10 : i32
    %b = arith.constant 20 : i32
    %c = arith.addi %a, %b : i32
    return %c : i32
  }
}

