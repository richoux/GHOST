#include <ghost/variable.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class VariableTest : public ::testing::Test
{
public:
	ghost::Variable var_ctor1 { std::vector<int>{1,9,5,7,3}, 3, "var_ctor1" };
	ghost::Variable *var_ctor2;
  ghost::Variable *var_ctor3;
  ghost::Variable *var_ctor4;

  VariableTest()
  {
	  var_ctor2 = new ghost::Variable( std::vector<int>{2,8,6,4,0}, "var_ctor2" );
    var_ctor3 = new ghost::Variable( 7, 10, 1 );
    var_ctor4 = new ghost::Variable( 4, 5, "var_ctor4" );
  }

  ~VariableTest()
  {
	  // delete var_ctor1;
    delete var_ctor2;
    delete var_ctor3;
    delete var_ctor4;
  }

	::testing::AssertionResult CanFind( bool v1, int value )
  {
	  std::vector<int> domain;
	  std::string name;
	  
	  if( v1 )
	  {
		  domain = var_ctor1.get_full_domain();
		  name = var_ctor1.get_name();
	  }
	  else
	  {
		  domain = var_ctor2->get_full_domain();
		  name = var_ctor2->get_name();
	  }
	  
    if( std::find( begin( domain ), end( domain ), value ) != end( domain ) )
      return ::testing::AssertionSuccess();
    else
	    return ::testing::AssertionFailure() << value << " is NOT in the domain of " << name;
  }
};

// Variable::_id is not generated anymore by Variable constructors since GHOST v2
// // Warning: each test calls the constructor of VariableTest,
// // so 4 new variables are created each time, with new IDs.
// TEST_F(VariableTest, IdAndCopy)
// {
//   ghost::Variable var_copy1( var_ctor1 );
//   ghost::Variable var_copy2( *var_ctor2 );
//   ghost::Variable var_copy3( *var_ctor3 );
//   var_copy3 = var_ctor1;
  
//   EXPECT_EQ( var_ctor1.get_id(), 0 );
//   EXPECT_EQ( var_ctor2->get_id(), 1 );
//   EXPECT_EQ( var_ctor3->get_id(), 2 );
//   EXPECT_EQ( var_ctor4->get_id(), 3 );
  
//   EXPECT_EQ( var_copy1.get_id(), 0 );
//   EXPECT_EQ( var_copy2.get_id(), 1 );
//   EXPECT_EQ( var_copy3.get_id(), 0 );

//   EXPECT_EQ( var_copy1.get_domain_size(), 5 );
//   EXPECT_EQ( var_copy2.get_domain_size(), 5 );

//   EXPECT_EQ( var_copy1.get_name(), "var_ctor1" );
//   EXPECT_EQ( var_copy2.get_name(), "var_ctor2" );

//   EXPECT_EQ( var_copy2.get_value(), 2 );
//   var_copy2.set_value( 8 );
//   EXPECT_EQ( var_copy2.get_value(), 8 );
//   EXPECT_EQ( var_ctor2->get_value(), 2 );
// }

TEST_F(VariableTest, Exceptions)
{
  EXPECT_ANY_THROW( var_ctor1.set_value( 2 ) );
  EXPECT_ANY_THROW( var_ctor1.set_value( 4 ) );

  EXPECT_ANY_THROW( var_ctor2->set_value( 1 ) );
  EXPECT_ANY_THROW( var_ctor2->set_value( 3 ) );

  EXPECT_ANY_THROW( var_ctor3->set_value( 6 ) );
  EXPECT_ANY_THROW( var_ctor3->set_value( 20 ) );

  EXPECT_ANY_THROW( var_ctor4->set_value( 3 ) );
  EXPECT_ANY_THROW( var_ctor4->set_value( 20 ) );
}

TEST_F(VariableTest, DomainSize)
{
  EXPECT_EQ( var_ctor1.get_domain_size(), 5 );
  EXPECT_EQ( var_ctor2->get_domain_size(), 5 );
  EXPECT_EQ( var_ctor3->get_domain_size(), 10 );
  EXPECT_EQ( var_ctor4->get_domain_size(), 5 );
}

TEST_F(VariableTest, Names)
{
  EXPECT_EQ( var_ctor1.get_name(), "var_ctor1" );
  EXPECT_EQ( var_ctor2->get_name(), "var_ctor2" );
  EXPECT_EQ( var_ctor3->get_name(), "" );
  EXPECT_EQ( var_ctor4->get_name(), "var_ctor4" );
}

TEST_F(VariableTest, Values)
{
  EXPECT_EQ( var_ctor1.get_value(), 7 );
  EXPECT_EQ( var_ctor2->get_value(), 2 );
  EXPECT_EQ( var_ctor3->get_value(), 8 );
  EXPECT_EQ( var_ctor4->get_value(), 4 );

  var_ctor1.set_value( 5 );
  var_ctor2->set_value( 0 );
  var_ctor3->set_value( 9 );
  var_ctor4->set_value( 8 );

  EXPECT_EQ( var_ctor1.get_value(), 5 );
  EXPECT_EQ( var_ctor2->get_value(), 0 );
  EXPECT_EQ( var_ctor3->get_value(), 9 );
  EXPECT_EQ( var_ctor4->get_value(), 8 );

  EXPECT_EQ( var_ctor1.get_domain_min_value(), 1 );
  EXPECT_EQ( var_ctor2->get_domain_min_value(), 0 );
  EXPECT_EQ( var_ctor3->get_domain_min_value(), 7 );
  EXPECT_EQ( var_ctor4->get_domain_min_value(), 4 );

  EXPECT_EQ( var_ctor1.get_domain_max_value(), 9 );
  EXPECT_EQ( var_ctor2->get_domain_max_value(), 8 );
  EXPECT_EQ( var_ctor3->get_domain_max_value(), 16 );
  EXPECT_EQ( var_ctor4->get_domain_max_value(), 8 );
 
  EXPECT_THAT( var_ctor1.get_full_domain(), ::testing::ElementsAre( 1,9,5,7,3 ) );
  EXPECT_THAT( var_ctor2->get_full_domain(), ::testing::ElementsAre( 2,8,6,4,0 ) );
  EXPECT_THAT( var_ctor3->get_full_domain(), ::testing::ElementsAre( 7,8,9,10,11,12,13,14,15,16 ) );
  EXPECT_THAT( var_ctor4->get_full_domain(), ::testing::ElementsAre( 4,5,6,7,8 ) );
}

// This also tests the correcness of the index assignment done by set_value
TEST_F(VariableTest, PartialDomains)
{
	EXPECT_THAT( var_ctor1.get_partial_domain( 3 ), ::testing::ElementsAre( 5,7,3 ) );
  EXPECT_THAT( var_ctor2->get_partial_domain( 3 ), ::testing::ElementsAre( 2,8,0 ) );
  EXPECT_THAT( var_ctor3->get_partial_domain( 5 ), ::testing::ElementsAre( 7,8,9,10,16 ) );
  EXPECT_THAT( var_ctor4->get_partial_domain( 3 ), ::testing::ElementsAre( 4,5,8 ) );

  EXPECT_THAT( var_ctor1.get_partial_domain( 5 ), ::testing::ElementsAre( 1,9,5,7,3 ) );
  EXPECT_THAT( var_ctor2->get_partial_domain( 5 ), ::testing::ElementsAre( 2,8,6,4,0 ) );
  EXPECT_THAT( var_ctor3->get_partial_domain( 10 ), ::testing::ElementsAre( 7,8,9,10,11,12,13,14,15,16 ) );
  EXPECT_THAT( var_ctor4->get_partial_domain( 5 ), ::testing::ElementsAre( 4,5,6,7,8 ) );

  var_ctor1.set_value( 5 );
  var_ctor2->set_value( 0 );
  var_ctor3->set_value( 9 );
  var_ctor4->set_value( 4 );
  
  EXPECT_THAT( var_ctor1.get_partial_domain( 4 ), ::testing::ElementsAre( 1,9,5,7 ) );
  EXPECT_THAT( var_ctor2->get_partial_domain( 4 ), ::testing::ElementsAre( 2,6,4,0 ) );
  EXPECT_THAT( var_ctor3->get_partial_domain( 5 ), ::testing::ElementsAre( 7,8,9,10,11 ) );
  EXPECT_THAT( var_ctor4->get_partial_domain( 1 ), ::testing::ElementsAre( 4 ) );

  var_ctor4->set_value( 6 );
  EXPECT_THAT( var_ctor4->get_partial_domain( 1 ), ::testing::ElementsAre( 6 ) );

  var_ctor4->set_value( 8 );
  EXPECT_THAT( var_ctor4->get_partial_domain( 1 ), ::testing::ElementsAre( 8 ) );
}

// Variable::pick_random_value() is private since GHOST v2
// TEST_F(VariableTest, RandomValue)
// {
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );
// 	var_ctor1.pick_random_value();
// 	EXPECT_TRUE( CanFind( true, var_ctor1.get_value() ) );

// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );
// 	var_ctor2->pick_random_value();
// 	EXPECT_TRUE( CanFind( false, var_ctor2->get_value() ) );

// 	std::vector<int> count(5);

// 	for( int i = 0 ; i < 10000 ; ++i )
// 	{
// 		var_ctor4->pick_random_value();
// 		++count[ var_ctor4->get_value() - 4 ];
// 	}

// 	// random draw distribution should be between 18% and 22% for each number
// 	EXPECT_GE( (double)count[0] / 100, 18 );
// 	EXPECT_GE( (double)count[1] / 100, 18 );
// 	EXPECT_GE( (double)count[2] / 100, 18 );
// 	EXPECT_GE( (double)count[3] / 100, 18 );
// 	EXPECT_GE( (double)count[4] / 100, 18 );

// 	EXPECT_LE( (double)count[0] / 100, 22 );
// 	EXPECT_LE( (double)count[1] / 100, 22 );
// 	EXPECT_LE( (double)count[2] / 100, 22 );
// 	EXPECT_LE( (double)count[3] / 100, 22 );
// 	EXPECT_LE( (double)count[4] / 100, 22 );

// 	// std::cout << (double)count[0] / 100 << "% "
// 	//           << (double)count[1] / 100 << "% "
// 	//           << (double)count[2] / 100 << "% "
// 	//           << (double)count[3] / 100 << "% "
// 	//           << (double)count[4] / 100 << "%\n";
// }

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
