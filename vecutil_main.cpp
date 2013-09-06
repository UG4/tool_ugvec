//	created by Sebastian Reiter
//	s.b.reiter@gmail.com
//	Feb. 2013

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <string>
#include <sstream>
#include <map>
#include "parameter_util.h"

using namespace std;

typedef double	number;
typedef unsigned int uint;

const number SMALL = 1.e-12;

class CommonError{};
#define CHECK(expr, msg) 	{if(!(expr)) {cout << "ERROR: " << msg << endl; throw CommonError();}}



struct Position{
	Position() : x(0), y(0), z(0), ci(0)	{}
	
	bool operator == (const Position& p) const
	{
		return (fabs(x-p.x) < SMALL) && (fabs(y-p.y) < SMALL)
				 && (fabs(z-p.z) < SMALL) && (ci == p.ci);
	}
	
	number x, y, z;
	int ci;			///< component index
};

ostream& operator << (ostream& out, const Position& p)
{
	out << "(" << p.x << ", " << p.y << ", " << p.z << ")[" << p.ci << "]";
	return out;
}


struct AlgebraicVector{
	AlgebraicVector() : worldDim(0)	{}
	
///	adds values with same positions and inserts the others
/**	returns a reference to this vector, so that add_vector can be chained.*/
	AlgebraicVector& add_vector(const AlgebraicVector& av);
	
///	subtracts values with same positions.
/**	If a position was not found in this vector, a default value of 0 will be assumed
 * returns a reference to this vector, so that add_vector can be chained.*/
	AlgebraicVector& subtract_vector(const AlgebraicVector& av);
	
///	inserts values from the specified vector which did not yet exist in this vector
/**	returns a reference to this vector, so that unite_with_vector can be chained.*/
	AlgebraicVector& unite_with_vector(const AlgebraicVector& av);

///	multiplies all data values by the given scalar
	AlgebraicVector& multiply_scalar(number s);
	
///	returns the first index at which the maximum can be found. -1 for empty vectors.
	int index_with_max_value() const;
///	returns the first index at which the minimum can be found. -1 for empty vectors.
	int index_with_min_value() const;
	
	int	worldDim;
	vector<Position> positions;
	vector<number>	 data;
};


AlgebraicVector& AlgebraicVector::
add_vector(const AlgebraicVector& av)
{
//todo:	Improve performance!!!
//	iterate over all entries of av. If the position matches with an entry of
//	this vector, then add the values. If not, insert the value and its position
//	into this vector.
	
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
		bool gotOne = false;
		for(size_t i = 0; i < positions.size(); ++i){
			if(p_av == positions[i]){
				gotOne = true;
				data[i] += av.data[i_av];
				break;
			}
		}
			
		if(!gotOne){
			positions.push_back(p_av);
			data.push_back(av.data[i_av]);
		}
	}
	
	return *this;
}

AlgebraicVector& AlgebraicVector::
unite_with_vector(const AlgebraicVector& av)
{
//todo:	Improve performance!!!
//	iterate over all entries of av. If the position matches with an entry of
//	this vector, then add the values. If not, insert the value and its position
//	into this vector.
	
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
		bool gotOne = false;
		for(size_t i = 0; i < positions.size(); ++i){
			if(p_av == positions[i]){
				gotOne = true;
				break;
			}
		}
			
		if(!gotOne){
			positions.push_back(p_av);
			data.push_back(av.data[i_av]);
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
index_with_max_value() const
{
	if(data.empty())
		return -1;
	int maxInd = 0;
	
	for(int i = 1; i < data.size(); ++i){
		if(data[i] > data[maxInd])
			maxInd = i;
	}
	
	return maxInd;
}

///	returns the first index at which the minimum can be found
int AlgebraicVector::
index_with_min_value() const
{
	if(data.empty())
		return -1;
	int minInd = 0;
	
	for(int i = 1; i < data.size(); ++i){
		if(data[i] < data[minInd])
			minInd = i;
	}
	
	return minInd;
}


	
	
bool LoadAlgebraicVector(AlgebraicVector& av, const char* filename, int numComponents)
{
	cout << "INFO -- loading vector from " << filename << endl;
	
	ifstream in(filename);
	if(!in){
		cout << "ERROR -- File not found: " << filename << endl;
		return false;
	}
	
	int blockSize;
	in >> blockSize;
	in >> av.worldDim;
	
	if(blockSize != 1){
		cout << "ERROR -- Only blocksize 1 is supported! In File: " << filename << endl;
		return false;
	}
	
	
	int numEntries;
	in >> numEntries;
	av.positions.clear();
	av.positions.resize(numEntries);
	av.data.clear();
	av.data.resize(numEntries, 0);
	
	
	for(int i = 0; i < numEntries; ++i){
		switch(av.worldDim){
			case 1:	in >> av.positions[i].x; break;
			case 2:	in >> av.positions[i].x >> av.positions[i].y; break;
			case 3:	in >> av.positions[i].x >> av.positions[i].y >> av.positions[i].z; break;
			default:
				cout << "ERROR -- Unsupported world-dimension (" << av.worldDim
					 << ") during write: " << filename << endl;
				return false;
		}
		av.positions[i].ci = (i % numComponents);
	}
	
	
	int someVal;
	in >> someVal;

	if(blockSize != 1){
		cout << "ERROR -- Value in between positions and data should be 1! In File: "
			 << filename << endl;
		return false;
	}

	
	for(int i = 0; i < numEntries; ++i){
		int ind1, ind2;
		number val;
		in >> ind1 >> ind2 >> val;
		if(ind1 != ind2){
			cout << "ERROR -- Only connections to self are supported! In File: "
				 << filename << endl;
		}
		
		if((ind1 < 0) || (ind1 >= numEntries)){
			cout << "ERROR -- Bad index: " << ind1 << ". In File: " << filename << endl;
		}
		av.data[ind1] = val;
	}
	
	return true;
}


bool LoadParallelVector(AlgebraicVector& av, const char* filename,
						bool makeConsistent, int numComponents)
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
	
	for(int i = 0; i < numFiles; ++i){
		//char serialFile[512];
		//inParallel.getline(serialFile, 511);
		string serialFile;
		inParallel >> serialFile;
		string tfilename = path + serialFile;
		AlgebraicVector tmpAv;
		if(LoadAlgebraicVector(tmpAv, tfilename.c_str(), numComponents)){
			if(makeConsistent)
				av.add_vector(tmpAv);
			else
				av.unite_with_vector(tmpAv);
		}
		else
			return false;
	}
	
	return true;
}

bool LoadVector(AlgebraicVector& av, const char* filename,
				bool makeConsistent, int numComponents)
{
	string name = filename;
	if(name.rfind(".pvec") != string::npos){
		return LoadParallelVector(av, filename, makeConsistent, numComponents);
	}
	else{
		return LoadAlgebraicVector(av, filename, numComponents);
	}
}

bool SaveAlgebraicVector(const AlgebraicVector& av, const char* filename)
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
		out << int(i) << " " << int(i) << " " << av.data[i] << endl;
	}
	
	return true;
}

/*
int oldMain(int argc, char** argv)
{
	string command;
	if(argc > 1)
		command = argv[1];
		
	if(command.find("make_consistent") == 0){
		if(argc != 4){
			cout << "ERROR: An infile and an outfile have to be specified!" << endl;
		}
		else{
			AlgebraicVector av;
			LoadParallelVector(av, argv[2], true);
			SaveAlgebraicVector(av, argv[3]);
		}
	}
	else if(command.find("unite") == 0){
		if(argc != 4){
			cout << "ERROR: An infile and an outfile have to be specified!" << endl;
		}
		else{
			AlgebraicVector av;
			LoadParallelVector(av, argv[2], false);
			SaveAlgebraicVector(av, argv[3]);
		}
	}
	else if(command.find("difference") == 0){
		if(argc != 5){
			cout << "ERROR: Two infiles and an outfile have to be specified!" << endl;
		}
		else{
			AlgebraicVector av1, av2;
			LoadAlgebraicVector(av1, argv[2]);
			LoadAlgebraicVector(av2, argv[3]);
			av1.subtract_vector(av2);
			SaveAlgebraicVector(av1, argv[4]);
			
			if(!av1.data.empty()){
				cout << "min: " << av1.data[av1.index_with_min_value()]
					 << ", at: " << av1.positions[av1.index_with_min_value()] << endl;
				cout << "max: " << av1.data[av1.index_with_max_value()]
					 << ", at: " << av1.positions[av1.index_with_max_value()] << endl;
			}
		}
	}
	else if(command.find("consdiff") == 0){
		if(argc != 5){
			cout << "ERROR: Two infiles and an outfile have to be specified!" << endl;
		}
		else{
			AlgebraicVector av1, av2;
			LoadVector(av1, argv[2], true);
			LoadVector(av2, argv[3], true);
			av1.subtract_vector(av2);
			SaveAlgebraicVector(av1, argv[4]);
			
			if(!av1.data.empty()){
				cout << "min: " << av1.data[av1.index_with_min_value()]
					 << ", at: " << av1.positions[av1.index_with_min_value()] << endl;
				cout << "max: " << av1.data[av1.index_with_max_value()]
					 << ", at: " << av1.positions[av1.index_with_max_value()] << endl;
			}
		}
	}
	else if(command.find("unitediff") == 0){
		if(argc != 5){
			cout << "ERROR: Two infiles and an outfile have to be specified!" << endl;
		}
		else{
			AlgebraicVector av1, av2;
			LoadVector(av1, argv[2], false);
			LoadVector(av2, argv[3], false);
			av1.subtract_vector(av2);
			SaveAlgebraicVector(av1, argv[4]);
			
			if(!av1.data.empty()){
				cout << "min: " << av1.data[av1.index_with_min_value()]
					 << ", at: " << av1.positions[av1.index_with_min_value()] << endl;
				cout << "max: " << av1.data[av1.index_with_max_value()]
					 << ", at: " << av1.positions[av1.index_with_max_value()] << endl;
			}
		}
	}
	else if(command.find("minmax") == 0){
		if(argc != 3){
			cout << "ERROR: An infile has to be specified!" << endl;
		}
		else{
			AlgebraicVector av;
			LoadAlgebraicVector(av, argv[2]);
			cout << "min: " << av.data[av.index_with_min_value()]
					 << ", at: " << av.positions[av.index_with_min_value()] << endl;
			cout << "max: " << av.data[av.index_with_max_value()]
				 << ", at: " << av.positions[av.index_with_max_value()] << endl;
		}
	}
	else{
		cout << "Invalid command. Please specify one of the following commands: " << endl;
		cout << "  make_consistent: Loads a parallel vector and unites it to a serial one" << endl;
		cout << "                   by summing up values with the same positions." << endl;
		cout << "  unite: Loads a parallel consistent vector and unites it to a serial one" << endl;
		cout << "         by simply inserting values in one large vector." << endl;
		cout << "  difference: Subtracts the second vector from the first and writes" << endl;
		cout << "              the result to a file." << endl;
		cout << "  consdiff: Subtracts the second vector from the first and writes" << endl;
		cout << "              the result to a file. Parallel vectors are made consistent before diff." << endl;
		cout << "  unitediff: Subtracts the second vector from the first and writes" << endl;
		cout << "              the result to a file. Parallel vectors are united before diff." << endl;
		cout << "  minmax: Prints the minimal and maximal value of a vector" << endl;
	}
	
	return 0;
}
*/

int main(int argc, char** argv)
{
	const char* in1 = NULL;
	const char* in2 = NULL;
	const char* outFile = NULL;

	ParamToString(&in1, "-i1", argc, argv);
	ParamToString(&in2, "-i2", argc, argv);
	ParamToString(&outFile, "-o", argc, argv);
	
	bool makeCons = FindParam("-makeCons", argc, argv);
	int numComponents = ParamToInt("-components", argc, argv, 1);

	if(makeCons)
		cout << "make consistent (-makeCons):        active" << endl;
	else
		cout << "make consistent (-makeCons):        inactive" << endl;
		
	cout << "num components (-components n):     " << numComponents << endl;
	
	string command;
	if(argc > 1)
		command = argv[1];


	try{
		if(command.find("combine") == 0){
			CHECK(in1 && outFile, "Files -i1 and -o have to be specified for combine.");
			AlgebraicVector av;
			LoadVector(av, in1, makeCons, numComponents);
			SaveAlgebraicVector(av, outFile);
		}
		else if(command.find("dif") == 0){
			CHECK(in1 && in2 && outFile, "Files -i1, -i2 and -o have to be specified for dif.");
			AlgebraicVector av1, av2;
			LoadVector(av1, in1, makeCons, numComponents);
			LoadVector(av2, in2, makeCons, numComponents);
			av1.subtract_vector(av2);
			SaveAlgebraicVector(av1, outFile);
		
			if(!av1.data.empty()){
				cout << "min: " << av1.data[av1.index_with_min_value()]
					 << ", at: " << av1.positions[av1.index_with_min_value()] << endl;
				cout << "max: " << av1.data[av1.index_with_max_value()]
					 << ", at: " << av1.positions[av1.index_with_max_value()] << endl;
			}
		}
		else if(command.find("minmax") == 0){
			CHECK(in1, "File -i1 has to be specified for minmax.");
			AlgebraicVector av;
			LoadVector(av, in1, makeCons, numComponents);
			cout << "min: " << av.data[av.index_with_min_value()]
					 << ", at: " << av.positions[av.index_with_min_value()] << endl;
			cout << "max: " << av.data[av.index_with_max_value()]
				 << ", at: " << av.positions[av.index_with_max_value()] << endl;
		}
		else{
			cout << "Invalid command. Please specify one of the following commands as first parameter: " << endl;
			cout << "  combine: Loads a parallel vector and combines all parts to a serial one" << endl;
			cout << "  dif: Subtracts the second vector from the first and writes the result to a file." << endl;
			cout << "  minmax: Prints the minimal and maximal value of a vector" << endl;
		}
	}
	catch(...){
	}
	return 0;
}


