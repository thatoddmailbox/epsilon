#ifndef POINCARE_INFINITY_H
#define POINCARE_INFINITY_H

#include <poincare/number.h>
#include <poincare/allocation_failed_expression_node.h>

namespace Poincare {

class AllocationFailureInfinityNode;

class InfinityNode : public NumberNode {
public:
  static InfinityNode * FailedAllocationStaticNode();

  void setNegative(bool negative) { m_negative = negative; }

  // TreeNode
  size_t size() const override { return sizeof(InfinityNode); }
#if TREE_LOG
  const char * description() const override { return "Infinity";  }
#endif

  // Properties
  Type type() const override { return Type::Infinity; }
  Sign sign() const override { return m_negative ? Sign::Negative : Sign::Positive; }

  // Approximation
  Evaluation<float> approximate(SinglePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override {
    return templatedApproximate<float>();
  }
  Evaluation<double> approximate(DoublePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override {
    return templatedApproximate<double>();
  }

  // Layout
  LayoutRef createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int writeTextInBuffer(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode = Preferences::PrintFloatMode::Decimal, int numberOfSignificantDigits = 0) const override;
private:
  template<typename T> Evaluation<T> templatedApproximate() const;
  bool m_negative;
};

class AllocationFailureInfinityNode : public InfinityNode, public AllocationFailedExpressionNode {
  using AllocationFailedExpressionNode::type;
  using AllocationFailedExpressionNode::approximate;
  using AllocationFailedExpressionNode::writeTextInBuffer;
  using AllocationFailedExpressionNode::createLayout;
  using AllocationFailedExpressionNode::numberOfChildren;
  using AllocationFailedExpressionNode::isAllocationFailure;
  size_t size() const override { return sizeof(AllocationFailureInfinityNode); }
#if TREE_LOG
  const char * description() const override { return "AllocationFailureInfinityNode";  }
#endif
};

class Infinity : public Number {
public:
  Infinity(bool negative) : Number(TreePool::sharedPool()->createTreeNode<InfinityNode>()) {
    node()->setNegative(negative);
  }
private:
  InfinityNode * node() { return static_cast<InfinityNode *>(Number::node()); }
};

}

#endif