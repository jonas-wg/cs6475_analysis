#include "IntRangeAnalysis.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/Matchers.h"

using namespace mlir;

namespace range {

void RangeAnalysis::setToEntryState(RangeLattice *lattice) {
  // Function arguments and other values entering the analysis are unknown.
  propagateIfChanged(lattice, lattice->join(RangeState::top()));
}

LogicalResult
RangeAnalysis::visitOperation(Operation *op,
                              ArrayRef<const RangeLattice *> operands,
                              ArrayRef<RangeLattice *> results) {

  // Raising a result to top means:
  //
  //   "We don't know anything useful about this operation."
  //
  // This is always a safe answer for an analysis that is trying to
  // over-approximate the possible values.
  auto unknown = [&] {
    setAllToEntryStates(results);
    return success();
  };

  // Only single-result integer operations are interesting for this simple
  // analysis.
  if (op->getNumResults() != 1 ||
      !op->getResult(0).getType().isIntOrIndex())
    return unknown();

  RangeLattice *result = results[0];

  // -------------------------------------------------------------------------
  // Rule 1: constants
  //
  //     %x = arith.constant 42 : i32
  //
  // produces:
  //
  //     %x -> [42, 42]
  // -------------------------------------------------------------------------

  IntegerAttr value;
  if (matchPattern(op, m_Constant(&value))) {
    int64_t constant = value.getValue().getSExtValue();

    RangeState state = RangeState::constant(constant);

    propagateIfChanged(result, result->join(state));
    return success();
  }

  // -------------------------------------------------------------------------
  // Rule 2: integer addition
  //
  //     %z = arith.addi %x, %y : i32
  //
  // If:
  //
  //     %x -> [10, 20]
  //     %y -> [3, 5]
  //
  // then:
  //
  //     %z -> [13, 25]
  //
  // because the smallest possible result is 10 + 3 and the largest possible
  // result is 20 + 5.
  // -------------------------------------------------------------------------

  if (isa<arith::AddIOp>(op)) {
    RangeState lhs = operands[0]->getValue();
    RangeState rhs = operands[1]->getValue();

    // The solver has not established useful information about one of the
    // operands yet. Wait for the operand lattice to change.
    if (lhs.isBottom() || rhs.isBottom())
      return success();

      // If either operand is completely unknown, the result is unknown.
    if (lhs.isTop() || rhs.isTop()) {
      propagateIfChanged(result, result->join(RangeState::top()));
      return success();
    }

    RangeState state(
        lhs.min + rhs.min,
        lhs.max + rhs.max);

    propagateIfChanged(result, result->join(state));
    return success();
  }

  // We don't know how to reason about this operation.
  return unknown();
}

} // namespace range

