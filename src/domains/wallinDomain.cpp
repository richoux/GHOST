/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP/COP. 
 * It is a generalization of the project Wall-in.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014 Florian Richoux
 *
 * This file is part of GHOST.
 * GHOST is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GHOST is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GHOST. If not, see http://www.gnu.org/licenses/.
 */


#include <iomanip>
#include <algorithm>

#include "../../include/domains/wallinDomain.hpp"

namespace ghost
{
	WallinDomain::WallinDomain(int maxX, int maxY, int nbVar, int sX, int sY, int tX, int tY)
		: Domain(maxX*maxY + 1, nbVar, -1),
		maxX_(maxX),
		maxY_(maxY),
		matrixType_(vector< vector<string> >(maxY, vector<string>(maxY, ""))),
		matrixId_(vector< vector< set<int> > >(maxY, vector< set<int> >(maxY, set<int>()))),
		startingTile(make_pair(sX, sY)),
		targetTile(make_pair(tX, tY))
	{
		matrixType_[sX][sY] += "@s";
		matrixType_[tX][tY] += "@t";
	}

	WallinDomain::WallinDomain(int maxX, int maxY,
		const vector< pair<int, int> > &unbuildables, const vector< Building > *variables,
		int sX, int sY, int tX, int tY)
		: WallinDomain(maxX, maxY, variables->size(), sX, sY, tX, tY)
	{
		for (const auto &u : unbuildables) {
// 			cout << "Assign # at point " << u.first << "," << u.second << endl;
			matrixType_[u.first][u.second].assign(3, '#');
		}

		for (const auto &v : *variables)
			domains[v.getId()] = possiblePos(v);
	}

  void WallinDomain::add(const Building& building)
  {
	  if (building.isSelected()) {
		  pair<int, int> pos = lin2mat(building.getValue());
		  int posX = pos.first;
		  int posY = pos.second;
// 		  cout << "Adding building " << building.getName() << " at " << posX << "," << posY << endl;

		  for (int x = posX; x < posX + building.getLength(); ++x)
			  for (int y = posY; y < posY + building.getHeight(); ++y)
				  add(x, y, building.getName(), building.getId());
// 		  cout << *this;
	  }
  }

  void WallinDomain::add(int x, int y, string b_short, int b_id)
  {
	  bool fail = !(matrixType_[x][y].empty()
		  || (matrixType_[x][y].find("@") != string::npos && matrixType_[x][y].size() <= 3));

	  matrixType_[x][y] += b_short;
	  matrixId_[x][y].insert(b_id);
	  if (fail) {
		  pair<int, int> key(x, y);
		  if (failures_.find(key) == failures_.end()) {
			  failures_.emplace(key, matrixType_[x][y]);
		  } else {
			  failures_.at(key) += b_short;
		  }

	  }
  }
  
  void WallinDomain::clear(const Building& building)
  {
	  if (building.isSelected()) {
		  pair<int, int> pos = lin2mat(building.getValue());
		  int posX = pos.first;
		  int posY = pos.second;

// 		  cout << "Clear building " << building.getName() << " at " << posX << "," << posY << endl;
		  for (int x = posX; x < posX + building.getLength(); ++x)
			  for (int y = posY; y < posY + building.getHeight(); ++y)
				  clear(x, y, building.getName(), building.getId());
// 		  cout << *this;
	  }
  }

  void WallinDomain::clear(int x, int y, string b_short, int b_id)
  {
	  auto it = matrixType_[x][y].find(b_short);
	  if (it != string::npos) {
		  matrixType_[x][y].replace(it, b_short.length(), "");
		  matrixId_[x][y].erase(b_id);

		  pair<int, int> key(x, y);
		  mapFail::iterator it = failures_.find(key);

		  if (it != failures_.end()) {
			  if (matrixType_[x][y].size() < 2
				  || matrixType_[x][y].compare("###") == 0
				  || (matrixType_[x][y].size() == 2 && matrixType_[x][y].find("@") != string::npos))
				  failures_.erase(it);
			  else
				  failures_.at(key) = matrixType_[x][y];
		  }
	  }
  }

  pair<int, int> WallinDomain::shift(Building &building)
  {
	  int overlaps = 0;
	  int unbuildables = 0;

	  if (building.isSelected()) {
		  pair<int, int> pos = lin2mat(building.getValue());
		  int posX = pos.first;
		  int posY = pos.second;
// 		  cout << *this;
// 		  cout << "Shift building " << building.getName() << " at " << posX << "," << posY << endl;

		  int shiftX = posX + building.getLength();
		  int shiftY = posY + building.getHeight();

		  pair<int, int> key;

		  for (int y = posY; y < shiftY; ++y) {
			  add(shiftX, y, building.getName(), building.getId());

			  key = make_pair(shiftX, y);
			  if (failures_.find(key) != failures_.end()) {
				  if (failures_.at(key).find("###") == std::string::npos)
					  ++overlaps;
				  else
					  ++unbuildables;
			  }

			  key = make_pair(posX, y);
			  if (failures_.find(key) != failures_.end())  {
				  if (failures_.at(key).find("###") == std::string::npos)
					  --overlaps;
				  else
					  --unbuildables;
			  }

			  clear(posX, y, building.getName(), building.getId());
		  }

		  building.shiftValue();
	  }

	  pair<int, int> pos = lin2mat(building.getValue());
// 	  cout << "Building " << building.getName() << " at " << pos.first << "," << pos.second << endl;
// 	  cout << *this;
// 	  cout << "Overlaps: " << overlaps << " Unbuildable: " << unbuildables << endl;
	  return make_pair(overlaps, unbuildables);
  }

  void WallinDomain::quickShift(Building &building)
  {
	  if (building.isSelected())  {
		  pair<int, int> pos = lin2mat(building.getValue());
		  int posX = pos.first;
		  int posY = pos.second;

		  int shiftX = posX + building.getLength();
		  int shiftY = posY + building.getHeight();

		  for (int y = posY; y < shiftY; ++y) {
			  add(shiftX, y, building.getName(), building.getId());
			  clear(posX, y, building.getName(), building.getId());
		  }

		  building.shiftValue();
	  }
  }

  void WallinDomain::swap( Building &first, Building &second )
  {
    clear( first );
    clear( second );
    first.swapValue( second );
    add( first );
    add( second );
  }  

  set< Building > WallinDomain::getBuildingsAround(const Building &b, const vector< Building > *variables) const
  {
	  set< Building > myNeighbors;
	  int left, top, right, bottom;
	  int left2, top2, right2, bottom2;

	  if (b.isSelected()) {
		  pair<int, int> pos = lin2mat(b.getValue());
		  left = pos.first;
		  top = pos.second;
		  right = left + b.getLength() - 1;
		  bottom = top + b.getHeight() - 1;

		  for (const auto& build2 : *variables) {
			  if (build2.getId() != b.getId() && build2.isSelected()) {
				  pair<int, int> pos2 = lin2mat(build2.getValue());
				  left2 = pos2.first;
				  top2 = pos2.second;
				  right2 = left2 + build2.getLength() - 1;
				  bottom2 = top2 + build2.getHeight() - 1;

				  if ((top == bottom2 + 1 && (right2 >= left && left2 <= right))
					  || (right == left2 - 1 && (bottom2 >= top - 1 && top2 <= bottom + 1))
					  || (bottom == top2 - 1 && (right2 >= left && left2 <= right))
					  || (left == right2 + 1 && (bottom2 >= top - 1 && top2 <= bottom + 1)))
				  {
					  myNeighbors.insert(build2);
				  }
			  }
		  }
	  }

	  return myNeighbors;
  }

  set< Building > WallinDomain::getBuildingsAbove(const Building &b, const vector< Building > *variables) const
  {
	  set< Building > myNeighbors;
	  int left, top, right, bottom;
	  int left2, top2, right2, bottom2;

	  if (b.isSelected())  {
		  pair<int, int> pos = lin2mat(b.getValue());
		  left = pos.first;
		  top = pos.second;
		  right = left + b.getLength() - 1;
		  bottom = top + b.getHeight() - 1;

		  for (const auto& build2 : *variables) {
			  if (build2.getId() != b.getId() && build2.isSelected()) {
				  pair<int, int> pos2 = lin2mat(build2.getValue());
				  left2 = pos2.first;
				  top2 = pos2.second;
				  right2 = left2 + build2.getLength() - 1;
				  bottom2 = top2 + build2.getHeight() - 1;

				  if (top == bottom2 + 1 && right2 >= left && left2 <= right)
					  myNeighbors.insert(build2);
			  }
		  }
	  }

	  return myNeighbors;
  }

  set< Building > WallinDomain::getBuildingsOnRight(const Building &b, const vector< Building > *variables) const
  {
	  set< Building > myNeighbors;
	  int left, top, right, bottom;
	  int left2, top2, right2, bottom2;

	  if (b.isSelected())  {
		  pair<int, int> pos = lin2mat(b.getValue());
		  left = pos.first;
		  top = pos.second;
		  right = left + b.getLength() - 1;
		  bottom = top + b.getHeight() - 1;

		  for (const auto& build2 : *variables) {
			  if (build2.getId() != b.getId() && build2.isSelected()) {
				  pair<int, int> pos2 = lin2mat(build2.getValue());
				  left2 = pos2.first;
				  top2 = pos2.second;
				  right2 = left2 + build2.getLength() - 1;
				  bottom2 = top2 + build2.getHeight() - 1;

				  if (right == left2 - 1 && bottom2 >= top - 1 && top2 <= bottom + 1)
					  myNeighbors.insert(build2);
			  }
		  }
	  }

	  return myNeighbors;
  }

  set< Building > WallinDomain::getBuildingsBelow(const Building &b, const vector< Building > *variables) const
  {
	  set< Building > myNeighbors;
	  int left, top, right, bottom;
	  int left2, top2, right2, bottom2;

	  if (b.isSelected())  {
		  pair<int, int> pos = lin2mat(b.getValue());
		  left = pos.first;
		  top = pos.second;
		  right = left + b.getLength() - 1;
		  bottom = top + b.getHeight() - 1;

		  for (const auto& build2 : *variables) {
			  if (build2.getId() != b.getId() && build2.isSelected()) {
				  pair<int, int> pos2 = lin2mat(build2.getValue());
				  left2 = pos2.first;
				  top2 = pos2.second;
				  right2 = left2 + build2.getLength() - 1;
				  bottom2 = top2 + build2.getHeight() - 1;

				  if (bottom == top2 - 1 && right2 >= left && left2 <= right)
					  myNeighbors.insert(build2);
			  }
		  }
	  }

	  return myNeighbors;
  }

  set< Building > WallinDomain::getBuildingsOnLeft(const Building &b, const vector< Building > *variables) const
  {
	  set< Building > myNeighbors;
	  int left, top, right, bottom;
	  int left2, top2, right2, bottom2;

	  if (b.isSelected())  {
		  pair<int, int> pos = lin2mat(b.getValue());
		  left = pos.first;
		  top = pos.second;
		  right = left + b.getLength() - 1;
		  bottom = top + b.getHeight() - 1;

		  for (const auto& build2 : *variables) {
			  if (build2.getId() != b.getId() && build2.isSelected()) {
				  pair<int, int> pos2 = lin2mat(build2.getValue());
				  left2 = pos2.first;
				  top2 = pos2.second;
				  right2 = left2 + build2.getLength() - 1;
				  bottom2 = top2 + build2.getHeight() - 1;

				  if (left == right2 + 1 && bottom2 >= top - 1 && top2 <= bottom + 1)
					  myNeighbors.insert(build2);
			  }
		  }
	  }

	  return myNeighbors;
  }

  int WallinDomain::distanceTo( int source, pair<int, int> target ) const
  {
    pair<int, int> sourcePair = lin2mat( source );
    return abs( target.first - sourcePair.first ) + abs( target.second - sourcePair.second );
  }

  void WallinDomain::unbuildable( vector< pair<int, int> > unbuildables )
  {
    for( const auto &u : unbuildables )
      this->unbuildable( u.first, u.second );    
  }

  bool WallinDomain::isStartingOrTargetTile( int id ) const
  {
    auto startingBuildings = buildingsAt( getStartingTile() );
    auto targetBuildings = buildingsAt( getTargetTile() );

    return startingBuildings.find( id ) != startingBuildings.end()
      || targetBuildings.find( id ) != targetBuildings.end();
  }

  bool WallinDomain::isNeightborOfSTTBuildings( const Building &building, vector< Building > others  ) const
  {
    auto startingBuildings = buildingsAt( getStartingTile() );
    auto targetBuildings = buildingsAt( getTargetTile() );

    remove_if( begin(others), end(others), [&](Building b)
	       {
		 return ( find( begin(startingBuildings), end(startingBuildings), b.getId() ) == end(startingBuildings) )
		   &&
		   ( find( begin(targetBuildings), end(targetBuildings), b.getId() ) == end(targetBuildings) );
	       }
	       );

    return getBuildingsAround( building, &others ).size() != 0;
  }
  
  int WallinDomain::countAround( const Building &b, const vector< Building > *variables ) const
  {
    if( b.isSelected() )
      return getBuildingsAround( b, variables ).size();
    else
      return 0;
  }

  vector<int> WallinDomain::possiblePos(const Building& b) const
  {
	  vector<int> possiblePositions;
	  possiblePositions.push_back(-1);

	  for (int x = 0; x <= maxX_ - b.getLength(); ++x) {
		  for (int y = 0; y <= maxY_ - b.getHeight(); ++y) {
			  // check if corners (topLeft, topRight, bottomLeft, bottomRight) are buildable
			  if (matrixType_[x][y].compare("###") != 0
				  && matrixType_[x][y + b.getHeight() - 1].compare("###") != 0
				  && matrixType_[x + b.getLength() - 1][y + b.getHeight() - 1].compare("###") != 0
				  && matrixType_[x + b.getLength() - 1][y].compare("###") != 0)
			  {
				  possiblePositions.push_back(mat2lin(x, y));
			  }
		  }
	  }

	  return possiblePositions;
  }

  void WallinDomain::v_restart( vector<Building> *variables )
  {
    for( const auto &v : *variables )
      clear( v );
    
    for( auto &v : *variables )
    {
      // 1 chance over 3 to be placed on the domain
      if( random.getRandNum(3) == 0)
      {
	v.setValue( randomValue( v ) );
	add( v );
      }
      else
	v.setValue( -1 );
    }
  }

  void WallinDomain::v_wipe( vector<Building> *variables )
  {
    for( const auto &v : *variables )
      clear( v );
  }
  
  void WallinDomain::v_rebuild( vector<Building> *variables )
  {
    for( const auto &v : *variables )
      add( v );
  }
  
  ostream& operator<<(ostream& os, const WallinDomain& g)
  {
	  string barLine = "";
	  for (size_t i = 0; i < g.matrixType_[0].size(); ++i)
		  barLine += "------";

// 	  os << "#max X: " << g.maxX_ << endl
// 		  << "#max Y: " << g.maxY_ << endl
// 		  << "Matrix Id:" << endl;
// 
// 	  for (int y = 0; y < g.maxY_; ++y) {
// 		  os << barLine << endl << "| ";
// 		  for (int x = 0; x < g.maxX_; ++x) {
// 			  if (g.matrixId_[x][y].empty()) {
// 				  os << setw(3) << "    | ";
// 			  } else {
// 				  for (const auto &id : g.matrixId_[x][y])
// 					  os << setw(3) << to_string(id) << " | ";
// 			  }
// 		  }
// 		  os << endl;
// 	  }
// 	  os << barLine << endl << endl;
// 
// 	  os << "Matrix Type:" << endl;
	  for (int y = 0; y < g.maxY_; ++y) {
		  os << barLine << endl << "| ";
		  for (int x = 0; x < g.maxX_; ++x) {
			  os << setw(3) << (g.matrixType_[x][y].empty() ? " " : g.matrixType_[x][y]) << " | ";
		  }
		  os << endl;
	  }
	  os << barLine << endl;

	  os << "Failures:" << endl;
	  for (const auto &m : g.failures_)
		  os << "(" << m.first.first << "," << m.first.second << "):" << m.second << endl;

	  return os;
  }

}
