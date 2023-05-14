#include "ClientHook.h"

ClientHook::ClientHook()
{
	memset(this, NULL, sizeof(ClientHook));
}

ClientHook::ClientHook(void* ppdwClassBase)
{
	initialize(ppdwClassBase);
}

ClientHook::~ClientHook()
{
	unhook();
}

bool ClientHook::initialize(void* ppdwClassBase)
{
	auto classBase = reinterpret_cast<PDWORD*>(ppdwClassBase);

	m_ClassBase = classBase;
	m_OldVT = *m_ClassBase;
	m_VTSize = calculateVmtLength(*m_ClassBase);

	m_NewVT = new DWORD[m_VTSize + 1];
	memcpy(&m_NewVT[1], m_OldVT, sizeof(DWORD) * m_VTSize);
	m_NewVT[0] = (uintptr_t)m_OldVT[-1];

	DWORD old;
	VirtualProtect(ppdwClassBase, sizeof(DWORD), PAGE_READWRITE, &old); // PAGE_EXECUTE_READWRITE
	*m_ClassBase = &m_NewVT[1];
	VirtualProtect(ppdwClassBase, sizeof(DWORD), old, &old);

	return true;
}

void ClientHook::unhook()
{
	if (m_ClassBase)
	{
		DWORD old;
		VirtualProtect(m_ClassBase, sizeof(DWORD), PAGE_READWRITE, &old);
		*m_ClassBase = m_OldVT;
		VirtualProtect(m_ClassBase, sizeof(DWORD), old, &old);
	}
}

void ClientHook::rehook()
{
	if (m_ClassBase) 
	{
		DWORD old;
		VirtualProtect(m_ClassBase, sizeof(DWORD), PAGE_READWRITE, &old);
		*m_ClassBase = &m_NewVT[1];
		VirtualProtect(m_ClassBase, sizeof(DWORD), old, &old);
	}
}

void ClientHook::restore()
{
	try {
		if (m_OldVT != nullptr) {
			auto guard = ProtectGuard{ m_ClassBase, sizeof(DWORD), PAGE_READWRITE };
			*(DWORD**)m_ClassBase = m_OldVT;
			m_OldVT = nullptr;
		}
	}
	catch (...) {
	}
}

DWORD ClientHook::calculateVmtLength(PDWORD pdwVMT)
{
	DWORD dwIndex = NULL;

	for (dwIndex = NULL; pdwVMT[dwIndex]; dwIndex++)
		if (IS_INTRESOURCE((FARPROC)pdwVMT[dwIndex])) 
			break;

	return dwIndex;
}
