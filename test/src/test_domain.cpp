#include <ghost/domain.hpp>
#include "gtest/gtest.h"

#include <vector>

class DomainTest : public ::testing::Test
{
public:
  ghost::Domain *domainOS5;
  ghost::Domain *domainOS4;
  ghost::Domain *domain_size5;
  ghost::Domain *domain_from1to3;
  std::vector< int > v {1,3,5,7,9};

  DomainTest()
  {
    domainOS5 = new ghost::Domain( 5 );
    domainOS4 = new ghost::Domain( v, 4 );
    domain_size5 = new ghost::Domain( v );
    domain_from1to3 = new ghost::Domain( 3, 1 );
  }

  ~DomainTest()
  {
    delete domainOS5;
    delete domainOS4;
    delete domain_size5;
    delete domain_from1to3;     
  }

  ::testing::AssertionResult CanFind( int n )
  {
    if( std::find( v.begin(), v.end(), n ) != v.end() )
      return ::testing::AssertionSuccess();
    else
      return ::testing::AssertionFailure() << n << " is NOT in the domain";
  }
};

TEST_F(DomainTest, isInitialized)
{
  EXPECT_FALSE( domainOS5->is_initialized() );
  EXPECT_TRUE( domainOS4->is_initialized() );
  EXPECT_TRUE( domain_size5->is_initialized() );
  EXPECT_TRUE( domain_from1to3->is_initialized() );
}

TEST(DomainThrowTest, ThrowException)
{
  std::vector< int > v {1,3,5,7,9};  
  EXPECT_ANY_THROW( new ghost::Domain( v, 5 ) );
}

TEST_F(DomainTest, getOutsideScope)
{
  EXPECT_EQ( domainOS5->get_outside_scope(), 5 );
  EXPECT_EQ( domainOS4->get_outside_scope(), 4 );
  EXPECT_EQ( domain_size5->get_outside_scope(), -1 );
  EXPECT_EQ( domain_from1to3->get_outside_scope(), 0 );
}

TEST_F(DomainTest, getSize)
{
  EXPECT_EQ( domainOS4->get_size(), 5 );
  EXPECT_EQ( domain_size5->get_size(), 5 );
  EXPECT_EQ( domain_from1to3->get_size(), 3 );
}

TEST_F(DomainTest, getValue)
{
  EXPECT_EQ( domainOS4->get_value( -1 ), 4 );
  EXPECT_EQ( domainOS4->get_value( 0 ), 1 );
  EXPECT_EQ( domainOS4->get_value( 1 ), 3 );
  EXPECT_EQ( domainOS4->get_value( 2 ), 5 );
  EXPECT_EQ( domainOS4->get_value( 3 ), 7 );
  EXPECT_EQ( domainOS4->get_value( 4 ), 9 );
  EXPECT_EQ( domainOS4->get_value( 5 ), 4 );

  EXPECT_EQ( domain_size5->get_value( -1 ), -1 );
  EXPECT_EQ( domain_size5->get_value( 0 ), 1 );
  EXPECT_EQ( domain_size5->get_value( 1 ), 3 );
  EXPECT_EQ( domain_size5->get_value( 2 ), 5 );
  EXPECT_EQ( domain_size5->get_value( 3 ), 7 );
  EXPECT_EQ( domain_size5->get_value( 4 ), 9 );
  EXPECT_EQ( domain_size5->get_value( 5 ), -1 );

  EXPECT_EQ( domain_from1to3->get_value( -1 ), 0 );
  EXPECT_EQ( domain_from1to3->get_value( 0 ), 1 );
  EXPECT_EQ( domain_from1to3->get_value( 1 ), 2 );
  EXPECT_EQ( domain_from1to3->get_value( 2 ), 3 );
  EXPECT_EQ( domain_from1to3->get_value( 3 ), 0 );
}

TEST_F(DomainTest, indexOf)
{
  EXPECT_EQ( domainOS4->index_of( -1 ), -1 );
  EXPECT_EQ( domainOS4->index_of( 1 ), 0 );
  EXPECT_EQ( domainOS4->index_of( 3 ), 1 );
  EXPECT_EQ( domainOS4->index_of( 5 ), 2 );
  EXPECT_EQ( domainOS4->index_of( 7 ), 3 );
  EXPECT_EQ( domainOS4->index_of( 9 ), 4 );
  EXPECT_EQ( domainOS4->index_of( 42 ), -1 );

  EXPECT_EQ( domain_size5->index_of( -1 ), -1 );
  EXPECT_EQ( domain_size5->index_of( 1 ), 0 );
  EXPECT_EQ( domain_size5->index_of( 3 ), 1 );
  EXPECT_EQ( domain_size5->index_of( 5 ), 2 );
  EXPECT_EQ( domain_size5->index_of( 7 ), 3 );
  EXPECT_EQ( domain_size5->index_of( 9 ), 4 );
  EXPECT_EQ( domain_size5->index_of( 42 ), -1 );

  EXPECT_EQ( domain_from1to3->index_of( -1 ), -1 );
  EXPECT_EQ( domain_from1to3->index_of( 1 ), 0 );
  EXPECT_EQ( domain_from1to3->index_of( 2 ), 1 );
  EXPECT_EQ( domain_from1to3->index_of( 3 ), 2 );
  EXPECT_EQ( domain_from1to3->index_of( 42 ), -1 );
}

TEST_F(DomainTest, randomValue)
{
  EXPECT_TRUE( CanFind( domainOS4->random_value() ) );
  EXPECT_TRUE( CanFind( domainOS4->random_value() ) );
  EXPECT_TRUE( CanFind( domainOS4->random_value() ) );
  EXPECT_TRUE( CanFind( domainOS4->random_value() ) );
  EXPECT_TRUE( CanFind( domainOS4->random_value() ) );

  EXPECT_TRUE( CanFind( domain_size5->random_value() ) );
  EXPECT_TRUE( CanFind( domain_size5->random_value() ) );
  EXPECT_TRUE( CanFind( domain_size5->random_value() ) );
  EXPECT_TRUE( CanFind( domain_size5->random_value() ) );
  EXPECT_TRUE( CanFind( domain_size5->random_value() ) );

  std::vector<int> count(5);
  for( int i = 0 ; i < 10000 ; ++i )
    ++count[ domain_size5->index_of( domain_size5->random_value() ) ];
  std::cout << (double)count[0] / 100 << "% "
	    << (double)count[1] / 100 << "% "
	    << (double)count[2] / 100 << "% "
	    << (double)count[3] / 100 << "% "
	    << (double)count[4] / 100 << "%\n";
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
