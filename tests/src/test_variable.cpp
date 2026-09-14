#include <ghost/variable.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace ghost
{
	class VariableTest : public ::testing::Test
	{
	public:
		std::vector<int> domain{1,3,5,7,9};
	
		ghost::Variable var_ctor1{ 0, &domain, 3, "var_ctor1" };
		ghost::Variable var_ctor2{ 1, &domain, 2 };
		ghost::Variable var_ctor3{ 2, &domain };
		ghost::Variable var_ctorcopy{ var_ctor3 };
		ghost::Variable var_ctorassign = var_ctor3;
	};

	TEST_F(VariableTest, GetValue)
	{
		EXPECT_EQ( var_ctor1.get_value(), 7 );
		EXPECT_EQ( var_ctor2.get_value(), 5 );
		EXPECT_EQ( var_ctor3.get_value(), 1 );
		EXPECT_EQ( var_ctorcopy.get_value(), 1 );
		EXPECT_EQ( var_ctorassign.get_value(), 1 );
	}

	TEST_F(VariableTest, SetValue)
	{
		var_ctor1.set_value( 5 );
		var_ctor2.set_value( 1 );
		var_ctor3.set_value( 9 );
		var_ctorcopy.set_value( 7 );
		var_ctorassign.set_value( 3 );

		EXPECT_EQ( var_ctor1.get_value(), 5 );
		EXPECT_EQ( var_ctor2.get_value(), 1 );
		EXPECT_EQ( var_ctor3.get_value(), 9 );
		EXPECT_EQ( var_ctorcopy.get_value(), 7 );
		EXPECT_EQ( var_ctorassign.get_value(), 3 );
	}

	TEST_F(VariableTest, Exceptions)
	{
		EXPECT_ANY_THROW( var_ctor1.set_value( 2 ) );
		EXPECT_ANY_THROW( var_ctor1.set_value( 4 ) );
		EXPECT_ANY_THROW( var_ctor1.set_value( -5 ) );
		EXPECT_ANY_THROW( var_ctor1.set_value( 500 ) );
		EXPECT_ANY_THROW( var_ctor1.set_value( 0 ) );

		EXPECT_ANY_THROW( var_ctor2.set_value( 2 ) );
		EXPECT_ANY_THROW( var_ctor2.set_value( 4 ) );
		EXPECT_ANY_THROW( var_ctor2.set_value( -5 ) );
		EXPECT_ANY_THROW( var_ctor2.set_value( 500 ) );
		EXPECT_ANY_THROW( var_ctor2.set_value( 0 ) );

		EXPECT_ANY_THROW( var_ctor3.set_value( 2 ) );
		EXPECT_ANY_THROW( var_ctor3.set_value( 4 ) );
		EXPECT_ANY_THROW( var_ctor3.set_value( -5 ) );
		EXPECT_ANY_THROW( var_ctor3.set_value( 500 ) );
		EXPECT_ANY_THROW( var_ctor3.set_value( 0 ) );

		EXPECT_ANY_THROW( var_ctorcopy.set_value( 2 ) );
		EXPECT_ANY_THROW( var_ctorcopy.set_value( 4 ) );
		EXPECT_ANY_THROW( var_ctorcopy.set_value( -5 ) );
		EXPECT_ANY_THROW( var_ctorcopy.set_value( 500 ) );
		EXPECT_ANY_THROW( var_ctorcopy.set_value( 0 ) );

		EXPECT_ANY_THROW( var_ctorassign.set_value( 2 ) );
		EXPECT_ANY_THROW( var_ctorassign.set_value( 4 ) );
		EXPECT_ANY_THROW( var_ctorassign.set_value( -5 ) );
		EXPECT_ANY_THROW( var_ctorassign.set_value( 500 ) );
		EXPECT_ANY_THROW( var_ctorassign.set_value( 0 ) );
	}

	TEST_F(VariableTest, GetNames)
	{
		EXPECT_EQ( var_ctor1.get_name(), "var_ctor1" );
		EXPECT_EQ( var_ctor2.get_name(), "" );
		EXPECT_EQ( var_ctor3.get_name(), "" );
		EXPECT_EQ( var_ctorcopy.get_name(), "" );
		EXPECT_EQ( var_ctorassign.get_name(), "" );
	}

	TEST_F(VariableTest, GetID)
	{
		EXPECT_EQ( var_ctor1.get_id(), 0 );
		EXPECT_EQ( var_ctor2.get_id(), 1 );
		EXPECT_EQ( var_ctor3.get_id(), 2 );
		EXPECT_EQ( var_ctorcopy.get_id(), 2 );
		EXPECT_EQ( var_ctorassign.get_id(), 2 );
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
