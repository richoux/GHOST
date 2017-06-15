#include "variable.hpp"
#include "gtest/gtest.h"

class VariableTest : public ::testing::Test
{
public:
  ghost::Variable *variable;

  VariableTest()
  {
    std::string name("Julie");
    std::string shortname("J");
    variable = new ghost::Variable( name, shortname );
  }

  ~VariableTest()
  {
    delete variable;
  }
};

TEST_F(VariableTest, isInitialized)
{
  //EXPECT_STREQ( variable->getName(), "Julie" );
  //EXPECT_STREQ( variable->getShortName(), "j" );
  EXPECT_FALSE( variable->hasInitializedDomain() );
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
