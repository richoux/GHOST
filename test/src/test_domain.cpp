#include "domain.hpp"
#include "gtest/gtest.h"

class DomainCtor1Test : public ::testing::Test
{
public:
  ghost::Domain *domainDefault;
  ghost::Domain *domainOS5;

  DomainCtor1Test()
  {
    domainDefault = new ghost::Domain();
    domainOS5 = new ghost::Domain( 5 );
  }

  ~DomainCtor1Test()
  {
    delete domainDefault;
    delete domainOS5;
  }
};

TEST_F(DomainCtor1Test, IsEmptyInitially)
{
  EXPECT_FALSE( domainDefault->isInitialized() );
  EXPECT_FALSE( domainOS5->isInitialized() );
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
