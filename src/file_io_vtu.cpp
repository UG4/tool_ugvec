// This file is part of ugvec, a program for analysing and comparing vectors
//
// Copyright (C) 2016,2017 Sebastian Reiter, G-CSC Frankfurt <sreiter@gcsc.uni-frankfurt.de>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdint.h>
#include <string>

#include "algebraic_vector.h"
#include "base64.h"
#include "file_io.h"
#include "vec_tools.h"
#include "rapidxml.hpp"

using ::int8_t;
using ::int16_t;
using ::int32_t;
using ::int64_t;
using ::int_fast8_t;
using ::int_fast16_t;
using ::int_fast32_t;
using ::int_fast64_t;
using ::int_least8_t;
using ::int_least16_t;
using ::int_least32_t;
using ::int_least64_t;
using ::intmax_t;
using ::uint8_t;
using ::uint16_t;
using ::uint32_t;
using ::uint64_t;
using ::uint_fast8_t;
using ::uint_fast16_t;
using ::uint_fast32_t;
using ::uint_fast64_t;
using ::uint_least8_t;
using ::uint_least16_t;
using ::uint_least32_t;
using ::uint_least64_t;
using ::uintmax_t;

using namespace std;
using namespace rapidxml;


static
int BigEndianSystem()
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1; 
}

void ByteSwap (char* p, int len)
{
	for(int i = 0; i + 1 < len; ++i){
		const char t = p[i];
		const int i2 = len - i;
		p[i] = p[i2];
		p[i2] = t;
	}
}

void SwapArrayEndianess (char* p, size_t size, int typeLen)
{
	for(size_t i = 0; i < size; i+= typeLen){
		ByteSwap (p + i, typeLen);
	}
}

///	Returns the value of the first attribute with the given name.
/** If no such attribute exists, the method throws.
 * The method compares lower-case versions of involved names.*/
static const char*
GetAttribVal (xml_node<>* node, const char* attribName)
{
	xml_attribute<>* a = node->first_attribute(attribName, 0, false);
	CHECK (a, "Required attribute not found: " << attribName);
	return a->value();
}


///	Returns the value of the first attribute with the given name.
/** If no such attribute exists, the method returns the default value.
 * The method compares lower-case versions of involved names.*/
static const char*
GetAttribVal (xml_node<>* node, const char* attribName, const char* defValue)
{
	xml_attribute<>* a = node->first_attribute(attribName, 0, false);
	if(!a)
		return defValue;
	return a->value();
}


template <class T>
static void
ReadDataArrayBINARY(std::vector<T>& dataOut,
					rapidxml::xml_node<>* dataNode,
					bool bigEndian,
					bool clearData)
{
	using namespace std;

	if(clearData)
		dataOut.clear();

//	create a buffer with which we can access the data
	string str = base64_decode((const unsigned char*)dataNode->value(), strlen(dataNode->value()));
	CHECK (str.size() % sizeof(T) == 0, "Bad base64 decoding");

	if(str.empty()){
		return;
	}

	if(BigEndianSystem() != bigEndian){
		SwapArrayEndianess(&str[0], str.size(), sizeof(T));
	}

	size_t curSize = dataOut.size();
	dataOut.resize(curSize + str.size() / sizeof(T));
	memcpy((void*)(&dataOut.front() + curSize), (void*)(&str[0]), str.size());
}

template <class T>
static void
ReadDataArrayASCII(std::vector<T>& dataOut,
				   rapidxml::xml_node<>* dataNode,
				   bool clearData)
{
	using namespace std;

	if(clearData)
		dataOut.clear();

//	create a buffer with which we can access the data
	string str(dataNode->value(), dataNode->value_size());
	stringstream ss(str, ios_base::in);

	while(!ss.eof()){
	//	read the data
		T d;
		ss >> d;

	//	make sure that everything went right
		if(ss.fail())
			break;

		dataOut.push_back(d);
	}
}

template <class T>
static void
ReadDataArray(std::vector<T>& dataOut,
			  rapidxml::xml_node<>* dataNode,
			  bool bigEndian,
			  const char* typeStr,
			  bool clearData)
{
	CHECK (strcmp(typeStr, GetAttribVal(dataNode, "type")) == 0,
	       "Wrong data format! Expected: " << typeStr << ", found: "
	       << GetAttribVal(dataNode, "type"));

	const char* format = GetAttribVal(dataNode, "format");

	if (strcmp(format, "ascii") == 0)
		ReadDataArrayASCII<T> (dataOut, dataNode, clearData);
	else if (strcmp(format, "binary") == 0)
		ReadDataArrayBINARY<T> (dataOut, dataNode, bigEndian, clearData);
	else {
		CHECK (0, "Bad format in DataArray: " << format);
	}
}

bool Load_VTU (AlgebraicVector& av, const char* filename)
{
//	read the file into an xml-tree
	ifstream in(filename, ios::binary);
	if(!in)
		return false;

//	get the length of the file
	streampos posStart = in.tellg();
	in.seekg(0, ios_base::end);
	streampos posEnd = in.tellg();
	streamsize size = posEnd - posStart;

//	go back to the start of the file
	in.seekg(posStart);

	rapidxml::xml_document<>	doc;

//	read the whole file en-block and terminate it with 0
	char* fileContent = doc.allocate_string(0, size + 1);
	in.read(fileContent, size);
	fileContent[size] = 0;
	in.close();

//	parse the xml-data
	doc.parse<0>(fileContent);


//	find the nodes of the first piece of the first unstructured grid
	xml_node<>* vtkNode = doc.first_node("VTKFile");
	CHECK(vtkNode, "Specified file is not a valid VTKFile!");

	xml_node<>* ugridNode = vtkNode->first_node("UnstructuredGrid");
	CHECK(ugridNode, "Specified file does not contain an unstructured grid!");

	xml_node<>* pieceNode = ugridNode->first_node("Piece");
	CHECK(pieceNode, "Specified grid does not contain a Piece node!");

	xml_node<>* pointsNode = pieceNode->first_node("Points");
	CHECK(pointsNode, "Specified piece does not contain a Points node!");

	xml_node<>* pointsDataArrayNode = pointsNode->first_node("DataArray");
	CHECK(pointsDataArrayNode, "Specified points node does not contain a DataArray node!");

	const bool bigEndian = 
			(strcmp(GetAttribVal(vtkNode, "byte_order"), "BigEndian") == 0);


	vector<float> data; // warning: reused several times during parsing!

//	read position data
	ReadDataArray<float>(data, pointsDataArrayNode, bigEndian, "Float32", true);

	const int pointDimTmp = atoi(GetAttribVal(pointsDataArrayNode, "NumberOfComponents"));
	CHECK(pointDimTmp > 0, "Bad coordinate dimension specified: " << pointDimTmp);

	const size_t pointDim = (size_t) pointDimTmp;
	const size_t numPoints = data.size() / pointDim;

	av.worldDim = pointDim;

	{
		const size_t numComps = pointDim;

		av.positions.clear();
		av.positions.resize(numPoints);

		for(size_t ipoint = 0; ipoint < numPoints; ++ipoint){
			const size_t dataInd = ipoint * pointDim;
			for(size_t ic = 0; ic < numComps; ++ic){
				av.positions[ipoint].coord[ic] = data[dataInd + ic];
			}
		}
	}


//	read data values
	xml_node<>* pointDataNode = pieceNode->first_node("PointData");
	CHECK(pointDataNode, "Specified piece does not contain a PointData node!");

	xml_node<>* curDataNode = pointDataNode->first_node("DataArray");
	CHECK(curDataNode, "At least one DataArray node in the PointData node is expected.");

	int compCounter = 0;

	cout << "  Components of '" << filename << "':" << endl;

	while (curDataNode) {

		if(strcmp(GetAttribVal(curDataNode, "type"), "Float32") != 0){
			cout << "   VTU WARNING: ignoring component '"
				 << GetAttribVal(curDataNode, "name", "unknown")
				 << "' due to unsupported type. Float32 expected.\n";
			continue;
		}

		const int numComps = atoi(GetAttribVal(curDataNode, "NumberOfComponents"));
		CHECK(numComps > 0, "Bad number of components in point data array");

		ReadDataArray<float>(data, curDataNode, bigEndian, "Float32", true);
		av.data.reserve(av.data.size() + data.size());

		for(int ic = 0; ic < numComps; ++ic){
			if(compCounter > 0) {
			//	copy position data for this component
				const size_t firstNew = av.positions.size();
				av.positions.resize(av.positions.size() + numPoints);
				for(size_t ip = 0; ip < numPoints; ++ip){
					av.positions[firstNew + ip] = av.positions[ip];
					av.positions[firstNew + ip].ci = compCounter;
				}
			}

			cout << "    " << compCounter << ":\t"
				 << GetAttribVal(curDataNode, "name", "unknown");
			if(numComps > 1){
				cout << " [" << ic << "]";
			}
			cout << endl;

			for(size_t i = 0; i < data.size(); i += numComps){
				av.data.push_back(data[i]);
			}

			++compCounter;
		}

		curDataNode = curDataNode->next_sibling("DataArray");
	}

	return true;
}


bool Load_PVTU (AlgebraicVector& av, const char* filename, bool makeConsistent)
{

//	extract the path from filename
	string strFilename = filename;
	string path;
	size_t lastSlash = strFilename.find_last_of("/");
	if(lastSlash == string::npos)
		lastSlash = strFilename.find_last_of("\\");
		
	if(lastSlash != string::npos)
		path = strFilename.substr(0, lastSlash + 1);

//	read the file into an xml-tree
	ifstream in(filename, ios::binary);
	if(!in)
		return false;

//	get the length of the file
	streampos posStart = in.tellg();
	in.seekg(0, ios_base::end);
	streampos posEnd = in.tellg();
	streamsize size = posEnd - posStart;

//	go back to the start of the file
	in.seekg(posStart);

	rapidxml::xml_document<>	doc;

//	read the whole file en-block and terminate it with 0
	char* fileContent = doc.allocate_string(0, size + 1);
	in.read(fileContent, size);
	fileContent[size] = 0;
	in.close();

//	parse the xml-data
	doc.parse<0>(fileContent);


//	iterate over the piece nodes unstructured grid node
	xml_node<>* vtkNode = doc.first_node("VTKFile");
	CHECK(vtkNode, "Specified file is not a valid VTKFile!");

	xml_node<>* ugridNode = vtkNode->first_node("PUnstructuredGrid");
	CHECK(ugridNode, "Specified file does not contain an PUnstructuredGrid node!");

	xml_node<>* curPieceNode = ugridNode->first_node("Piece");
	CHECK(curPieceNode, "At least one Piece node is expected.");

	bool useGlobPosMap = false;
	#ifdef PARALLEL_LOAD_SPEEDUP
		useGlobPosMap = (numFiles > 1);
	#endif

	std::map<Position, size_t> globPosMap;

	while (curPieceNode) {
        string serialFile = GetAttribVal(curPieceNode, "Source");
        string tfilename = path + serialFile;
        AlgebraicVector tmpAv;
        if(Load_VTU(tmpAv, tfilename.c_str())){
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

        curPieceNode = curPieceNode->next_sibling("Piece");
    }

	return true;
}