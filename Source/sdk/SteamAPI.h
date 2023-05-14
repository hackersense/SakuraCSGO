#pragma once
#include <iostream>

template<typename FuncType>
__forceinline static FuncType CallVFunction(void* ppClass, int index)
{
	int* pVTable = *(int**)ppClass;
	int dwAddress = pVTable[index];
	return (FuncType)(dwAddress);
}

enum class SteamResult
{
	OK = 0,
	NoMessage = 1,           // There is no message in the queue
	BufferTooSmall = 2,      // The buffer is too small for the requested message
	NotLoggedOn = 3,         // The client is not logged onto Steam
	InvalidMessage = 4,      // Something was wrong with the message being sent with SendMessage
};

class SteamGameCoordinator
{
public:
	virtual SteamResult sendMessage(int unMsgType, const void* pubData, int cubData) = 0;
	virtual bool isMessageAvailable(int* pcubMsgSize) = 0;
	virtual SteamResult retrieveMessage(int* punMsgType, void* pubDest, int cubDest, int* pcubMsgSize) = 0;
};

class SteamAccountID
{
public:
	SteamAccountID()
	{
		steamid.comp.unAccountID = 0;
		steamid.comp.accountType = 0;
		steamid.comp.universe = 0;
		steamid.comp.unAccountInstance = 0;
	}
	uint32_t getAccountID() const { return steamid.comp.unAccountID; }

private:
	union SteamID
	{
		struct SteamIDComponent
		{
			uint32_t			unAccountID : 32;			// unique account identifier
			unsigned int		unAccountInstance : 20;	// dynamic instance ID (used for multiseat type accounts only)
			unsigned int		accountType : 4;			// type of account - can't show as EAccountType, due to signed / unsigned difference
			int					universe : 8;	// universe this account belongs to
		} comp;

		uint64_t unAll64Bits;
	} steamid;
};

class SteamUser
{
public:
	virtual uint32_t getSteamUser() = 0;
	virtual bool loggedOn() = 0;
	virtual SteamAccountID getSteamID() = 0;
};

class SteamClient
{
public:
	SteamUser* getUser(void* user, void* pipe, const char* pchVersion)
	{
		typedef SteamUser* (__stdcall* func)(void*, void*, const char*);
		return CallVFunction<func>(this, 5)(user, pipe, pchVersion);
	}

	SteamGameCoordinator* getGenericInterface(void* user, void* pipe, const char* pchVersion)
	{
		typedef SteamGameCoordinator* (__stdcall* func)(void*, void*, const char*);
		return CallVFunction<func>(this, 12)(user, pipe, pchVersion);
	}
};
