#pragma once

typedef __int64 MKCharacter;
typedef __int64 PlayerInfo;
typedef __int64 FGGameInfo;

enum PLAYER_NUM
{
	PLAYER1,
	PLAYER2,
	PLAYER3,
	PLAYER4,
	MAX_PLAYERS,
};

#include <Windows.h>

class MK12HOOKSDK {
public:
	static MKCharacter* (*GetCharacterObject)(PLAYER_NUM);
	static PlayerInfo*(*GetPlayerInfo)(PLAYER_NUM);
	static FGGameInfo* (*GetGameInfo)(void);
	static bool (*IsMenuActive)(void);
	static bool (*ImGui_Checkbox)(const char*, bool*);
	static void (*ImGui_Text)(const char*);
	static bool (*ImGui_Button)(const char*);
	static bool (*ImGui_InputInt)(const char*, int*);
	static bool (*ImGui_InputFloat)(const char*, float*);
	static bool (*ImGui_InputText)(const char*, char*, size_t);
	static bool (*ImGui_BeginCombo)(const char*, const char*);
	static void (*ImGui_EndCombo)();
	static bool (*ImGui_Selectable)(const char*, bool);
	static void (*ImGui_SetItemDefaultFocus)();
	static void (*ImGui_Separator)();
	static bool (*ImGui_CollapsingHeader)(const char*);
	static bool (*ImGui_ColorEdit4)(const char*, float*);
	static uintptr_t (*GetPattern)(const char*, int);
	static int (*CreateHook)(LPVOID, LPVOID, LPVOID*);
	static bool ms_bIsInitialized;
	static void Initialize(HMODULE hMod);
	static bool IsOK();
};