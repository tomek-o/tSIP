/** \file	StaticMutex.h
	\brief	Mutex for static objects locked during shutdown
*/

#ifndef StaticMutexH
#define StaticMutexH

/** \brief Mutex with no destructor, usable until the process ends

	VCL unit finalization is not ordered against destructors of C++ static
	objects, so a plain static Mutex can already be deleted when the last
	call comes in. Skipping DeleteCriticalSection() allocates nothing, so
	this costs no leak.

	\note Expects <windows.h> to be included first, same as Mutex.
*/
class StaticMutex
{
public:
	StaticMutex () { InitializeCriticalSection(&cs); }
	// no destructor on purpose
	inline void lock () { EnterCriticalSection(&cs); }
	inline void unlock () { LeaveCriticalSection(&cs); }
private:
	CRITICAL_SECTION cs;
	StaticMutex(const StaticMutex&);
	StaticMutex& operator = (const StaticMutex&);
};

#endif
