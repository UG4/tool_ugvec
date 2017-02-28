//	created by Sebastian Reiter
//	s.b.reiter@gmail.com

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

const number DEFAULT_SMALL = 1.e-8;
number SMALL = DEFAULT_SMALL;

class CommonError{};
#define CHECK(expr, msg) 	{if(!(expr)) {cout << "ERROR: " << msg << endl; throw CommonError();}}



struct Position{
	Position() : x(0), y(0), z(0), ci(0)	{}
	
	bool operator == (const Position& p) const
	{
		return (fabs(x-p.x) < SMALL)
			&& (fabs(y-p.y) < SMALL)
			&& (fabs(z-p.z) < SMALL)
			&& (ci == p.ci);
	}

	bool operator < (const Position& p) const
	{
		const bool cEqual = ci == p.ci;
		const bool xEqual = fabs(x-p.x) < SMALL;
		const bool yEqual = fabs(y-p.y) < SMALL;
		const bool zEqual = fabs(z-p.z) < SMALL;

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
	
///	returns the maximum component index stored in positions. ATTENTION: O(n)
/** \return The highest component index in positions. -1 if positions is empty.*/
	int max_component_index() const;

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
max_component_index() const
{
	int maxCI = -1;
	for(size_t i = 0; i < positions.size(); ++i)
		maxCI = max(maxCI, positions[i].ci);
	return maxCI;
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

	
bool LoadAlgebraicVector(AlgebraicVector& av, const char* filename)
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


bool LoadParallelVector(AlgebraicVector& av, const char* filename,
						bool makeConsistent)
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
		if(LoadAlgebraicVector(tmpAv, tfilename.c_str())){
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
				bool makeConsistent, int component = -1)
{
	string name = filename;
	
	if(component == -1){
		if(name.rfind(".pvec") != string::npos)
			return LoadParallelVector(av, filename, makeConsistent);
		else
			return LoadAlgebraicVector(av, filename);
	}
	else {
		AlgebraicVector tmpAv;
		bool success = false;

		if(name.rfind(".pvec") != string::npos)
			success = LoadParallelVector(tmpAv, filename, makeConsistent);
		else
			success = LoadAlgebraicVector(tmpAv, filename);
		
		if(success)
			ExtractComponent(av, tmpAv, component);

		return success;
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


int main(int argc, char** argv)
{
	int		defHistoSecs = 5;

	bool	makeCons 		= true;
	int		component		= -1;
	int		histoSecs		= defHistoSecs;
	bool	histoAbs		= false;
	bool	histoLog		= false;

	static const int maxNumFiles = 3;
	const char* file[maxNumFiles];
	int numFiles = 0;

	for(int i = 2; i < argc; ++i){
		if(argv[i][0] == '-'){
			if(strcmp(argv[i], "-small") == 0){
				if (i + 1 < argc)
				{
					SMALL = strtod(argv[i+1], NULL);
					++i;
				}
				else{
					cout << "Invalid use of '-small': A floating point value has to be supplied." << endl;
					return 1;
				}
			}

			else if(strcmp(argv[i], "-consistent") == 0){
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
			SaveAlgebraicVector(av, file[1]);
		}
		else if(command.find("dif") == 0){
			CHECK(numFiles == 3, "Two in-files and an out-file have to be specified");
			AlgebraicVector av1, av2;
			LoadVector(av1, file[0], makeCons, component);
			LoadVector(av2, file[1], makeCons, component);
			av1.subtract_vector(av2);
			SaveAlgebraicVector(av1, file[2]);
			PrintMinMax(av1);
		}
		else if(command.find("minmax") == 0){
			CHECK(numFiles == 1, "An in-file has to be specified.");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, component);
			PrintMinMax(av);
		}
		else if(command.find("histogram") == 0){
			CHECK(numFiles == 2, "An in-file and an out-file have to be specified");
			AlgebraicVector av;
			LoadVector(av, file[0], makeCons, component);
			SaveHistogramToUGX(av, file[1], histoSecs, histoAbs, histoLog);
		}
		else{
			cout << "vecutil - (c) 2013-2017 Sebastian Reiter, G-CSC Frankfurt" << endl;
			cout << endl;
			cout << "USAGE: vecutil command [options] [files]" << endl;
			cout << "OR:    vecutil command [files] [options]" << endl << endl;

			cout << "SAMPLE: vecutil dif -consistent vec1.vec vec2.pvec dif.vec" << endl << endl;

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


			cout << "OPTIONS:" << endl;
			cout << "  -small:           The maximal distance that two coordinates may have to be considered equal." << endl;
			cout << "                    Default is " << DEFAULT_SMALL << endl << endl;

			cout << "  -consistent:      If this parameter is specified, parallel vectors are assumed" << endl;
			cout << "                    to be in consistent storage mode. Otherwise they are assumed" << endl;
			cout << "                    to be in additive storage mode." << endl << endl;

			cout << "  -component n:     The number n specifies the component index (0 <= n < #comp) on which to work." << endl;
			cout << "                    All other components will be dismissed." << endl << endl;

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


