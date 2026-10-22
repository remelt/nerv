#pragma once
#include <sdk/vfunc/vfunc.hpp>
#include <cstdint>

class i_network_string_dict
{
public:
	unsigned int FindStringIndex(const char* PathToVnmSkel)
	{
		return VFUNC(this, unsigned int, 6, PathToVnmSkel);
	}
};

class i_network_string_table
{
public:

	int64_t AddString(bool bIsServer, const char* PathToVnmSkel,void* userdata)
	{
		return VFUNC(this, int64_t, 7, bIsServer, PathToVnmSkel, userdata);
	}

	char pad_0000[120]; //0x0000
	i_network_string_dict* m_network_string_dict; //0x0078

};