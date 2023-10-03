// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "utils/addr.h"
#include "utils/MemoryMgr.h"
#include "utils/Trampoline.h"
#include "utils/Patterns.h"
#include "exports.h"
#include "minhook/include/MinHook.h"
#include "IniReader.h"

#ifdef HOOKPLUGIN
#include "PluginInfo.h"
#include "SDK.h"
#endif 

using namespace Memory::VP;
using namespace hook::txn;

static bool ms_bEnable;
static bool ms_bIncreaseBrigthness;
static int  ms_colors[3];
static float ms_fColors[4] = {};

static CIniReader ini("BloodCustomizer.ini");


struct FLinearColor {
    float R, G, B, A;
};


static void (*orgSetBloodColor)(int64 info, int64 ptr);
void SetBloodColor(int64 info, int64 ptrs)
{
    if (ms_bEnable)
    {
        float scale = 1000.0f;

        if (ms_bIncreaseBrigthness)
            scale = 100.0f;

        float red = ((ms_colors[0] * 100.0f) / 255.0f) / scale;
        float green = ((ms_colors[1] * 100.0f) / 255.0f) / scale;
        float blue = ((ms_colors[2] * 100.0f) / 255.0f) / scale;

        FLinearColor color;
        color.R = red;
        color.G = green;
        color.B = blue;
        color.A = 1.0f;

        *(FLinearColor*)(*(int64*)info + 0x30) = color;
        *(FLinearColor*)(*(int64*)info + 0x40) = color;
        *(FLinearColor*)(*(int64*)info + 0x50) = color;
        *(FLinearColor*)(*(int64*)info + 0x60) = color;
    }
    orgSetBloodColor(info, ptrs);
}

void SaveINI()
{
    ini.WriteBoolean("Settings", "Enable", ms_bEnable);
    ini.WriteBoolean("Settings", "IncreaseBrightness", ms_bIncreaseBrigthness);
    ini.WriteInteger("Settings", "R", ms_colors[0]);
    ini.WriteInteger("Settings", "G", ms_colors[1]);
    ini.WriteInteger("Settings", "B", ms_colors[2]);
}

void Init()
{
    ms_bEnable = ini.ReadBoolean("Settings", "Enable", true);
    ms_bIncreaseBrigthness = ini.ReadBoolean("Settings", "IncreaseBrightness", false);
    ms_colors[0] = ini.ReadInteger("Settings", "R", 0);
    ms_colors[1] = ini.ReadInteger("Settings", "G", 255);
    ms_colors[2] = ini.ReadInteger("Settings", "B", 0);


    for (int i = 0; i < 3; i++)
    {
        if (ms_colors[i] > 255)
            ms_colors[i] = 255;
        if (ms_colors[i] < 0)
            ms_colors[i] = 0;
    }
    float red = ((ms_colors[0] * 100.0f) / 255.0f) / 100.0f;
    float green = ((ms_colors[1] * 100.0f) / 255.0f) / 100.0f;
    float blue = ((ms_colors[2] * 100.0f) / 255.0f) / 100.0f;
    ms_fColors[0] = red;
    ms_fColors[1] = green;
    ms_fColors[2] = blue;
    ms_fColors[3] = 1.0f;

    if (ms_bEnable)
    {
#ifndef HOOKPLUGIN
        MH_Initialize();
#endif
        static uintptr_t setBloodColorPtr = 0;

        setBloodColorPtr = (uintptr_t)get_pattern("48 83 EC 38 48 8B 01 4C 8B D2 48 85 C0 74 11 0F 10 40 40 EB 0E");

        if (setBloodColorPtr)
        {
#ifndef HOOKPLUGIN       
            MH_STATUS s = MH_CreateHook((void*)setBloodColorPtr, SetBloodColor, (void**)&orgSetBloodColor);

            if (s == MH_OK)
                MH_EnableHook((void*)setBloodColorPtr);
#else
            MK12HOOKSDK::CreateHook((void*)setBloodColorPtr, SetBloodColor, (void**)&orgSetBloodColor);
#endif

        }
        else
            MessageBoxA(0, "Failed to find SetBloodColor pattern!", "MK1.BloodCustomizer", MB_ICONERROR);
    }

}



#ifdef HOOKPLUGIN
// Plugin name to use when loading and printing errors to log
extern "C" PLUGIN_API const char* GetPluginName()
{
    return "Blood Customizer (1.0)";
}

// Hook project name that this plugin is compatible with
extern "C" PLUGIN_API const char* GetPluginProject()
{
    return "MK12HOOK";
}

// GUI tab name that will be used in the Plugins section
extern "C" PLUGIN_API const char* GetPluginTabName()
{
    return "Blood Customizer";
}

// Initialization
extern "C" PLUGIN_API void OnInitialize(HMODULE hMod)
{
    MK12HOOKSDK::Initialize(hMod);
    Init();
}

// Shutdown
extern "C" PLUGIN_API void OnShutdown()
{

}

// Called every game tick
extern "C" PLUGIN_API void OnFrameTick()
{

}

// Called on match/fight start
extern "C" PLUGIN_API void OnFightStartup()
{
    // not implemented yet
}

// Tab data for menu, remove this if you don't want a plugin tab
extern "C" PLUGIN_API void TabFunction()
{
    if (!MK12HOOKSDK::IsOK())
        return;

    MK12HOOKSDK::ImGui_Checkbox("Enable##blc", &ms_bEnable);

    if (ms_bEnable)
    {
        MK12HOOKSDK::ImGui_Separator();
        MK12HOOKSDK::ImGui_Text("Blood FX is reloaded on each match, not during gameplay.");
        MK12HOOKSDK::ImGui_Checkbox("Increase Brightness", &ms_bIncreaseBrigthness);

        MK12HOOKSDK::ImGui_Text("Blood Color (Alpha is ignored)");
        if (MK12HOOKSDK::ImGui_ColorEdit4("Pick RGB", ms_fColors))
        {
            ms_colors[0] = (int)(ms_fColors[0] * 255.0f);
            ms_colors[1] = (int)(ms_fColors[1] * 255.0f);
            ms_colors[2] = (int)(ms_fColors[2] * 255.0f);
        }

        if (MK12HOOKSDK::ImGui_Button("Save Settings"))
        {
            SaveINI();
        }
        
    }

}
#endif

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
#ifndef HOOKPLUGIN
        LoadOriginalDLL();
        Init();
#endif
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

