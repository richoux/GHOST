#include "domain.hpp"
#include "gtest/gtest.h"

#include <vector>

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

class DomainCtor2Test : public ::testing::Test
{
public:
  ghost::Domain *domainDefault;
  ghost::Domain *domainOS4;
  std::vector< int > v = {1,3,5,7,9};
  
  DomainCtor2Test()
  {
    domainDefault = new ghost::Domain( v );
    domainOS4 = new ghost::Domain( v, 4 );
  }

  ~DomainCtor2Test()
  {
    delete domainDefault;
    delete domainOS4;
  }

  ::testing::AssertionResult CanFind( int n )
  {
    if( std::find( v.begin(), v.end(), n ) != v.end() )
      return ::testing::AssertionSuccess();
    else
      return ::testing::AssertionFailure() << n << " is NOT in the domain";
  }
};

TEST_F(DomainCtor1Test, isInitialized)
{
  EXPECT_FALSE( domainDefault->isInitialized() );
  EXPECT_FALSE( domainOS5->isInitialized() );
}

TEST(DomainCtor2ThrowTest, ThrowException)
{
  std::vector< int > v {1,3,5,7,9};  
  EXPECT_ANY_THROW( new ghost::Domain( v, 5 ) );
}

TEST_F(DomainCtor2Test, isInitialized)
{
  EXPECT_TRUE( domainDefault->isInitialized() );
  EXPECT_TRUE( domainOS4->isInitialized() );
}

TEST_F(DomainCtor2Test, getOutsideScope)
{
  EXPECT_EQ( domainDefault->getOutsideScope(), -1 );
  EXPECT_EQ( domainOS4->getOutsideScope(), 4 );
}

TEST_F(DomainCtor2Test, getSize_getInitialSize)
{
  EXPECT_EQ( domainDefault->getSize(), 5 );
  EXPECT_EQ( domainOS4->getSize(), 5 );
  EXPECT_EQ( domainDefault->getInitialSize(), 5 );
  EXPECT_EQ( domainOS4->getInitialSize(), 5 );
}

TEST_F(DomainCtor2Test, maxValues_maxInitialValue)
{
  EXPECT_EQ( domainDefault->maxValue(), 9 );
  EXPECT_EQ( domainOS4->maxValue(), 9 );
  EXPECT_EQ( domainDefault->maxInitialValue(), 9 );
  EXPECT_EQ( domainOS4->maxInitialValue(), 9 );
}

TEST_F(DomainCtor2Test, minValues_minInitialValue)
{
  EXPECT_EQ( domainDefault->minValue(), 1 );
  EXPECT_EQ( domainOS4->minValue(), 1 );
  EXPECT_EQ( domainDefault->minInitialValue(), 1 );
  EXPECT_EQ( domainOS4->minInitialValue(), 1 );
}

TEST_F(DomainCtor2Test, getValue)
{
  EXPECT_EQ( domainDefault->getValue( -1 ), -1 );
  EXPECT_EQ( domainDefault->getValue( 0 ), 1 );
  EXPECT_EQ( domainDefault->getValue( 1 ), 3 );
  EXPECT_EQ( domainDefault->getValue( 2 ), 5 );
  EXPECT_EQ( domainDefault->getValue( 3 ), 7 );
  EXPECT_EQ( domainDefault->getValue( 4 ), 9 );
  EXPECT_EQ( domainDefault->getValue( 5 ), -1 );

  EXPECT_EQ( domainOS4->getValue( -1 ), 4 );
  EXPECT_EQ( domainOS4->getValue( 0 ), 1 );
  EXPECT_EQ( domainOS4->getValue( 1 ), 3 );
  EXPECT_EQ( domainOS4->getValue( 2 ), 5 );
  EXPECT_EQ( domainOS4->getValue( 3 ), 7 );
  EXPECT_EQ( domainOS4->getValue( 4 ), 9 );
  EXPECT_EQ( domainOS4->getValue( 5 ), 4 );
}

TEST_F(DomainCtor2Test, indexOf)
{
  EXPECT_EQ( domainDefault->indexOf( -1 ), -1 );
  EXPECT_EQ( domainDefault->indexOf( 1 ), 0 );
  EXPECT_EQ( domainDefault->indexOf( 3 ), 1 );
  EXPECT_EQ( domainDefault->indexOf( 5 ), 2 );
  EXPECT_EQ( domainDefault->indexOf( 7 ), 3 );
  EXPECT_EQ( domainDefault->indexOf( 9 ), 4 );
  EXPECT_EQ( domainDefault->indexOf( 42 ), -1 );

  EXPECT_EQ( domainOS4->indexOf( -1 ), -1 );
  EXPECT_EQ( domainOS4->indexOf( 1 ), 0 );
  EXPECT_EQ( domainOS4->indexOf( 3 ), 1 );
  EXPECT_EQ( domainOS4->indexOf( 5 ), 2 );
  EXPECT_EQ( domainOS4->indexOf( 7 ), 3 );
  EXPECT_EQ( domainOS4->indexOf( 9 ), 4 );
  EXPECT_EQ( domainOS4->indexOf( 42 ), -1 );
}

TEST_F(DomainCtor2Test, randomValue)
{
  EXPECT_TRUE( CanFind( domainDefault->randomValue() ) );
  EXPECT_TRUE( CanFind( domainDefault->randomValue() ) );
  EXPECT_TRUE( CanFind( domainDefault->randomValue() ) );
  EXPECT_TRUE( CanFind( domainDefault->randomValue() ) );
  EXPECT_TRUE( CanFind( domainDefault->randomValue() ) );

  EXPECT_TRUE( CanFind( domainOS4->randomValue() ) );
  EXPECT_TRUE( CanFind( domainOS4->randomValue() ) );
  EXPECT_TRUE( CanFind( domainOS4->randomValue() ) );
  EXPECT_TRUE( CanFind( domainOS4->randomValue() ) );
  EXPECT_TRUE( CanFind( domainOS4->randomValue() ) );
}

TEST_F(DomainCtor2Test, RemoveAndReset)
{
  EXPECT_TRUE( domainDefault->removeValue(3) );
  EXPECT_FALSE( domainDefault->removeValue(3) );
  EXPECT_FALSE( domainDefault->removeValue(2) );
  EXPECT_EQ( domainDefault->getSize(), 4 );
  EXPECT_EQ( domainDefault->getInitialSize(), 5 );
  EXPECT_EQ( domainDefault->indexOf( 3 ), -1 );
  EXPECT_EQ( domainDefault->indexOf( 1 ), 0 );
  EXPECT_EQ( domainDefault->indexOf( 5 ), 1 );
  EXPECT_EQ( domainDefault->getValue( 1 ), 5 );
  EXPECT_TRUE( domainDefault->removeValue(1) );
  EXPECT_TRUE( domainDefault->removeValue(9) );
  EXPECT_EQ( domainDefault->indexOf( 1 ), -1 );
  EXPECT_EQ( domainDefault->indexOf( 9 ), -1 );
  EXPECT_EQ( domainDefault->maxValue(), 7 );
  EXPECT_EQ( domainDefault->minValue(), 5 );
  EXPECT_EQ( domainDefault->maxInitialValue(), 9 );
  EXPECT_EQ( domainDefault->minInitialValue(), 1 );
  EXPECT_EQ( domainDefault->getSize(), 2 );
  EXPECT_EQ( domainDefault->getInitialSize(), 5 );
  domainDefault->resetToInitial();
  EXPECT_EQ( domainDefault->getSize(), 5 );
  EXPECT_EQ( domainDefault->getInitialSize(), 5 );
  EXPECT_EQ( domainDefault->maxValue(), 9 );
  EXPECT_EQ( domainDefault->minValue(), 1 );
  EXPECT_EQ( domainDefault->maxInitialValue(), 9 );
  EXPECT_EQ( domainDefault->minInitialValue(), 1 );
  EXPECT_EQ( domainDefault->indexOf( 1 ), 0 );
  EXPECT_EQ( domainDefault->indexOf( 9 ), 4 );
  EXPECT_EQ( domainDefault->getValue( 0 ), 1 );
  EXPECT_EQ( domainDefault->getValue( 4 ), 9 );

  EXPECT_TRUE( domainOS4->removeValue(3) );
  EXPECT_FALSE( domainOS4->removeValue(3) );
  EXPECT_FALSE( domainOS4->removeValue(2) );
  EXPECT_EQ( domainOS4->getSize(), 4 );
  EXPECT_EQ( domainOS4->getInitialSize(), 5 );
  EXPECT_EQ( domainOS4->indexOf( 3 ), -1 );
  EXPECT_EQ( domainOS4->indexOf( 1 ), 0 );
  EXPECT_EQ( domainOS4->indexOf( 5 ), 1 );
  EXPECT_EQ( domainOS4->getValue( 1 ), 5 );
  EXPECT_TRUE( domainOS4->removeValue(1) );
  EXPECT_TRUE( domainOS4->removeValue(9) );
  EXPECT_EQ( domainOS4->indexOf( 1 ), -1 );
  EXPECT_EQ( domainOS4->indexOf( 9 ), -1 );
  EXPECT_EQ( domainOS4->maxValue(), 7 );
  EXPECT_EQ( domainOS4->minValue(), 5 );
  EXPECT_EQ( domainOS4->maxInitialValue(), 9 );
  EXPECT_EQ( domainOS4->minInitialValue(), 1 );
  EXPECT_EQ( domainOS4->getSize(), 2 );
  EXPECT_EQ( domainOS4->getInitialSize(), 5 );
  domainOS4->resetToInitial();
  EXPECT_EQ( domainOS4->getSize(), 5 );
  EXPECT_EQ( domainOS4->getInitialSize(), 5 );
  EXPECT_EQ( domainOS4->maxValue(), 9 );
  EXPECT_EQ( domainOS4->minValue(), 1 );
  EXPECT_EQ( domainOS4->maxInitialValue(), 9 );
  EXPECT_EQ( domainOS4->minInitialValue(), 1 );
  EXPECT_EQ( domainOS4->indexOf( 1 ), 0 );
  EXPECT_EQ( domainOS4->indexOf( 9 ), 4 );
  EXPECT_EQ( domainOS4->getValue( 0 ), 1 );
  EXPECT_EQ( domainOS4->getValue( 4 ), 9 );
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
