//........HEXRAZE | From GitHub:https://github.com/Barracuda1900/HexrazeMemoryMacro_Rust

#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <iostream>
#include <fstream>

#include "process.h"
#include "mouse.h"


#pragma comment (lib, "Winmm.lib")
#pragma comment (lib, "ntdll.lib")

//Type
class WaitableTimer
{
public:

	WaitableTimer()
	{
		m_timer = ::CreateWaitableTimer(NULL, FALSE, NULL);
		if (!m_timer)
			throw std::runtime_error("Failed to create waitable time (CreateWaitableTimer), error:" + std::to_string(::GetLastError()));
	}

	~WaitableTimer()
	{
		::CloseHandle(m_timer);
		m_timer = NULL;
	}

	inline void SetAndWait(unsigned relativeTime100Ns)
	{
		LARGE_INTEGER dueTime = { 0 };
		dueTime.QuadPart = static_cast<LONGLONG>(relativeTime100Ns) * -1;

		BOOL res = ::SetWaitableTimer(m_timer, &dueTime, 0, NULL, NULL, FALSE);
		if (!res)
			throw std::runtime_error("SetAndWait: failed set waitable time (SetWaitableTimer), error:" + std::to_string(::GetLastError()));

		DWORD waitRes = ::WaitForSingleObject(m_timer, INFINITE);
		if (waitRes == WAIT_FAILED)
			throw std::runtime_error("SetAndWait: failed wait for waitable time (WaitForSingleObject)" + std::to_string(::GetLastError()));
	}

private:
	HANDLE m_timer;
};
struct Vector3 { float x, y, z; };
struct Vector2 { float x, y; };

//Prototype
extern "C" NTSYSAPI NTSTATUS NTAPI NtSetTimerResolution(ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution);
DWORD64 find_BasePlayer(DWORD64 BaseNetworkable);
inline void my_sleep(int durationMS);
inline void record(DWORD64 PlayerInput, Vector2* macro);
inline void play(Vector2* macro);
std::string get_module_path();

//Global var
#define oBaseNetworkable 50303664 // BaseNetworkable_c*
#define max_size_macro 4500 //максимальный размер макроса
int time_sleep = 5; //задержки в ms .Тут нет смысла ставить слишком маленькое значение,потому что сама игра неспособна с такой периодичностью обновлять кадры
Vector2* macro = new Vector2[max_size_macro];
WaitableTimer timer;
bool what_sleep = TRUE;//Включаем обычный Sleep если прироста от WaitableTimer нет


int main()
{

	//преднастройки
	SetProcessPriorityBoost(GetCurrentProcess(), false);
	SetThreadIdealProcessor(GetCurrentThread(), MAXIMUM_PROCESSORS);
	if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) && !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
	{
		std::cout << "Run as admin.Error: " << GetLastError() << std::endl;
		system("pause");
		return 0;
	}

	//проверка точности таймера
	timeBeginPeriod(1);
	int start = clock();
	for (int i = 0; i <= 1000; i++)
	{
		my_sleep(1);
	}
	int duration = clock() - start;
	if (duration >= 1400)what_sleep = FALSE;
	std::cout << "Test delay timer 1000=" << duration << std::endl;
	timeEndPeriod(1);


	static int act = 1;
	std::cout << "1-Record macro(Use RustClient.exe without EAC)" << std::endl;
	std::cout << "2-Load and play macro(You can play with EAC)" << std::endl;
	std::cout << "do:";
	std::cin >> act;

	if (act == 1)
	{
		process_Manager.attach("RustClient.exe");
		DWORD64 UnityPlayerBase = process_Manager.GetModuleBaseAddress("UnityPlayer.dll");
		DWORD64 GameAssemblyBase = process_Manager.GetModuleBaseAddress("GameAssembly.dll");
		DWORD64 BasePlayer = find_BasePlayer(read(GameAssemblyBase + oBaseNetworkable, DWORD64));
		DWORD64 PlayerInput = read(BasePlayer + 0x4C8, DWORD64);
		while (true)
		{
			//Vector3 angle= read(PlayerInput + 0x64, Vector3);
			//std::cout << "x:"<< angle.x<<" y:"<< angle.y<<" z:"<< angle.z<<std::endl;
			std::cout << "Record started.Shoot both in the game" << std::endl;
			while (true)
			{
				my_sleep(1);

				if (GetAsyncKeyState(0x01) && GetAsyncKeyState(0x02) & 0x8000)
				{
					record(PlayerInput, macro);
					break;
					
					//for (int i = 0; i < 100; i++)std::cout << ak47[i].x << " " << ak47[i].y << std::endl;
					
				}
			}
			act = 1;
			std::cout << "1-Save" << std::endl;
			std::cout << "2-Record again" << std::endl;
			std::cout << "do:";
			std::cin >> act;
			if (act == 1)
			{
				std::string name = "";
				std::cout << "Input name file(example ak47):";
				std::cin >> name;
				std::string path_and_name = get_module_path() + name + ".hmm";

				std::ofstream out; //результат
				out.open(path_and_name);
				out << time_sleep << std::endl;

				for (int i = 0; (i <= max_size_macro) /*|| (macro[i].x != -100.0f && macro[i].y != -100.0f)*/; i++)
				{
					//убираем первые нули
					static bool is_optimized = FALSE;
					if (macro[i].x == 0.0f && macro[i].y == 0.0f && !is_optimized)continue;
					else is_optimized = TRUE;

					out << macro[i].x << " " << macro[i].y << " " << std::endl;
					if (macro[i].x == -100.0f && macro[i].y == -100.0f)	break;
				}
				out.close();
				std::cout << "Success save" << std::endl;
				system("pause");
				return 0;
			}
			else continue;
		}
	}
	else if (act == 2)
	{
		//загрузка
		{
			std::string macroName = "";
			std::cout << "Input name file(example ak47):" << std::endl;
			std::cin >> macroName;
			macroName += ".hmm";
			std::ifstream in(macroName);
			if (!in.is_open())
			{
				std::cout << "File not exist..." << std::endl;
				system("pause");
				return 1;
			}

			std::string _sleep;
			getline(in, _sleep);
			time_sleep = atoi(_sleep.c_str());

			std::string x, y;
			for (int i = 0; (i <= max_size_macro) && (getline(in, x, ' ') && getline(in, y, ' ')); i++)
			{
				macro[i].x = std::stof(x);
				macro[i].y = std::stof(y);
			}

		}
		std::cout << "Loaded" << std::endl;
		std::cout << "Use VK_UP and VK_DOWN to change coefficient" << std::endl;
		static float coeff = 18.7f;
		set_game_param(1, 90.0);
		set_macro_param(1, 90.0);
		set_aim_param(coeff);
		while (true)
		{
			my_sleep(1);


			
			if (GetAsyncKeyState(VK_UP) & 0x8000)
			{
				coeff += 0.1f;
				set_aim_param(coeff);
				std::cout << "coeff: " << coeff << std::endl;
				Sleep(80);
			}
			if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			{
				coeff -= 0.1f;
				set_aim_param(coeff);
				std::cout << "coeff: " << coeff << std::endl;
				Sleep(80);
			}


			//no mode
			if (GetAsyncKeyState(VK_NUMPAD1))
			{
				set_aim_param(coeff);
				std::cout << "coeff: " << coeff << std::endl;
				Sleep(800);
			}
			//8x
			if (GetAsyncKeyState(VK_NUMPAD2))
			{
				set_aim_param(coeff * 3.8636f);
				std::cout << "coeff: " << coeff * 1.2045f << std::endl;
				Sleep(800);
			}
			//16x
			if (GetAsyncKeyState(VK_NUMPAD3))
			{
				set_aim_param(coeff * 7.6818f);
				std::cout << "coeff: " << coeff * 7.6818f << std::endl;
				Sleep(800);
			}
			//golograf
			if (GetAsyncKeyState(VK_NUMPAD4))

			{
				set_aim_param(coeff * 1.2045f);
				std::cout << "coeff: " << coeff * 1.2045f << std::endl;
				Sleep(800);
			}
			//samod
			if (GetAsyncKeyState(VK_NUMPAD5))
			{
				set_aim_param(coeff * 0.8030f);
				std::cout << "coeff: " << coeff * 0.8030f << std::endl;
				Sleep(800);
			}
			

			if (GetAsyncKeyState(0x01) && GetAsyncKeyState(0x02) & 0x8000)play(macro);


		}
	}


	return 0;
}



inline void my_sleep(int durationMS)
{
	if (what_sleep)timer.SetAndWait(durationMS);
	else Sleep(durationMS);
	//int start = clock(); while (clock() - start <= durationMS){}
}

inline void record(DWORD64 PlayerInput, Vector2* macro)
{
	timeBeginPeriod(1);
	Vector2 old_angle = { 0.0f,0.0f };

	for (int i = 0; i < max_size_macro; i++)
	{
		my_sleep(time_sleep);
		Vector3 angle = read(PlayerInput + 0x64, Vector3);

		if (GetAsyncKeyState(0x01) & 0x8000 || angle.x != 0.0f)
		{
			macro[i].x = old_angle.y - angle.y;
			macro[i].y = old_angle.x - angle.x;

			old_angle.x = angle.x;
			old_angle.y = angle.y;

		}
		else
		{
			macro[i].x = -100.0f;
			macro[i].y = -100.0f;
			break;
		}

	}
	std::cout << "record success" << std::endl;
	timeEndPeriod(1);
}

inline void play(Vector2* macro)
{
	timeBeginPeriod(1);
	for (int i = 0; (i < max_size_macro) && (macro[i].y != -100.0f && macro[i].x != -100.0f) && (GetAsyncKeyState(0x01) & 0x8000); i++)
	{
		//std::cout <<"move: "<< ak47[i].x <<"  "<< ak47[i].y<< std::endl;

		my_sleep(time_sleep);
		mousemove(macro[i].x, macro[i].y);
	}
	while (GetAsyncKeyState(0x01) && GetAsyncKeyState(0x02) & 0x8000)
	{
		Sleep(1);
	}
	timeEndPeriod(1);
	zero_accumulation();
}

DWORD64 find_BasePlayer(DWORD64 BaseNetworkable)
{
	DWORD64 EntityRealm = read(BaseNetworkable + 0xB8, DWORD64);
	DWORD64 ClientEntities = read(EntityRealm, DWORD64);
	DWORD64 ClientEntities_list = read(ClientEntities + 0x10, DWORD64);
	DWORD64 ClientEntities_values = read(ClientEntities_list + 0x28, DWORD64);

	if (!ClientEntities_values) return NULL;
	int EntityCount = read(ClientEntities_values + 0x10, int);

	DWORD64 EntityBuffer = read(ClientEntities_values + 0x18, DWORD64);


	for (int i = 0; i < EntityCount; i++)
	{
		DWORD64 Entity = read(EntityBuffer + 0x20 + (i * 0x8), DWORD64);//BaseEntity
		if (Entity <= 100000) continue;

		DWORD64 Object = read(Entity + 0x10, DWORD64);
		if (Object <= 100000) continue;

		DWORD64 ObjectClass = read(Object + 0x30, DWORD64); //CGameObject
		if (ObjectClass <= 100000) continue;

		DWORD64 addr_name = read(ObjectClass + 0x60, DWORD64);

		static char BNname[110] = { '\0' }; //имя текущего обьекта 
		process_Manager.read_memory((LPCVOID)addr_name, (LPVOID)BNname, sizeof(BNname));
		BNname[109] = { '\0' };


		if (strstr(BNname, "LocalPlayer"))
		{
			return Entity;
			//std::cout << std::hex<< Entity <<std::endl;
		}
	}
	return NULL;
}

std::string get_module_path()
{
	char path_and_name[MAX_PATH];
	char path[MAX_PATH];
	char name[MAX_PATH];
	GetModuleFileName(NULL, path_and_name, sizeof(path_and_name) / sizeof(path_and_name[0]));

	int last_slash = 0;
	for (int i = 0; i < MAX_PATH; i++)
	{
		if (path_and_name[i] == '\\' || path_and_name[i] == '/')last_slash = i;
		if (path_and_name[i] == '\0') break;
	}
	strcpy(name, &path_and_name[last_slash + 1]);


	for (int i = 0; i <= last_slash; i++)
	{
		path[i] = path_and_name[i];
	}
	path[last_slash + 1] = '\0';

	return path;
}
