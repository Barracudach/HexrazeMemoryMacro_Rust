#include "process.h"


processManager process_Manager;

uintptr_t processManager::GetModuleBaseAddress(const char* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processPID);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!strcmp(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

bool processManager::attach(const char* process_name) {
	PROCESSENTRY32 process_entry{};
	process_entry.dwSize = sizeof(PROCESSENTRY32);
	auto snapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL) };
	if (snapshot == INVALID_HANDLE_VALUE)
		return false;
	if (Process32First(snapshot, &process_entry)) {
		do {
			if (!strcmp(process_name, process_entry.szExeFile)) {
				processPID = process_entry.th32ProcessID;
				processNAME = process_name;
				processHANDLE = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_entry.th32ProcessID);
				CloseHandle(snapshot);
				return 1;
			}
		} while (Process32Next(snapshot, &process_entry));
	}
	CloseHandle(snapshot);
	return 0;
}


size_t processManager::read_memory(LPCVOID src_addr, LPVOID dst_addr, size_t size)
{
	size_t bytes_read = 0;
	ReadProcessMemory(processHANDLE, src_addr, dst_addr, size, &bytes_read);
	return bytes_read;
}


size_t processManager::write_memory(LPVOID dst_addr, LPCVOID src_addr, size_t size)
{
	size_t bytes_write = 0;
	return WriteProcessMemory(processHANDLE, dst_addr, src_addr, size, &bytes_write);

	return bytes_write;
}