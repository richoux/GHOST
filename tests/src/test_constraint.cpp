#include <ghost/constraint.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <functional>

class MyConstraint : public ghost::Constraint
{
  double required_cost( const std::vector<Variable*>& variables ) const override
  {
    return 0.;
  }

public:
  MyConstraint() = default;
  
  MyConstraint( const std::vector< ghost::Variable >& variables )
    : Constraint( variables ) {}
};

class ConstraintTest : public ::testing::Test
{
public:
  ghost::Variable var1;
  ghost::Variable var2;
  ghost::Variable var3;

	std::vector< ghost::Variable > vec1;
	std::vector< ghost::Variable > vec2;
  
  MyConstraint *ctr1;
  MyConstraint *ctr2;

  ConstraintTest()
    : var1 { {1,3,5,7,9} },
      var2 { {2,4,6,8} },
      var3 { {1,2,3,4,5,6,7,8,9} }
  {
    vec1 = { var1, var2 };
    vec2 = { var1, var3 };
    ctr1 = new MyConstraint( vec1 );
    ctr2 = new MyConstraint( vec2 );
  }

  ~ConstraintTest()
  {
    delete ctr1;
    delete ctr2;
  }
};

// Constraint::_id is not generated anymore by Constraint constructors since GHOST v2
// TEST_F(ConstraintTest, IDs)
// {
//   EXPECT_EQ( ctr1->get_id(), 0 );
//   EXPECT_EQ( ctr2->get_id(), 1 );
// }

TEST_F(ConstraintTest, Copy)
{
  MyConstraint ctr_copy1( *ctr1 );
  
  // EXPECT_EQ( ctr1->get_id(), 2 );
  // EXPECT_EQ( ctr_copy1.get_id(), 2 );

  EXPECT_TRUE( ctr_copy1.has_variable( var1 ) );
  EXPECT_TRUE( ctr_copy1.has_variable( var2 ) );
  EXPECT_FALSE( ctr_copy1.has_variable( var3 ) );

  EXPECT_THAT( ctr1->get_var(0).possible_values(), ::testing::ElementsAre( 1,3,5,7,9 ) );
  EXPECT_THAT( ctr1->get_var(1).possible_values(), ::testing::ElementsAre( 2,4,6,8 ) );
  EXPECT_THAT( ctr2->get_var(1).possible_values(), ::testing::ElementsAre( 1,2,3,4,5,6,7,8,9 ) );

  EXPECT_EQ( ctr1->get_var(0).get_id(), ctr_copy1.get_var(0).get_id() );
  EXPECT_EQ( ctr1->get_var(0).get_value(), ctr_copy1.get_var(0).get_value() );

  ctr1->get_var(0).set_value( 5 );
  ctr_copy1.get_var(0).set_value( 3 );
  EXPECT_EQ( ctr1->get_var(0).get_value(), 3 );
  EXPECT_EQ( ctr_copy1.get_var(0).get_value(), 3 );
}

TEST_F(ConstraintTest, has_variable)
{
  EXPECT_TRUE( ctr1->has_variable( var1 ) );
  EXPECT_TRUE( ctr1->has_variable( var2 ) );
  EXPECT_FALSE( ctr1->has_variable( var3 ) );

  EXPECT_TRUE( ctr2->has_variable( var1 ) );
  EXPECT_TRUE( ctr2->has_variable( var3 ) );
  EXPECT_FALSE( ctr2->has_variable( var2 ) );
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
