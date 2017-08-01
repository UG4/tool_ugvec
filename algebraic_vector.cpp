// This file is part of ugvec, a program for analysing and comparing vectors
//
// Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <iomanip>
#include <limits>
#include <cmath>
#include <cassert>
#include <string>
#include <sstream>
#include <cstring>
#include <map>

#include "algebraic_vector.h"

using namespace std;

ostream& operator << (ostream& out, const Position& p)
{
	out << "(" << p.x << ", " << p.y << ", " << p.z << ")[" << p.ci << "]";
	return out;
}


AlgebraicVector& AlgebraicVector::
add_vector(const AlgebraicVector& av)
{
//  iterate over all entries of av. If the position matches with an entry of
//  this vector, then add the values. If not, insert the value and its position
//  into this vector.

    assert(av.positions.size() == av.data.size());
    assert(positions.size() == data.size());

    if(worldDim == 0){
        assert(positions.size() == 0);
        worldDim = av.worldDim;
    }
    else{
        if(worldDim != av.worldDim){
            cout << "ERROR -- Can't add vectors with different world dimensions!" << endl;
            return *this;
        }
    }

    typedef map<Position, size_t>   PosMap;
    PosMap  posMap;
    for(size_t i = 0; i < positions.size(); ++i)
        posMap[positions[i]] = i;

    for(size_t i_av = 0; i_av < av.positions.size(); ++i_av){
        const Position& p_av = av.positions[i_av];
        PosMap::iterator piter = posMap.find(p_av);
        if(piter == posMap.end()){
            positions.push_back(p_av);
            data.push_back(av.data[i_av]);
        }
        else{
            data[piter->second] += av.data[i_av];
        }
    }

    return *this;
}


AlgebraicVector& AlgebraicVector::
add_vector(const AlgebraicVector& av, std::map<Position, size_t>& globPosMap)
{
//  iterate over all entries of av. If the position matches with an entry of
//  this vector, then add the values. If not, insert the value and its position
//  into this vector.

    assert(av.positions.size() == av.data.size());
    assert(positions.size() == data.size());

    if(worldDim == 0){
        assert(positions.size() == 0);
        worldDim = av.worldDim;
    }
    else{
        if(worldDim != av.worldDim){
            cout << "ERROR -- Can't add vectors with different world dimensions!" << endl;
            return *this;
        }
    }

    for(size_t i_av = 0; i_av < av.positions.size(); ++i_av){
        const Position& p_av = av.positions[i_av];
        std::map<Position, size_t>::iterator piter = globPosMap.find(p_av);
        if(piter == globPosMap.end()){
            positions.push_back(p_av);
            data.push_back(av.data[i_av]);
            globPosMap[p_av] = positions.size()-1;
        }
        else{
            data[piter->second] += av.data[i_av];
        }
    }

    return *this;
}

AlgebraicVector& AlgebraicVector::
unite_with_vector(const AlgebraicVector& av)
{
//  iterate over all entries of av. If the position matches with an entry of
//  this vector, then ignore the values. If not, insert the value and its position
//  into this vector.

    assert(av.positions.size() == av.data.size());
    assert(positions.size() == data.size());

    if(worldDim == 0){
        assert(positions.size() == 0);
        worldDim = av.worldDim;
    }
    else{
        if(worldDim != av.worldDim){
            cout << "ERROR -- Can't unite vectors with different world dimensions!" << endl;
            return *this;
        }
    }

    typedef map<Position, size_t>   PosMap;
    PosMap  posMap;
    for(size_t i = 0; i < positions.size(); ++i)
        posMap[positions[i]] = i;


    for(size_t i_av = 0; i_av < av.positions.size(); ++i_av){
        const Position& p_av = av.positions[i_av];
        PosMap::iterator piter = posMap.find(p_av);
        if(piter == posMap.end()){
            positions.push_back(p_av);
            data.push_back(av.data[i_av]);
        }
    }

    return *this;
}


AlgebraicVector& AlgebraicVector::
unite_with_vector(const AlgebraicVector& av, std::map<Position, size_t>& globPosMap)
{
//  iterate over all entries of av. If the position matches with an entry of
//  this vector, then ignore the values. If not, insert the value and its position
//  into this vector.

    assert(av.positions.size() == av.data.size());
    assert(positions.size() == data.size());

    if(worldDim == 0){
        assert(positions.size() == 0);
        worldDim = av.worldDim;
    }
    else{
        if(worldDim != av.worldDim){
            cout << "ERROR -- Can't unite vectors with different world dimensions!" << endl;
            return *this;
        }
    }

    for(size_t i_av = 0; i_av < av.positions.size(); ++i_av){
        const Position& p_av = av.positions[i_av];
        std::map<Position, size_t>::iterator piter = globPosMap.find(p_av);
        if(piter == globPosMap.end()){
            positions.push_back(p_av);
            data.push_back(av.data[i_av]);
            globPosMap[p_av] = positions.size()-1;
        }
    }

    return *this;
}

AlgebraicVector& AlgebraicVector::
subtract_vector(const AlgebraicVector& av)
{
//	scale local data by -1, add the vector and scale the data by -1 again.
	multiply_scalar(-1.);
	add_vector(av);
	multiply_scalar(-1.);
	return *this;
}

AlgebraicVector& AlgebraicVector::
multiply_scalar(number s)
{
	for(size_t i = 0; i < data.size(); ++i)
		data[i] *= s;
	return *this;
}


int AlgebraicVector::
max_component_index() const
{
	int maxCI = -1;
	for(size_t i = 0; i < positions.size(); ++i)
		maxCI = max(maxCI, positions[i].ci);
	return maxCI;
}
