// This file is part of ugvec, a program for analysing and comparing vectors
//
// Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef __H__ugvec_base
#define __H__ugvec_base

typedef double	number;
typedef unsigned int uint;

class CommonError{};
#define CHECK(expr, msg) 	{if(!(expr)) {cout << "ERROR: " << msg << endl; throw CommonError();}}

#endif	//__H__ugvec_base


