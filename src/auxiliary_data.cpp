#include <algorithm>

#include "auxiliary_data.hpp"

AuxiliaryData::AuxiliaryData( const std::vector<Variable>& variables )
{
	std::transform( variables.begin(),
	                variables.end(),
	                std::back_inserter( ptr_variables ),
	                [&](const Variable &v){ return &v; } );
}

void AuxiliaryData::update()
{
	for( auto p_v : ptr_variables )
		update( p_v, p_v->get_value() );
}

