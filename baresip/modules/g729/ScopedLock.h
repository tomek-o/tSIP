#ifndef ScopedLockH
#define ScopedLockH

// Usage: ScopedLock<Mutex> lock(mutex);
template <class M> class ScopedLock
{
public:
	ScopedLock(M& mutex): m_mutex(mutex) {
		//OutputDebugString("lock");
		m_mutex.lock();
	}
	~ScopedLock() {
		//OutputDebugString("unlock");
		m_mutex.unlock();
	}
private:
	M& m_mutex;
	ScopedLock();
	ScopedLock(const ScopedLock&);
	ScopedLock& operator = (const ScopedLock&);
};

#endif
