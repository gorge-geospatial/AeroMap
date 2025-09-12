#ifndef TEXTFILE_H
#define TEXTFILE_H

#include <vector>
#include <xstring>

class TextFile
{
public:
	TextFile(const char* fileName, bool trimLines = false);
	~TextFile();

	std::string GetText(const char* lineEnding = nullptr);

	int GetLineCount() { return (int)mv_Lines.size(); }
	std::string GetLine(int lineIndex) { return mv_Lines[lineIndex]; }

private:
	int LoadFile(const char* fileName);
	std::string ReadLine(FILE* pInFile);
	std::string TrimString(std::string str);

private:
	std::vector<std::string> mv_Lines;		// ordered table of text file lines
	bool mb_TrimLines;						// trim white space from begin/end of lines
};

#endif // #ifndef TEXTFILE_H
