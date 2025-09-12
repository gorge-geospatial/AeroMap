// TestXString.cpp
// CppUnitLite test harness for XString class
//
//	TODO:
//	- no test case for XString::Extract()
//	- Trim(const char*) is not implemented
//	- apparently, map<XString,foo> doesn't work -> add test case after fix
//	- no test case for: inline void XString::operator += ( XString& str )
//	- no test case for: inline void XString::Format()
//	- A + B concatenation has no test case
//	- operator[] has no test case
//

#include <stdio.h>
#include <stdlib.h>

#include <limits.h>

#include "CppUnitLite/TestHarness.h"
#include "XString.h"			// interface to class under test

#include <map>

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

//----------------------------------------------------------------------------
TEST(XString, Constructor)
{
	// test constructor(s) / copy constructor(s)
	//

	// constructor that accepts unsigned long int

	{
		unsigned long int num = 123;
		XString str(num);
		CHECK(str.Compare("123"));
	}
	{
		unsigned long int num = 4294967295UL;	// max ulong on 32-bit
		XString str(num);
		CHECK(str.Compare("4294967295"));
	}

	// constructor that accepts XString

	{
		XString str1;
		LONGS_EQUAL(0, str1.GetLength());

		str1 = "Hello World";
		XString str2 = str1;

		CHECK(str1.Compare("Hello World"));
		CHECK(str2.Compare("Hello World"));
	}

	{
		XString str1;
		LONGS_EQUAL(0, str1.GetLength());

		str1 = "123456789012345678901234567890123456789012345678901234567890";
		XString str2 = str1;

		CHECK(str1.Compare("123456789012345678901234567890123456789012345678901234567890"));
		CHECK(str2.Compare("123456789012345678901234567890123456789012345678901234567890"));
	}

	// constructor that accepts char*

	{
		// previously-detected failure mode

		XString str1 = "Hello World";
		CHECK(str1.Compare("Hello World"));

		str1 += "123";

		XString str2 = str1;
		CHECK(str2.Compare("Hello World123"));

		// don't crash on null pointer
		//      XString str3 = NULL;
	}

	// constructor that accepts char

	{
		XString str1('c');
		CHECK(str1.Compare("c"));
	}

	// constructor that accepts char, count

	{
		XString str1('c', 5);
		CHECK(str1.Compare("ccccc"));
	}

	{
		// trigger memory allocation on source, then assign

		XString str1 = "Hello World";
		CHECK(str1.Compare("Hello World"));

		str1 += "12345678901234567890123456789012345678901234567890";       // 50 chars
		CHECK(str1.Compare("Hello World12345678901234567890123456789012345678901234567890"));

		XString str2 = str1;
		CHECK(str2.Compare("Hello World12345678901234567890123456789012345678901234567890"));
	}
}

//----------------------------------------------------------------------------
TEST(XString, STL_MAP)
/*
	test constructor(s) / copy constructor(s)
*/
{
	//TODO:
	//this works fine if i supply the cmp_xstring comparison function, but it doesn't seem
	//to take the < operators provided by the XString class!
	//std::map<XString, int, cmp_xstring> mapTest;
	//mapTest.clear();
	//std::map<XString, int, cmp_xstring>::iterator i;
	//XString s = "";

	//s = "string1";
	//mapTest.insert(std::make_pair(s, 1));
	//s = "string2";
	//mapTest.insert(std::make_pair(s, 2));
	//s = "string3";
	//mapTest.insert(std::make_pair(s, 3));

	//CHECK(mapTest.size() == 3);

	//i = mapTest.begin();
	//while (i != mapTest.end())
	//{
	//	XString sCur = i->first;
	//	//printf("%s=%d\n", sCur.c_str(), i->second);
	//	i++;
	//}

	//XString str = "string2";
	//i = mapTest.find("string2");
	//CHECK(i != mapTest.end());

	//i = mapTest.find(str);
	//CHECK(i != mapTest.end());

	//i = mapTest.find((const char*)"string2");
	//CHECK(i != mapTest.end());
}

//----------------------------------------------------------------------------
TEST(XString, GetAt)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "0123456789";
	LONGS_EQUAL(10, str.GetLength());

	// valid conditions

	CHECK(str.GetAt(0) == '0');
	CHECK(str.GetAt(5) == '5');
	CHECK(str.GetAt(9) == '9');

	// invalid conditions

	CHECK(str.GetAt(-2) != '0');
	CHECK(str.GetAt(25) != '0');

	str = "";
	LONGS_EQUAL(0, str.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, ConcatChars)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "";
	LONGS_EQUAL(0, str.GetLength());

	// trigger multiple memory reallocations

	str = "";
	for (int i = 0; i < 25000; i++)
	{
		str += 'X';
		LONGS_EQUAL(i + 1, str.GetLength());
	}

	LONGS_EQUAL(25000, str.GetLength());

	str = "";
	LONGS_EQUAL(0, str.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, ConcatStrings)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	// trigger multiple memory reallocations

	str = "";
	for (int i = 0; i < 5000; i++)
	{
		str += "ABCD";
	}

	LONGS_EQUAL(20000, str.GetLength());

	str = "";
	LONGS_EQUAL(0, str.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, Tokenizer)
{
	// test general-purpose tokenizer

	XString str1;
	XString str2;
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());

	str1 = "Hello,brave,new,world!";        // basic test case
	int cnt = str1.Tokenize(",");
	LONGS_EQUAL(4, cnt);

	str1 = "";                              // empty string
	cnt = str1.Tokenize("");
	LONGS_EQUAL(0, cnt);                  // no tokens in string

	str1 = "Hello,brave,new,world!";        // no delimiters in string
	cnt = str1.Tokenize("?");
	LONGS_EQUAL(1, cnt);                  // should be one - the entire string

	str1 = "Hello,,brave,new,world!";       // multiple delimiters == single delimiter
	cnt = str1.Tokenize(",");
	LONGS_EQUAL(4, cnt);

	str1 = "Hello,,brave,new,world!,";      // terminating delimiter(s) should be ignored
	cnt = str1.Tokenize(",");
	LONGS_EQUAL(4, cnt);

	// validate individual tokens
	str2 = str1.GetToken(0);    CHECK(str2.Compare("Hello"));
	str2 = str1.GetToken(1);    CHECK(str2.Compare("brave"));
	str2 = str1.GetToken(2);    CHECK(str2.Compare("new"));
	str2 = str1.GetToken(3);    CHECK(str2.Compare("world!"));

	// don't crash on null pointer
	str1 = "Hello,brave,new,world!";
	str1.Tokenize(NULL);

	str1 = "";
	str2 = "";
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, CmdTokenizer)
{
	// test command tokenizer

	// commands have the form: "foo( 1, 2, 3.4 )"

	XString str1;
	XString str2;
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());

	str1 = "foo( 1, 2, 3.4 )";      // basic test case
	int cnt = str1.Tokenize(",()");
	LONGS_EQUAL(4, cnt);

	// validate individual tokens
	str2 = str1.GetToken(0);    CHECK(str2.Compare("foo"));
	str2 = str1.GetToken(1);    CHECK(str2.Compare("1"));
	str2 = str1.GetToken(2);    CHECK(str2.Compare("2"));
	str2 = str1.GetToken(3);    CHECK(str2.Compare("3.4"));

	str1 = "";
	str2 = "";
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, Trim)
{
	XString str1;
	LONGS_EQUAL(0, str1.GetLength());

	// TrimRight()

	str1 = "Hello  ";                                   // simple case
	str1.TrimRight();
	CHECK(str1.Compare("Hello"));

	str1 = "Hello";
	str1.TrimRight();
	CHECK(str1.Compare("Hello"));

	str1 = "  ";
	str1.TrimRight();
	CHECK(str1.GetLength() == 0);

	str1 = "";
	str1.TrimRight();
	CHECK(str1.GetLength() == 0);

	// TrimLeft()

	str1 = " Hello";                        // single leading space
	str1.TrimLeft();
	CHECK(str1.Compare("Hello"));

	str1 = "    Hello";                     // multiple leading spaces
	str1.TrimLeft();
	CHECK(str1.Compare("Hello"));

	str1 = "        ";                      // all spaces
	str1.TrimLeft();
	CHECK(str1.Compare(""));

	// Trim()

	str1 = " Hello";                        // single leading space
	str1.Trim();
	CHECK(str1.Compare("Hello"));

	str1 = "    Hello";                     // multiple leading spaces
	str1.Trim();
	CHECK(str1.Compare("Hello"));

	str1 = "        ";                      // all spaces
	str1.Trim();
	CHECK(str1.Compare(""));

	str1 = "";
	LONGS_EQUAL(0, str1.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, Pad)
{
	XString str1;
	LONGS_EQUAL(0, str1.GetLength());

	// PadLeft()

	str1 = "Hello";
	str1.PadLeft('X', 5);
	CHECK(str1.Compare("XXXXXHello"));

	str1 = "Hello";
	str1.PadRight('X', 5);
	CHECK(str1.Compare("HelloXXXXX"));
}

//----------------------------------------------------------------------------
TEST(XString, Left)
{
	// test XString .Left() method

	XString str1;
	XString str2;
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());

	str1 = "Hello World";                           // simple Left() test
	str2 = str1.Left(4);
	CHECK(str2.Compare("Hell"));

	str1 = "Hello World";                           // gt string length
	str2 = str1.Left(40);
	CHECK(str2.Compare("Hello World"));

	str1 = "Hello World";                           // should return ""
	str2 = str1.Left(0);
	CHECK(str2.Compare(""));

	str1 = "Hello World";
	str2 = str1.Left(1);
	CHECK(str2.Compare("H"));

	str1 = "Hello World";                           // invalid value
	str2 = str1.Left(-1);
	CHECK(str2.Compare(""));

	str1 = "";
	str2 = "";
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, Right)
{
	XString str1;
	XString str2;
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());

	str1 = "Hello World";                               // simple Right() test
	str2 = str1.Right(4);
	CHECK(str2.Compare("orld"));

	str1 = "Hello World";                               // Right() gt string length
	str2 = str1.Right(500);
	CHECK(str2.Compare("Hello World"));

	str1 = "Hello World";
	str2 = str1.Right(1);
	CHECK(str2.Compare("d"));

	str1 = "Hello World";                               // Right() invalid value
	str2 = str1.Right(-2);
	CHECK(str2.Compare(""));

	str1 = "";
	str2 = "";
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, Mid)
{
	XString str1;
	XString str2;
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());

	str1 = "Hello World";                               // simple Mid() test
	str2 = str1.Mid(1, 4);
	CHECK(str2.Compare("ello"));

	str1 = "Hello World";                               // Mid() past end
	str2 = str1.Mid(1, 40);
	CHECK(str2.Compare("ello World"));

	str1 = "Hello World";                               // Mid() default to end
	str2 = str1.Mid(1);
	CHECK(str2.Compare("ello World"));

	str1 = "";
	str2 = "";
	LONGS_EQUAL(0, str1.GetLength());
	LONGS_EQUAL(0, str2.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, Delete)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "0123456789";
	str.Delete(0, 1);
	CHECK(str.Compare("123456789"));

	str = "0123456789";
	str.Delete(1, 1);
	CHECK(str.Compare("023456789"));

	str = "0123456789";
	str.Delete(5, 3);
	CHECK(str.Compare("0123489"));

	str = "0123456789";
	str.Delete(15, 3);
	CHECK(str.Compare("0123456789"));

	str = "0123456789";
	str.Delete(-15, 3);
	CHECK(str.Compare("0123456789"));

	str = "0123456789";
	str.Delete(5, -3);
	CHECK(str.Compare("0123456789"));

	str = "0123456789";
	for (int i = 0; i < 10; i++)
		str.Delete(0);
	CHECK(str.Compare(""));
}

//----------------------------------------------------------------------------
TEST(XString, DeleteLeft)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "0123456789";
	LONGS_EQUAL(10, str.GetLength());

	str.DeleteLeft();   // default count == 1
	CHECK(str.Compare("123456789"));

	for (int i = 0; i < 100; i++)
		str.DeleteLeft();
	LONGS_EQUAL(0, str.GetLength());

	str = "Hello World";
	str.DeleteLeft(4);
	CHECK(str.Compare("o World"));

	str.DeleteLeft(20);
	LONGS_EQUAL(0, str.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, DeleteRight)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "0123456789";
	LONGS_EQUAL(10, str.GetLength());
	CHECK(str.Compare("0123456789"));

	str.DeleteRight();  // default count == 1
	LONGS_EQUAL(9, str.GetLength());

	for (int i = 0; i < 100; i++)
		str.DeleteRight();
	LONGS_EQUAL(0, str.GetLength());

	str = "Hello World";
	str.DeleteRight(4);
	LONGS_EQUAL(7, str.GetLength());

	str = "Hello World";
	str.DeleteRight(20);
	LONGS_EQUAL(0, str.GetLength());

	str = "Hello World";
	str.DeleteRight(-1);
	CHECK(str.Compare("Hello World"));
}

//----------------------------------------------------------------------------
TEST(XString, Insert)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	//  void Insert( int pos, char ch );                // insert character

	str = "Hello World";
	CHECK(str.Compare("Hello World"));
	str.Insert(5, '!');
	CHECK(str.Compare("Hello! World"));

	str = "Hello World";
	str.Insert(0, '_');
	CHECK(str.Compare("_Hello World"));

	str = "Hello World";
	str.Insert(11, '_');
	CHECK(str.Compare("Hello World_"));

	str = "Hello World";
	str.Insert(20, '_');
	CHECK(str.Compare("Hello World"));

	//  void Insert( int pos, const char* str );        // insert string

	str = "Hello World";
	CHECK(str.Compare("Hello World"));
	str.Insert(6, "THERE ");
	CHECK(str.Compare("Hello THERE World"));

	str = "Hello World";
	CHECK(str.Compare("Hello World"));
	str.Insert(0, "01234");
	CHECK(str.Compare("01234Hello World"));

	str = "Hello World";
	CHECK(str.Compare("Hello World"));
	str.Insert(11, "01234");
	CHECK(str.Compare("Hello World01234"));

	str = "Hello World";
	CHECK(str.Compare("Hello World"));
	str.Insert(20, "01234");
	CHECK(str.Compare("Hello World"));
}

//----------------------------------------------------------------------------
TEST(XString, Comparison)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "Hello World";

	// .Compare()

	CHECK(str.Compare("Hello World"));            // match case
	CHECK(!str.Compare("HELLO"));                 // different length case
	CHECK(!str.Compare("HELLO WORLD"));           // different string case
	CHECK(!str.Compare(""));

	// .CompareNoCase()

	CHECK(str.CompareNoCase("HELLO WORLD"));      // case-insensitive match case
	CHECK(!str.CompareNoCase("HELLO"));           // different length case
	CHECK(!str.CompareNoCase("HELLO there"));     // different string case
	CHECK(!str.CompareNoCase(""));

	// comparison operators

	// ==

	XString str2 = "Hello World";
	CHECK(str == "Hello World");      // char* == xstring
	CHECK(str == str2);               // xstring == xstring

	// !=

	CHECK(str != "Hello_World");      // char* != xstring
	CHECK(str != "Hello");            // char* != xstring
	str2 = "Hello_World";
	CHECK(str != str2);               // xstring != xstring
	str2 = "Hello";
	CHECK(str != str2);               // xstring != xstring

	// >

	str = "Hello World";
	str2 = "Aello World";
	CHECK(str > "Aello World");       // char* > xstring
	CHECK(str > str2);                // xstring > xstring

	// <

	str = "Hello World";
	str2 = "Zello World";
	CHECK(str < "Zello World");       // char* < xstring
	CHECK(str < str2);                // xstring < xstring

	// >=

	str = "Hello World";
	str2 = "Aello World";
	CHECK(str >= "Aello World");      // char* >= xstring
	CHECK(str >= str2);               // xstring >= xstring

	// <=

	str = "Hello World";
	str2 = "Zello World";
	CHECK(str <= "Zello World");      // char* <= xstring
	CHECK(str <= "Hello World");      // char* <= xstring
	CHECK(str <= str2);               // xstring <= xstring
	str2 = "Hello World";
	CHECK(str <= str2);               // xstring <= xstring

	// don't crash on null pointer
	str.Compare(NULL);
	str.CompareNoCase(NULL);
}

//----------------------------------------------------------------------------
TEST(XString, CaseMapping)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "Hello World";

	str.MakeUpper();
	CHECK(str.Compare("HELLO WORLD"));

	str.MakeLower();
	CHECK(str.Compare("hello world"));
}

//----------------------------------------------------------------------------
TEST(XString, Replace)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	// Replace( char, char )

	str = "Hello World";

	int cnt = str.Replace('o', 'X');
	LONGS_EQUAL(2, cnt);

	CHECK(str.Compare("HellX WXrld"));

	// Replace( char*, char* )

	str = "part = right_upper_arm";
	cnt = str.Replace(" ", "");
	cnt = str.Replace("\t", "");
	CHECK(str.Compare("part=right_upper_arm"));

	str = " dir     = < -1, 0, 0 >";
	cnt = str.Replace(" ", "");
	cnt = str.Replace("\t", "");
	CHECK(str.Compare("dir=<-1,0,0>"));

	// test replacing string with itself

	str = "hello world";
	int count = str.Replace(" ", " ");
	CHECK(count == 1);
}

//----------------------------------------------------------------------------
TEST(XString, Find)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	// Find( char )

	str = "Hello World";
	int pos = str.Find('o');        // find first occurrence of ch
	LONGS_EQUAL(4, pos);

	pos = str.Find('x');
	LONGS_EQUAL(-1, pos);

	str = "";
	pos = str.Find('A');
	LONGS_EQUAL(-1, pos);

	// Find( const char* )

	str = "Hello World";
	pos = str.Find("World");
	LONGS_EQUAL(6, pos);
	pos = str.Find("test");
	LONGS_EQUAL(-1, pos);
	pos = str.Find("d");
	LONGS_EQUAL(10, pos);
	pos = str.Find("l");
	LONGS_EQUAL(2, pos);
	pos = str.Find("l", 5);
	LONGS_EQUAL(9, pos);

	// ReverseFind(

	str = "0123456789";
	pos = str.ReverseFind('5');		// found case
	CHECK(pos == 5);
	pos = str.ReverseFind('C');		// not found case
	CHECK(pos == -1);
	pos = str.ReverseFind('0');		// end case
	CHECK(pos == 0);
	pos = str.ReverseFind('9');		// end case
	CHECK(pos == 9);

	// FindOneOf( const char* )

	str = "1234A67890";
	pos = str.FindOneOf("CAT");
	LONGS_EQUAL(4, pos);

	str = "1234A67890";
	pos = str.FindOneOf("DOG");
	LONGS_EQUAL(-1, pos);

	str = "";
	pos = str.FindOneOf("DOG");
	LONGS_EQUAL(-1, pos);

	str = "1234A67890";
	pos = str.FindOneOf("");
	LONGS_EQUAL(-1, pos);

	str = "";
	pos = str.FindOneOf("");
	LONGS_EQUAL(-1, pos);

	str = "";
	LONGS_EQUAL(0, str.GetLength());
}

//----------------------------------------------------------------------------
TEST(XString, GetCount)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "hello world";
	int n = str.GetCount('l');
	CHECK(n == 3);

	n = str.GetCount('x');
	CHECK(n == 0);

	n = str.GetCount("or");
	CHECK(n == 1);

	n = str.GetCount("not found");
	CHECK(n == 0);
}

//----------------------------------------------------------------------------
TEST(XString, BeginsWith)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "hello world";
	CHECK(str.BeginsWith("hello"));
	CHECK(str.BeginsWith("h"));
	CHECK(str.BeginsWith("hello world"));
	CHECK(str.BeginsWith("hello worldx") == false);
	CHECK(str.BeginsWith("jello") == false);
}

//----------------------------------------------------------------------------
TEST(XString, BeginsWithNoCase)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "hello world";
	CHECK(str.BeginsWithNoCase("hello"));
	CHECK(str.BeginsWithNoCase("Hello"));
	CHECK(str.BeginsWithNoCase("Hello "));
	CHECK(str.BeginsWithNoCase("h"));
	CHECK(str.BeginsWithNoCase("H"));
	CHECK(str.BeginsWithNoCase("hello world"));
	CHECK(str.BeginsWithNoCase("hello worldx") == false);
	CHECK(str.BeginsWithNoCase("jello") == false);
}

//----------------------------------------------------------------------------
TEST(XString, EndsWith)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "hello world";
	CHECK(str.EndsWith("world"));
	CHECK(str.EndsWith(" world"));
	CHECK(str.EndsWith("hello world"));
	CHECK(str.EndsWith("d"));
	CHECK(str.EndsWith("x") == false);
	CHECK(str.EndsWith("werld") == false);
}

//----------------------------------------------------------------------------
TEST(XString, EndsWithNoCase)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "hello world";
	CHECK(str.EndsWithNoCase("world"));
	CHECK(str.EndsWithNoCase("World"));
	CHECK(str.EndsWithNoCase(" world"));
	CHECK(str.EndsWithNoCase("hello world"));
	CHECK(str.EndsWithNoCase("HELLO WORLD"));
	CHECK(str.EndsWithNoCase("HeLlO WoRlD"));
	CHECK(str.EndsWithNoCase("d"));
	CHECK(str.EndsWithNoCase("D"));
	CHECK(str.EndsWithNoCase("x") == false);
	CHECK(str.EndsWithNoCase("X") == false);
	CHECK(str.EndsWithNoCase("werld") == false);
}

//----------------------------------------------------------------------------
TEST(XString, GetPathName)
{
	{
		XString str = "/this/is/a/test.txt";

		XString pathName = str.GetPathName();
		CHECK(pathName == "/this/is/a/");
	}

	{
		XString str = "C:\\this\\is\\a\\test.txt";

		XString pathName = str.GetPathName();
		CHECK(pathName == "C:/this/is/a/");
	}

	{
		XString str = "test.txt";

		XString pathName = str.GetPathName();
		CHECK(pathName == "");
	}
}

//----------------------------------------------------------------------------
TEST(XString, GetFileName)
{
	{
		XString str = "/this/is/a/test.txt";

		XString fileName = str.GetFileName();
		CHECK(fileName == "test.txt");
	}

	{
		XString str = "C:\\this\\is\\a\\test.txt";

		XString fileName = str.GetFileName();
		CHECK(fileName == "test.txt");
	}

	{
		XString str = "test.txt";

		XString fileName = str.GetFileName();
		CHECK(fileName == "");
	}
}

//----------------------------------------------------------------------------
TEST(XString, FormatPathName)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "/media/4EECAED6ECAEB79B/Geodata";
	str.FormatPathName();
	CHECK(str == "/media/4EECAED6ECAEB79B/Geodata/");

	str = "/media/4EECAED6ECAEB79B/Geodata/";
	str.FormatPathName();
	CHECK(str == "/media/4EECAED6ECAEB79B/Geodata/");

	str = "\\media\\4EECAED6ECAEB79B\\Geodata";
	str.FormatPathName();
	CHECK(str == "/media/4EECAED6ECAEB79B/Geodata/");

	str = "\\media\\4EECAED6ECAEB79B\\Geodata\\";
	str.FormatPathName();
	CHECK(str == "/media/4EECAED6ECAEB79B/Geodata/");

	str = "/";
	str.FormatPathName();
	CHECK(str == "/");

	str = "\\";
	str.FormatPathName();
	CHECK(str == "/");
}

//----------------------------------------------------------------------------
TEST(XString, Clear)
{
	XString str;
	LONGS_EQUAL(0, str.GetLength());

	str = "hello world";
	CHECK(str.GetLength() == 11);

	str.Clear();
	CHECK(str.GetLength() == 0);

	str = "hello world";
	CHECK(str.GetLength() == 11);

	str.Clear();
	CHECK(str.GetLength() == 0);
}
