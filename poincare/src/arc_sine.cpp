#include <poincare/arc_sine.h>
#include <poincare/complex.h>
#include <poincare/layout_helper.h>
#include <poincare/serialization_helper.h>
#include <poincare/simplification_helper.h>
#include <cmath>

namespace Poincare {

constexpr Expression::FunctionHelper ArcSine::s_functionHelper;

int ArcSineNode::numberOfChildren() const { return ArcSine::s_functionHelper.numberOfChildren(); }

Layout ArcSineNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(ArcSine(this), floatDisplayMode, numberOfSignificantDigits, ArcSine::s_functionHelper.name());
}

int ArcSineNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, ArcSine::s_functionHelper.name());
}

Expression ArcSineNode::shallowReduce(Context & context, Preferences::AngleUnit angleUnit, bool replaceSymbols) {
  return ArcSine(this).shallowReduce(context, angleUnit, replaceSymbols);
}

template<typename T>
Complex<T> ArcSineNode::computeOnComplex(const std::complex<T> c, Preferences::AngleUnit angleUnit) {
  std::complex<T> result;
  if (c.imag() == 0 && std::fabs(c.real()) <= 1.0) {
    /* asin: [-1;1] -> R
     * In these cases we rather use std::asin(double) because asin on complexes
     * is not as precise as asin on double in std library. For instance,
     * - asin(complex<double>(0.03,0.0) = complex(0.0300045,1.11022e-16)
     * - asin(0.03) = 0.0300045 */
    result = std::asin(c.real());
  } else {
    result = std::asin(c);
    /* asin has a branch cut on ]-inf, -1[U]1, +inf[: it is then multivalued on
     * this cut. We followed the convention chosen by the lib c++ of llvm on
     * ]-inf+0i, -1+0i[ (warning: asin takes the other side of the cut values on
     * ]-inf-0i, -1-0i[) and choose the values on ]1+0i, +inf+0i[ to comply with
     * asin(-x) = -asin(x) and tan(arcsin(x)) = x/sqrt(1-x^2). */
    if (c.imag() == 0 && c.real() > 1) {
      result.imag(-result.imag()); // other side of the cut
    }
  }
  result = Trigonometry::RoundToMeaningfulDigits(result, c);
  return Complex<T>(Trigonometry::ConvertRadianToAngleUnit(result, angleUnit));
}

Expression ArcSine::shallowReduce(Context & context, Preferences::AngleUnit angleUnit, bool replaceSymbols) {
  {
    Expression e = Expression::defaultShallowReduce(context, angleUnit);
    if (e.isUndefined()) {
      return e;
    }
  }
#if MATRIX_EXACT_REDUCING
  if (childAtIndex(0).type() == Type::Matrix) {
    return SimplificationHelper::Map(*this, context, angleUnit);
  }
#endif
  return Trigonometry::shallowReduceInverseFunction(*this, context, angleUnit);
}

}
