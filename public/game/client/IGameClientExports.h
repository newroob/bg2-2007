//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IGAMECLIENTEXPORTS_H
#define IGAMECLIENTEXPORTS_H
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"

//-----------------------------------------------------------------------------
// Purpose: Exports a set of functions for the GameUI interface to interact with the game client
//-----------------------------------------------------------------------------
abstract_class IGameClientExports : public IBaseInterface
{
public:
#ifndef _XBOX
	// ingame voice manipulation
	virtual bool IsPlayerGameVoiceMuted(int playerIndex) = 0;
	virtual void MutePlayerGameVoice(int playerIndex) = 0;
	virtual void UnmutePlayerGameVoice(int playerIndex) = 0;

	// notification of gameui state changes
	//virtual void OnGameUIActivated() = 0; //BG2 - Keep an eye on this. Code merge in progress. -HairyPotter
	//virtual void OnGameUIHidden() = 0; //BG2 - Keep an eye on this. Code merge in progress. -HairyPotter
#endif
};

#define GAMECLIENTEXPORTS_INTERFACE_VERSION "GameClientExports001"


#endif // IGAMECLIENTEXPORTS_H