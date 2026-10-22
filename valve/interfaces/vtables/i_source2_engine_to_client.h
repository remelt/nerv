#pragma once
#include "i_demo_file.h"

class ISource2EngineToClient
{
public:

    bool IsConnected()
    {
        return VFUNC(this, bool, 39);
    }

    bool IsInGame()
    {
        return VFUNC(this, bool, 40);
    }

    bool IsPlayingDemo()
    {
        return VFUNC(this, bool, 42);
    }

    void ExecuteClientCmd(int iUnk0MaybeSplitScreenSlotSetTo0, const char* pszCommands, bool bUnk2SetToTrue)
    {
        VFUNC(this, void, 51, iUnk0MaybeSplitScreenSlotSetTo0, pszCommands, bUnk2SetToTrue);
    }

    const char* GetLevelName()
    {
        return VFUNC(this, const char*, 64);
    }

    const char* GetLevelNameShort()
    {
        return VFUNC(this, const char*, 65);
    }

    IDemoFile* GetDemoFile()
    {
        return VFUNC(this, IDemoFile*, 69);
    }

    // https://github.com/advancedfx/advancedfx/blob/03f972b4710ab9a8b1601fc0c34ce33c9c0268cc/AfxHookSource2/MirvTime.cpp#L60-L68
    int GetCurrentDemoTick() {
        if (this) {
            if (auto pDemoFile = this->GetDemoFile()) {
                return pDemoFile->GetDemoTick();
            }
        }
    }

};