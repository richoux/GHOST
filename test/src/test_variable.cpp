#include <ghost/variable.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class VariableTest : public ::testing::Test
{
public:
  ghost::Variable *var_ctor1;
  ghost::Variable *var_ctor2;
  ghost::Variable *var_ctor3;

  VariableTest()
  {
    var_ctor1 = new ghost::Variable( "Thomas", "T", std::vector<int>{1,3,5,7,9}, 3 );
    var_ctor2 = new ghost::Variable( "Leo", "L", std::vector<int>{1,3,5,7,9} );
    var_ctor3 = new ghost::Variable( "Julie", "J", 7, 3, 1 );
  }

  ~VariableTest()
  {
    delete var_ctor1;
    delete var_ctor2;
    delete var_ctor3;
  }
};

TEST_F(VariableTest, IDs)
{
  EXPECT_EQ( var_ctor1->get_id(), 0 );
  EXPECT_EQ( var_ctor2->get_id(), 1 );
  EXPECT_EQ( var_ctor3->get_id(), 2 );
}

TEST_F(VariableTest, Copy)
{
  ghost::Variable var_copy1( *var_ctor1 );
  ghost::Variable var_copy2( *var_ctor3 );
  var_copy2 = *var_ctor2;
  
  EXPECT_EQ( var_copy1.get_id(), 3 );
  EXPECT_EQ( var_copy2.get_id(), 4 );
  EXPECT_EQ( var_ctor1->get_id(), 3 );
  EXPECT_EQ( var_ctor2->get_id(), 4 );

  EXPECT_EQ( var_copy1.get_domain_size(), 5 );
  EXPECT_EQ( var_copy2.get_domain_size(), 5 );

  EXPECT_EQ( var_copy1.get_name(), "Thomas" );
  EXPECT_EQ( var_copy2.get_name(), "Leo" );

  EXPECT_EQ( var_copy1.get_short_name(), "T" );
  EXPECT_EQ( var_copy2.get_short_name(), "L" );

  EXPECT_EQ( var_copy2.get_value(), 1 );
  var_copy2.set_value( 3 );
  EXPECT_EQ( var_copy2.get_value(), 3 );
  EXPECT_EQ( var_ctor2->get_value(), 1 );
}

TEST_F(VariableTest, Exceptions)
{
  EXPECT_ANY_THROW( var_ctor1->set_value( 2 ) );
  EXPECT_ANY_THROW( var_ctor1->set_value( 4 ) );

  EXPECT_ANY_THROW( var_ctor2->set_value( 2 ) );
  EXPECT_ANY_THROW( var_ctor2->set_value( 4 ) );

  EXPECT_ANY_THROW( var_ctor3->set_value( 6 ) );
  EXPECT_ANY_THROW( var_ctor3->set_value( 10 ) );
}

TEST_F(VariableTest, DomainSize)
{
  EXPECT_EQ( var_ctor1->get_domain_size(), 5 );
  EXPECT_EQ( var_ctor2->get_domain_size(), 5 );
  EXPECT_EQ( var_ctor3->get_domain_size(), 3 );
}

TEST_F(VariableTest, Names)
{
  EXPECT_EQ( var_ctor1->get_name(), "Thomas" );
  EXPECT_EQ( var_ctor2->get_name(), "Leo" );
  EXPECT_EQ( var_ctor3->get_name(), "Julie" );

  EXPECT_EQ( var_ctor1->get_short_name(), "T" );
  EXPECT_EQ( var_ctor2->get_short_name(), "L" );
  EXPECT_EQ( var_ctor3->get_short_name(), "J" );
}

TEST_F(VariableTest, Values)
{
  EXPECT_EQ( var_ctor1->get_value(), 7 );
  EXPECT_EQ( var_ctor2->get_value(), 1 );
  EXPECT_EQ( var_ctor3->get_value(), 8 );

  var_ctor1->set_value( 5 );
  var_ctor2->set_value( 5 );
  var_ctor3->set_value( 9 );

  EXPECT_EQ( var_ctor1->get_value(), 5 );
  EXPECT_EQ( var_ctor2->get_value(), 5 );
  EXPECT_EQ( var_ctor3->get_value(), 9 );

  EXPECT_THAT( var_ctor1->possible_values(), ::testing::ElementsAre( 1,3,5,7,9 ) );
  EXPECT_THAT( var_ctor2->possible_values(), ::testing::ElementsAre( 1,3,5,7,9 ) );
  EXPECT_THAT( var_ctor3->possible_values(), ::testing::ElementsAre( 7,8,9 ) );
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
