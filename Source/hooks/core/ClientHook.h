#pragma once

#include <Windows.h>
#include <cstdint>
#include <stdexcept>
#include <assert.h>

class ProtectGuard
{
public:
	ProtectGuard(void* base, uint32_t length, uint32_t protect)
	{
		this->base = base;
		this->length = length;

		if (!VirtualProtect(base, length, protect, (PDWORD)&old_protect))
			throw std::runtime_error("Failed to protect region.");
	}

	~ProtectGuard()
	{
		VirtualProtect(base, length, old_protect, (PDWORD)&old_protect);
	}
private:
	void*		base;
	uint32_t	length;
	uint32_t	old_protect;
};

class ClientHook
{
public:
	ClientHook();

	ClientHook(void* ppdwClassBase);

	~ClientHook();

	bool initialize(void* ppdwClassBase);

	void unhook();

	void rehook();

	void restore();

	template<typename T>
	constexpr T getOriginal(int index) const noexcept
	{
		if (index >= 0 && index <= (int)m_VTSize && m_OldVT != NULL)
			return reinterpret_cast<T>(m_OldVT[index]);

		return NULL;
	}

	template <typename T>
	constexpr DWORD hookAt(size_t index, T fun) const noexcept
	{
		if (m_NewVT && m_OldVT && index <= m_VTSize)
		{
			m_NewVT[index + 1] = reinterpret_cast<uintptr_t>(fun);
			return m_OldVT[index];
		}

		return NULL;
	}
private:
	DWORD calculateVmtLength(PDWORD pdwVMT);

	PDWORD*	m_ClassBase;
	PDWORD	m_NewVT, m_OldVT;
	DWORD	m_VTSize;
};
