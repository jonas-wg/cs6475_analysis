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

	    // Raising a result to top says "this operation could produce anything",
	    // which is always a sound answer and is what every unhandled case does.
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

	    // Getting rid of constants
	    IntegerAttr value;
	    if (matchPattern(op, m_Constant(&value))) {
		int64_t constant = value.getValue().getSExtValue();

		RangeState state = RangeState::constant(constant);

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

	    // Bitwise OR with constant mask.
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

	    // Bitwise XOR with constant mask.
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

	    // Logical left shift. Nonnegative, known mask value.
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

		if (value.min < 0)
		    return unknown();

		int64_t factor = int64_t{1} << shift;

		// Handle signed overflow.
		if (value.max > std::numeric_limits<int32_t>::max() / factor)
		    return unknown();

		RangeState state(
			value.min * factor,
			value.max * factor);

		propagateIfChanged(result, result->join(state));
		return success();
	    }

	    // Bitwise negation. Done with assumed XOr -1 implementation.
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

	    // Unable to conclude anything
	    return unknown();
	}

} // namespace range

