//	created by Sebastian Reiter
//	s.b.reiter@gmail.com
//	Feb. 2013
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
		return (fabs(x) < fabs(x-p.x) < SMALL*(SMALL+max(fabs(x),fabs(p.x))))
			&& (fabs(y-p.y) < SMALL*(SMALL+max(fabs(y),fabs(p.y))))
			&& (fabs(z-p.z) < SMALL*(SMALL+max(fabs(z),fabs(p.z))))
			&& (ci == p.ci);
	}

	bool operator < (const Position& p) const
	{
		const bool cEqual = ci == p.ci;
		const bool xEqual = fabs(x-p.x) < SMALL*(SMALL+max(fabs(x),fabs(p.x)));
		const bool yEqual = fabs(y-p.y) < SMALL*(SMALL+max(fabs(y),fabs(p.y)));
		const bool zEqual = fabs(z-p.z) < SMALL*(SMALL+max(fabs(z),fabs(p.z)));

		if(cEqual){
			if(xEqual){
				if(yEqual){
					if(zEqual)
						return false;
					else
						return z < p.z;
				}
				else
					return y < p.y;
			}
			else
				return x < p.x;
		}
		else
			return ci < p.ci;
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
	AlgebraicVector() : worldDim(0), numComponents(1)	{}
	
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
	int numComponents;
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
	
	typedef map<Position, size_t>	PosMap;
	PosMap	posMap;
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
unite_with_vector(const AlgebraicVector& av)
{
//todo:	Improve performance!!!
//	iterate over all entries of av. If the position matches with an entry of
//	this vector, then ignore the values. If not, insert the value and its position
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
	
	typedef map<Position, size_t>	PosMap;
	PosMap	posMap;
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
	av.numComponents = numComponents;
	
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


	size_t numNANs = 0;
	for(int i = 0; i < numEntries; ++i){
		int ind1, ind2;
		string valstr;
		in >> ind1 >> ind2 >> valstr;
		if(ind1 != ind2){
			cout << "ERROR -- Only connections to self are supported! In File: "
				 << filename << endl;
		}
		
		if((ind1 < 0) || (ind1 >= numEntries)){
			cout << "ERROR -- Bad index: " << ind1 << ". In File: " << filename << endl;
		}

		if(valstr.find('n') != string::npos or valstr.find('N') != string::npos)
			++numNANs;

		try {av.data[ind1] = strtod(valstr.c_str(), NULL);}
		catch(exception& e) {cout << e.what();}
	}

	if(numNANs > 0)
		cout << "  -> WARNING: vector contains " << numNANs << " 'nan' entries!" << endl;
	
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
	
	av.numComponents = numComponents;

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
		out << int(i) << " " << int(i) << " "
			<< setprecision(numeric_limits<number>::digits10 + 1)
			<< av.data[i] << endl;
	}
	
	return true;
}


bool SaveAlgebraicVector(const AlgebraicVector& av, const char* filename, size_t nComp, size_t comp)
{
	cout << "INFO -- saving vector to " << filename << endl;

	if (av.data.size() != av.positions.size())
	{
		cout << "ERROR -- Invalid algebra vector - data and position size does not match."
			 << " During write to " << filename << endl;
		return false;
	}

	// check that size of data is multiple of #components
	if (av.data.size() % nComp)
	{
		cout << "ERROR -- Given number of components does not match number of overall values in vector.\n"
			 << "During write to " << filename << endl;
		return false;
	}

	// check that 0 <= comp < nComp
	if (comp >= nComp)
	{
		cout << "ERROR -- Requested component " << comp << " is not valid. Choose from 0 .. " << nComp << ".\n"
			 << "During write to " << filename << endl;
		return false;
	}

	ofstream out(filename);
	if (!out)
	{
		cout << "ERROR -- File can not be opened for write: " << filename << endl;
		return false;
	}

	out << int(1) << endl;
	out << av.worldDim << endl;
	out << av.positions.size()/nComp << endl;

	for (size_t i = comp; i < av.positions.size(); i += nComp)
	{
		switch (av.worldDim)
		{
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

	for (size_t i = comp; i < av.data.size(); i += nComp)
		out << int((i-comp)/5) << " " << int((i-comp)/5) << " "
		<< setprecision(numeric_limits<number>::digits10 + 1)
		<< av.data[i] << endl;

	return true;
}


void CreateHistogram(vector<int>& histOut, const AlgebraicVector& av,
					 int numSections, bool absoluteValues, bool logScale)
{
	CHECK(numSections > 0, "Invalid number of sections provided: " << numSections);
	CHECK(!logScale || absoluteValues, "Log scale histogram needs absolute values.")

	number minVal = numeric_limits<number>::max();
	number maxVal = numeric_limits<number>::min();
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


int main(int argc, char** argv)
{
	int		defHistoSecs = 5;

	bool	makeCons 		= true;
	int		numComponents	= 1;
	size_t	extractComp		= -1;
	int		histoSecs		= defHistoSecs;
	bool	histoAbs		= false;
	bool	histoLog		= false;

	static const int maxNumFiles = 3;
	const char* file[maxNumFiles];
	int numFiles = 0;

	for(int i = 2; i < argc; ++i){
		if(argv[i][0] == '-'){
			if(strcmp(argv[i], "-consistent") == 0){
				makeCons = false;
			}

			else if(strcmp(argv[i], "-components") == 0){
				if(i + 1 < argc){
					numComponents = atoi(argv[i+1]);
					++i;
				}
				else{
					cout << "Invalid use of '-components': An integer value has to be supplied." << endl;
					return 1;
				}
			}

			else if (strcmp(argv[i], "-extractComp") == 0){
				if (i + 1 < argc)
				{
					extractComp = (size_t) atoi(argv[i+1]);
					++i;
				}
				else{
					cout << "Invalid use of '-extractComp': An integer value has to be supplied." << endl;
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
		if(command.find("combine") == 0){
			CHECK(numFiles == 2, "An in-file and an out-file have to be specified");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, numComponents);
			SaveAlgebraicVector(av, file[1]);
		}
		else if(command.find("extract") == 0){
			CHECK(numFiles == 2, "An in-file and an out-file have to be specified");
			CHECK(extractComp != (size_t) -1, "Specifying -extractComp n is mandatory with 'extract'.");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, numComponents);
			SaveAlgebraicVector(av, file[1], numComponents, extractComp);
		}
		else if(command.find("dif") == 0){
			CHECK(numFiles == 3, "Two in-files and an out-file have to be specified");
			AlgebraicVector av1, av2;
			LoadVector(av1, file[0], makeCons, numComponents);
			LoadVector(av2, file[1], makeCons, numComponents);
			av1.subtract_vector(av2);
			SaveAlgebraicVector(av1, file[2]);
		
			if(!av1.data.empty()){
				cout << "min: " << av1.data[av1.index_with_min_value()]
					 << ", at: " << av1.positions[av1.index_with_min_value()] << endl;
				cout << "max: " << av1.data[av1.index_with_max_value()]
					 << ", at: " << av1.positions[av1.index_with_max_value()] << endl;
			}
		}
		else if(command.find("minmax") == 0){
			CHECK(numFiles == 1, "An in-file has to be specified.");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, numComponents);
			cout << "min: " << av.data[av.index_with_min_value()]
					 << ", at: " << av.positions[av.index_with_min_value()] << endl;
			cout << "max: " << av.data[av.index_with_max_value()]
				 << ", at: " << av.positions[av.index_with_max_value()] << endl;
		}
		else if(command.find("histogram") == 0){
			CHECK(numFiles == 2, "An in-file and an out-file have to be specified");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, numComponents);
			SaveHistogramToUGX(av, file[1], histoSecs, histoAbs, histoLog);
		}
		else{
			cout << "vecutil - (c) 2015 Sebastian Reiter, G-CSC Frankfurt" << endl;
			cout << endl;
			cout << "USAGE: vecutil command [options] [files]" << endl;
			cout << "OR:    vecutil command [files] [options]" << endl << endl;

			cout << "SAMPLE: vecutil dif -consistent vec1.vec vec2.pvec dif.vec" << endl << endl;

			cout << "COMMANDS:" << endl;
			cout << "  combine:   Loads a parallel vector and combines all parts to a serial one" << endl;
			cout << "             The parallel vector is assumed to be in additive storage." << endl;
			cout << "             If this is not the case, please specify the paramter -consistent" << endl;
			cout << "             2 Files required - 1: in-file, 2: out-file" << endl << endl;

			cout << "  extract:   Saves a specific component of a multi-component vector." << endl;
			cout << "             The additional options '-components m' and '-extractComp n'" << endl;
			cout << "             are mandatory for this mode of operation." << endl;
			cout << "             2 Files required - 1: in-file, 2: out-file" << endl << endl;

  			cout << "  dif:       Subtracts the second vector from the first and writes the result to a file." << endl;
  			cout << "             If a parallel input vectors are assumed to be in additive storage unless" << endl;
  			cout << "             the option -consistent was specified" << endl;
  			cout << "             3 Files required - 1: in-file-1, 2: in-file-2, 3: out-file" << endl << endl;

  			cout << "  minmax:    Prints the minimal and maximal value of a vector" << endl;
			cout << "             1 File required - 1: in-file" << endl << endl;

			cout << "  histogram: Creates a histogram using the options -histoSecs and -histoAbs and writes" << endl;
			cout << "             the result to a .ugx file." << endl;
			cout << "             2 Files required - 1: in-files, 2: out-file ('.ugx')" << endl << endl;


			cout << "OPTIONS:" << endl;
			cout << "  -consistent:      If this parameter is specified, parallel vectors are assumed" << endl;
			cout << "                    to be in consistent storage mode. Otherwise they are assumed" << endl;
			cout << "                    to be in additive storage mode." << endl << endl;

			cout << "  -components n:    If more than one component is stored in a vector, this has to" << endl;
			cout << "                    be indicated through this option. The option takes an additional" << endl;
			cout << "                    integer value 'n' that specifies the number of components." << endl << endl;

			cout << "  -extractComp n:   Mandatory option with the 'extract' mode of operation." << endl;
			cout << "                    The number n specifies the component index (0 <= n < #comp) to be extracted." << endl << endl;

			cout << "  -histoSecs n:     Define the number of histogram-sections if a hostogram-command is used." << endl;
			cout << "                    default is "<< defHistoSecs << endl << endl;

			cout << "  -histoAbs:        If specified, histogram command will operate on absolute values." << endl << endl;
			cout << "  -histoLog:        If specified, histogram command will use sections on a logarithmic scale" << endl
				 << "                    (this implies -histoAbs)." << endl << endl;
		}
	}
	catch(...){
		return 1;
	}
	return 0;
}


