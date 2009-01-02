/*
	All this does is recieves network vars for KOTH mode.
*/

#include "cbase.h"
#include "c_koth.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT( C_Koth, DT_Koth, CKoth )
	RecvPropInt( RECVINFO( m_iLastTeam ) ),
	RecvPropFloat( RECVINFO( m_flNextCapture ) ),
	RecvPropInt( RECVINFO( m_iCapturePlayers ) ),
	RecvPropInt( RECVINFO( m_iNearbyPlayers ) ),
	RecvPropInt( RECVINFO( m_iForTeam ) ),	
	RecvPropFloat( RECVINFO( m_flCaptureTime ) ),
	RecvPropString( RECVINFO( m_sTriggerName ) ),		
	RecvPropInt( RECVINFO( m_iHUDSlot ) ),			
	//RecvPropBool( RECVINFO( m_bActive ) ), //Tweaked. See flag.cpp in the server project. -HairyPotter


END_RECV_TABLE()

// Global list of client side team entities
CUtlVector< C_Koth * > g_Flags;

//=================================================================================================
// C_Team functionality

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_Koth::C_Koth()
{

	m_sTriggerName[0] = 0;	//nullterm just in case

	// Add myself to the global list of team entities
	g_Flags.AddToTail( this );
}

C_Koth::~C_Koth()
{
	g_Flags.FindAndRemove( this );
}
