
#pragma once

#include <cstring>
#include <cstdio>

#include <fcntl.h>
#include <linux/limits.h>

#include <errno.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>


class CSharedMemory
{
	static constexpr std::size_t PATH_MAX_SIZE = 256;
public:
	enum class Flags
	{
		READER = O_RDONLY,
		WRITER = O_WRONLY,
		RDWR = O_RDWR	// both read and write
	};
public:
	/*!
	  @param[in] name shared mem path or name
	  @param[in] flags access flags
	*/
	CSharedMemory(const char* name, Flags flags);

	~CSharedMemory();

	/*!
	  @param[in] size
	  @param[in] addr beginning of memory block or null
	*/
	void* create(std::size_t size, void* addr = nullptr);

	/*!
	  Create projection of already created block, make it client usable
	  @param[in] addr beginning of memory block or null
	*/
	void* open(void* addr = nullptr);

	/*!
	  Destructor called inside
	*/
	void close();

	/*!
	  @return size of memory block
	*/
	std::size_t size() const;

	/*!
	  @return beginning of memory block
	*/
	void* address() const;

private:
	int         shmemd_;
	int         flags_;
	char        path_[PATH_MAX_SIZE];
	bool        creator_ = false;
	std::size_t size_;
	void*       address_;
};
