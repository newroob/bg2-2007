//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2_playerlocaldata.h"
#include "hl2_player.h"
#include "mathlib/mathlib.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_SEND_TABLE_NOBASE( CHL2PlayerLocalData, DT_HL2Local )
	//SendPropInt( SENDINFO(m_bZooming), 1, SPROP_UNSIGNED ), //BG2 - Don't need this. -HairyPotter
	//SendPropInt( SENDINFO(m_bitsActiveDevices), MAX_SUIT_DEVICES, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO(m_bWeaponLowered) ),
	SendPropEHandle( SENDINFO(m_hLadder) ),
END_SEND_TABLE()

BEGIN_SIMPLE_DATADESC( CHL2PlayerLocalData )
	//DEFINE_FIELD( m_bZooming, FIELD_BOOLEAN ), //BG2 - Don't need this. -HairyPotter
	//DEFINE_FIELD( m_bitsActiveDevices, FIELD_INTEGER ),
	DEFINE_FIELD( m_bWeaponLowered, FIELD_BOOLEAN ),

	// Ladder related stuff
	DEFINE_FIELD( m_hLadder, FIELD_EHANDLE ),
	DEFINE_EMBEDDED( m_LadderMove ),
END_DATADESC()

CHL2PlayerLocalData::CHL2PlayerLocalData()
{
	//m_flSuitPower = 0.0;
	//m_bZooming = false;
	m_bWeaponLowered = false;
	m_hLadder.Set(NULL);
	//m_bDisplayReticle = false;
}

