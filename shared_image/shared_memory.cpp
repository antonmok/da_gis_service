#include "shared_memory.h"

CSharedMemory::CSharedMemory(const char* name, Flags flags)
	: shmemd_{ -1 }, flags_{ static_cast<decltype(flags_)>(flags) }, size_{ 0 }, address_{ nullptr }
{
	std::strncpy(path_, name, PATH_MAX_SIZE);
}

void* CSharedMemory::create(std::size_t size, void *addr)
{
	shmemd_ = shm_open(path_, flags_ | O_CREAT | O_EXCL, 0644);

	if (shmemd_ == -1) {
		return nullptr;
	}

	if (ftruncate(shmemd_, static_cast<off_t>(size)) == -1) {
		return nullptr;
	}

	size_ = size;

	address_ = mmap(addr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shmemd_, 0);

	if (address_ == MAP_FAILED) {
		address_ = nullptr;
		close();
	}

	creator_ = true;

	return address_;
}

void* CSharedMemory::open(void *addr)
{
	shmemd_ = shm_open(path_, flags_, 0644);

	if (shmemd_ == -1) {
        printf("CSharedMemory::open %s\n", strerror(errno));
		return nullptr;
	}

	struct stat buf;

	if (fstat(shmemd_, &buf) == -1) {
		close();
		return nullptr;
	}

	size_ = static_cast<decltype (size_)>(buf.st_size);

	address_ = mmap(addr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shmemd_, 0);

	if (address_ == MAP_FAILED) {
		address_ = nullptr;
		close();
	}

	return address_;
}

void CSharedMemory::close()
{
	if (shmemd_ == -1) {
		return;
	}

	if (address_ != nullptr) {
		munmap(address_, size_);
		address_ = nullptr;
	}

	::close(shmemd_);
	shmemd_ = -1;

	if (creator_) {
		shm_unlink(path_);
	}
}

std::size_t CSharedMemory::size() const
{
	return size_;
}

void* CSharedMemory::address() const
{
	return address_;
}

CSharedMemory::~CSharedMemory()
{
    close();
}
