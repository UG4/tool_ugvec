// This file is part of ugvec, a program for analysing and comparing vectors
//
// Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef __H__ugvec_vec_tools
#define __H__ugvec_vec_tools

#include <vector>

struct AlgebraicVector;

void PrintInfo(const AlgebraicVector& av);

void PrintMinMax(const AlgebraicVector& av);

void ExtractComponent(AlgebraicVector& out, const AlgebraicVector& av, int ci);

void CreateHistogram(std::vector<int>& histOut, const AlgebraicVector& av,
					 int numSections, bool absoluteValues, bool logScale);

bool SaveHistogramToUGX(const AlgebraicVector& av, const char* filename,
						int numSections, bool absoluteValues, bool logScale);


#endif	//__H__ugvec_vec_tools


