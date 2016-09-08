#include "variable.hpp"
#include "gtest/gtest.h"

#include <vector>

class VariableCtor1Test : public ::testing::Test
{
public:
  ghost::Variable *variable;

  VariableCtor1Test()
  {
    variable = new ghost::Variable( "Julie", "j" );
  }

  ~VariableCtor1Test()
  {
    delete variable;
  }
};

TEST_F(VariableCtor1Test, isInitialized)
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
