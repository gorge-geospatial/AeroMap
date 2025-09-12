#include "TestResult.h"
#include "Failure.h"

#include <stdio.h>


TestResult::TestResult()
	: m_failureCount(0)
	, m_successCount(0)
{
}

void TestResult::testsStarted()
{
}

void TestResult::addSuccess()
{
	m_successCount++;
}

void TestResult::addFailure (const Failure& failure) 
{
	fprintf (stdout, "%s%s%s%s%ld%s%s\n",
		"Failure: \"",
		failure.message.asCharString (),
		"\" " ,
		"line ",
		failure.lineNumber,
		" in ",
		failure.fileName.asCharString ());
		
	++m_failureCount;
}

void TestResult::testsEnded()
{
	if (m_failureCount > 1)
		fprintf (stdout, "  ==> There were %ld failures\n", m_failureCount);
	else if (m_failureCount == 1)
		fprintf (stdout, "  ==> There was %ld failure\n", m_failureCount);
	else
		fprintf (stdout, "  ==> There were no test failures\n");
}
