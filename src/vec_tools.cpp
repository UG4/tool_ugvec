// This file is part of ugvec, a program for analysing and comparing vectors
//
// Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

#include "algebraic_vector.h"
#include "vec_tools.h"

using namespace std;

void PrintInfo(const AlgebraicVector& av)
{
	const int numComps = av.max_component_index() + 1;

	vector<size_t>	numEntriesPerComp(numComps, 0);
	for(size_t i = 0; i < av.positions.size(); ++i){
		const Position& p = av.positions[i];
		++numEntriesPerComp[p.ci];
	}

	cout << "Entries per component:" << endl;
	for(int i = 0; i < numComps; ++i)
		cout << "  [" << i << "]: " << numEntriesPerComp[i] << endl;
}


void PrintMinMax(const AlgebraicVector& av)
{
	if(av.data.empty())
		return;

	CHECK(av.positions.size() == av.data.size(),
		  "There should be as many position as data entries in an AlgebraicVector!");

	const int numCIs = av.max_component_index() + 1;

	vector<number> mins(numCIs, numeric_limits<number>::max());
	vector<number> maxs(numCIs, -numeric_limits<number>::max());
	vector<Position> minPos(numCIs);
	vector<Position> maxPos(numCIs);

	for(size_t i = 0; i < av.positions.size(); ++i){
		const int ci = av.positions[i].ci;
		if(av.data[i] < mins[ci]){
			mins[ci] = av.data[i];
			minPos[ci] = av.positions[i];
		}

		if(av.data[i] > maxs[ci]){
			maxs[ci] = av.data[i];
			maxPos[ci] = av.positions[i];
		}
	}

	for(int ci = 0; ci < numCIs; ++ci){
		cout << "Component " << ci << endl;
		cout << "  min: " << mins[ci] << "\tat   " << minPos[ci] << endl;
		cout << "  max: " << maxs[ci] << "\tat   " << maxPos[ci] << endl;
	}
}


void ExtractComponent(AlgebraicVector& out, const AlgebraicVector& av, int ci)
{
	out.positions.clear();
	out.data.clear();
	out.worldDim = av.worldDim;

	for(size_t i = 0; i < av.positions.size(); ++i){
		if(av.positions[i].ci == ci){
			out.positions.push_back(av.positions[i]);
			out.positions.back().ci = 0;
			out.data.push_back(av.data[i]);
		}
	}
}

	
void CreateHistogram(vector<int>& histOut, const AlgebraicVector& av,
					 int numSections, bool absoluteValues, bool logScale)
{
	CHECK(numSections > 0, "Invalid number of sections provided: " << numSections);
	CHECK(!logScale || absoluteValues, "Log scale histogram needs absolute values.")

	number minVal = numeric_limits<number>::max();
	number maxVal = -numeric_limits<number>::max();
	number minPosNonZeroVal = numeric_limits<number>::max();

	if(absoluteValues){
		for(size_t i = 0; i < av.data.size(); ++i){
			minVal = min(minVal, fabs(av.data[i]));
			maxVal = max(maxVal, fabs(av.data[i]));
			if (fabs(av.data[i]) > 0.0) minPosNonZeroVal = min(minPosNonZeroVal, fabs(av.data[i]));
		}
	}
	else{
		for(size_t i = 0; i < av.data.size(); ++i){
			minVal = min(minVal, av.data[i]);
			maxVal = max(maxVal, av.data[i]);
		}
	}

	number range;
	if (logScale)
	{
		// ensure valid data range for log
		CHECK(minVal >= 0, "Minimal absolute value cannot be negative!");
		CHECK(minPosNonZeroVal != numeric_limits<number>::max(),
			  "There needs to be at least one non-zero value for log scale histogram.");
		range = log(maxVal) - log(minPosNonZeroVal);
	}
	else
		range = maxVal - minVal;

	if(range <= 0){
		histOut.resize(av.data.size(), 0);
		return;
	}

	histOut.resize(av.data.size());
	vector<int> numEntriesPerSection(numSections, 0);

	if(absoluteValues){
		for(size_t i = 0; i < av.data.size(); ++i){
			int section;
			if (logScale)
			{
				if (av.data[i] == 0.0) section = 0;
				else section = (int)((number)numSections * (log(fabs(av.data[i])) - log(minPosNonZeroVal)) / range);
			}
			else
				section = (int)((number)numSections * (fabs(av.data[i]) - minVal) / range);
			if(section < 0) section = 0;
			if(section >= numSections) section = numSections - 1;
			histOut[i] = section;
			++numEntriesPerSection[section];
		}
	}
	else{
		for(size_t i = 0; i < av.data.size(); ++i){
			int section = (int)((number)numSections * (av.data[i] - minVal) / range);
			if(section < 0) section = 0;
			if(section >= numSections) section = numSections - 1;
			histOut[i] = section;
			++numEntriesPerSection[section];
		}
	}

	cout << "Histogram Created:" << endl;
	for(int isec = 0; isec < numSections; ++isec){
		cout << "section " << isec << ":\t";
		if (logScale) cout << (isec == 0 ? minVal : minPosNonZeroVal*exp((number)isec * range / (number)numSections));
		else cout << minVal + (number)isec * range / (number)numSections;
		cout << " - ";
		if (logScale) cout << minPosNonZeroVal * exp((number)(isec+1) * range / (number)numSections);
		else cout << minVal + (number)(isec+1) * range / (number)numSections;
		cout << ":\t" << numEntriesPerSection[isec] << " entries." << endl;
	}
}


bool SaveHistogramToUGX(const AlgebraicVector& av, const char* filename,
						int numSections, bool absoluteValues, bool logScale)
{
	cout << "INFO -- saving histogram to " << filename << endl;
	
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

	out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
	out << "<grid name=\"defGrid\">" << endl;
	out << "<vertices coords=\"" << av.worldDim << "\">";
	
	for(size_t i = 0; i < av.positions.size(); ++i){
		switch(av.worldDim){
			case 1:	out << av.positions[i].x; break;
			case 2:	out << av.positions[i].x << " " << av.positions[i].y; break;
			case 3:	out << av.positions[i].x << " " << av.positions[i].y << " " << av.positions[i].z; break;
			default:
				cout << "ERROR -- Unsupported world-dimension (" << av.worldDim
					 << ") during write: " << filename << endl;
				return false;
		}

		if(i + 1 < av.positions.size())
			out << " ";
	}

	out << "</vertices>" << endl;

	vector<int> hist;
	CreateHistogram(hist, av, numSections, absoluteValues, logScale);
	
	out << "<subset_handler name=\"defSH\">" << endl;
	for(int isec = 0; isec < numSections; ++isec){
		number ia = 0;
		if(numSections > 1)
			ia = (number)isec / (number)(numSections-1);
		number r = max<number>(0, -1 + 2 * ia);
		number g = 1. - fabs(2 * (ia - 0.5));
		number b = max<number>(0, 1. - 2 * ia);

		out << "<subset name=\"section " << isec
			<< "\" color=\"" << r << " " << g << " " << b << " 1\">" << endl;

		out << "<vertices>";
		for(size_t i = 0; i < hist.size(); ++i){
			if(hist[i] == isec){
				out << " " << int(i);
			}
		}
		out << "</vertices>" << endl;
		out << "</subset>" << endl;
	}
	out << "</subset_handler>" << endl;
	out << "</grid>" << endl;
	
	return true;
}
