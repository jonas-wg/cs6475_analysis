#ifndef RANGE_DOMAIN_H
#define RANGE_DOMAIN_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include "llvm/Support/raw_ostream.h"

namespace range {

struct RangeState {
  using Value = int64_t;

  Value min;
  Value max;

  // Bottom = unreachable / no information has arrived yet.
  //
  // We represent bottom as min > max.
  RangeState() : min(1), max(0) {}

  RangeState(Value min, Value max) : min(min), max(max) {}

  static RangeState bottom() {
    return RangeState();
  }

  // Top = any int64_t value.
  static RangeState top() {
    return RangeState(std::numeric_limits<Value>::min(),
                      std::numeric_limits<Value>::max());
  }

  static RangeState constant(Value value) {
    return RangeState(value, value);
  }

  bool isBottom() const {
    return min > max;
  }

  bool isTop() const {
    return min == std::numeric_limits<Value>::min() &&
           max == std::numeric_limits<Value>::max();
  }

  /// Least upper bound of two intervals.
  ///
  /// [a, b] U [c, d] in the abstract domain becomes:
  ///
  /// [min(a,c), max(b,d)]
  ///
  /// Note that this is an interval hull, not an exact set union.
  static RangeState join(const RangeState &lhs,
                         const RangeState &rhs) {
    if (lhs.isBottom())
      return rhs;

    if (rhs.isBottom())
      return lhs;

    return RangeState(
        std::min(lhs.min, rhs.min),
        std::max(lhs.max, rhs.max));
  }

  bool operator==(const RangeState &other) const {
    return min == other.min && max == other.max;
  }

  bool operator!=(const RangeState &other) const {
    return !(*this == other);
  }

  void print(llvm::raw_ostream &os) const {
    if (isBottom()) {
      os << "bottom";
      return;
    }

    os << "[" << min << ", " << max << "]";
  }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const RangeState &state) {
  state.print(os);
  return os;
}

} // namespace range

#endif

