#pragma once

class IKeyValuesSystem {
public:
	virtual ~IKeyValuesSystem() = 0;

	// registers the size of the KeyValues in the specified instance
	// so it can build a properly sized memory pool for the KeyValues objects
	// the sizes will usually never differ but this is for versioning safety
	virtual void RegisterSizeofKeyValues(int size) = 0;

	// allocates/frees a KeyValues object from the shared mempool
	virtual void* AllocKeyValuesMemory(int size) = 0;
	virtual void FreeKeyValuesMemory(void* pMem) = 0;

	// symbol table access (used for key names)
	virtual int GetSymbolForString(const char* name, bool bCreate = true) = 0;
	virtual const char* GetStringForSymbol(int symbol) = 0;

	// for debugging, adds KeyValues record into global list so we can track memory leaks
	virtual void AddKeyValuesToMemoryLeakList(void* pMem, int name) = 0;
	virtual void RemoveKeyValuesFromMemoryLeakList(void* pMem) = 0;

	// set/get a value for keyvalues resolution symbol
	// e.g.: SetKeyValuesExpressionSymbol( "LOWVIOLENCE", true ) - enables [$LOWVIOLENCE]
	virtual void SetKeyValuesExpressionSymbol(const char* name, bool bValue) = 0;
	virtual bool GetKeyValuesExpressionSymbol(const char* name) = 0;

	// symbol table access from code with case-preserving requirements (used for key names)
	virtual int GetSymbolForStringCaseSensitive(int& hCaseInsensitiveSymbol, const char* name, bool bCreate = true) = 0;
};