// XString.h
// String class that has no mfc, atl or stl dependencies.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#ifndef XSTRING_H
#define XSTRING_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>		// toupper()

#ifdef QT_CORE_LIB
#include "qstring.h"
#endif

//----------------------------------------------------------------------------
// interface

class XString
{
private:
	char*	mp_buffer;			// points to string buffer
	int		mi_BufLen;			// size of buffer, exact
	int		mi_StrLen;			// size of string, excluding terminator, < mi_BufLen

	struct TokenMap
	{
		int Offset;
		int Length;
	};

	TokenMap* mp_TokenMap;		// token map
	int mi_TokenMapSize;		// size of buffer used for token map
	int mi_TokenMapCount;		// # of actual entries in map

public:

	// construction / destruction

	XString();
	XString(const XString& str);
	XString(const char* str);
	XString(char ch);
	XString(char ch, unsigned int count);
	XString(unsigned long int num);
#ifdef QT_CORE_LIB
	XString(const QString& str);
#endif
	virtual ~XString();

	// methods

	void Clear();									// reset string to empty

	void Delete(int pos, int count = 1);			// delete n characters beginning at pos
	void DeleteLeft(int count = 1);					// delete leftmost n characters
	void DeleteRight(int count = 1);				// delete rightmost n characters
	void Insert(int pos, char ch);					// insert character
	void Insert(int pos, const char* str);			// insert string

	void TrimLeft(char ch = ' ');
	void TrimLeft(const char* str);					// trim all chars in str from beginning of string
	void TrimRight(char ch = ' ');
	void TrimRight(const char* str);				// trim all chars in str from end of string
	void Trim(char ch = ' ') { TrimLeft(ch); TrimRight(ch); }
	void Trim(const char* str) { TrimLeft(str); TrimRight(str); }

	void PadLeft(char ch, unsigned int count);		// pad left of string with count chars
	void PadRight(char ch, unsigned int count);		// pad right of string with count chars

	void Swap(int pos1, int pos2);					// exchange characters at pos1 & pos2
	void TruncateAt(char ch);						// truncate string from first occurrence of ch to end
	void TruncateAtAny(const char* str);			// truncate string at first occurrence of any character in str

	int  Find(char ch, int start = 0);						// find first occurrence of ch
	int  Find(const char* str, int start = 0);
	int  ReverseFind(char ch);								// find first occurrence of ch, starting at end of string and searching backward
	int  FindOneOf(const char* str);						// find first occurrence of any char in str
	int  Replace(char ch1, char ch2);						// replace all occurrences of ch1 with ch2
	int  Replace(const char* str1, const char* str2);		// replace all occurrences of str1 with str2

	bool Compare(const char* str);				// case-sensitive comparison, returns TRUE if equal
	bool CompareNoCase(const char* str);		// case-insensitive comparison, returns TRUE if equal
	bool CompareNoCase(XString str);

	bool BeginsWith(const char* str) const;
	bool EndsWith(const char* str) const;
	bool BeginsWithNoCase(const char* str) const;
	bool EndsWithNoCase(const char* str) const;

	void MakeUpper();							// convert entire string to upper case
	void MakeLower();							// convert entire string to lower case

	XString Left(int count);					// return leftmost n characters
	XString Right(int count);					// return rightmost n characters
	XString Mid(int index, int count = 0);		// return substring beginning at index
	XString Extract(int begin, int end);		// return substring from char 'begin' to 'end', inclusive

	static XString Format(const char* sFormat, ...);

	int Tokenize(const char* del);
	XString GetToken(int index);
	int GetTokenCount() { return mi_TokenMapCount; }

	char GetAt(int pos);
	void SetAt(int pos, char ch);
	int GetLength() const { return mi_StrLen; }
	const char* c_str() { return mp_buffer; }		// return a const ptr to a c string

	bool IsEmpty() const { return mi_StrLen == 0; }

	int GetCount(const char ch);
	int GetCount(const char* str);

	// interpreting string as path, return path or file name respectively
	XString GetPathName();
	XString GetFileName();

	void FormatPathName();      // interpreting as path, ensure ends with valid path delimiter
	void NormalizePath();		// interpreting as path, ensure all delimiters consistent
	static XString CombinePath(const char* path1, const char* path2);
	static XString CombinePath(const XString& path1, const XString& path2);
	static XString CombinePath(const XString& path1, const char* path2);

	// numeric conversions

	double	GetDouble() { return atof(mp_buffer); }
	int		GetInt() { return atoi(mp_buffer); }
	long	GetLong() { return atol(mp_buffer); }

	// operators

	// treating "const char*" as operator allows XString to be used anywhere a "char*" is used
	typedef const char* CSTR;
	operator CSTR() const { return mp_buffer; }

	void	operator =  (const char* s);		// assignment
	void	operator =  (XString s);
	XString operator +  (XString s);			// concatenation
	void	operator += (char ch);
	void	operator += (const char* s);
	void	operator += (XString s);
	bool	operator == (const char* s);		// equality
	bool	operator == (XString& s);
	bool	operator != (const char* s);		// ne
	bool	operator != (XString& s);
	bool	operator >  (const char* s);		// gt
	bool	operator >  (XString& s);
	bool	operator >= (const char* s);		// gte
	bool	operator >= (XString& s);
	bool	operator <  (const char* s);		// lt
	bool	operator <  (XString& s);
	bool	operator <= (const char* s);		// lte
	bool	operator <= (XString& s);
	char	operator[](int pos) const;			// get character at pos

private:
	void grow_token_map();					// increase size of token map
	void initialize();
	void resize(int NewSize);				// resize string buffer
	void shift_left(int pos, int count);	// shift string left count bytes
	void shift_right(int pos, int count);	// shift string right count bytes
	char to_lower(char ch);
	char to_upper(char ch);
};

inline XString::XString()
	: mp_buffer(nullptr)		// points to string buffer
	, mi_BufLen(0)				// size of buffer, exact
	, mi_StrLen(0)				// size of string, excluding terminator, < mi_BufLen
	, mp_TokenMap(nullptr)		// token map
	, mi_TokenMapSize(0)		// size of buffer used for token map
	, mi_TokenMapCount(0) 		// # of actual entries in map
{
	// default constructor
	//

	initialize();
}

inline XString::XString(const XString& str)
{
	// constructor that accepts XString (uses reference so we don't get in infinite
	// loop with copy constructor creating one on the stack, calling copy constructor, etc.)
	//

	initialize();

	// perform deep copy

	resize(str.mi_BufLen);								// safely resize buffer
	strncpy(mp_buffer, str.mp_buffer, mi_BufLen);		// copy src buffer to dest buffer
	mi_StrLen = str.mi_StrLen;							// update string length
	mp_buffer[mi_StrLen] = 0;							// terminate buffer

	// if there's an active token map, copy that too
	if (mi_TokenMapSize > 0)
	{
		if (mp_TokenMap)
			delete mp_TokenMap;
		mp_TokenMap = new TokenMap[mi_TokenMapSize];
		memcpy(mp_TokenMap, str.mp_TokenMap, mi_TokenMapCount*sizeof(TokenMap));
	}
}

inline XString::XString(const char* str)
{
	// constructor that accepts (const char*)
	//

	initialize();

	if (str)		// valid pointer
	{
		int size = (int)strlen(str);
		resize(size + 1);					// add 1 for null terminator
		strncpy(mp_buffer, str, size);
		mi_StrLen = size;
		mp_buffer[mi_StrLen] = 0;			// terminate buffer
	}
}

inline XString::XString(char ch)
{
	// constructor that accepts char
	//

	initialize();

	mp_buffer[0] = ch;
	mp_buffer[1] = 0;			// terminate buffer
	mi_StrLen = 1;
}

inline XString::XString(char ch, unsigned int count)
{
	// constructor that creates string of count characters
	//

	initialize();
	resize(count + 1);

	for (unsigned int i = 0; i < count; ++i)
		mp_buffer[i] = ch;

	mi_StrLen = count;
	mp_buffer[mi_StrLen] = 0;			// terminate buffer
}

inline XString::XString(unsigned long int num)
{
	// constructor that accepts unsigned long int
	//

	// max ulong (on 64-bit) = 18446744073709551615, so the initial 32-byte buffer
	// will always be large enough
	initialize();
	mi_StrLen = sprintf(mp_buffer, "%lu", num);
}

#ifdef QT_CORE_LIB
inline XString::XString(const QString& str)
{
	// constructor that accepts QT string object
	//

	initialize();

	int size = (int)str.length();
	resize(size + 1);					// add 1 for null terminator
	for (int i = 0; i < size; ++i)
		mp_buffer[i] = str.at(i).toLatin1();
	mi_StrLen = size;
	mp_buffer[mi_StrLen] = 0;			// terminate buffer
}
#endif

inline XString::~XString()
{
	if (mp_TokenMap)
	{
		delete mp_TokenMap;
		mp_TokenMap = nullptr;
	}
	if (mp_buffer)
	{
		delete mp_buffer;
		mp_buffer = nullptr;
	}
}

inline int XString::Tokenize(const char* del)
{
	// Tokenize(","):	"Hello,there,,world" => "Hello"
	//											"there"
	//											"world"
	//

	if (!del)	// invalid pointer
		return 0;

	mi_TokenMapSize = 16;		// initial size of buffer used for token map
	mi_TokenMapCount = 0;		// # of actual entries in map
	if (mp_TokenMap)
		delete mp_TokenMap;
	mp_TokenMap = new TokenMap[mi_TokenMapSize];

	bool bPrevCharDelimiter = true;
	unsigned int nCurOffset = 0;

	for (int i = 0; i < mi_StrLen; ++i)
	{
		if (strchr(del, mp_buffer[i]))			// current char == delimiter
		{
			if (bPrevCharDelimiter == false)	// only interested in first non-delimiter
			{
				if (mi_TokenMapCount == mi_TokenMapSize)	// token map full, grow it
					grow_token_map();
				TokenMap tok;
				tok.Offset = nCurOffset;
				tok.Length = i - nCurOffset;
				mp_TokenMap[mi_TokenMapCount++] = tok;
			}
		}
		else	// current char != delimiter
		{
			if (bPrevCharDelimiter == true)	// found first char in token
				nCurOffset = i;			// update offset to current token
		}
		bPrevCharDelimiter = strchr(del, mp_buffer[i]) != nullptr;
	}

	if (!bPrevCharDelimiter)
	{
		// reached end of string and it is not a delimiter, add final token
		// to list
		if (mi_TokenMapCount == mi_TokenMapSize)	// token map full, grow it
			grow_token_map();
		TokenMap tok;
		tok.Offset = nCurOffset;
		tok.Length = mi_StrLen - nCurOffset;
		mp_TokenMap[mi_TokenMapCount++] = tok;
	}

	return mi_TokenMapCount;
}

inline XString XString::GetToken(int index)
{
	// return token from previously tokenized string
	//

	XString sReturn;	// allocate the object that will be returned

	if (index > -1 && index < mi_TokenMapCount)
	{
		int str_idx = mp_TokenMap[index].Offset;
		int str_cnt = mp_TokenMap[index].Length;
		sReturn = Mid(str_idx, str_cnt);
		sReturn.Trim();			// trim leading/trailing spaces by default, may want option in future
	}

	return sReturn;		// return new object
}

inline void XString::Clear()
{
	memset(mp_buffer, 0, mi_BufLen);		// clear buffer
	mi_StrLen = 0;							// reset string length
}

inline XString XString::Left(int count)
{
	XString sReturn;	// allocate the object that will be returned

	if (count > 0)
	{
		if (mi_StrLen < count)			// string is shorter than specified count
			sReturn = mp_buffer;		// return entire string
		else
		{
			sReturn = mp_buffer;			// assignment allocates mem & inits members properly
			sReturn.mp_buffer[count] = 0;	// truncate at specified count
			sReturn.mi_StrLen = count;
		}
	}

	return sReturn;		// return new object
}

inline XString XString::Right(int count)
{
	XString sReturn;	// allocate the object that will be returned

	if (count > 0)
	{
		if (mi_StrLen < count)		// string is shorter than specified count
			sReturn = mp_buffer;	// return entire string
		else
			sReturn = &mp_buffer[mi_StrLen - count];	// use assignment op so dest is properly resized
	}

	return sReturn;		// return new object
}

inline XString XString::Mid(int index, int count /* = 0*/)
{
	// return a substring

	// inputs:
	//		index = first character
	//		count = # of characters, return all characters to end of string if 0
	//

	XString sReturn;	// allocate the object that will be returned

	if (index < 0 || index > mi_StrLen)		// invalid index, return empty string
		return sReturn;

	int last = mi_StrLen;
	if (count > 0)
	{
		last = index + count;
		if (last > mi_StrLen)
			last = mi_StrLen;
	}
	last -= 1;

	for (int i = index; i <= last; ++i)
		sReturn += mp_buffer[i];

	return sReturn;					// return the object
}

inline XString XString::Extract(int begin, int end)
{
	// return substring from char 'begin' to 'end', inclusive
	return Mid(begin, end - begin + 1);
}

inline char XString::GetAt(int pos)
{
	if (pos > -1 && pos < mi_StrLen)
		return mp_buffer[pos];
	else
		return (char)0;
}

inline void XString::SetAt(int pos, char ch)
{
	if (pos > -1 && pos < mi_StrLen)
		mp_buffer[pos] = ch;
}

inline bool XString::Compare(const char* str)
{
	if (!str)
		return false;

	// return true if strings match

	size_t size = strlen(str);
	if (size != (size_t)mi_StrLen)		// they don't match if they are different lengths
		return false;

	for (unsigned int i = 0; i < size; i++)
	{
		if (mp_buffer[i] != str[i])
			return false;
	}

	return true;
}

inline bool XString::CompareNoCase(const char* str)
{
	if (!str)
		return false;

	// return TRUE if strings match

	size_t size = strlen(str);
	if (size != (size_t)mi_StrLen)		// they don't match if they are different lengths
		return false;

	for (unsigned int i = 0; i < size; ++i)
	{
		if (to_lower(mp_buffer[i]) != to_lower(str[i]))
			return false;
	}

	return true;
}

inline bool XString::CompareNoCase(XString str)
{
	return CompareNoCase(str.c_str());
}

inline bool XString::BeginsWith(const char* str) const
{
	int len = (int)strlen(str);
	if (len > mi_StrLen)		// argument too long, return false
		return false;

	for (int i = 0; i < len; ++i)
	{
		if (str[i] != mp_buffer[i])
			return false;
	}

	return true;
}

inline bool XString::BeginsWithNoCase(const char* str) const
{
	int len = (int)strlen(str);
	if (len > mi_StrLen)		// argument too long, return false
		return false;

	for (int i = 0; i < len; ++i)
	{
		if (toupper(str[i]) != toupper(mp_buffer[i]))
			return false;
	}

	return true;
}

inline bool XString::EndsWith(const char* str) const
{
	int len = (int)strlen(str);
	if (len > mi_StrLen)		// argument too long, return false
		return false;

	int i = mi_StrLen;
	while (len > 0)
	{
		if (str[--len] != mp_buffer[--i])
			return false;
	}

	return true;
}

inline bool XString::EndsWithNoCase(const char* str) const
{
	int len = (int)strlen(str);
	if (len > mi_StrLen)		// argument too long, return false
		return false;

	int i = mi_StrLen;
	while (len > 0)
	{
		if (toupper(str[--len]) != toupper(mp_buffer[--i]))
			return false;
	}

	return true;
}

inline void XString::MakeUpper()
{
	// convert XString to upper case
	//

	for (int i = 0; i < mi_StrLen; ++i)
		mp_buffer[i] = to_upper(mp_buffer[i]);
}

inline void XString::MakeLower()
{
	// convert XString to lower case
	//

	for (int i = 0; i < mi_StrLen; ++i)
		mp_buffer[i] = to_lower(mp_buffer[i]);
}

inline int XString::Find(char ch, int start /* = 0 */)
{
	// Find the first occurrence of a single character
	//
	// Inputs:
	//		ch = character to find
	//
	// Outputs:
	//		return	= 0-based index of character if found
	//				= -1 if not found
	//

	int nReturn = -1;

	char* p = strchr(&mp_buffer[start], ch);
	if (p)
		nReturn = (int)(p - mp_buffer);

	return nReturn;
}

inline int XString::Find(const char* str, int start /* = 0 */)
{
	// find substring
	//
	// inputs:
	//		str		= string to find
	//		start	= starting search position
	//
	// outputs:
	//		return	= 0-based index of character if found
	//				= -1 if not found
	//

	int nReturn = -1;

	char* p = strstr(&mp_buffer[start], str);
	if (p)
		nReturn = (int)(p - mp_buffer);

	return nReturn;
}

inline int XString::ReverseFind(char ch)
{
	// find the first occurrence of a single character - search starts at end
	// and continues toward beginning of string
	//
	// inputs:
	//		ch = character to find
	//
	// outputs:
	//		return	= 0-based index of character if found
	//				= -1 if not found
	//

	int nReturn = -1;

	for (int i = mi_StrLen - 1; i > -1; --i)
	{
		if (mp_buffer[i] == ch)
			return i;
	}

	return nReturn;
}

inline int XString::FindOneOf(const char* str)
{
	// find the first occurrence of any char in str
	//
	// inputs:
	//		str = set of characters to find
	//
	// outputs:
	//		return = 0-based index of character if found
	//				= -1 if not found
	//

	int nReturn = -1;

	if (str)		// valid pointer
	{
		for (int i = 0; i < mi_StrLen; ++i)
		{
			if (strchr(str, mp_buffer[i]))
				return i;
		}
	}

	return nReturn;
}

inline int XString::Replace(char ch1, char ch2)
{
	// replace all occurrences of ch1 with ch2
	//
	// returns # of characters replaced
	//

	int nReturn = 0;    // # of replacements

	for (int i = 0; i < mi_StrLen; ++i)
	{
		if (mp_buffer[i] == ch1)
		{
			mp_buffer[i] = ch2;
			++nReturn;
		}
	}

	return nReturn;
}

inline int XString::Replace(const char* str1, const char* str2)
{
	// replace all occurrences of str1 with str2
	//
	// returns # of substrings replaced
	//

	int nReturn = 0;    // # of replacements

	int strSize1 = (int)strlen(str1);
	int strSize2 = (int)strlen(str2);
	int nPos = Find(str1);
	while (nPos > -1)
	{
		Delete(nPos, strSize1);
		Insert(nPos, str2);
		// start next search after chars just inserted
		nPos = Find(str1, nPos + strSize2);
		++nReturn;
	}

	return nReturn;
}

inline void XString::Insert(int pos, char ch)
{
	// insert ch at pos
	//

	if (pos < 0 || pos > mi_StrLen)		// invalid start offset, exit
		return;

	shift_right(pos, 1);
	mp_buffer[pos] = ch;

	mp_buffer[mi_StrLen] = 0;			// terminate new string
}

inline void XString::Insert(int pos, const char* str)
{
	// insert str at pos
	//

	if (strlen(str) < 1)				// inserting empty string, do nothing
		return;
	if (pos < 0 || pos > mi_StrLen)		// invalid start offset, exit
		return;

	int size = (int)strlen(str);
	shift_right(pos, size);

	for (int i = 0; i < size; ++i)
		mp_buffer[pos + i] = str[i];

	mp_buffer[mi_StrLen] = 0;			// terminate new string
}

inline void XString::Delete(int pos, int count /* = 1*/)
{
	// delete count characters, starting at pos
	//

	if (pos < 0 || pos > mi_StrLen)		// invalid start offset, exit
		return;
	if (count < 1)						// invalid count
		return;

	if (pos + count > mi_StrLen)			// past end, lop off all chars from pos to end
		mi_StrLen = pos;
	else
		shift_left(pos, count);

	mp_buffer[mi_StrLen] = 0;			// terminate new string
}

inline void XString::DeleteLeft(int count /* = 1*/)
{
	// delete the leftmost n characters
	//

	if (count < 1)			// invalid count
		return;

	if (count > mi_StrLen)	// count greater than length of string, delete entire string
		mi_StrLen = 0;
	else
		shift_left(0, count);
	mp_buffer[mi_StrLen] = 0;
}

inline void XString::DeleteRight(int count /* = 1*/)
{
	// delete the rightmost n characters
	//

	if (count < 1)			// invalid count
		return;

	if (count > mi_StrLen)	// count greater than length of string, delete entire string
		mi_StrLen = 0;
	else
		mi_StrLen -= count;
	mp_buffer[mi_StrLen] = 0;
}

inline void XString::TrimLeft(char ch)
{
	// trim spaces off beginning of string
	//

	if (mi_StrLen == 0)			// empty string
		return;

	int i;
	for (i = 0; i < mi_StrLen; ++i)		// look for first non-blank
	{
		if (mp_buffer[i] != ch)
			break;
	}
	if (i == mi_StrLen)			// string is all spaces
	{
		mp_buffer[0] = 0;		// just clear it
		mi_StrLen = 0;
	}
	else if (i > 0)				// some leading spaces
	{
		shift_left(0, i);		// shift string left i bytes
	}
}

inline void XString::TrimLeft(const char* str)
{
	// trim all chars in str off beginning of string
	//

	if (mi_StrLen == 0)			// empty string
		return;
	if (str == nullptr)			// invalid trim mask
		return;

	int i;
	for (i = 0; i < mi_StrLen; ++i)		// look for first non-mask char
	{
		if (strchr(str, mp_buffer[i]) == nullptr)	// this char not in trim mask
			break;
	}
	if (i == mi_StrLen)			// string is all spaces
	{
		mp_buffer[0] = 0;		// just clear it
		mi_StrLen = 0;
	}
	else if (i > 0)				// some leading spaces
	{
		shift_left(0, i);		// shift string left i bytes
	}
}

inline void XString::TrimRight(char ch)
{
	// trim spaces off end of string
	//

	if (mi_StrLen == 0)		// empty string
		return;

	while (mi_StrLen > 0)
	{
		if (mp_buffer[mi_StrLen - 1] != ch)
			break;
		mp_buffer[--mi_StrLen] = 0;
	}
}

inline void XString::TrimRight(const char* str)
{
	// trim all chars in str off end of string
	//

	if (mi_StrLen == 0)		// empty string
		return;
	if (str == nullptr)		// invalid mask string
		return;

	while (mi_StrLen > 0)
	{
		if (strchr(str, mp_buffer[mi_StrLen - 1]) == nullptr)	// char not in trim mask
			break;
		mp_buffer[--mi_StrLen] = 0;
	}
}

inline void XString::PadLeft(char ch, unsigned int count)
{
	// pad left of string with count chars

	for (unsigned int i = 0; i < count; ++i)
		Insert(0, ch);
}

inline void XString::PadRight(char ch, unsigned int count)
{
	// pad right of string with count chars

	resize(mi_StrLen + count + 1);
	for (unsigned int i = 0; i < count; ++i)
	{
		mp_buffer[mi_StrLen + i] = ch;
	}
	mi_StrLen = mi_StrLen + count;
	mp_buffer[mi_StrLen] = 0;
}

inline void XString::Swap(int pos1, int pos2)
{
	// exchange characters at pos1 & pos2
	//

	if (pos1 < 0 || pos1 >= mi_StrLen || pos2 < 0 || pos2 >= mi_StrLen)
		return;

	char ch = mp_buffer[pos2];
	mp_buffer[pos2] = mp_buffer[pos1];
	mp_buffer[pos1] = ch;
}

inline void XString::TruncateAt(char ch)
{
	// truncate string from first occurrence of ch to end
	//

	if (mi_StrLen == 0)		// empty string
		return;

	for (int i = 0; i < mi_StrLen; ++i)		// look for first non-blank
	{
		if (mp_buffer[i] == ch)
		{
			mi_StrLen = i;
			mp_buffer[mi_StrLen] = 0;
			break;
		}
	}
}

inline void XString::TruncateAtAny(const char* str)
{
	// truncate string from first occurrence of 
	// of any character in ch
	//

	if (mi_StrLen == 0)		// empty string
		return;

	for (int i = 0; i < mi_StrLen; ++i)		// look for first non-blank
	{
		if (strchr(str, mp_buffer[i]))
		{
			mi_StrLen = i;
			mp_buffer[mi_StrLen] = 0;
			break;
		}
	}
}

/* static */ inline XString XString::Format(const char* sFormat, ...)
{
	// inputs:
	//		sFormat = printf style formatting string
	//		...	    = parameters to formatting string
	//
	// static so i don't have to create an instance every time
	// i want to format a string. eg,
	//
	//		XString str;
	//		str.Format("%u", myVal);
	//		FunctionCall(str);
	//
	// vs,
	//
	//		FunctionCall(XString::Format("%lu", myVal));
	//

	char buf[255];			// work string

	va_list ap;
	va_start(ap, sFormat);
	vsnprintf(buf, sizeof(buf) - 1, sFormat, ap);
	va_end(ap);

	return buf;				// return formatted string to this instance
}

inline int XString::GetCount(const char ch)
{
	int nReturn = 0;
	for (char* p = mp_buffer; p < (mp_buffer + mi_StrLen); p++)
	{
		if (*p == ch)
			nReturn++;
	}

	return nReturn;
}

inline int XString::GetCount(const char* str)
{
	int nReturn = 0;
	int pos = Find(str, 0);
	while (pos > -1)
	{
		nReturn++;
		pos++;						// advance starting search pos
		pos = Find(str, pos);
	}

	return nReturn;
}

inline XString XString::GetPathName()
{
	// interpret string as path/file and return path name portion
	//
	// accept:
	//		"../path/file.ext"		->	"../path/"
	//		"./path/file.ext"			"./path/"
	//		"/path/file.ext"			"/path/"
	//		"path/file.ext"				"path/"
	//		"path/"						"path/"
	//		"file.ext"					""
	//		"/blah"						"/"				-> rule is: last token w/o '/' interpreted as file name
	//		"/blah/"					"/blah/"

	XString pathName = "";		// return value

	NormalizePath();
	int pos = ReverseFind('/');
	if (pos > -1)
	{
		// return string from start through last '/'
		pathName = Left(pos + 1);
	}

	return pathName;
}

inline XString XString::GetFileName()
{
	// interpret string as path/file and return file name portion
	//
	// accept:
	//		"../path/file.ext"		->	"file.ext"
	//		"./path/file.ext"			"file.ext"
	//		"/path/file.ext"			"file.ext"
	//		"path/file.ext"				"file.ext"
	//		"path/"						""
	//		"file.ext"					"file.ext"
	//		"file"						"file"
	//		"/blah"						"blah"			-> rule is: last token w/o '/' interpreted as file name
	//		"/blah/"					""

	//TODO:
	//logic incorrect - should return file name for no-slash strings, not empty string
	XString fileName = "";		// return value

	int pos1 = ReverseFind('/');
	int pos2 = ReverseFind('\\');
	int pos = pos1 > pos2 ? pos1 : pos2;
	if (pos > -1)
	{
		// return string from start through last '/'
		fileName = Mid(pos + 1);
	}

	return fileName;
}

inline void XString::FormatPathName()
{
	// interpreting as path, ensure valid format

	// 1. make all delimiters same
	NormalizePath();

	// 2. ensure ends with delimiter
	if (mp_buffer[mi_StrLen - 1] != '/')
	{
		if (mi_StrLen > mi_BufLen - 2)	// need at least 2 past end of "string"
			resize(mi_BufLen + 1);

		mp_buffer[mi_StrLen++] = '/';
		mp_buffer[mi_StrLen] = 0;
	}
}

inline void XString::NormalizePath()
{
	// interpreting as path, ensure all delimiters consistent

	for (int i = 0; i < mi_StrLen; ++i)
	{
		if (mp_buffer[i] == '\\')
			mp_buffer[i] = '/';
	}
}

//----------------------------------------------------------------------------
// operators

inline void XString::operator = (const char* str)
{
	// assignment
	//

	if (!str)
		return;

	int size = (int)strlen(str);
	resize(size + 1);						// add 1 for null terminator
	strncpy(mp_buffer, str, size);
	mi_StrLen = size;
	mp_buffer[mi_StrLen] = 0;				// terminate buffer
}

inline void XString::operator = (XString str)
{
	// assignment
	//

	int size = str.GetLength();
	resize(size + 1);						// add 1 for null terminator
	strcpy(mp_buffer, str.c_str());
	mi_StrLen = size;
	mp_buffer[mi_StrLen] = 0;				// terminate buffer
}

inline XString XString::operator + (XString str)
{
	// concatenate and return the result - do not modify lvalue or rvalue
	//

	XString sReturn;
	sReturn = *this;
	sReturn += str;
	return sReturn;
}

inline void XString::operator += (char ch)
{
	// concatenation
	//

	if (mi_StrLen > mi_BufLen - 2)	// need at least 2 past end of "string"
		resize(mi_BufLen + 1);

	mp_buffer[mi_StrLen++] = ch;
	mp_buffer[mi_StrLen] = 0;
}

inline void XString::operator += (const char* str)
{
	// concatenation
	//

	if (!str)
		return;

	int arg_len = (int)strlen(str);				// size of argument
	int new_size = mi_StrLen + arg_len;			// size of concatenated string

	if (new_size > mi_BufLen - 1)					// new string would be larger than current buffer
		resize(new_size + 1);

	for (int i = 0; i < arg_len; ++i)			// append new characters
		mp_buffer[mi_StrLen + i] = str[i];

	mi_StrLen = new_size;						// update string length
	mp_buffer[mi_StrLen] = 0;					// terminate buffer
}

inline void XString::operator += (XString str)
{
	// concatenation
	//

	if (str.GetLength() < 1)
		return;

	int arg_len = (int)str.GetLength();			// size of argument
	int new_size = mi_StrLen + arg_len;			// size of concatenated string

	if (new_size > mi_BufLen - 1)				// new string would be larger than current buffer
		resize(new_size + 1);

	for (int i = 0; i < arg_len; ++i)			// append new characters
		mp_buffer[mi_StrLen + i] = str.GetAt(i);

	mi_StrLen = new_size;						// update string length
	mp_buffer[mi_StrLen] = 0;					// terminate buffer
}

inline bool XString::operator == (const char* str)
{
	// comparison
	//

	if (!str)
		return false;

	return Compare(str);
}

inline bool XString::operator == (XString& str)
{
	// comparison
	//

	return Compare(str.c_str());
}

inline bool XString::operator != (const char* str)
{
	// not equal
	//

	if (!str)
		return false;

	return !Compare(str);
}

inline bool XString::operator != (XString& str)
{
	// not equal
	//

	return !Compare(str.c_str());
}

inline bool XString::operator > (const char* str)
{
	// greater than
	//

	if (!str)
		return false;

	return strcmp(mp_buffer, str) > 0;
}

inline bool XString::operator > (XString& str)
{
	// greater than
	//

	return strcmp(mp_buffer, str.c_str()) > 0;
}

inline bool XString::operator >= (const char* str)
{
	// greater than or equal to
	//

	if (!str)
		return false;

	return strcmp(mp_buffer, str) >= 0;
}

inline bool XString::operator >= (XString& str)
{
	// greater than or equal to
	//

	return strcmp(mp_buffer, str.c_str()) >= 0;
}

inline bool XString::operator < (const char* str)
{
	// less than
	//

	if (!str)
		return false;

	return strcmp(mp_buffer, str) < 0;
}

inline bool XString::operator < (XString& str)
{
	// less than
	//

	return strcmp(mp_buffer, str.c_str()) < 0;
}

inline bool XString::operator <= (const char* str)
{
	// less than or equal to
	//

	if (!str)
		return false;

	return strcmp(mp_buffer, str) <= 0;
}

inline bool XString::operator <= (XString& str)
{
	// less than or equal to
	//

	return strcmp(mp_buffer, str.c_str()) <= 0;
}

inline char XString::operator[](int pos) const
{
	if (pos > -1 && pos < mi_StrLen)
		return mp_buffer[pos];
	else
		return (char)0;
}

//----------------------------------------------------------------------------
// static methods

inline XString XString::CombinePath(const char* path1, const char* path2)
{
	XString pathName = path1;
	pathName.FormatPathName();
	pathName += path2;
	return pathName;
}

inline XString XString::CombinePath(const XString& path1, const XString& path2)
{
	XString pathName = path1;
	pathName.FormatPathName();
	pathName += path2;
	return pathName;
}

inline XString XString::CombinePath(const XString& path1, const char* path2)
{
	XString pathName = path1;
	pathName.FormatPathName();
	pathName += path2;
	return pathName;
}

//----------------------------------------------------------------------------
// private methods

inline void XString::initialize()
{
	mp_buffer = nullptr;		// string buffer
	mp_TokenMap = nullptr;
	mi_BufLen = 0;
	mi_StrLen = 0;
	mi_TokenMapSize = 0;		// size of buffer used for token map
	mi_TokenMapCount = 0;		// # of actual entries in map

	resize(32);				// initial allocation of string buffer

	// token map buffer is not initially allocated, only if a Tokenizing method is invoked
}

inline char XString::to_lower(char ch)
{
	if (ch >= 'A' && ch <= 'Z')
		ch |= 0x20;

	return ch;
}

inline char XString::to_upper(char ch)
{
	if (ch >= 'a' && ch <= 'z')
		ch &= ~0x20;

	return ch;
}

inline void XString::shift_left(int pos, int count)
{
	// shift string left count bytes, starting at offset pos
	//

	mi_StrLen -= count;							// update string length
	for (int i = pos; i < mi_StrLen; i++)		// shift string
		mp_buffer[i] = mp_buffer[i + count];
	mp_buffer[mi_StrLen] = 0;					// terminate new string
}

inline void XString::shift_right(int pos, int count)
{
	// shift string right count bytes, starting at offset pos
	//
	// grow string as necessary, inserting spaces
	//

	int nMoveCount = mi_StrLen - pos + 1;		// # of chars to move, +1 to move terminating 0 along with rest of chars

	resize(mi_StrLen + count + 1);				// grow buffer

	// shift chars to end

	char* pdest = &mp_buffer[mi_StrLen + count];	// dest = position of new last char
	char* psrc = &mp_buffer[mi_StrLen];				// src  = position of current last char

	for (int i = 0; i < nMoveCount; ++i)
	{
		*pdest = *psrc;		// move chars to end
		*psrc = ' ';		// fill new area with spaces
		psrc--;
		pdest--;
	}

	mi_StrLen += count;						// update string length
	mp_buffer[mi_StrLen] = 0;				// terminate buffer
}

inline void XString::resize(int newSize)
{
	// resize buffer

	if (!mp_buffer)		// buffer being initialized
	{
		mp_buffer = new char[newSize];		// allocate init buffer
		memset(mp_buffer, 0, newSize);		// clear buffer
		mi_BufLen = newSize;				// update buffer size
		mi_StrLen = 0;						// no data in buffer yet
		return;
	}

	// the "NewSize" arg is taken literally as the desired size - actual size increments
	// are handled here; currently, I am doubling the desired size along the lines of the
	// stl vector

	if (mi_BufLen >= newSize)	// if for some reason it's already big enough, return
		return;

	// double it so we're not always reallocating memory for each new byte or two
	newSize *= 2;

	// allocate new larger buffer and release old one

	char* pTemp = mp_buffer;				// save pointer to old buffer
	mp_buffer = new char[newSize];			// allocate new buffer
	memset(mp_buffer, 0, newSize);			// clear new buffer
	strncpy(mp_buffer, pTemp, mi_BufLen);	// copy contents of old buffer to new
	delete pTemp;							// release old buffer

	// update buffer size; str length did not change, do not update
	mi_BufLen = newSize;
}

inline void XString::grow_token_map()
{
	// increase size of token map
	//

	mi_TokenMapSize *= 2;
	TokenMap* pTemp = mp_TokenMap;
	mp_TokenMap = new TokenMap[mi_TokenMapSize];
	memcpy(mp_TokenMap, pTemp, sizeof(TokenMap)*mi_TokenMapSize / 2);
	delete pTemp;
}

// xstring comparison function - stl needs this for maps, even though a "<" operator is
// supplied - seems like i'm missing something here...
struct cmp_xstring
{
	bool operator()(XString a, XString b)
	{
		return strcmp(a.c_str(), b.c_str()) < 0;
	}
};

#endif // #ifndef XSTRING_H
