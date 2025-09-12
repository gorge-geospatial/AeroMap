///////////////////////////////////////////////////////////////////////////////
//
// TESTRESULT.H
// 
// A TestResult is a collection of the history of some test runs.  Right now
// it just collects failures.
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef TESTRESULT_H
#define TESTRESULT_H

class Failure;

class TestResult
{
public:
	TestResult ();
	// 12/15/2007 - added virtual dtor; gcc warns on classes with virtual functions and non-virtual dtor
	virtual ~TestResult() {}

	virtual void	testsStarted ();
	virtual void	addFailure (const Failure& failure);
	virtual void	addSuccess ();
	virtual void	testsEnded ();

	int GetFailureCount() { return m_failureCount; }

private:
	long int m_failureCount;
	long int m_successCount;
};

#endif // #ifndef TESTRESULT_H

