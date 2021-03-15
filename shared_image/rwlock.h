// TODO: use C++14 reader-write locks
#pragma once

#include <pthread.h>
#include <stdio.h>


class CRWLock
{

public:
	enum class AccessFlags
	{
		PRIVATE = PTHREAD_PROCESS_PRIVATE, /// access limited to process initialized rwlock
		SHARED  = PTHREAD_PROCESS_SHARED /// free access
	};

public:
	CRWLock() = default;

	/*!
	 * @param[in] lock existing pthread_rwlock_t pointer
	 */
	CRWLock(pthread_rwlock_t* lock);

	~CRWLock();

	/*!
	 * @param[in] lock pointer to beginning of shared memory block
	 * @param[in] flags access flags
	 */
	void init(pthread_rwlock_t* lock, int flags = static_cast<int>(AccessFlags::PRIVATE));

	/*!
	 * @brief Copy existing lock
	 * @param lock[in] lock to copy
	 * @return
	 */
	void open(pthread_rwlock_t* lock);

	/*!
	 * @brief safely destroy rwlock
	 */
	void destroy();

	/*!
	 * @brief Lock for read
	 */
	void rdlock();

	/*!
	 * @brief Lock for write
	 */
	void wrlock();

	/*!
	 * @brief What you expected to see here?
	 */
	void unlock();

	/*!
	 * @return posix pthread_rwlock_t pointer
	 */
	pthread_rwlock_t* raw_rwlock() const;

private:
	pthread_rwlock_t* lock_addr_  = nullptr;
	bool              owner_ = false;
	bool 			  locked_ = false;
};
