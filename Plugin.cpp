#include "Annotate.h"
#include "IntRangeAnalysis.h"

#include "mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"
#include "mlir/Analysis/DataFlow/DeadCodeAnalysis.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {

struct IntRangeAnalysisPass
    : PassWrapper<IntRangeAnalysisPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(IntRangeAnalysisPass)

  StringRef getArgument() const final {
    return "range-analysis";
  }

  StringRef getDescription() const final {
    return "Determine the possible integer range of values";
  }

  void runOnOperation() override {
    DataFlowConfig config;
    config.setInterprocedural(false);

    DataFlowSolver solver(config);

    // DeadCodeAnalysis supplies reachability information.
    // SparseConstantPropagation resolves branch conditions.
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();

    // Load our range analysis.
    solver.load<range::RangeAnalysis>();

    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation().emitError(
          "range analysis failed to reach a fixed point");
      return signalPassFailure();
    }

    // Query states only after the solver has converged.
    auto describe = [&](Value value, AsmState &asmState) -> std::string {

      // Try skipping the constants
      if (value.getDefiningOp<arith::ConstantOp>())
        return {};

      const auto *lattice =
          solver.lookupState<range::RangeLattice>(value);

      if (!lattice)
        return {};

      const range::RangeState &state = lattice->getValue();

      // Bottom means that the value is not reachable yet.
      // Top means that we don't know anything useful about the value.
      if (state.isBottom() || state.isTop())
        return {};
     
      // More constant filtering
      //if (state.min == state.max)
      //  return {};
      
      // prints the top ranges too
      //if (state.isBottom())
      //  return {};

      std::string description;
      llvm::raw_string_ostream os(description);

      value.printAsOperand(os, asmState);

      os << " uniqkey is ";
      state.print(os);

      return description;
    };

    // Print annotations to stderr so that stdout remains the original MLIR.
    range::printAnnotated(getOperation(), describe, llvm::errs());

    markAllAnalysesPreserved();
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return {
      MLIR_PLUGIN_API_VERSION,
      "IntRangeAnalysis",
      LLVM_VERSION_STRING,
      []() {
        PassRegistration<IntRangeAnalysisPass>();
      }};
}

