// TextFile.cpp
// Encapsulates reading/writing of simple text files
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//
// Formerly based on XString class, but that has memory 
// allocation bugs when used in stl vectors.
//

#include <stdio.h>			// fopen(), etc.

#include "TextFile.h"

TextFile::TextFile(const char* fileName, bool trimLines)
	: mb_TrimLines(trimLines)
{
	LoadFile(fileName);
}

TextFile::~TextFile()
{
	// Deleting the vector will call the destructor of
	// each object (not so for a vector of pointers).

	mv_Lines.clear();
}

std::string TextFile::GetText(const char* lineEnding)
{
	// return entire file as string
	//
	// inputs:
	//		lineEnding = chars to use as line ending
	//					 default == '\n'
	//

	std::string str;

	unsigned int lineCount = GetLineCount();
	for (unsigned int i = 0; i < lineCount; ++i)
	{
		str += GetLine(i);
		if (lineEnding)
			str += lineEnding;
		else
			str += '\n';
	}

	return str;
}

int TextFile::LoadFile(const char* fileName)
{
	mv_Lines.clear();

	FILE* pInFile = fopen(fileName, "rt");
	if (pInFile)
	{
		while (!feof(pInFile))
		{
			std::string line = ReadLine(pInFile);
			if (mb_TrimLines)
				line = TrimString(line);
			if (line.length() > 0)
				mv_Lines.push_back(line);
		}
		fclose(pInFile);
	}

	return (int)mv_Lines.size();
}

std::string TextFile::TrimString(std::string str)
{
	// Remove all leading / trailing white space.

	// characters to trim (\n & \r have already been
	// removed on load)
	char mask[] = " \t";

	std::string strTrim = str;
	size_t pos = str.find_first_not_of(mask);
	if (pos > 0 && pos != std::string::npos)
		strTrim = strTrim.substr(pos);
	pos = strTrim.find_last_not_of(mask);
	if (pos < (int)(strTrim.length() - 1) && pos != std::string::npos)
		strTrim = strTrim.substr(0, pos + 1);

	return strTrim;
}

std::string TextFile::ReadLine(FILE* pInFile)
{
	// 	inputs:
	//		pInputFile = file handle
	// 	outputs:
	//		return = text line

	std::string line;		// return value

	char ch = fgetc(pInFile);

	// spin off newlines
	while (!feof(pInFile) && (ch == '\n' || ch == '\r'))
	{
		ch = fgetc(pInFile);
	}

	// read text line
	while (!feof(pInFile) && ch != '\n' && ch != '\r')
	{
		line += ch;
		ch = fgetc(pInFile);
	}

	return line;
}
