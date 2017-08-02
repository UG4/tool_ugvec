// This file is part of ugvec, a program for analysing and comparing vectors
//
// Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef __H__ugvec_file_io
#define __H__ugvec_file_io

struct AlgebraicVector;

///	determines Load_... method by filename and fills the 'av' from 'filename'
/**	If component >= 0, only the specified component will be loaded into 'av'*/
bool LoadVector(AlgebraicVector& av, const char* filename,
				bool makeConsistent, int component = -1);


/// Loads serial vectors in the vec format
bool Load_VEC (AlgebraicVector& av, const char* filename);

///	Loads parallel vectors in the pvec format
bool Load_PVEC (AlgebraicVector& av, const char* filename, bool makeConsistent);

/// Loads serial vectors in the vtu format
bool Load_VTU (AlgebraicVector& av, const char* filename);

///	Loads parallel vectors in the pvtu format
bool Load_PVTU (AlgebraicVector& av, const char* filename, bool makeConsistent);



bool Save_VEC(const AlgebraicVector& av, const char* filename);

#endif	//__H__ugvec_file_io


