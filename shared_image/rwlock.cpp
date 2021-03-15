#include "rwlock.h"


CRWLock::CRWLock(pthread_rwlock_t* lock)
	: lock_addr_{ lock }
{}

CRWLock::~CRWLock()
{
	if (lock_addr_ && locked_) {
		pthread_rwlock_unlock(lock_addr_);
		if (owner_) destroy();
	}	
}

void CRWLock::init(pthread_rwlock_t* lock, int flags)
{
	lock_addr_ = lock;
	owner_ = true;

	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
	pthread_rwlockattr_setpshared(&attr, flags);

	pthread_rwlock_init(lock_addr_, &attr);
}

void CRWLock::open(pthread_rwlock_t *lock)
{
	lock_addr_ = lock;
}

void CRWLock::destroy()
{
	if (locked_) {
		unlock();
	}

	if (lock_addr_) {
		wrlock();
		pthread_rwlock_destroy(lock_addr_);
		lock_addr_ = nullptr;
	}
}

void CRWLock::rdlock()
{
	if (locked_) {
		unlock();
	}
	
	pthread_rwlock_rdlock(lock_addr_);
	locked_ = true;
}

void CRWLock::wrlock()
{
	if (locked_) {
		unlock();
	}

	pthread_rwlock_wrlock(lock_addr_);
	locked_ = true;
}

void CRWLock::unlock()
{
	if (locked_) {
		pthread_rwlock_unlock(lock_addr_);
	}

	locked_ = false;
}

pthread_rwlock_t* CRWLock::raw_rwlock() const
{
	return lock_addr_;
}
