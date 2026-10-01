#include "IntRangeAnalysis.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
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

	    // AND with a constant mask.
	    if (isa<LLVM::AndOp>(op)) {
		RangeState lhs = operands[0]->getValue();
		RangeState rhs = operands[1]->getValue();

		if (lhs.isBottom() || rhs.isBottom())
		    return success();

		if (rhs.min == rhs.max && rhs.min >= 0) {
		    RangeState state(0, rhs.max);

		    propagateIfChanged(
			    result,
			    result->join(state));

		    return success();
		}

		return unknown();
	    }

	    // Bitwise OR.
	    //
	    // For now, only handle small, fully bounded ranges by enumerating all
	    // possible pairs. This is simple and sound.
	    if (isa<LLVM::OrOp>(op)) {
		RangeState lhs = operands[0]->getValue();
		RangeState rhs = operands[1]->getValue();

		if (lhs.isBottom() || rhs.isBottom())
		    return success();

		if (lhs.isTop() || rhs.isTop())
		    return unknown();

		constexpr int64_t MaxEnumerate = 256;

		if (lhs.max - lhs.min > MaxEnumerate ||
			rhs.max - rhs.min > MaxEnumerate)
		    return unknown();

		int64_t minResult = std::numeric_limits<int64_t>::max();
		int64_t maxResult = std::numeric_limits<int64_t>::min();

		for (int64_t x = lhs.min; x <= lhs.max; ++x) {
		    for (int64_t y = rhs.min; y <= rhs.max; ++y) {
			int64_t v = x | y;
			minResult = std::min(minResult, v);
			maxResult = std::max(maxResult, v);
		    }
		}

		propagateIfChanged(
			result,
			result->join(RangeState(minResult, maxResult)));
		return success();
	    }

	    // Bitwise XOR.
	    //
	    // Same strategy: enumerate small bounded ranges.
	    if (isa<LLVM::XOrOp>(op)) {
		RangeState lhs = operands[0]->getValue();
		RangeState rhs = operands[1]->getValue();

		if (lhs.isBottom() || rhs.isBottom())
		    return success();

		if (lhs.isTop() || rhs.isTop())
		    return unknown();

		constexpr int64_t MaxEnumerate = 256;

		if (lhs.max - lhs.min > MaxEnumerate ||
			rhs.max - rhs.min > MaxEnumerate)
		    return unknown();

		int64_t minResult = std::numeric_limits<int64_t>::max();
		int64_t maxResult = std::numeric_limits<int64_t>::min();

		for (int64_t x = lhs.min; x <= lhs.max; ++x) {
		    for (int64_t y = rhs.min; y <= rhs.max; ++y) {
			int64_t v = x ^ y;
			minResult = std::min(minResult, v);
			maxResult = std::max(maxResult, v);
		    }
		}

		propagateIfChanged(
			result,
			result->join(RangeState(minResult, maxResult)));
		return success();
	    }

	    // Logical left shift.
	    //
	    // Initial conservative version:
	    // - shift amount must be known
	    // - value must be non-negative
	    // - result must remain representable
	    if (isa<LLVM::ShlOp>(op)) {
		RangeState value = operands[0]->getValue();
		RangeState amount = operands[1]->getValue();

		if (value.isBottom() || amount.isBottom())
		    return success();

		if (value.isTop() || amount.isTop())
		    return unknown();

		// Only handle a constant shift amount.
		if (amount.min != amount.max)
		    return unknown();

		int64_t shift = amount.min;

		if (shift < 0 || shift >= 32)
		    return unknown();

		// Keep the first implementation simple and sound.
		if (value.min < 0)
		    return unknown();

		int64_t factor = int64_t{1} << shift;

		// Avoid signed overflow.
		if (value.max > std::numeric_limits<int32_t>::max() / factor)
		    return unknown();

		RangeState state(
			value.min * factor,
			value.max * factor);

		propagateIfChanged(result, result->join(state));
		return success();
	    }

	    // Bitwise negation.
	    //
	    // MLIR represents ~x conveniently as:
	    //     x ^ -1
	    //
	    // Therefore recognize XOR with the constant -1.
	    if (auto xorOp = dyn_cast<arith::XOrIOp>(op)) {
		RangeState lhs = operands[0]->getValue();
		RangeState rhs = operands[1]->getValue();

		if (lhs.isBottom() || rhs.isBottom())
		    return success();

		// x ^ -1 == ~x
		if (!rhs.isTop() &&
			rhs.min == -1 &&
			rhs.max == -1) {

		    if (lhs.isTop())
			return unknown();

		    // ~x == -x - 1
		    //
		    // If x ∈ [a,b], then
		    // ~x ∈ [-b-1, -a-1].
		    //
		    // Check for overflow before doing the arithmetic.
		    if (lhs.min == std::numeric_limits<int64_t>::min() ||
			    lhs.max == std::numeric_limits<int64_t>::min())
			return unknown();

		    RangeState state(
			    -lhs.max - 1,
			    -lhs.min - 1);

		    propagateIfChanged(result, result->join(state));
		    return success();
		}
	    }

	    // We don't know how to reason about this operation.
	    return unknown();
	}

} // namespace range

