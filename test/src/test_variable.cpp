#include "variable.hpp"
#include "gtest/gtest.h"

class VariableTest : public ::testing::Test
{
public:
  ghost::Variable *var_ctor1;
  ghost::Variable *var_ctor2;
  ghost::Variable *var_ctor2_bis;
  ghost::Variable *var_ctor3;

  VariableTest()
  {
    // std::string name("Julie");
    // std::string shortname("J");
    var_ctor1 = new ghost::Variable( "Thomas", "T" );
    var_ctor2 = new ghost::Variable( "Leo", "L", 0, std::vector(1,3,5,7,9), 0 );
    var_ctor2_bis = new ghost::Variable( "Leo_b", "L_b", 0, std::vector(1,3,5,7,9) );
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
  //EXPECT_STREQ( variable->getName(), "Julie" );
  //EXPECT_STREQ( variable->getShortName(), "j" );
  EXPECT_FALSE( var_ctor1->hasInitializedDomain() );
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
