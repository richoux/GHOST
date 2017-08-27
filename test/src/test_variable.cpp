#include <ghost/variable.hpp>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class VariableTest : public ::testing::Test
{
public:
  ghost::Variable *var_ctor2;
  ghost::Variable *var_ctor2_bis;
  ghost::Variable *var_ctor3;

  VariableTest()
  {
    var_ctor2 = new ghost::Variable( "Leo", "L", 0, std::vector<int>{1,3,5,7,9}, 0 );
    var_ctor2_bis = new ghost::Variable( "Leo_b", "L_b", 0, std::vector<int>{1,3,5,7,9} );
    var_ctor3 = new ghost::Variable( "Julie", "J", 1, 3, 7 );
  }

  ~VariableTest()
  {
    delete var_ctor2;
    delete var_ctor2_bis;
    delete var_ctor3;
  }
};

TEST_F(VariableTest, IDs)
{
  EXPECT_EQ( var_ctor2->get_id(), 0 );
  EXPECT_EQ( var_ctor2_bis->get_id(), 1 );
  EXPECT_EQ( var_ctor3->get_id(), 2 );
}

TEST_F(VariableTest, Copy)
{
  ghost::Variable var_copy1( *var_ctor3 );
  ghost::Variable var_copy2( *var_ctor3 );
  var_copy2 = *var_ctor2;
  
  EXPECT_EQ( var_copy1.get_id(), 5 );
  EXPECT_EQ( var_copy2.get_id(), 3 );
  EXPECT_EQ( var_ctor3->get_id(), 5 );
  EXPECT_EQ( var_ctor2->get_id(), 3 );

  EXPECT_EQ( var_copy1.get_name(), "Julie" );
  EXPECT_EQ( var_copy2.get_name(), "Leo" );

  EXPECT_EQ( var_copy1.get_short_name(), "J" );
  EXPECT_EQ( var_copy2.get_short_name(), "L" );

  EXPECT_EQ( var_copy2.get_value(), 1 );
  var_copy2.shift_value();
  EXPECT_EQ( var_copy2.get_value(), 3 );
  EXPECT_EQ( var_ctor2->get_value(), 1 );
}

TEST_F(VariableTest, Names)
{
  EXPECT_EQ( var_ctor2->get_name(), "Leo" );
  EXPECT_EQ( var_ctor2_bis->get_name(), "Leo_b" );
  EXPECT_EQ( var_ctor3->get_name(), "Julie" );

  EXPECT_EQ( var_ctor2->get_short_name(), "L" );
  EXPECT_EQ( var_ctor2_bis->get_short_name(), "L_b" );
  EXPECT_EQ( var_ctor3->get_short_name(), "J" );
}

TEST_F(VariableTest, Values)
{
  EXPECT_TRUE( var_ctor2->is_assigned() );
  EXPECT_TRUE( var_ctor2_bis->is_assigned() );
  EXPECT_TRUE( var_ctor3->is_assigned() );
  
  EXPECT_EQ( var_ctor2->get_value(), 1 );
  EXPECT_EQ( var_ctor2_bis->get_value(), 1 );
  EXPECT_EQ( var_ctor3->get_value(), 8 );

  var_ctor2->shift_value();
  var_ctor2_bis->shift_value();
  var_ctor3->shift_value();

  EXPECT_EQ( var_ctor2->get_value(), 3 );
  EXPECT_EQ( var_ctor2_bis->get_value(), 3 );
  EXPECT_EQ( var_ctor3->get_value(), 9 );

  var_ctor2->shift_value();
  var_ctor2_bis->shift_value();
  var_ctor3->shift_value();

  EXPECT_EQ( var_ctor2->get_value(), 5 );
  EXPECT_EQ( var_ctor2_bis->get_value(), 5 );
  EXPECT_EQ( var_ctor3->get_value(), 7 );

  var_ctor2->unshift_value();
  var_ctor2_bis->unshift_value();
  var_ctor3->unshift_value();

  EXPECT_EQ( var_ctor2->get_value(), 3 );
  EXPECT_EQ( var_ctor2_bis->get_value(), 3 );
  EXPECT_EQ( var_ctor3->get_value(), 9 );

  var_ctor2->set_value( 7 );
  var_ctor2_bis->set_value( 8 );
  var_ctor3->set_value( 8 );

  EXPECT_EQ( var_ctor2->get_value(), 7 );
  EXPECT_EQ( var_ctor2_bis->get_value(), -1 );
  EXPECT_EQ( var_ctor3->get_value(), 8 );

  var_ctor2->set_value( 8 );
  var_ctor2_bis->set_value( 7 );
  var_ctor3->set_value( 4 );

  EXPECT_EQ( var_ctor2->get_value(), 0 );
  EXPECT_EQ( var_ctor2_bis->get_value(), 7 );
  EXPECT_EQ( var_ctor3->get_value(), 6 );

  EXPECT_FALSE( var_ctor2->is_assigned() );
  EXPECT_TRUE( var_ctor2_bis->is_assigned() );
  EXPECT_FALSE( var_ctor3->is_assigned() );

  EXPECT_THAT( var_ctor2->possible_values(), ::testing::ElementsAre( 1,3,5,7,9 ) );
  EXPECT_THAT( var_ctor2_bis->possible_values(), ::testing::ElementsAre( 1,3,5,7,9 ) );
  EXPECT_THAT( var_ctor3->possible_values(), ::testing::ElementsAre( 7,8,9 ) );
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
