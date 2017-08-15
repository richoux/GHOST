#include <ghost/constraint.hpp>
#include "gtest/gtest.h"

#include <vector>

class MyConstraint : public ghost::Constraint
{
  double v_cost() const override
  {
    return 0.;
  }

public:
  MyConstraint( const std::vector< ghost::Variable* >& variables )
    : Constraint( variables ) {}
};

class ConstraintTest : public ::testing::Test
{
public:
  ghost::Variable var1;
  ghost::Variable var2;
  ghost::Variable var3;
  
  MyConstraint *constraint;

  ConstraintTest()
    : var1 { "v1", "v1", 0, std::vector<int>{1,3,5,7,9}, 0 },
      var2 { "v2", "v2", 0, std::vector<int>{2,4,6,8}, 0 },
      var3 { "v3", "v3", 0, std::vector<int>{1,2,3,4,5,6,7,8,9}, 0 }
  {
    constraint = new MyConstraint( std::vector< ghost::Variable* >{ &var1, &var2 } );
  }

  ~ConstraintTest()
  {
    delete constraint;
  }
};

TEST_F(ConstraintTest, hasVariable)
{
  EXPECT_TRUE( constraint->hasVariable( var1 ) );
  EXPECT_TRUE( constraint->hasVariable( var2 ) );
  EXPECT_FALSE( constraint->hasVariable( var3 ) );
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
