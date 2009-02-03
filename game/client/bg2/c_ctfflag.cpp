/*
	All this does is recieves network vars for CTF mode.
*/

#include "cbase.h"
#include "c_ctfflag.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT( C_CtfFlag, DT_CtfFlag, CtfFlag )

	RecvPropInt( RECVINFO( m_iForTeam ) ),	
	RecvPropBool( RECVINFO( m_bIsCarried ) ),	
	RecvPropString( RECVINFO( n_cFlagName ) ),	

END_RECV_TABLE()

// Global list of client side team entities
CUtlVector< C_CtfFlag * > g_CtfFlags;

//=================================================================================================
// C_Team functionality

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CtfFlag::C_CtfFlag()
{

	n_cFlagName[0] = 0;	//nullterm just in case

	// Add myself to the global list of team entities
	g_CtfFlags.AddToTail( this );
}

C_CtfFlag::~C_CtfFlag()
{
	g_CtfFlags.FindAndRemove( this );
}
