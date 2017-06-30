#include "variable.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class VariableTest : public ::testing::Test
{
public:
  ghost::Variable *var_ctor1;
  ghost::Variable *var_ctor2;
  ghost::Variable *var_ctor2_bis;
  ghost::Variable *var_ctor3;

  VariableTest()
  {
    var_ctor1 = new ghost::Variable( "Thomas", "T" );
    var_ctor2 = new ghost::Variable( "Leo", "L", 0, std::vector<int>{1,3,5,7,9}, 0 );
    var_ctor2_bis = new ghost::Variable( "Leo_b", "L_b", 0, std::vector<int>{1,3,5,7,9} );
    var_ctor3 = new ghost::Variable( "Julie", "J", 1, 3, 7 );
  }

  ~VariableTest()
  {
    delete var_ctor1;
    delete var_ctor2;
    delete var_ctor2_bis;
    delete var_ctor3;
  }
};

TEST_F(VariableTest, isInitialized)
{
  EXPECT_FALSE( var_ctor1->hasInitializedDomain() );
  EXPECT_TRUE( var_ctor2->hasInitializedDomain() );
  EXPECT_TRUE( var_ctor2_bis->hasInitializedDomain() );
  EXPECT_TRUE( var_ctor3->hasInitializedDomain() );
}

TEST_F(VariableTest, Names)
{
  EXPECT_EQ( var_ctor1->getName(), "Thomas" );
  EXPECT_EQ( var_ctor2->getName(), "Leo" );
  EXPECT_EQ( var_ctor2_bis->getName(), "Leo_b" );
  EXPECT_EQ( var_ctor3->getName(), "Julie" );

  EXPECT_EQ( var_ctor1->getShortName(), "T" );
  EXPECT_EQ( var_ctor2->getShortName(), "L" );
  EXPECT_EQ( var_ctor2_bis->getShortName(), "L_b" );
  EXPECT_EQ( var_ctor3->getShortName(), "J" );
}

TEST_F(VariableTest, Values)
{
  EXPECT_EQ( var_ctor2->getValue(), 1 );
  EXPECT_EQ( var_ctor2_bis->getValue(), 1 );
  EXPECT_EQ( var_ctor3->getValue(), 8 );

  var_ctor2->shiftValue();
  var_ctor2_bis->shiftValue();
  var_ctor3->shiftValue();

  EXPECT_EQ( var_ctor2->getValue(), 3 );
  EXPECT_EQ( var_ctor2_bis->getValue(), 3 );
  EXPECT_EQ( var_ctor3->getValue(), 9 );

  var_ctor2->shiftValue();
  var_ctor2_bis->shiftValue();
  var_ctor3->shiftValue();

  EXPECT_EQ( var_ctor2->getValue(), 5 );
  EXPECT_EQ( var_ctor2_bis->getValue(), 5 );
  EXPECT_EQ( var_ctor3->getValue(), 7 );

  var_ctor2->unshiftValue();
  var_ctor2_bis->unshiftValue();
  var_ctor3->unshiftValue();

  EXPECT_EQ( var_ctor2->getValue(), 3 );
  EXPECT_EQ( var_ctor2_bis->getValue(), 3 );
  EXPECT_EQ( var_ctor3->getValue(), 9 );

  var_ctor2->setValue( 7 );
  var_ctor2_bis->setValue( 8 );
  var_ctor3->setValue( 8 );

  EXPECT_EQ( var_ctor2->getValue(), 7 );
  EXPECT_EQ( var_ctor2_bis->getValue(), -1 );
  EXPECT_EQ( var_ctor3->getValue(), 8 );

  var_ctor2->setValue( 8 );
  var_ctor2_bis->setValue( 7 );
  var_ctor3->setValue( 4 );

  EXPECT_EQ( var_ctor2->getValue(), 0 );
  EXPECT_EQ( var_ctor2_bis->getValue(), 7 );
  EXPECT_EQ( var_ctor3->getValue(), 6 );

  EXPECT_THAT( var_ctor2->possibleValues(), ::testing::ElementsAre( 1,3,5,7,9 ) );
  EXPECT_THAT( var_ctor2_bis->possibleValues(), ::testing::ElementsAre( 1,3,5,7,9 ) );
  EXPECT_THAT( var_ctor3->possibleValues(), ::testing::ElementsAre( 7,8,9 ) );  
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
