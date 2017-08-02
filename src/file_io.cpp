// This file is part of ugvec, a program for analysing and comparing vectors
//
// Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

#include "algebraic_vector.h"
#include "file_io.h"
#include "vec_tools.h"

using namespace std;

bool LoadVector(AlgebraicVector& av, const char* filename,
				bool makeConsistent, int component)
{
	string name = filename;
	
	bool success = false;

	if(name.rfind(".pvec") != string::npos)
		success = Load_PVEC(av, filename, makeConsistent);
	else if(name.rfind(".vec") != string::npos)
		success = Load_VEC(av, filename);
	else if(name.rfind(".pvtu") != string::npos)
		success = Load_PVTU(av, filename, makeConsistent);
	else if(name.rfind(".vtu") != string::npos)
		success = Load_VTU(av, filename);


	if(success && component >= 0){
		AlgebraicVector tmpAv;
		ExtractComponent(tmpAv, av, component);
		av.swap (tmpAv);
	}

	return success;
}
