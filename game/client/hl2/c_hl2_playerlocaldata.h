//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#if !defined( C_HL2_PLAYERLOCALDATA_H )
#define C_HL2_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif


#include "dt_recv.h"

#include "hl2/hl_movedata.h"

EXTERN_RECV_TABLE( DT_HL2Local );


class C_HL2PlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE( C_HL2PlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	C_HL2PlayerLocalData();

	//float	m_flSuitPower;
	//bool	m_bZooming;
	//int		m_bitsActiveDevices;
	bool	m_bWeaponLowered;

	// Ladder related data
	EHANDLE			m_hLadder;
	LadderMove_t	m_LadderMove;
};


#endif
