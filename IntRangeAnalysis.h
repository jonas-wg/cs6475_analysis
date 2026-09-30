#ifndef RANGE_ANALYSIS_H
#define RANGE_ANALYSIS_H

#include "IntRangeDom.h"
#include "mlir/Analysis/DataFlow/SparseAnalysis.h"

namespace range {

using RangeLattice = mlir::dataflow::Lattice<RangeState>;

class RangeAnalysis
    : public mlir::dataflow::SparseForwardDataFlowAnalysis<RangeLattice> {
public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;

  /// Transfer function: given the states of `op`'s operands, set the states of
  /// its results.
  mlir::LogicalResult
  visitOperation(mlir::Operation *op,
                 llvm::ArrayRef<const RangeLattice *> operands,
                 llvm::ArrayRef<RangeLattice *> results) override;

  /// Set the initial state for values entering the analysis from outside,
  /// such as function arguments.
  void setToEntryState(RangeLattice *lattice) override;
};

} // namespace range

#endif

