# makefile for CppUnitLite for Linux
#
# creates static library: libCppUnitLite.a
#
# notes:
#	'ar' is the linux archive utility
#		r = replacement => replace files with same name (default is to add to end)
#		c = create
#		s = create object-file index

all:
	gcc -c Failure.cpp SimpleString.cpp Test.cpp TestRegistry.cpp TestResult.cpp
	ar rcs libCppUnitLite.a Failure.o SimpleString.o Test.o TestRegistry.o TestResult.o

clean:
	rm -f *.o a.out
	rm -f lib* *.a

