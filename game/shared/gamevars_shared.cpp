//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "gamevars_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// some shared cvars used by game rules
ConVar mp_forcecamera( 
	"mp_forcecamera", 
	"0", 
	FCVAR_REPLICATED,
	"Restricts spectator modes for dead players" ); //BG2 - let players roam free! (By default) - HairyPotter
	
ConVar mp_allowspectators(
	"mp_allowspectators", 
	"1.0", 
	FCVAR_REPLICATED,
	"toggles whether the server allows spectator mode or not" );

ConVar friendlyfire(
	"mp_friendlyfire",
	"0",
#ifdef TF_DLL
	FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_DEVELOPMENTONLY
#else
	FCVAR_REPLICATED | FCVAR_NOTIFY
#endif
	);
