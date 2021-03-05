//........HEXRAZE | From GitHub:https://github.com/Barracuda1900/HexrazeMemoryMacro_Rust
//........04.03.2021 C++ ISO 14
#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

class processManager
{
public:
	std::string processNAME;
	HANDLE processHANDLE;
	uint32_t processPID;


	uintptr_t GetModuleBaseAddress(const char* modName);
	bool attach(const char* process_name);

	size_t read_memory(LPCVOID src_addr, LPVOID dst_addr, size_t size);
	size_t write_memory(LPVOID dst_addr, LPCVOID src_addr, size_t size);

	template <typename type>
	type readmem(uint64_t src)
	{
		type ret = {};
		size_t bytes_read = 0;
		//std::cout << (LPVOID)&ret << " " << (type) *(&ret)<<std::endl;
		ReadProcessMemory(processHANDLE, (LPCVOID)src, (LPVOID)&ret, sizeof(type), &bytes_read);
		return ret;
	}

	template <typename type>
	void writemem(uint64_t dst_addr, type var)
	{
		size_t bytes_write = 0;
		WriteProcessMemory(processHANDLE, (LPVOID)&var, (LPCVOID)dst_addr, sizeof(type), &bytes_write);
	}
};

extern processManager process_Manager;

#define read(src_addr,type)  process_Manager.readmem<type>(src_addr)
#define write(dst_addr,var,type)  process_Manager.writemem<type>(dst_addr,var)