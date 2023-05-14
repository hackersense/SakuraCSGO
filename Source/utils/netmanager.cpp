#include "netmanager.hpp"

#include "..\includes.hpp"

int netvars::get_offset(const char* tableName, const char* propName) 
{
	int offset = get_prop(tableName, propName);

	if (!offset)
		return 0;

	return offset;
}

int netvars::get_prop(const char* tableName, const char* propName, RecvProp** prop) 
{
	RecvTable* recvTable = this->get_table(tableName);

	if (!recvTable)
		return 0;

	int offset = get_prop(recvTable, propName, prop);

	if (!offset)
		return 0;

	return offset;
}

int netvars::get_prop(RecvTable* recvTable, const char* propName, RecvProp** prop) 
{
	int extraOffsets = 0;

	for (int i = 0; i < recvTable->propCount; i++) 
	{
		auto* recvProp = &recvTable->props[i];
		auto recvChild = recvProp->dataTable;

		if (recvChild && (recvChild->propCount > 0)) 
		{
			int tmp = get_prop(recvChild, propName, prop);

			if (tmp)
                extraOffsets += (recvProp->offset + tmp);
		}

		if (strcmp(recvProp->name, propName) != 0)
			continue;

		if (prop)
			*prop = recvProp;

		return (recvProp->offset + extraOffsets);
	}

	return extraOffsets;
}

RecvTable* netvars::get_table(const char* tableName) 
{
	if (tables.empty())
		return nullptr;

	for (auto& table : tables) 
		if (!strcmp(table.first.c_str(), tableName))
			return table.second;

	return nullptr;
}