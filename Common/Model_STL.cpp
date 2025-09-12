// Model_STL.cpp
// Manager for STL polygonal model files.
//
// File format:
// 

// based on:
//https://github.com/sreiter/stl_reader/blob/master/stl_reader.h

#include <stdio.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <sstream>
#include <vector>

#ifdef STL_READER_NO_EXCEPTIONS
#define STL_READER_THROW(msg) return false;
#define STL_READER_COND_THROW(cond, msg) if(cond) return false;
#else
/// Throws an std::runtime_error with the given message.
#define STL_READER_THROW(msg) {std::stringstream ss; ss << msg; throw(std::runtime_error(ss.str()));}

/// Throws an std::runtime_error with the given message, if the given condition evaluates to true.
#define STL_READER_COND_THROW(cond, msg)  if(cond){std::stringstream ss; ss << msg; throw(std::runtime_error(ss.str()));}
#endif

#include "TextFile.h"
#include "Logger.h"
#include "Model_STL.h"

Model_STL::Model_STL()
	: Model()
{
}

Model_STL::~Model_STL()
{
}

bool Model_STL::Load(const char* fileName)
{
	// Load WaveFront OBJ format file.
	//

	FILE* pFile = fopen(fileName, "rt");
	if (pFile == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open file '%s'.", fileName);
		return false;
	}
	fclose(pFile);

	if (StlFileHasASCIIFormat(fileName))
	{
		ReadStlFile_ASCII(fileName);
	}
	else
	{
		//ReadStlFile_Binary(fileName);
	}

	ms_FileName = fileName;

	return true;
}

/// Reads an ASCII or binary stl file into several arrays
/** Reads a stl file and writes its coordinates, normals and triangle-corner-indices
* to the provided containers. It also fills a container solidRangesOut, which
* provides the triangle ranges for individual solids.
*
* Double vertex entries are removed on the fly, so that triangle corners with
* equal coordinates are represented by a single coordinate entry in coordsOut.
*
*
* \param filename  [in] The name of the file which shall be read
*
*
*/
//template <class TNumberContainer1, class TNumberContainer2,
	//class TIndexContainer1, class TIndexContainer2>

//TODO:
//don't actually expect to return these as parms - would like to load base structures directly here
bool Model_STL::ReadStlFile_ASCII(const char* fileName)
//	TNumberContainer1& coordsOut,
//	TNumberContainer2& normalsOut,
//	TIndexContainer1& trisOut,
//	TIndexContainer2& solidRangesOut)
{
	using namespace std;
	//using namespace stl_reader_impl;

	//typedef typename TNumberContainer1::value_type  number_t;
	//typedef typename TIndexContainer1::value_type index_t;

	// Face normals are written to this container.On termination,
	// it has size numFaces * 3. Each triple of entries forms a
	//  3d normal.The type TNumberContainer should have the same
	//                           interface as std::vector<float>.
	std::vector<VEC3> normalsOut;
	//normalsOut.clear();

	// Coordinates are written to this container.On termination,
	// it has size numVertices * 3. Each triple of entries forms a
	//  3d coordinate.The type TNumberContainer should have the same
	//                         interface as std::vector<float>.
	std::vector<VEC3> coordsOut;
	//coordsOut.clear();

	// Triangle corner indices are written to this container.
	// On termination, it has size numFaces * 3. Each triple of
	// entries defines a triangle. The type TIndexContainer should
	// have the same interface as std::vector<size_t>.
	// Multiply corner indices from trisOut by 3 to obtain the index
	// of the first coordinate of that corner in coordsOut.
	std::vector<int> trisOut;
	//trisOut.clear();

	// On termination, it holds the ranges of triangle indices
	// for each solid.It has the size numSolids + 1. Each entry
	// can be interpreted as a end / begin triangle index for the
	// previous / next solid.E.g., if there are 3 solids, the
	// returned array would look like this:
	// 
	//		{ sol1Begin, sol1End / sol2Begin, sol2End / sol3Begin, sol3End }.
	// 
	// The type TIndexContainer should have the same interface
	// as std::vector<size_t>.

	std::vector<int> solidRangesOut;
	solidRangesOut.clear();

	vector<CoordWithIndex> coordsWithIndex;

	//string buffer;
	//vector<string> tokens;
	//int maxNumTokens = 0;
	size_t numFaceVrts = 0;

	TextFile textFile(fileName, true);

	for (int i = 0; i < textFile.GetLineCount(); ++i)
	{
		XString line = textFile.GetLine(i).c_str();

		int tokenCount = line.Tokenize(" \t");
		if (tokenCount > 0)
		{
			XString& tok = line.GetToken(0);
			if (tok.CompareNoCase("vertex"))
			{
				if (tokenCount < 4)
				{
					Logger::Write(__FUNCTION__, "Error reading %s: invalid format at line %d", fileName, i);
					assert(false);
				}
				else
				{
					//  read the position
					CoordWithIndex c;
					for (int i = 0; i < 3; ++i)
						c[i] = line.GetToken(i + 1).GetDouble();
					c.index = static_cast<int>(coordsWithIndex.size());
					coordsWithIndex.push_back(c);
					++numFaceVrts;
				}
			}
			else if (tok.CompareNoCase("facet"))
			{
				if (tokenCount < 5)
				{
					Logger::Write(__FUNCTION__, "Error reading %s: invalid format at line %d", fileName, i);
					assert(false);
				}
				else
				{
					if (line.GetToken(1).CompareNoCase("normal") == false)
					{
						Logger::Write(__FUNCTION__, "Error reading %s: missing normal specifier at line %d", fileName, i);
						assert(false);
					}

					//  read the normal
					VEC3 normal;
					normal.x = line.GetToken(2).GetDouble();
					normal.y = line.GetToken(3).GetDouble();
					normal.z = line.GetToken(4).GetDouble();
					//for (size_t i = 0; i < 3; ++i)
						//normalsOut.push_back(static_cast<number_t> (atof(tokens[i + 2].c_str())));
					normalsOut.push_back(normal);
					numFaceVrts = 0;
				}
			}
			else if (tok.CompareNoCase("outer"))
			{
				if ((tokenCount < 2) || (line.GetToken(1).CompareNoCase("loop") == false))
				{
					Logger::Write(__FUNCTION__, "Error reading %s: expected outer loop at line %d", fileName, i);
					assert(false);
				}
			}
			else if (tok.CompareNoCase("endfacet"))
			{
				if (numFaceVrts != 3)
				{
					Logger::Write(__FUNCTION__, "Error reading %s: invalid number of vertices at line %d", fileName, i);
					assert(false);
				}
				else
				{
					trisOut.push_back(static_cast<int> (coordsWithIndex.size() - 3));
					trisOut.push_back(static_cast<int> (coordsWithIndex.size() - 2));
					trisOut.push_back(static_cast<int> (coordsWithIndex.size() - 1));
				}
			}
			else if (tok.CompareNoCase("solid"))
			{
				solidRangesOut.push_back(static_cast<int> (trisOut.size() / 3));
			}
		}
	}

	solidRangesOut.push_back(static_cast<int> (trisOut.size() / 3));

	RemoveDoubles(coordsOut, trisOut, coordsWithIndex);

	return true;
}

bool Model_STL::ReadStlFile_BINARY(const char* fileName)
	//TNumberContainer1& coordsOut,
	//TNumberContainer2& normalsOut,
	//TIndexContainer1& trisOut,
	//TIndexContainer2& solidRangesOut)
{
	(void*)(fileName);

	//using namespace std;
	//using namespace stl_reader_impl;

	//typedef typename TNumberContainer1::value_type  number_t;
	//typedef typename TIndexContainer1::value_type index_t;

	//coordsOut.clear();
	//normalsOut.clear();
	//trisOut.clear();
	//solidRangesOut.clear();

	//ifstream in(filename, ios::binary);
	//STL_READER_COND_THROW(!in, "Couldnt open file " << filename);

	//char stl_header[80];
	//in.read(stl_header, 80);
	//STL_READER_COND_THROW(!in, "Error while parsing binary stl header in file " << filename);

	//unsigned int numTris = 0;
	//in.read((char*)&numTris, 4);
	//STL_READER_COND_THROW(!in, "Couldnt determine number of triangles in binary stl file " << filename);

	//vector<CoordWithIndex <number_t, index_t> > coordsWithIndex;

	//for (unsigned int tri = 0; tri < numTris; ++tri) {
	//	float d[12];
	//	in.read((char*)d, 12 * 4);
	//	STL_READER_COND_THROW(!in, "Error while parsing trianlge in binary stl file " << filename);

	//	for (int i = 0; i < 3; ++i)
	//		normalsOut.push_back(d[i]);

	//	for (size_t ivrt = 1; ivrt < 4; ++ivrt) {
	//		CoordWithIndex <number_t, index_t> c;
	//		for (size_t i = 0; i < 3; ++i)
	//			c[i] = d[ivrt * 3 + i];
	//		c.index = static_cast<index_t>(coordsWithIndex.size());
	//		coordsWithIndex.push_back(c);
	//	}

	//	trisOut.push_back(static_cast<index_t> (coordsWithIndex.size() - 3));
	//	trisOut.push_back(static_cast<index_t> (coordsWithIndex.size() - 2));
	//	trisOut.push_back(static_cast<index_t> (coordsWithIndex.size() - 1));

	//	char addData[2];
	//	in.read(addData, 2);
	//	STL_READER_COND_THROW(!in, "Error while parsing additional triangle data in binary stl file " << filename);
	//}

	//solidRangesOut.push_back(0);
	//solidRangesOut.push_back(static_cast<index_t> (trisOut.size() / 3));

	//RemoveDoubles(coordsOut, trisOut, coordsWithIndex);

	return true;
}

bool Model_STL::StlFileHasASCIIFormat(const char* filename)
{
	using namespace std;
	ifstream in(filename);
	STL_READER_COND_THROW(!in, "Couldnt open file " << filename);

	char chars[256] = { 0 };
	in.read(chars, 256);
	string buffer(chars, in.gcount());
	transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);

	bool isAscii = buffer.find("solid") != string::npos &&
		buffer.find("\n") != string::npos &&
		buffer.find("facet") != string::npos &&
		buffer.find("normal") != string::npos;

	return isAscii;
}

// sorts the array coordsWithIndexInOut and copies unique indices to coordsOut.
// Triangle-corners are re-indexed on the fly and degenerated triangles are removed.
//template <class TNumberContainer, class TIndexContainer>
void Model_STL::RemoveDoubles(
	std::vector<VEC3>& uniqueCoordsOut,
	std::vector<int>& trisInOut,
	std::vector <CoordWithIndex> &coordsWithIndexInOut)
{
	//typedef typename TNumberContainer::value_type number_t;
	//typedef typename TIndexContainer::value_type  index_t;

	std::sort(coordsWithIndexInOut.begin(), coordsWithIndexInOut.end());

	//  first count unique indices
	int numUnique = 1;
	for (size_t i = 1; i < coordsWithIndexInOut.size(); ++i)
	{
		if (coordsWithIndexInOut[i] != coordsWithIndexInOut[i - 1])
			++numUnique;
	}

	uniqueCoordsOut.resize(numUnique * 3);
	std::vector<int> newIndex(coordsWithIndexInOut.size());

	// copy unique coordinates to 'uniqueCoordsOut' and create an index-map
	// 'newIndex', which allows to re-index triangles later on.
	//int curInd = 0;
	newIndex[0] = 0;
	//for (int i = 0; i < 3; ++i)
		//uniqueCoordsOut[i] = coordsWithIndexInOut[0][i];

	for (size_t i = 1; i < coordsWithIndexInOut.size(); ++i)
	{
//TODO:
//was here...
	//	const CoordWithIndex <number_t, index_t> c = coordsWithIndexInOut[i];
	//	if (c != coordsWithIndexInOut[i - 1]) {
	//		++curInd;
	//		for (index_t j = 0; j < 3; ++j)
	//			uniqueCoordsOut[curInd * 3 + j] = coordsWithIndexInOut[i][j];
	//	}

	//	newIndex[c.index] = static_cast<index_t> (curInd);
	}

	// re-index triangles, so that they refer to 'uniqueCoordsOut'
	// make sure to only add triangles which refer to three different indices
	int numUniqueTriInds = 0;
	for (int i = 0; i < trisInOut.size(); i += 3)
	{
	//	int ni[3];
	//	for (int j = 0; j < 3; ++j)
	//		ni[j] = newIndex[trisInOut[i + j]];

	//	if ((ni[0] != ni[1]) && (ni[0] != ni[2]) && (ni[1] != ni[2])) {
	//		for (int j = 0; j < 3; ++j)
	//			trisInOut[numUniqueTriInds + j] = ni[j];
	//		numUniqueTriInds += 3;
	//	}
	}

	if (numUniqueTriInds < trisInOut.size())
		trisInOut.resize(numUniqueTriInds);
}
