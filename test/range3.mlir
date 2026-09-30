module {
  func.func @test(%x : i32) -> i32 {
    %zero = arith.constant 0 : i32
    %ten = arith.constant 10 : i32

    %cond = arith.cmpi slt, %x, %ten : i32
    cf.cond_br %cond, ^yes, ^no

  ^yes:
    %y = arith.addi %x, %ten : i32
    return %y : i32

  ^no:
    return %zero : i32
  }
}

