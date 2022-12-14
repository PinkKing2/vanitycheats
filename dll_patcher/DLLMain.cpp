#include <Windows.h>
#include <iostream>
#include <cstdint>
#include <intrin.h>
#include <wininet.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <format>
#include <nlohmann/json.hpp>

#pragma comment(lib, "wininet.lib")

#include "bins.hpp"
#include "min_hook.hpp"

using v_connect_to_server = void*(__cdecl*)(void* actual_config, const char* type_name, const char* command, const char* name, const char* config);
v_connect_to_server o_connect_to_server = 0;

//#pragma optimize( "", off )

__declspec(noinline) bool load_config(void* address, std::string type_name, const char* name)
{
	std::string sz_name = std::string(name, strlen(name));
	std::cout << "[+] load " << sz_name << std::endl;
	std::string get_line = "";
	std::string load = "";
	std::ifstream rfile;
	rfile.open(std::format("C:\\vanity_configs_new\\{}_{}", type_name, sz_name), std::ios::out);
	if (rfile.is_open())
	{
		while (std::getline(rfile, get_line))
		{
			load += std::string(get_line + "\n");
		}
		rfile.close();
		load.pop_back();
	}
	nlohmann::json testing = nlohmann::json::parse(load);
	//const char* loaded_config = load.data();
	std::string loaded_config = testing.dump();
	__asm
	{
		mov ecx, [address]
		lea eax, loaded_config
		push eax
		mov ebx, 0x45FBC1D0
		call ebx
		lea ecx, [loaded_config]
		mov ebx, 0x45FC0740
		call ebx
	}
	return true;
}

__declspec(noinline) bool save_config(const char* type_name, const char* name, const char* config)
{
	std::string sz_name = std::string(name, strlen(name));
	std::string sz_type_name = std::string(type_name, strlen(type_name));
	std::cout << "[+] save " << sz_name << std::endl;
	std::ofstream file;
	file.open(std::format("C:\\vanity_configs_new\\{}_{}", sz_type_name, sz_name));
	file << config;
	file.close();
	return true;
}

void* __cdecl hooked_connect_to_server(void* actual_config, const char* type_name, const char* command, const char* name, const char* config)
{
	//std::cout << type_name << "->" << command << std::endl;
	std::string sz_command = std::string(command, strlen(command));
	std::string sz_type_name = std::string(type_name, strlen(type_name));

	if (strcmp(sz_command.data(), "fetch") == 0)
	{
		std::cout << "[+] init " << sz_type_name << std::endl;
		std::string get_line = "";
		std::string configs = "";
		std::ifstream rfile;
		rfile.open(std::format("C:\\vanity_configs_new\\{}.txt", sz_type_name), std::ios::out);
		if (rfile.is_open())
		{
			while (std::getline(rfile, get_line))
			{
				configs += std::string(get_line + "\n");
			}
			rfile.close();
			configs.pop_back();
		}
		const char* config2 = configs.data();
		__asm
		{
			mov ecx, [actual_config]
			lea eax, config2
			push eax
			mov ebx, 0x45FBC1D0
			call ebx
			lea ecx, [config2]
			mov ebx, 0x45FC0740
			call ebx
		}
	}
	else if (strcmp(sz_command.data(), "load") == 0)
	{
		load_config(actual_config, sz_type_name, name);
	}
	else if (strcmp(sz_command.data(), "save") == 0)
	{
		save_config(type_name, name, config);
	}
	return actual_config;
}

//#pragma optimize( "", on )

SOCKET sock = 0;

PVOID o_connect = NULL;
int __stdcall hooked_connect(SOCKET s, const struct sockaddr* name, int namelen) //Don't allow actual connection to the server lol
{
	static auto fn_connect = static_cast<decltype(&hooked_connect)>(o_connect);

	sockaddr_in* adr = (sockaddr_in*)name;
	if (adr->sin_addr.s_addr == 1657217606) // "70.34.199.98"
	{
		sock = s;
		printf("[+] Cheat tried to connect to server... (DENIED)\n");
		return static_cast<SOCKET>(0xDEAD); // 57005
	}

	return fn_connect(s, name, namelen);
}

void main()
{
	printf("[+] vanitycheats.xyz crack (26.01.2022 build)\n");
	printf("[+] allocating mem...\n");
	LPVOID hack_address = VirtualAlloc(reinterpret_cast<LPVOID>(0x45F50000), 0x768000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!hack_address)
	{
		printf("[-] failed to allocate memory (restart game and instantly inject)\n");
		Sleep(3000);
		TerminateProcess(reinterpret_cast<HANDLE>(-1), 0);
	}

	printf("[+] allocated mem!\n");
	printf("[+] creating files...\n");

	if (!std::filesystem::exists("C:\\vanity_configs_new"))
		std::filesystem::create_directory("C:\\vanity_configs_new");

	printf("[+] created files!\n");
	printf("[+] waiting for serverbrowser.dll ...\n");
	while (!GetModuleHandleA("serverbrowser.dll")) Sleep(100);
	printf("[+] found serverbrowser.dll !\n");

	printf("[+] writing file...\n");
	memcpy(hack_address, hack_bin, sizeof(hack_bin));
	printf("[+] wrote file!\n");

	printf("[+] patching...\n");
	std::vector<uint8_t> patch = { 0x90, 0x90, 0x90, 0x90, 0x90 };
	memcpy(reinterpret_cast<void*>(0x461731DF), patch.data(), patch.size());
	printf("[+] patched security!\n");
	memcpy(reinterpret_cast<void*>(0x461B287D), patch.data(), patch.size());
	printf("[+] patched cloud_configs::load_default_settings()\n");
	uint32_t internet = reinterpret_cast<uint32_t>(VirtualAlloc(0, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	std::string pfp = "https://kibbewater.xyz/vc/a.php";
	memcpy(reinterpret_cast<void*>(internet), pfp.data(), pfp.length());

	int place = pfp.length() + 2;

	std::vector<uint8_t> patch3 = { 0xBA, 0xEF, 0xBE, 0xAD, 0xDE, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	*reinterpret_cast<uint32_t*>(patch3.data() + 0x1) = internet;

	memcpy(reinterpret_cast<void*>(0x460A258B), patch3.data(), patch3.size());

	std::string weapon_skins = "https://kibbewater.xyz/vc/w.txt";
	memcpy(reinterpret_cast<void*>(internet + place), weapon_skins.data(), weapon_skins.length());

	std::vector<uint8_t> patch7 = { 0xBA, 0xEF, 0xBE, 0xAD, 0xDE, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90 };
	*reinterpret_cast<uint32_t*>(patch7.data() + 0x1) = internet + place;

	memcpy(reinterpret_cast<void*>(0x461B2B8B), patch7.data(), patch7.size());

	place += weapon_skins.length() + 2;

	std::string agent = "https://kibbewater.xyz/vc/a.txt";

	memcpy(reinterpret_cast<void*>(internet + place), agent.data(), agent.length());

	std::vector<uint8_t> patch8 = { 0xB8, 0xEF, 0xBE, 0xAD, 0xDE, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	*reinterpret_cast<uint32_t*>(patch8.data() + 0x1) = internet + place;
	
	memcpy(reinterpret_cast<void*>(0x461BE31E), patch8.data(), patch8.size());

	place += agent.length() + 2;

	std::vector<uint8_t> patch9 = { 0xBA, 0xEF, 0xBE, 0xAD, 0xDE, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

	std::string sticker = "https://kibbewater.xyz/vc/s.txt";

	memcpy(reinterpret_cast<void*>(internet + place), sticker.data(), sticker.length());

	std::vector<uint8_t> patch10 = { 0xBA, 0xEF, 0xBE, 0xAD, 0xDE, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	*reinterpret_cast<uint32_t*>(patch10.data() + 0x1) = internet + place;

	memcpy(reinterpret_cast<void*>(0x461B7CA5), patch10.data(), patch10.size());

	place += sticker.length() + 2;

	std::string	knife = "https://kibbewater.xyz/vc/k.txt";

	memcpy(reinterpret_cast<void*>(internet + place), knife.data(), knife.length());

	std::vector<uint8_t> patch11 = { 0xBA, 0xEF, 0xBE, 0xAD, 0xDE, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	*reinterpret_cast<uint32_t*>(patch11.data() + 0x1) = internet + place;

	memcpy(reinterpret_cast<void*>(0x461B5459), patch11.data(), patch11.size());

	place += knife.length() + 2;

	std::string	glove = "https://kibbewater.xyz/vc/g.txt";

	memcpy(reinterpret_cast<void*>(internet + place), glove.data(), glove.length());

	std::vector<uint8_t> patch12 = { 0xBA, 0xEF, 0xBE, 0xAD, 0xDE, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	*reinterpret_cast<uint32_t*>(patch12.data() + 0x1) = internet + place;

	memcpy(reinterpret_cast<void*>(0x461B94F5), patch12.data(), patch12.size());

	printf("[+] patched internet connection!\n");
	std::vector<uint8_t> patch2 = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	memcpy(reinterpret_cast<void*>(0x45FCCDC4), patch2.data(), patch2.size());
	std::vector<uint8_t> patch6 = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	memcpy(reinterpret_cast<void*>(0x45FCC21E), patch6.data(), patch6.size());
	printf("[+] patched crashing code!\n");
	std::vector<uint8_t> patch4 = { 0xE9, 0x38, 0x01, 0x00, 0x00, 0x90 };
	memcpy(reinterpret_cast<void*>(0x460A2041), patch4.data(), patch4.size());
	std::vector<uint8_t> patch5 = { 0x50, 0x69, 0x6E, 0x6B, 0x4B, 0x69, 0x6E, 0x67 };
	memcpy(reinterpret_cast<void*>(0x4648902C), patch5.data(), patch5.size());
	printf("[+] patched username!\n");
	printf("[+] patched!\n");
	printf("[+] creating hooks...\n");
	if (MH_Initialize() != MH_OK)
	{
		printf("[-] failed to initialize minhook\n");
		return;
	}

#if 1
	if (MH_CreateHook(connect, &hooked_connect, &o_connect) != MH_OK)
	{
		printf("[-] failed to create hook at connect\n");
		return;
	}
#endif

#if 1
	if (MH_CreateHook(reinterpret_cast<LPVOID>(0x45FC82C0), hooked_connect_to_server, (void**)&o_connect_to_server) != MH_OK)
	{
		printf("[-] failed to create hook at connect_to_server\n");
		return;
	}
#endif

	//printf("[+] created hook at cloud_configs::process_configs()\n");

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
	{
		printf("[-] failed to enable hooks\n");
		return;
	}
	printf("[+] created hooks!\n");

	printf("[+] allocating mem for shellcode...\n");
	std::vector<uint8_t> shellcode =
	{
		0x00, 0x00, 0xF5, 0x45, 0x40, 0x01, 0xF5, 0x45, 0x00, 0x70, 0x69, 0x46,
		0x4C, 0x6F, 0x48, 0x46, 0xD0, 0x0B, 0x29, 0x75, 0x50, 0xF5, 0x28, 0x75,
		0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x4C, 0x8B, 0x45, 0x08, 0x89, 0x45, 0xFC,
		0x8B, 0x4D, 0xFC, 0x8B, 0x51, 0x08, 0x89, 0x55, 0xF8, 0x8B, 0x45, 0xFC,
		0x8B, 0x48, 0x04, 0x8B, 0x55, 0xFC, 0x8B, 0x02, 0x2B, 0x41, 0x34, 0x89,
		0x45, 0xCC, 0x8B, 0x4D, 0xF8, 0x83, 0x39, 0x00, 0x0F, 0x84, 0x87, 0x00,
		0x00, 0x00, 0x8B, 0x55, 0xF8, 0x83, 0x7A, 0x04, 0x08, 0x72, 0x6D, 0x8B,
		0x45, 0xF8, 0x8B, 0x48, 0x04, 0x83, 0xE9, 0x08, 0xD1, 0xE9, 0x89, 0x4D,
		0xD0, 0x8B, 0x55, 0xF8, 0x83, 0xC2, 0x08, 0x89, 0x55, 0xE0, 0xC7, 0x45,
		0xEC, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x09, 0x8B, 0x45, 0xEC, 0x83, 0xC0,
		0x01, 0x89, 0x45, 0xEC, 0x8B, 0x4D, 0xEC, 0x3B, 0x4D, 0xD0, 0x7D, 0x3C,
		0x8B, 0x55, 0xEC, 0x8B, 0x45, 0xE0, 0x0F, 0xB7, 0x0C, 0x50, 0x85, 0xC9,
		0x74, 0x2C, 0x8B, 0x55, 0xEC, 0x8B, 0x45, 0xE0, 0x0F, 0xB7, 0x0C, 0x50,
		0x81, 0xE1, 0xFF, 0x0F, 0x00, 0x00, 0x8B, 0x55, 0xF8, 0x8B, 0x02, 0x03,
		0xC1, 0x8B, 0x4D, 0xFC, 0x03, 0x01, 0x89, 0x45, 0xDC, 0x8B, 0x55, 0xDC,
		0x8B, 0x02, 0x03, 0x45, 0xCC, 0x8B, 0x4D, 0xDC, 0x89, 0x01, 0xEB, 0xB3,
		0x8B, 0x55, 0xF8, 0x8B, 0x45, 0xF8, 0x03, 0x42, 0x04, 0x89, 0x45, 0xF8,
		0xE9, 0x6D, 0xFF, 0xFF, 0xFF, 0x8B, 0x4D, 0xFC, 0x8B, 0x51, 0x0C, 0x89,
		0x55, 0xF0, 0x8B, 0x45, 0xF0, 0x83, 0x38, 0x00, 0x0F, 0x84, 0xEF, 0x00,
		0x00, 0x00, 0x8B, 0x4D, 0xFC, 0x8B, 0x11, 0x8B, 0x45, 0xF0, 0x03, 0x10,
		0x89, 0x55, 0xF4, 0x8B, 0x4D, 0xFC, 0x8B, 0x11, 0x8B, 0x45, 0xF0, 0x03,
		0x50, 0x10, 0x89, 0x55, 0xE8, 0x8B, 0x4D, 0xFC, 0x8B, 0x51, 0x10, 0x89,
		0x55, 0xC8, 0x8B, 0x45, 0xFC, 0x8B, 0x08, 0x8B, 0x55, 0xF0, 0x03, 0x4A,
		0x0C, 0x51, 0xFF, 0x55, 0xC8, 0x89, 0x45, 0xE4, 0x83, 0x7D, 0xE4, 0x00,
		0x75, 0x07, 0x33, 0xC0, 0xE9, 0xE3, 0x00, 0x00, 0x00, 0x8B, 0x45, 0xF4,
		0x83, 0x38, 0x00, 0x0F, 0x84, 0x92, 0x00, 0x00, 0x00, 0x8B, 0x4D, 0xF4,
		0x8B, 0x11, 0x81, 0xE2, 0x00, 0x00, 0x00, 0x80, 0x74, 0x35, 0x8B, 0x45,
		0xFC, 0x8B, 0x48, 0x14, 0x89, 0x4D, 0xC4, 0x8B, 0x55, 0xF4, 0x8B, 0x02,
		0x25, 0xFF, 0xFF, 0x00, 0x00, 0x50, 0x8B, 0x4D, 0xE4, 0x51, 0xFF, 0x55,
		0xC4, 0x89, 0x45, 0xD8, 0x83, 0x7D, 0xD8, 0x00, 0x75, 0x07, 0x33, 0xC0,
		0xE9, 0x9F, 0x00, 0x00, 0x00, 0x8B, 0x55, 0xE8, 0x8B, 0x45, 0xD8, 0x89,
		0x02, 0xEB, 0x39, 0x8B, 0x4D, 0xFC, 0x8B, 0x11, 0x8B, 0x45, 0xF4, 0x03,
		0x10, 0x89, 0x55, 0xC0, 0x8B, 0x4D, 0xFC, 0x8B, 0x51, 0x14, 0x89, 0x55,
		0xBC, 0x8B, 0x45, 0xC0, 0x83, 0xC0, 0x02, 0x50, 0x8B, 0x4D, 0xE4, 0x51,
		0xFF, 0x55, 0xBC, 0x89, 0x45, 0xD4, 0x83, 0x7D, 0xD4, 0x00, 0x75, 0x04,
		0x33, 0xC0, 0xEB, 0x64, 0x8B, 0x55, 0xE8, 0x8B, 0x45, 0xD4, 0x89, 0x02,
		0x8B, 0x4D, 0xF4, 0x83, 0xC1, 0x04, 0x89, 0x4D, 0xF4, 0x8B, 0x55, 0xE8,
		0x83, 0xC2, 0x04, 0x89, 0x55, 0xE8, 0xE9, 0x62, 0xFF, 0xFF, 0xFF, 0x8B,
		0x45, 0xF0, 0x83, 0xC0, 0x14, 0x89, 0x45, 0xF0, 0xE9, 0x05, 0xFF, 0xFF,
		0xFF, 0x8B, 0x4D, 0xFC, 0x8B, 0x51, 0x04, 0x83, 0x7A, 0x28, 0x00, 0x74,
		0x26, 0x8B, 0x45, 0xFC, 0x8B, 0x48, 0x04, 0x8B, 0x55, 0xFC, 0x8B, 0x02,
		0x03, 0x41, 0x28, 0x89, 0x45, 0xB8, 0x8B, 0x4D, 0xB8, 0x89, 0x4D, 0xB4,
		0x6A, 0x00, 0x6A, 0x01, 0x8B, 0x55, 0xFC, 0x8B, 0x02, 0x50, 0xFF, 0x55,
		0xB4, 0xEB, 0x05, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x8B, 0xE5, 0x5D, 0xC2,
		0x04, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x83,
		0xEC, 0x10, 0x8B, 0x45, 0x0C, 0x50, 0xE8, 0xD1, 0xB8, 0x05, 0x00, 0x83,
		0xC4, 0x04, 0x89, 0x45, 0xF0, 0xC7, 0x45, 0xF4, 0x00, 0x00, 0x00, 0x00,
		0xEB, 0x09, 0x8B, 0x4D, 0xF4, 0x83, 0xC1, 0x01, 0x89, 0x4D, 0xF4, 0x8B,
		0x55, 0xF4, 0x3B, 0x55, 0x14, 0x7D, 0x60, 0xC6, 0x45, 0xFF, 0x01, 0xC7,
		0x45, 0xF8, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x09, 0x8B, 0x45, 0xF8, 0x83,
		0xC0, 0x01, 0x89, 0x45, 0xF8, 0x8B, 0x4D, 0xF8, 0x3B, 0x4D, 0xF0, 0x7D,
		0x30, 0x8B, 0x55, 0x0C, 0x03, 0x55, 0xF8, 0x0F, 0xBE, 0x02, 0x83, 0xF8,
		0x3F, 0x74, 0x20, 0x8B, 0x4D, 0x08, 0x03, 0x4D, 0xF8, 0x0F, 0xBE, 0x11,
		0x8B, 0x45, 0x10, 0x03, 0x45, 0xF4, 0x8B, 0x4D, 0xF8, 0x0F, 0xBE, 0x04,
		0x08, 0x3B, 0xD0, 0x74, 0x06, 0xC6, 0x45, 0xFF, 0x00, 0xEB, 0x02, 0xEB,
		0xBF, 0x0F, 0xB6, 0x4D, 0xFF, 0x85, 0xC9, 0x74, 0x08, 0x8B, 0x45, 0x10,
		0x03, 0x45, 0xF4, 0xEB, 0x04, 0xEB, 0x8F, 0x33, 0xC0, 0x8B, 0xE5, 0x5D,
		0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x40, 0xC7, 0x45, 0xEC, 0x00, 0x00, 0x00,
		0x00, 0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x45, 0xCC, 0x00,
		0x10, 0x00, 0x00, 0x6A, 0x1C, 0x8D, 0x45, 0xC0, 0x50, 0x8B, 0x4D, 0x10,
		0x51, 0x8B, 0x55, 0x18, 0x52, 0x55, 0xE8, 0x30, 0xE3, 0x3E, 0x00, 0x8B,
		0x45, 0x10, 0x89, 0x45, 0xF8, 0xEB, 0x09, 0x8B, 0x4D, 0xF8, 0x03, 0x4D,
		0xCC, 0x89, 0x4D, 0xF8, 0x8B, 0x55, 0x10, 0x03, 0x55, 0x14, 0x39, 0x55,
		0xF8, 0x0F, 0x83, 0xD6, 0x00, 0x00, 0x00, 0x6A, 0x1C, 0x8D, 0x45, 0xC0,
		0x50, 0x8B, 0x4D, 0xF8, 0x51, 0x8B, 0x55, 0x18, 0x52, 0xE8, 0xCF, 0x56,
		0x22, 0x00, 0xC3, 0x85, 0xC0, 0x75, 0x02, 0xEB, 0xCE, 0x81, 0x7D, 0xD0,
		0x00, 0x10, 0x00, 0x00, 0x75, 0x06, 0x83, 0x7D, 0xD4, 0x01, 0x75, 0x02,
		0xEB, 0xBD, 0x8B, 0x45, 0xFC, 0x89, 0x45, 0xE8, 0x8B, 0x4D, 0xE8, 0x51,
		0xE8, 0x7C, 0xF0, 0x02, 0x00, 0x83, 0xC4, 0x04, 0x8B, 0x55, 0xCC, 0x52,
		0xE8, 0x02, 0xF1, 0x02, 0x00, 0x83, 0xC4, 0x04, 0x89, 0x45, 0xE4, 0x8B,
		0x45, 0xE4, 0x89, 0x45, 0xFC, 0x8D, 0x4D, 0xF4, 0x51, 0x6A, 0x40, 0x8B,
		0x55, 0xCC, 0x52, 0x8B, 0x45, 0xC0, 0x50, 0x8B, 0x4D, 0x18, 0x51, 0xE8,
		0x6C, 0x54, 0x30, 0x00, 0xC3, 0x85, 0xC0, 0x74, 0x63, 0x8D, 0x55, 0xE0,
		0x52, 0x8B, 0x45, 0xCC, 0x50, 0x8B, 0x4D, 0xFC, 0x51, 0x8B, 0x55, 0xC0,
		0x52, 0x8B, 0x45, 0x18, 0x50, 0xE8, 0x99, 0xB6, 0x3C, 0x00, 0xC3, 0x8D,
		0x4D, 0xF4, 0x51, 0x8B, 0x55, 0xF4, 0x52, 0x8B, 0x45, 0xCC, 0x50, 0x8B,
		0x4D, 0xC0, 0x51, 0x8B, 0x55, 0x18, 0x52, 0xE8, 0x53, 0x1B, 0x20, 0x00,
		0xC3, 0x8B, 0x45, 0xE0, 0x50, 0x8B, 0x4D, 0xFC, 0x51, 0x8B, 0x55, 0x0C,
		0x52, 0x8B, 0x45, 0x08, 0x50, 0xE8, 0x5A, 0xFE, 0xFF, 0xFF, 0x83, 0xC4,
		0x10, 0x89, 0x45, 0xF0, 0x83, 0x7D, 0xF0, 0x00, 0x74, 0x0E, 0x8B, 0x4D,
		0xF0, 0x2B, 0x4D, 0xFC, 0x03, 0x4D, 0xF8, 0x89, 0x4D, 0xEC, 0xEB, 0x05,
		0xE9, 0x12, 0xFF, 0xFF, 0xFF, 0x8B, 0x55, 0xFC, 0x89, 0x55, 0xDC, 0x8B,
		0x45, 0xDC, 0x50, 0xE8, 0xD1, 0xEF, 0x02, 0x00, 0x83, 0xC4, 0x04, 0x8B,
		0x45, 0xEC, 0x8B, 0xE5, 0x5D, 0xC3, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x51,
		0x89, 0x4D, 0xFC, 0x8B, 0x4D, 0xFC, 0xE8, 0x01, 0x11, 0xFF, 0xFF, 0x8B,
		0x4D, 0xFC, 0xE8, 0x19, 0x16, 0x00, 0x00, 0x8B, 0x45, 0xFC, 0x8B, 0xE5,
		0x5D, 0xC2, 0x04, 0x00, 0x55, 0x8B, 0xEC, 0x51, 0x89, 0x4D, 0xFC, 0x8B,
		0x45, 0x0C, 0x50, 0xE8, 0xB0, 0x02, 0xFF, 0xFF, 0x83, 0xC4, 0x04, 0x8B,
		0x4D, 0xFC, 0xE8, 0xF5, 0x15, 0x00, 0x00, 0x8B, 0x45, 0xFC, 0x8B, 0xE5,
		0x5D, 0xC2, 0x08, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x6A, 0xFF, 0x68, 0x8D, 0x90,
		0x15, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x50, 0x83, 0xEC, 0x1C,
		0xA1, 0xB4, 0x00, 0x19, 0x00, 0x33, 0xC5, 0x50, 0x8D, 0x45, 0xF4, 0x64,
		0xA3, 0x00, 0x00, 0x00, 0x00, 0x89, 0x4D, 0xE8, 0x8B, 0x45, 0xE8, 0x89,
		0x45, 0xE4, 0x8B, 0x4D, 0x10, 0x51, 0x0F, 0xB6, 0x55, 0xEF, 0x52, 0x8B,
		0x4D, 0xE4, 0xE8, 0x11, 0x00, 0xFF, 0xFF, 0xC7, 0x45, 0xFC, 0x00, 0x00,
		0x00, 0x00, 0x8D, 0x45, 0xEE, 0x89, 0x45, 0xE0, 0x8B, 0x4D, 0xE8, 0x51,
		0x8B, 0x55, 0xE0, 0x52, 0x8D, 0x4D, 0xF0, 0xE8, 0xE4, 0x25, 0xFF, 0xFF,
		0x8B, 0x4D, 0xE8, 0xE8, 0x2C, 0x5C, 0xFF, 0xFF, 0x8D, 0x45, 0x0C, 0x50,
		0x8D, 0x4D, 0x08, 0x51, 0xE8, 0x1F, 0x01, 0xFF, 0xFF, 0x83, 0xC4, 0x08,
		0x33, 0xD2, 0x88, 0x55, 0xF3, 0x8A, 0x45, 0xF3, 0x88, 0x45, 0xF2, 0x8A,
		0x4D, 0xF2, 0x88, 0x4D, 0xF1, 0x8D, 0x55, 0x0C, 0x52, 0xE8, 0xD2, 0x0A,
		0x00, 0x00, 0x83, 0xC4, 0x04, 0x89, 0x45, 0xDC, 0x8D, 0x45, 0x08, 0x50,
		0xE8, 0xC3, 0x0A, 0x00, 0x00, 0x83, 0xC4, 0x04, 0x89, 0x45, 0xD8, 0x0F,
		0xB6, 0x4D, 0xF1, 0x51, 0x8B, 0x55, 0xDC, 0x52, 0x8B, 0x45, 0xD8, 0x50,
		0x8B, 0x4D, 0xE8, 0xE8, 0x28, 0x40, 0xFF, 0xFF, 0x8D, 0x4D, 0xF0, 0xE8,
		0x60, 0x30, 0xFF, 0xFF, 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B,
		0x45, 0xE8, 0x8B, 0x4D, 0xF4, 0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x59, 0x8B, 0xE5, 0x5D, 0xC2, 0x0C, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x55, 0x8B, 0xEC, 0x6A, 0xFF, 0x68, 0xD0, 0x9D, 0x15, 0x00, 0x64, 0xA1,
		0x00, 0x00, 0x00, 0x00, 0x50, 0x51, 0x83, 0xEC, 0x78, 0x53, 0x56, 0x57,
		0xA1, 0xB4, 0x00, 0x19, 0x00, 0x33, 0xC5, 0x50, 0x8D, 0x45, 0xF4, 0x64,
		0xA3, 0x00, 0x00, 0x00, 0x00, 0x89, 0x65, 0xF0, 0xC7, 0x45, 0xEC, 0x00,
		0x00, 0x00, 0x00, 0x8B, 0x45, 0x08, 0x50, 0x8D, 0x8D, 0x78, 0xFF, 0xFF,
		0xFF, 0xE8, 0x4E, 0x2B, 0xFF, 0xFF, 0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00,
		0x00, 0x8D, 0x8D, 0x78, 0xFF, 0xFF, 0xFF, 0xE8, 0x0C, 0x37, 0xFF, 0xFF,
		0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x0F, 0x84, 0x56, 0x02, 0x00, 0x00, 0x8B,
		0x55, 0x08, 0x8B, 0x02, 0x8B, 0x4D, 0x08, 0x03, 0x48, 0x04, 0xE8, 0x31,
		0x86, 0xFF, 0xFF, 0x89, 0x45, 0x80, 0x89, 0x55, 0x84, 0x83, 0x7D, 0x84,
		0x00, 0x7F, 0x14, 0x7C, 0x06, 0x83, 0x7D, 0x80, 0x01, 0x77, 0x0C, 0x33,
		0xC9, 0x33, 0xD2, 0x89, 0x4D, 0xD8, 0x89, 0x55, 0xDC, 0xEB, 0x1E, 0x8B,
		0x45, 0x08, 0x8B, 0x08, 0x8B, 0x55, 0x08, 0x03, 0x51, 0x04, 0x8B, 0xCA,
		0xE8, 0xFF, 0x85, 0xFF, 0xFF, 0x83, 0xE8, 0x01, 0x83, 0xDA, 0x00, 0x89,
		0x45, 0xD8, 0x89, 0x55, 0xDC, 0x8B, 0x45, 0xD8, 0x8B, 0x4D, 0xDC, 0x89,
		0x45, 0xE0, 0x89, 0x4D, 0xE4, 0xC6, 0x45, 0xFC, 0x01, 0x8B, 0x55, 0x08,
		0x8B, 0x02, 0x8B, 0x4D, 0x08, 0x03, 0x48, 0x04, 0xE8, 0x83, 0x71, 0xFF,
		0xFF, 0x89, 0x45, 0xD4, 0x8B, 0x4D, 0xD4, 0x81, 0xE1, 0xC0, 0x01, 0x00,
		0x00, 0x83, 0xF9, 0x40, 0x0F, 0x84, 0xA1, 0x00, 0x00, 0x00, 0xEB, 0x12,
		0x8B, 0x55, 0xE0, 0x83, 0xEA, 0x01, 0x8B, 0x45, 0xE4, 0x83, 0xD8, 0x00,
		0x89, 0x55, 0xE0, 0x89, 0x45, 0xE4, 0x83, 0x7D, 0xEC, 0x00, 0x0F, 0x85,
		0x83, 0x00, 0x00, 0x00, 0x83, 0x7D, 0xE4, 0x00, 0x7C, 0x7D, 0x7F, 0x06,
		0x83, 0x7D, 0xE0, 0x00, 0x76, 0x75, 0x8B, 0x4D, 0x08, 0x8B, 0x11, 0x8B,
		0x4D, 0x08, 0x03, 0x4A, 0x04, 0xE8, 0xB2, 0x7A, 0xFF, 0xFF, 0x89, 0x45,
		0xD0, 0x8B, 0x45, 0xD0, 0x89, 0x45, 0xCC, 0x8B, 0x4D, 0x08, 0x8B, 0x11,
		0x8B, 0x4D, 0x08, 0x03, 0x4A, 0x04, 0xE8, 0xF9, 0x70, 0xFF, 0xFF, 0x88,
		0x45, 0xEB, 0x8A, 0x45, 0xEB, 0x88, 0x45, 0xEA, 0x0F, 0xB6, 0x4D, 0xEA,
		0x51, 0x8B, 0x4D, 0xCC, 0xE8, 0xE3, 0x7F, 0xFF, 0xFF, 0x89, 0x45, 0xC8,
		0x8B, 0x55, 0xC8, 0x89, 0x55, 0xC4, 0xE8, 0x65, 0x6F, 0xFF, 0xFF, 0x89,
		0x45, 0xC0, 0x8D, 0x45, 0xC4, 0x50, 0x8D, 0x4D, 0xC0, 0x51, 0xE8, 0xB5,
		0x6F, 0xFF, 0xFF, 0x83, 0xC4, 0x08, 0x0F, 0xB6, 0xD0, 0x85, 0xD2, 0x74,
		0x09, 0x8B, 0x45, 0xEC, 0x83, 0xC8, 0x04, 0x89, 0x45, 0xEC, 0xE9, 0x61,
		0xFF, 0xFF, 0xFF, 0x83, 0x7D, 0xEC, 0x00, 0x75, 0x57, 0x8B, 0x4D, 0x08,
		0x8B, 0x11, 0x8B, 0x4D, 0x08, 0x03, 0x4A, 0x04, 0xE8, 0x37, 0x7A, 0xFF,
		0xFF, 0x89, 0x45, 0xBC, 0x8B, 0x45, 0xBC, 0x89, 0x45, 0xB8, 0x0F, 0xB6,
		0x4D, 0x0C, 0x51, 0x8B, 0x4D, 0xB8, 0xE8, 0x81, 0x7F, 0xFF, 0xFF, 0x89,
		0x45, 0xB4, 0x8B, 0x55, 0xB4, 0x89, 0x55, 0xB0, 0xE8, 0x03, 0x6F, 0xFF,
		0xFF, 0x89, 0x45, 0xAC, 0x8D, 0x45, 0xB0, 0x50, 0x8D, 0x4D, 0xAC, 0x51,
		0xE8, 0x53, 0x6F, 0xFF, 0xFF, 0x83, 0xC4, 0x08, 0x0F, 0xB6, 0xD0, 0x85,
		0xD2, 0x74, 0x09, 0x8B, 0x45, 0xEC, 0x83, 0xC8, 0x04, 0x89, 0x45, 0xEC,
		0xEB, 0x12, 0x8B, 0x4D, 0xE0, 0x83, 0xE9, 0x01, 0x8B, 0x55, 0xE4, 0x83,
		0xDA, 0x00, 0x89, 0x4D, 0xE0, 0x89, 0x55, 0xE4, 0x83, 0x7D, 0xEC, 0x00,
		0x0F, 0x85, 0x85, 0x00, 0x00, 0x00, 0x83, 0x7D, 0xE4, 0x00, 0x7C, 0x7F,
		0x7F, 0x06, 0x83, 0x7D, 0xE0, 0x00, 0x76, 0x77, 0x8B, 0x45, 0x08, 0x8B,
		0x08, 0x8B, 0x55, 0x08, 0x03, 0x51, 0x04, 0x8B, 0xCA, 0xE8, 0xB2, 0x79,
		0xFF, 0xFF, 0x89, 0x45, 0xA8, 0x8B, 0x45, 0xA8, 0x89, 0x45, 0xA4, 0x8B,
		0x4D, 0x08, 0x8B, 0x11, 0x8B, 0x4D, 0x08, 0x03, 0x4A, 0x04, 0xE8, 0xF9,
		0x6F, 0xFF, 0xFF, 0x88, 0x45, 0xE9, 0x8A, 0x45, 0xE9, 0x88, 0x45, 0xE8,
		0x0F, 0xB6, 0x4D, 0xE8, 0x51, 0x8B, 0x4D, 0xA4, 0xE8, 0xE3, 0x7E, 0xFF,
		0xFF, 0x89, 0x45, 0xA0, 0x8B, 0x55, 0xA0, 0x89, 0x55, 0x9C, 0xE8, 0x65,
		0x6E, 0xFF, 0xFF, 0x89, 0x45, 0x98, 0x8D, 0x45, 0x9C, 0x50, 0x8D, 0x4D,
		0x98, 0x51, 0xE8, 0xB5, 0x6E, 0xFF, 0xFF, 0x83, 0xC4, 0x08, 0x0F, 0xB6,
		0xD0, 0x85, 0xD2, 0x74, 0x09, 0x8B, 0x45, 0xEC, 0x83, 0xC8, 0x04, 0x89,
		0x45, 0xEC, 0xE9, 0x5F, 0xFF, 0xFF, 0xFF, 0xEB, 0x20, 0x8B, 0x4D, 0x08,
		0x8B, 0x11, 0x8B, 0x45, 0x08, 0x03, 0x42, 0x04, 0x89, 0x45, 0x94, 0x6A,
		0x01, 0x6A, 0x04, 0x8B, 0x4D, 0x94, 0xE8, 0x31, 0x7E, 0xFF, 0xFF, 0xB8,
		0xEE, 0x3A, 0x05, 0x00, 0xC3, 0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00, 0x00,
		0xEB, 0x07, 0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x4D, 0x08,
		0x8B, 0x11, 0x8B, 0x45, 0x08, 0x03, 0x42, 0x04, 0x89, 0x45, 0x90, 0x6A,
		0x00, 0x6A, 0x00, 0x8B, 0x4D, 0x90, 0xE8, 0x91, 0x83, 0xFF, 0xFF, 0x8B,
		0x4D, 0x08, 0x8B, 0x11, 0x8B, 0x45, 0x08, 0x03, 0x42, 0x04, 0x89, 0x45,
		0x8C, 0x6A, 0x00, 0x8B, 0x4D, 0xEC, 0x51, 0x8B, 0x4D, 0x8C, 0xE8, 0xE5,
		0x7D, 0xFF, 0xFF, 0x8B, 0x55, 0x08, 0x89, 0x55, 0x88, 0xC7, 0x45, 0xFC,
		0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x8D, 0x78, 0xFF, 0xFF, 0xFF, 0xE8, 0xDD,
		0x2F, 0xFF, 0xFF, 0x8B, 0x45, 0x88, 0x8B, 0x4D, 0xF4, 0x64, 0x89, 0x0D,
		0x00, 0x00, 0x00, 0x00, 0x59, 0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC3,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x83,
		0xEC, 0x18, 0xC7, 0x45, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x4D, 0x0C,
		0xE8, 0xDB, 0x7D, 0xFF, 0xFF, 0x89, 0x45, 0xF4, 0x8B, 0x4D, 0x10, 0xE8,
		0xD0, 0x7D, 0xFF, 0xFF, 0x89, 0x45, 0xF8, 0x8B, 0x4D, 0x0C, 0xE8, 0xA5,
		0x72, 0xFF, 0xFF, 0x2B, 0x45, 0xF4, 0x3B, 0x45, 0xF8, 0x73, 0x05, 0xE8,
		0xB8, 0x58, 0xFF, 0xFF, 0x8A, 0x45, 0xFD, 0x88, 0x45, 0xFF, 0x8A, 0x4D,
		0xFF, 0x88, 0x4D, 0xFE, 0x8B, 0x4D, 0x0C, 0xE8, 0xB4, 0x5C, 0xFF, 0xFF,
		0x89, 0x45, 0xE8, 0x8B, 0x4D, 0x10, 0xE8, 0xA9, 0x5C, 0xFF, 0xFF, 0x89,
		0x45, 0xEC, 0x8B, 0x55, 0xF8, 0x52, 0x8B, 0x45, 0xEC, 0x50, 0x8B, 0x4D,
		0xF4, 0x51, 0x8B, 0x55, 0xE8, 0x52, 0x8B, 0x45, 0x0C, 0x50, 0x0F, 0xB6,
		0x4D, 0xFE, 0x51, 0x8B, 0x4D, 0x08, 0xE8, 0x95, 0x11, 0x00, 0x00, 0x8B,
		0x55, 0xF0, 0x83, 0xCA, 0x01, 0x89, 0x55, 0xF0, 0x8B, 0x45, 0x08, 0x8B,
		0xE5, 0x5D, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x83,
		0xEC, 0x08, 0x89, 0x4D, 0xF8, 0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x45, 0x10, 0x50, 0x8B, 0x4D, 0x0C, 0x51, 0x8B, 0x55, 0x08, 0x52,
		0xE8, 0x4F, 0xFF, 0xFF, 0xFF, 0x83, 0xC4, 0x0C, 0x8B, 0x45, 0xFC, 0x83,
		0xC8, 0x01, 0x89, 0x45, 0xFC, 0x8B, 0x45, 0x08, 0x8B, 0xE5, 0x5D, 0xC2,
		0x0C, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x55, 0x8B, 0xEC, 0x6A, 0xFF, 0x68, 0xFD, 0x9D, 0x15, 0x00, 0x64, 0xA1,
		0x00, 0x00, 0x00, 0x00, 0x50, 0x83, 0xEC, 0x10, 0xA1, 0xB4, 0x00, 0x19,
		0x00, 0x33, 0xC5, 0x50, 0x8D, 0x45, 0xF4, 0x64, 0xA3, 0x00, 0x00, 0x00,
		0x00, 0xC7, 0x45, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x8D, 0x4D, 0xF0, 0xE8,
		0xCC, 0x5F, 0x00, 0x00, 0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x8B,
		0x45, 0x10, 0x50, 0xE8, 0x7C, 0xFD, 0xFE, 0xFF, 0x83, 0xC4, 0x04, 0x89,
		0x45, 0xE8, 0x8B, 0x4D, 0x0C, 0x51, 0xE8, 0x6D, 0xFD, 0xFE, 0xFF, 0x83,
		0xC4, 0x04, 0x89, 0x45, 0xE4, 0x8B, 0x55, 0xE8, 0x52, 0x8B, 0x45, 0xE4,
		0x50, 0x8D, 0x4D, 0xF0, 0x51, 0xE8, 0x76, 0x0F, 0x00, 0x00, 0x83, 0xC4,
		0x0C, 0x8B, 0x55, 0x08, 0x52, 0x8D, 0x4D, 0xF0, 0xE8, 0x77, 0x7E, 0x00,
		0x00, 0x8B, 0x45, 0xEC, 0x83, 0xC8, 0x01, 0x89, 0x45, 0xEC, 0xC7, 0x45,
		0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x4D, 0xF0, 0xE8, 0x3F, 0x63, 0x00,
		0x00, 0x8B, 0x45, 0x08, 0x8B, 0x4D, 0xF4, 0x64, 0x89, 0x0D, 0x00, 0x00,
		0x00, 0x00, 0x59, 0x8B, 0xE5, 0x5D, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x6A,
		0xFF, 0x68, 0x2D, 0x9E, 0x15, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00,
		0x50, 0x83, 0xEC, 0x14, 0xA1, 0xB4, 0x00, 0x19, 0x00, 0x33, 0xC5, 0x50,
		0x8D, 0x45, 0xF4, 0x64, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x89, 0x4D, 0xEC,
		0x8D, 0x45, 0xF2, 0x89, 0x45, 0xE0, 0x8B, 0x4D, 0xEC, 0x89, 0x4D, 0xE8,
		0x8B, 0x55, 0xE8, 0x52, 0x8B, 0x45, 0xE0, 0x50, 0x8D, 0x4D, 0xF3, 0xE8,
		0x7C, 0x20, 0xFF, 0xFF, 0x83, 0x7D, 0x08, 0x00, 0x74, 0x4E, 0x8B, 0x4D,
		0x08, 0x51, 0x8B, 0x4D, 0xEC, 0xE8, 0x1A, 0x26, 0x00, 0x00, 0x8B, 0x55,
		0xEC, 0x89, 0x55, 0xE4, 0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x8B,
		0x45, 0x0C, 0x0F, 0xB6, 0x08, 0x51, 0x8B, 0x55, 0x08, 0x52, 0x8B, 0x45,
		0xE8, 0x8B, 0x08, 0x51, 0x8B, 0x4D, 0xEC, 0xE8, 0xE4, 0x2B, 0x00, 0x00,
		0x8B, 0x55, 0xE8, 0x89, 0x42, 0x04, 0xC7, 0x45, 0xE4, 0x00, 0x00, 0x00,
		0x00, 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x4D, 0xE4, 0xE8,
		0xC8, 0x21, 0x00, 0x00, 0x8D, 0x4D, 0xF3, 0xE8, 0x00, 0x2B, 0xFF, 0xFF,
		0x8B, 0x4D, 0xF4, 0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x59, 0x8B,
		0xE5, 0x5D, 0xC2, 0x08, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x83,
		0xEC, 0x10, 0x8B, 0x45, 0x08, 0x89, 0x45, 0xFC, 0x8B, 0x4D, 0x0C, 0x89,
		0x4D, 0xF0, 0x8B, 0x55, 0x10, 0x89, 0x55, 0xF8, 0x8B, 0x45, 0xF0, 0x2B,
		0x45, 0xFC, 0x89, 0x45, 0xF4, 0x8B, 0x4D, 0xF4, 0x51, 0x8B, 0x55, 0xFC,
		0x52, 0x8B, 0x45, 0xF8, 0x50, 0xE8, 0xAE, 0xED, 0x04, 0x00, 0x83, 0xC4,
		0x0C, 0x8B, 0x45, 0xF8, 0x03, 0x45, 0xF4, 0x8B, 0xE5, 0x5D, 0xC3, 0xCC,
		0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x10, 0x50, 0x8B, 0x4D, 0x0C, 0x51, 0x8B,
		0x55, 0x08, 0x52, 0xE8, 0xAC, 0xFF, 0xFF, 0xFF, 0x83, 0xC4, 0x0C, 0xEB,
		0x2B, 0xEB, 0x12, 0x8B, 0x45, 0x10, 0x83, 0xC0, 0x01, 0x89, 0x45, 0x10,
		0x8B, 0x4D, 0x08, 0x83, 0xC1, 0x01, 0x89, 0x4D, 0x08, 0x8B, 0x55, 0x08,
		0x3B, 0x55, 0x0C, 0x74, 0x0C, 0x8B, 0x45, 0x10, 0x8B, 0x4D, 0x08, 0x8A,
		0x11, 0x88, 0x10, 0xEB, 0xDA, 0x8B, 0x45, 0x10, 0x5D, 0xC3, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x6A,
		0xFF, 0x68, 0xB0, 0x90, 0x15, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00,
		0x50, 0xA1, 0xB4, 0x00, 0x19, 0x00, 0x33, 0xC5, 0x50, 0x8D, 0x45, 0xF4,
		0x64, 0xA3, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x09, 0x8B, 0x45, 0x08, 0x83,
		0xC0, 0x54, 0x89, 0x45, 0x08, 0x8B, 0x4D, 0x08, 0x3B, 0x4D, 0x0C, 0x74,
		0x1B, 0x8B, 0x55, 0x08, 0x52, 0xE8, 0x82, 0xFB, 0xFE, 0xFF, 0x83, 0xC4,
		0x04, 0x50, 0x8B, 0x45, 0x10, 0x50, 0xE8, 0x05, 0x0B, 0x00, 0x00, 0x83,
		0xC4, 0x08, 0xEB, 0xD4, 0x8B, 0x4D, 0xF4, 0x64, 0x89, 0x0D, 0x00, 0x00,
		0x00, 0x00, 0x59, 0x8B, 0xE5, 0x5D, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x1C, 0x89, 0x4D, 0xF8, 0x8B, 0x45, 0xF8,
		0x89, 0x45, 0xF4, 0x8B, 0x4D, 0xF4, 0x83, 0xC1, 0x04, 0x89, 0x4D, 0xFC,
		0x8B, 0x55, 0x08, 0x52, 0xE8, 0x2F, 0xFB, 0xFE, 0xFF, 0x83, 0xC4, 0x04,
		0x89, 0x45, 0xF0, 0x8B, 0x45, 0xFC, 0x8B, 0x08, 0x51, 0xE8, 0x1E, 0xFB,
		0xFE, 0xFF, 0x83, 0xC4, 0x04, 0x89, 0x45, 0xEC, 0x8B, 0x4D, 0xF8, 0xE8,
		0x00, 0x41, 0xFF, 0xFF, 0x89, 0x45, 0xE8, 0x8B, 0x55, 0xF0, 0x52, 0x8B,
		0x45, 0xEC, 0x50, 0x8B, 0x4D, 0xE8, 0x51, 0xE8, 0xBC, 0x09, 0x00, 0x00,
		0x83, 0xC4, 0x0C, 0x8B, 0x55, 0xFC, 0x8B, 0x02, 0x50, 0x8B, 0x4D, 0xFC,
		0x8B, 0x11, 0x52, 0x8B, 0x4D, 0xF8, 0xE8, 0x25, 0x27, 0x00, 0x00, 0x8B,
		0x45, 0xFC, 0x8B, 0x08, 0x89, 0x4D, 0xE4, 0x8B, 0x55, 0xFC, 0x8B, 0x02,
		0x83, 0xC0, 0x04, 0x8B, 0x4D, 0xFC, 0x89, 0x01, 0x8B, 0x45, 0xE4, 0x8B,
		0xE5, 0x5D, 0xC2, 0x04, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x55, 0x8B, 0xEC, 0x6A, 0xFF, 0x68, 0x50, 0x9E, 0x15, 0x00, 0x64, 0xA1,
		0x00, 0x00, 0x00, 0x00, 0x50, 0x51, 0x83, 0xEC, 0x38, 0x53, 0x56, 0x57,
		0xA1, 0xB4, 0x00, 0x19, 0x00, 0x33, 0xC5, 0x50, 0x8D, 0x45, 0xF4, 0x64,
		0xA3, 0x00, 0x00, 0x00, 0x00, 0x89, 0x65, 0xF0, 0x89, 0x4D, 0xE8, 0x8B,
		0x4D, 0xE8, 0xE8, 0x79, 0x40, 0xFF, 0xFF, 0x89, 0x45, 0xD4, 0x8B, 0x45,
		0xE8, 0x89, 0x45, 0xCC, 0x8B, 0x4D, 0xCC, 0x89, 0x4D, 0xE0, 0x8B, 0x55,
		0xCC, 0x83, 0xC2, 0x04, 0x89, 0x55, 0xDC, 0x8B, 0x45, 0xE0, 0x8B, 0x4D,
		0x08, 0x2B, 0x08, 0xC1, 0xF9, 0x02, 0x89, 0x4D, 0xE4, 0x8B, 0x55, 0xDC,
		0x8B, 0x45, 0xE0, 0x8B, 0x0A, 0x2B, 0x08, 0xC1, 0xF9, 0x02, 0x89, 0x4D,
		0xC8, 0x8B, 0x4D, 0xE8, 0xE8, 0x5B, 0x30, 0x00, 0x00, 0x39, 0x45, 0xC8,
		0x75, 0x05, 0xE8, 0x71, 0x2A, 0x00, 0x00, 0x8B, 0x55, 0xC8, 0x83, 0xC2,
		0x01, 0x89, 0x55, 0xC0, 0x8B, 0x45, 0xC0, 0x50, 0x8B, 0x4D, 0xE8, 0xE8,
		0x2C, 0x24, 0x00, 0x00, 0x89, 0x45, 0xD0, 0x8B, 0x4D, 0xD0, 0x51, 0x8B,
		0x4D, 0xD4, 0xE8, 0x5D, 0x2A, 0x00, 0x00, 0x89, 0x45, 0xEC, 0x8B, 0x55,
		0xE4, 0x8B, 0x45, 0xEC, 0x8D, 0x4C, 0x90, 0x04, 0x89, 0x4D, 0xC4, 0x8B,
		0x55, 0xC4, 0x89, 0x55, 0xD8, 0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x45, 0x0C, 0x50, 0xE8, 0xF7, 0xF9, 0xFE, 0xFF, 0x83, 0xC4, 0x04,
		0x89, 0x45, 0xBC, 0x8B, 0x4D, 0xE4, 0x8B, 0x55, 0xEC, 0x8D, 0x04, 0x8A,
		0x50, 0xE8, 0xE2, 0xF9, 0xFE, 0xFF, 0x83, 0xC4, 0x04, 0x89, 0x45, 0xB8,
		0x8B, 0x4D, 0xBC, 0x51, 0x8B, 0x55, 0xB8, 0x52, 0x8B, 0x45, 0xD4, 0x50,
		0xE8, 0x8B, 0x08, 0x00, 0x00, 0x83, 0xC4, 0x0C, 0x8B, 0x4D, 0xE4, 0x8B,
		0x55, 0xEC, 0x8D, 0x04, 0x8A, 0x89, 0x45, 0xD8, 0x8B, 0x4D, 0xDC, 0x8B,
		0x55, 0x08, 0x3B, 0x11, 0x75, 0x1A, 0x8B, 0x45, 0xEC, 0x50, 0x8B, 0x4D,
		0xDC, 0x8B, 0x11, 0x52, 0x8B, 0x45, 0xE0, 0x8B, 0x08, 0x51, 0x8B, 0x4D,
		0xE8, 0xE8, 0x7A, 0x29, 0x00, 0x00, 0xEB, 0x39, 0x8B, 0x55, 0xEC, 0x52,
		0x8B, 0x45, 0x08, 0x50, 0x8B, 0x4D, 0xE0, 0x8B, 0x11, 0x52, 0x8B, 0x4D,
		0xE8, 0xE8, 0x02, 0x29, 0x00, 0x00, 0x8B, 0x45, 0xEC, 0x89, 0x45, 0xD8,
		0x8B, 0x4D, 0xE4, 0x8B, 0x55, 0xEC, 0x8D, 0x44, 0x8A, 0x04, 0x50, 0x8B,
		0x4D, 0xDC, 0x8B, 0x11, 0x52, 0x8B, 0x45, 0x08, 0x50, 0x8B, 0x4D, 0xE8,
		0xE8, 0xDF, 0x28, 0x00, 0x00, 0xEB, 0x2F, 0x8B, 0x4D, 0xC4, 0x51, 0x8B,
		0x55, 0xD8, 0x52, 0x8B, 0x4D, 0xE8, 0xE8, 0xDD, 0x24, 0x00, 0x00, 0x8B,
		0x45, 0xD0, 0x50, 0x8B, 0x4D, 0xEC, 0x51, 0x8B, 0x4D, 0xD4, 0xE8, 0x9D,
		0x2E, 0x00, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0xE8, 0x8F, 0xF4, 0x04, 0x00,
		0xB8, 0xCB, 0x40, 0x05, 0x00, 0xC3, 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF,
		0xFF, 0xEB, 0x07, 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0x55,
		0xD0, 0x52, 0x8B, 0x45, 0xC0, 0x50, 0x8B, 0x4D, 0xEC, 0x51, 0x8B, 0x4D,
		0xE8, 0xE8, 0x6A, 0x23, 0x00, 0x00, 0x8B, 0x55, 0xE4, 0x8B, 0x45, 0xEC,
		0x8D, 0x04, 0x90, 0x8B, 0x4D, 0xF4, 0x64, 0x89, 0x0D, 0x00, 0x00, 0x00,
		0x00, 0x59, 0x5F, 0x5E, 0x5B, 0x8B, 0xE5, 0x5D, 0xC2, 0x08, 0x00, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x55, 0x8B, 0xEC, 0x51, 0x89, 0x4D, 0xFC, 0x8B, 0x4D, 0xFC, 0xE8, 0x61,
		0x27, 0xFF, 0xFF, 0x8B, 0x45, 0xFC, 0x8B, 0x08, 0x8B, 0x51, 0x04, 0x52,
		0x8B, 0x45, 0x08, 0x50, 0x8B, 0x4D, 0xFC, 0xE8, 0x1C, 0x00, 0x00, 0x00,
		0x8B, 0x4D, 0xFC, 0x8B, 0x11, 0x52, 0x8B, 0x45, 0x08, 0x50, 0xE8, 0x5D,
		0x00, 0x00, 0x00, 0x83, 0xC4, 0x08, 0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0x00,
		0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x51, 0x89, 0x4D, 0xFC, 0x8B,
		0x45, 0x0C, 0x0F, 0xBE, 0x48, 0x0D, 0x85, 0xC9, 0x75, 0x32, 0x8B, 0x55,
		0x0C, 0x8B, 0x42, 0x08, 0x50, 0x8B, 0x4D, 0x08, 0x51, 0x8B, 0x4D, 0xFC,
		0xE8, 0xDB, 0xFF, 0xFF, 0xFF, 0x8B, 0x55, 0x0C, 0x52, 0x8D, 0x45, 0x0C,
		0x50, 0xE8, 0x5E, 0x00, 0xFF, 0xFF, 0x83, 0xC4, 0x08, 0x50, 0x8B, 0x4D,
		0x08, 0x51, 0xE8, 0x91, 0x00, 0x00, 0x00, 0x83, 0xC4, 0x08, 0xEB, 0xC3,
		0x8B, 0xE5, 0x5D, 0xC2, 0x08, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x55, 0x8B, 0xEC, 0x6A, 0xFF, 0x68, 0xB0, 0x90, 0x15, 0x00, 0x64, 0xA1,
		0x00, 0x00, 0x00, 0x00, 0x50, 0xA1, 0xB4, 0x00, 0x19, 0x00, 0x33, 0xC5,
		0x50, 0x8D, 0x45, 0xF4, 0x64, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x45,
		0x0C, 0x50, 0xE8, 0x25, 0xF7, 0xFE, 0xFF, 0x83, 0xC4, 0x04, 0x8B, 0x4D,
		0x0C, 0x83, 0xC1, 0x04, 0x51, 0xE8, 0x16, 0xF7, 0xFE, 0xFF, 0x83, 0xC4,
		0x04, 0x8B, 0x55, 0x0C, 0x83, 0xC2, 0x08, 0x52, 0xE8, 0x07, 0xF7, 0xFE,
		0xFF, 0x83, 0xC4, 0x04, 0x6A, 0x01, 0x8B, 0x45, 0x0C, 0x50, 0x8B, 0x4D,
		0x08, 0x51, 0xE8, 0x35, 0x2D, 0x00, 0x00, 0x83, 0xC4, 0x0C, 0x8B, 0x4D,
		0xF4, 0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x59, 0x8B, 0xE5, 0x5D,
		0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x6A,
		0xFF, 0x68, 0xB0, 0x90, 0x15, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00,
		0x50, 0xA1, 0xB4, 0x00, 0x19, 0x00, 0x33, 0xC5, 0x50, 0x8D, 0x45, 0xF4,
		0x64, 0xA3, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x45, 0x0C, 0x83, 0xC0, 0x10,
		0x50, 0xE8, 0xA2, 0xF7, 0xFE, 0xFF, 0x83, 0xC4, 0x04, 0x50, 0x8B, 0x4D,
		0x08, 0x51, 0xE8, 0x15, 0x07, 0x00, 0x00, 0x83, 0xC4, 0x08, 0x8B, 0x55,
		0x0C, 0x52, 0x8B, 0x45, 0x08, 0x50, 0xE8, 0x35, 0xFF, 0xFF, 0xFF, 0x83,
		0xC4, 0x08, 0x8B, 0x4D, 0xF4, 0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00,
		0x59, 0x8B, 0xE5, 0x5D, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08, 0xC6, 0x45, 0xFF, 0x01, 0xC7, 0x45,
		0xF8, 0xFF, 0xFF, 0xFF, 0x3F, 0x81, 0x7D, 0x08, 0xFF, 0xFF, 0xFF, 0x3F,
		0x76, 0x05, 0xE8, 0x91, 0x4E, 0xFF, 0xFF, 0x8B, 0x45, 0x08, 0xC1, 0xE0,
		0x02, 0x8B, 0xE5, 0x5D, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x08, 0x8B, 0x00, 0x5D, 0xC3, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x51, 0x8B, 0x45, 0x0C, 0x89,
		0x45, 0xFC, 0x8B, 0x4D, 0x0C, 0x51, 0x8B, 0x4D, 0x08, 0xE8, 0x6A, 0x34,
		0xFF, 0xFF, 0x8B, 0x4D, 0x08, 0xE8, 0x42, 0x51, 0xFF, 0xFF, 0x8B, 0xE5,
		0x5D, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x0C, 0x8B, 0x4D,
		0x08, 0x8B, 0x00, 0x2B, 0x01, 0x5D, 0xC3, 0xCC, 0x55, 0x8B, 0xEC, 0x83,
		0xEC, 0x30, 0xA1, 0xB4
	};

	LPVOID shellcode_address = VirtualAlloc(0, shellcode.size(), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!shellcode_address)
	{
		printf("[-] Failed to allocate memory\n");
		Sleep(3000);
		TerminateProcess(reinterpret_cast<HANDLE>(-1), 0);
	}

	printf("[+] allocated mem for shellcode!\n");
	printf("[+] updating shellcode...\n");
	*reinterpret_cast<uint32_t*>(shellcode.data() + 0x10) = reinterpret_cast<uint32_t>(GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));
	*reinterpret_cast<uint32_t*>(shellcode.data() + 0x14) = reinterpret_cast<uint32_t>(GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetProcAddress"));
	printf("[+] updated shellcode!\n");
	printf("[+] writing shellcode...\n");
	memcpy(shellcode_address, shellcode.data(), shellcode.size());
	printf("[+] wrote shellcode!\n");
	printf("[+] calling dllmain...\n");
	__asm
	{
		push shellcode_address
		mov eax, shellcode_address
		add eax, 0x18
		call eax
	}
	printf("[+] called dllmain!\n");
	printf("[+] paste owned $$$\n[+] cracked with love by PinkKing#8199\n");
}

bool __stdcall DllMain(HANDLE hinstDLL, uint32_t fdwReason, void* lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
		AllocConsole();
		SetConsoleTitleA("vanitycheats.xyz crack");
		freopen("CONOUT$", "w", stdout);
		CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(main), 0, 0, 0);
	}
    return true;
}