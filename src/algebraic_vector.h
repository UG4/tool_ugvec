// This file is part of ugvec, a program for analysing and comparing vectors
//
// Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef __H__algebraic_vector
#define __H__algebraic_vector

#include <vector>
#include <map>
#include <iostream>
#include "ugvec_base.h"


struct Position{
	Position() : x(0), y(0), z(0), ci(0)	{}
	
	bool operator == (const Position& p) const
	{
		return x == p.x && y == p.y && z == p.z && ci == p.ci;
	}

	bool operator < (const Position& p) const
	{
		if(ci < p.ci)
			return true;
		else if(ci > p.ci)
			return false;
		
		if(x < p.x)
			return true;
		else if(x > p.x)
			return false;

		if(y < p.y)
			return true;
		else if(y > p.y)
			return false;

		if(z < p.z)
			return true;
		else if(z > p.z)
			return false;

		return false;
	}
	
	union {
		struct {
			number x;
			number y;
			number z;
		};

		number coord[3];
	};
	
	int ci;			///< component index
};


std::ostream& operator << (std::ostream& out, const Position& p);


struct AlgebraicVector{
	AlgebraicVector() : worldDim(0)	{}
	
///	adds values with same positions and inserts the others
/**	returns a reference to this vector, so that add_vector can be chained.*/
    AlgebraicVector& add_vector(const AlgebraicVector& av);
    AlgebraicVector& add_vector(const AlgebraicVector& av, std::map<Position, size_t>& globPosMap);

///	subtracts values with same positions.
/**	If a position was not found in this vector, a default value of 0 will be assumed
 * returns a reference to this vector, so that add_vector can be chained.*/
	AlgebraicVector& subtract_vector(const AlgebraicVector& av);
	
///	inserts values from the specified vector which did not yet exist in this vector
/**	returns a reference to this vector, so that unite_with_vector can be chained.*/
	AlgebraicVector& unite_with_vector(const AlgebraicVector& av);
	AlgebraicVector& unite_with_vector(const AlgebraicVector& av, std::map<Position, size_t>& globPosMap);

///	multiplies all data values by the given scalar
	AlgebraicVector& multiply_scalar(number s);
	
///	returns the maximum component index stored in positions. ATTENTION: O(n)
/** \return The highest component index in positions. -1 if positions is empty.*/
	int max_component_index() const;

	void swap (AlgebraicVector& av);
	
	int	worldDim;
	std::vector<Position>	positions;
	std::vector<number>		data;
};


#endif	//__H__algebraic_vector
