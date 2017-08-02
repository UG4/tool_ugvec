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

bool Load_VEC (AlgebraicVector& av, const char* filename)
{
	cout << "INFO -- loading vector from " << filename << endl;
	
	string line;
	ifstream in(filename);
	if(!in){
		cout << "ERROR -- File not found: " << filename << endl;
		return false;
	}
	
	int blockSize;
	in >> blockSize;
	in >> av.worldDim;
	
	int numEntries;
	in >> numEntries;
	av.positions.clear();
	av.positions.reserve(numEntries);
	
	std::map<Position, size_t> posMap;


	for(int i = 0; i < numEntries; ++i){
		Position p;
		switch(av.worldDim){
			case 1:	in >> p.x; break;
			case 2:	in >> p.x >> p.y; break;
			case 3:	in >> p.x >> p.y >> p.z; break;
			default:
				cout << "ERROR -- Unsupported world-dimension (" << av.worldDim
					 << ") during write: " << filename << endl;
				return false;
		}

	//	during the lookup p.ci should always be 0
		size_t& ci = posMap[p];
		p.ci = ci;
		++ci;
		av.positions.push_back(p);
	}
	
//	get the rest of the last line
	getline(in, line);

//	read some arbitrary value which separates positions and connections
	getline(in, line);

//	read data values
	av.data.clear();
	av.data.resize(numEntries, 0);

	size_t numNANs = 0;
	vector<double> values;

	for(int i = 0; i < numEntries; ++i){
		getline(in, line);
		size_t start = 0;
		values.clear();
		while(start < line.size()) {
			const char cur = line[start];
			if(cur == ' '){
				++start;
				continue;
			}
			else if (cur == '[' || cur == ']'){
				++start;
				continue;
			}
			else{
				size_t end = line.find(' ', start);
				if(end == string::npos)
					end = line.size();
				const size_t num = end - start;
				if((line.find("n", start, num) != string::npos)
					|| (line.find("N", start, num) != string::npos))
				{
					++numNANs;
					continue;
				}

				char* endPtr;
				const char* startPtr = line.c_str() + start;
				values.push_back(strtod(startPtr, &endPtr));
				start += endPtr - startPtr;
			}
		}

		if(values.size() < 3){
			cout << "ERROR -- Not enough values specified in connection. In File: "
				<< filename << endl;
			cout << "line read: " << line << endl;
			continue;
		}

		const int ind1 = static_cast<int>(values[0]);
		const int ind2 = static_cast<int>(values[1]);

		if(ind1 != ind2){
			cout << "ERROR -- Only connections to self are supported! In File: "
				 << filename << endl;
		}
		
		if((ind1 < 0) || (ind1 >= numEntries)){
			cout << "ERROR -- Bad index: " << ind1 << ". In File: " << filename << endl;
		}

		av.data[ind1] = values[2];

		for(size_t i = 3; i < values.size(); ++i){
			Position p = av.positions[ind1];
			p.ci = i-2;
			av.positions.push_back(p);
			av.data.push_back(values[i]);
		}
	}

	if(numNANs > 0)
		cout << "  -> WARNING: vector contains " << numNANs << " 'nan' entries!" << endl;
	
	return true;
}


bool Load_PVEC(AlgebraicVector& av, const char* filename, bool makeConsistent)
{
	cout << "INFO -- loading parallel vector from " << filename << endl;
	
//	extract the path from filename
	string strFilename = filename;
	string path;
	size_t lastSlash = strFilename.find_last_of("/");
	if(lastSlash == string::npos)
		lastSlash = strFilename.find_last_of("\\");
		
	if(lastSlash != string::npos)
		path = strFilename.substr(0, lastSlash + 1);
	
//	load the parallel file
	ifstream inParallel(filename);
	if(!inParallel){
		cout << "ERROR -- File not found: " << filename << endl;
		return false;
	}
	
	int numFiles;
	inParallel >> numFiles;
	
	bool useGlobPosMap = false;
	#ifdef PARALLEL_LOAD_SPEEDUP
		useGlobPosMap = (numFiles > 1);
	#endif

	std::map<Position, size_t> globPosMap;

	for(int i = 0; i < numFiles; ++i){
        //char serialFile[512];
        //inParallel.getline(serialFile, 511);
        string serialFile;
        inParallel >> serialFile;
        string tfilename = path + serialFile;
        AlgebraicVector tmpAv;
        if(Load_VEC(tmpAv, tfilename.c_str())){
        	if(useGlobPosMap){
        		cout << "  using parallel load speedup.\n";
				if(makeConsistent)
					av.add_vector(tmpAv, globPosMap);
				else
					av.unite_with_vector(tmpAv, globPosMap);
			}
			else{
				if(makeConsistent)
					av.add_vector(tmpAv);
				else
					av.unite_with_vector(tmpAv);
			}
        }
        else
            return false;
    }

	return true;
}


bool Save_VEC(const AlgebraicVector& av, const char* filename)
{
	cout << "INFO -- saving vector to " << filename << endl;
	
	if(av.data.size() != av.positions.size()){
		cout << "ERROR -- Invalid algebra vector - data and position size does not match."
			 << " During write to " << filename << endl;
		return false;
	}
	
	ofstream out(filename);
	if(!out){
		cout << "ERROR -- File can not be opened for write: " << filename << endl;
		return false;
	}
	
	out << int(1) << endl;
	out << av.worldDim << endl;
	out << av.positions.size() << endl;
	
	for(size_t i = 0; i < av.positions.size(); ++i){
		switch(av.worldDim){
			case 1:	out << av.positions[i].x << endl; break;
			case 2:	out << av.positions[i].x << " " << av.positions[i].y << endl; break;
			case 3:	out << av.positions[i].x << " " << av.positions[i].y << " " << av.positions[i].z << endl; break;
			default:
				cout << "ERROR -- Unsupported world-dimension (" << av.worldDim
					 << ") during write: " << filename << endl;
				return false;
		}
	}
	
	out << int(1) << endl;
	
	for(size_t i = 0; i < av.data.size(); ++i){
		out << int(i) << " " << int(i) << " "
			<< setprecision(numeric_limits<number>::digits10 + 1)
			<< av.data[i] << endl;
	}
	
	return true;
}