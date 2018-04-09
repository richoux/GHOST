#include <ghost/domain.hpp>
#include <gtest/gtest.h>

#include <vector>

class DomainTest : public ::testing::Test
{
public:
  ghost::Domain *domain1;
  ghost::Domain *domain2;
  ghost::Domain *domain3;
  std::vector< int > v {1,3,5,7,9};

  DomainTest()
  {
    domain1 = new ghost::Domain( v );
    domain2 = new ghost::Domain( v );
    domain3 = new ghost::Domain( 3, 1 );
  }

  ~DomainTest()
  {
    delete domain1;
    delete domain2;
    delete domain3;     
  }

  ::testing::AssertionResult CanFind( int n )
  {
    if( std::find( v.begin(), v.end(), n ) != v.end() )
      return ::testing::AssertionSuccess();
    else
      return ::testing::AssertionFailure() << n << " is NOT in the domain";
  }
};

TEST_F(DomainTest, ThrowException)
{
  EXPECT_ANY_THROW( domain1->get_value( -1 ) );
  EXPECT_ANY_THROW( domain2->get_value( 5 ) );
  EXPECT_ANY_THROW( domain3->get_value( 3 ) );

  EXPECT_ANY_THROW( domain1->index_of( -1 ) );
  EXPECT_ANY_THROW( domain1->index_of( 0 ) );
  EXPECT_ANY_THROW( domain2->index_of( 2 ) );
  EXPECT_ANY_THROW( domain2->index_of( 4 ) );
  EXPECT_ANY_THROW( domain3->index_of( 0 ) );
  EXPECT_ANY_THROW( domain3->index_of( 4 ) );
}

TEST_F(DomainTest, getSize)
{
  EXPECT_EQ( domain1->get_size(), 5 );
  EXPECT_EQ( domain2->get_size(), 5 );
  EXPECT_EQ( domain3->get_size(), 3 );
}

TEST_F(DomainTest, getValue)
{
  EXPECT_EQ( domain1->get_value( 0 ), 1 );
  EXPECT_EQ( domain1->get_value( 1 ), 3 );
  EXPECT_EQ( domain1->get_value( 2 ), 5 );
  EXPECT_EQ( domain1->get_value( 3 ), 7 );
  EXPECT_EQ( domain1->get_value( 4 ), 9 );

  EXPECT_EQ( domain2->get_value( 0 ), 1 );
  EXPECT_EQ( domain2->get_value( 1 ), 3 );
  EXPECT_EQ( domain2->get_value( 2 ), 5 );
  EXPECT_EQ( domain2->get_value( 3 ), 7 );
  EXPECT_EQ( domain2->get_value( 4 ), 9 );

  EXPECT_EQ( domain3->get_value( 0 ), 1 );
  EXPECT_EQ( domain3->get_value( 1 ), 2 );
  EXPECT_EQ( domain3->get_value( 2 ), 3 );
}

TEST_F(DomainTest, indexOf)
{
  EXPECT_EQ( domain1->index_of( 1 ), 0 );
  EXPECT_EQ( domain1->index_of( 3 ), 1 );
  EXPECT_EQ( domain1->index_of( 5 ), 2 );
  EXPECT_EQ( domain1->index_of( 7 ), 3 );
  EXPECT_EQ( domain1->index_of( 9 ), 4 );

  EXPECT_EQ( domain2->index_of( 1 ), 0 );
  EXPECT_EQ( domain2->index_of( 3 ), 1 );
  EXPECT_EQ( domain2->index_of( 5 ), 2 );
  EXPECT_EQ( domain2->index_of( 7 ), 3 );
  EXPECT_EQ( domain2->index_of( 9 ), 4 );

  EXPECT_EQ( domain3->index_of( 1 ), 0 );
  EXPECT_EQ( domain3->index_of( 2 ), 1 );
  EXPECT_EQ( domain3->index_of( 3 ), 2 );
}

TEST_F(DomainTest, randomValue)
{
  EXPECT_TRUE( CanFind( domain1->random_value() ) );
  EXPECT_TRUE( CanFind( domain1->random_value() ) );
  EXPECT_TRUE( CanFind( domain1->random_value() ) );
  EXPECT_TRUE( CanFind( domain1->random_value() ) );
  EXPECT_TRUE( CanFind( domain1->random_value() ) );

  EXPECT_TRUE( CanFind( domain2->random_value() ) );
  EXPECT_TRUE( CanFind( domain2->random_value() ) );
  EXPECT_TRUE( CanFind( domain2->random_value() ) );
  EXPECT_TRUE( CanFind( domain2->random_value() ) );
  EXPECT_TRUE( CanFind( domain2->random_value() ) );

  std::vector<int> count(5);
  for( int i = 0 ; i < 10000 ; ++i )
    ++count[ domain2->index_of( domain2->random_value() ) ];
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
