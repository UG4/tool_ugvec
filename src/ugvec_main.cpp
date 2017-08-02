// This file is part of ugvec, a program for analysing and comparing vectors
//
// Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <vector>
#include <cmath>
#include <cassert>
#include <string>
#include <sstream>
#include <cstring>
#include <map>

#include "algebraic_vector.h"
#include "file_io.h"
#include "vec_tools.h"


using namespace std;


int main(int argc, char** argv)
{
	int		defHistoSecs = 5;

	bool	makeCons 		= true;
	int		component		= -1;
	int		histoSecs		= defHistoSecs;
	bool	histoAbs		= false;
	bool	histoLog		= false;
	bool	verbose			= false;

	static const int maxNumFiles = 3;
	const char* file[maxNumFiles];
	int numFiles = 0;

	for(int i = 2; i < argc; ++i){
		if(argv[i][0] == '-'){
			
			if(strcmp(argv[i], "-consistent") == 0){
				makeCons = false;
			}

			else if (strcmp(argv[i], "-component") == 0){
				if (i + 1 < argc)
				{
					component = atoi(argv[i+1]);
					++i;
				}
				else{
					cout << "Invalid use of '-component': An integer value has to be supplied." << endl;
					return 1;
				}
			}

			else if(strcmp(argv[i], "-histoSecs") == 0){
				if(i + 1 < argc){
					histoSecs = atoi(argv[i+1]);
					++i;
				}
				else{
					cout << "Invalid use of '-histoSecs': An integer value has to be supplied." << endl;
					return 1;
				}
			}

			else if(strcmp(argv[i], "-histoAbs") == 0){
				histoAbs = true;
			}

			else if(strcmp(argv[i], "-histoLog") == 0){
				histoLog = true;
				histoAbs = true;
			}

			else if(strcmp(argv[i], "-verbose") == 0){
				verbose = true;
			}

			else{
				cout << "Invalid option supplied: " << argv[i] << endl;
				return 1;
			}
		}

		else if(numFiles < maxNumFiles){
			file[numFiles] = argv[i];
			++numFiles;
		}

		else{
			cout << "Can't interpret parameter " << argv[i] << ": Too many parameters specified." << endl;
			return 1;
		}
	}
	
	string command;
	if(argc > 1)
		command = argv[1];


	try{
		if(command.find("process") == 0){
			CHECK(numFiles == 2, "An in-file and an out-file have to be specified");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, component);
			if(verbose){
				cout << "vector properties:\n";
				PrintInfo(av);
			}
			Save_VEC(av, file[1]);
		}
		else if(command.find("dif") == 0){
			CHECK(numFiles == 3, "Two in-files and an out-file have to be specified");
			AlgebraicVector av1, av2;
			LoadVector(av1, file[0], makeCons, component);
			LoadVector(av2, file[1], makeCons, component);
			
			if(verbose){
				cout << "Properties of v1:\n";
				PrintInfo(av1);
				cout << "Properties of v2:\n";
				PrintInfo(av1);
			}
			
			av1.subtract_vector(av2);

			if(verbose){
				cout << "Properties of v1-v2:\n";
				PrintInfo(av1);
			}

			Save_VEC(av1, file[2]);
			PrintMinMax(av1);
		}
		else if(command.find("minmax") == 0){
			CHECK(numFiles == 1, "An in-file has to be specified.");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, component);
			if(verbose){
				cout << "vector properties:\n";
				PrintInfo(av);
			}
			PrintMinMax(av);
		}
		else if(command.find("histogram") == 0){
			CHECK(numFiles == 2, "An in-file and an out-file have to be specified");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, component);
			if(verbose){
				cout << "vector properties:\n";
				PrintInfo(av);
			}
			SaveHistogramToUGX(av, file[1], histoSecs, histoAbs, histoLog);
		}
		else if(command.find("info") == 0){
			CHECK(numFiles == 1, "An in-file has to be specified");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, component);
			PrintInfo(av);
		}
		else{
			cout << "ugvec - (c) 2013-2017 Sebastian Reiter, G-CSC Frankfurt" << endl;
			cout << endl;
			cout << "USAGE: ugvec command [options] [files]" << endl;
			cout << "OR:    ugvec command [files] [options]" << endl << endl;

			cout << "SAMPLE: ugvec dif -consistent vec1.vec vec2.pvec dif.vec" << endl << endl;

			cout << "COMMANDS:" << endl;

			cout << "  process:   Loads a vector, processes it, and saves it to the specified file." << endl;
			cout << "             If the vector is parallel, it will combine it to a serial one before saving." << endl;
			cout << "             The default storage type assumed is 'additive'. If the provided vector has" << endl;
			cout << "             consistent storage type, please specify the option '-consistent'." << endl;
			cout << "             If a component is specified through the '-component' option, only the specified" << endl;
			cout << "             component will be written to the resulting file." << endl;
			cout << "             2 Files required - 1: in-file, 2: out-file" << endl << endl;

  			cout << "  dif:       Subtracts the second vector from the first and writes the result to a file." << endl;
  			cout << "             Parallel input vectors are assumed to be in additive storage unless" << endl;
  			cout << "             the option -consistent was specified" << endl;
  			cout << "             3 Files required - 1: in-file-1, 2: in-file-2, 3: out-file" << endl << endl;

  			cout << "  minmax:    Prints the minimal and maximal values of each component of a vector" << endl;
			cout << "             1 File required - 1: in-file" << endl << endl;

			cout << "  histogram: Creates a histogram using the options -histoSecs and -histoAbs and writes" << endl;
			cout << "             the result to a .ugx file." << endl;
			cout << "             2 Files required - 1: in-files, 2: out-file ('.ugx')" << endl << endl;

			cout << "  info:      Prints Information on the number of entries, components, etc." << endl << endl;


			cout << "OPTIONS:" << endl;

			cout << "  -consistent:      If this parameter is specified, parallel vectors are assumed" << endl;
			cout << "                    to be in consistent storage mode. Otherwise they are assumed" << endl;
			cout << "                    to be in additive storage mode." << endl << endl;

			cout << "  -component n:     The number n specifies the component index (0 <= n < #comp) on which to work." << endl;
			cout << "                    All other components will be dismissed." << endl << endl;

			cout << "  -histoSecs n:     Define the number of histogram-sections if a hostogram-command is used." << endl;
			cout << "                    default is "<< defHistoSecs << endl << endl;

			cout << "  -histoAbs:        If specified, histogram command will operate on absolute values." << endl << endl;

			cout << "  -histoLog:        If specified, histogram command will use sections on a logarithmic scale" << endl;
			cout << "                    (this implies -histoAbs)." << endl << endl;

			cout << "  -verbose:         If specified, additional information is printed for each processed vector." << endl << endl;
		}
	}
	catch(...){
		return 1;
	}
	return 0;
}


