/*
	The Battle Grounds 2 - A Source modification
	Copyright (C) 2005, The Battle Grounds 2 Team and Contributors

	The Battle Grounds 2 free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	The Battle Grounds 2 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

	Contact information:
		Tomas "Tjoppen" Härdin		tjoppen@gamedev.se

	You may also contact the (future) team via the Battle Grounds website and/or forum at:
		www.bgmod.com

	 Note that because of the sheer volume of files in the Source SDK this
	notice cannot be put in all of them, but merely the ones that have any
	changes from the original SDK.
	 In order to facilitate easy searching, all changes are and must be
	commented on the following form:

	//BG2 - <name of contributer>[ - <small description>]
*/

#include "cbase.h"
#include "flag.h"
/*#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else*/
	#include "hl2mp_player.h"
	#include "hl2mp_gamerules.h"
	#include "gamerules.h"
	#include "team.h"
	#include "engine/IEngineSound.h"
	#include "bg2/spawnpoint.h"
//#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//#ifndef CLIENT_DLL
//BG2 - Tjoppen - just make these global so we can reset them
float	flNextClientPrintAll = 0;
bool	bNextClientPrintAllForce = false;

const int CFlag_START_DISABLED = 1;		// spawnflag definition

void ClientPrintAll( char *str, bool printfordeadplayers = false, bool forcenextclientprintall = false )
{
	if( !str )
		return;

	//Msg( "ClientPrintAll: %s printfordeadplayers=%i forcenextclientprintall=%i\t", str, printfordeadplayers, forcenextclientprintall );

	if( flNextClientPrintAll > gpGlobals->curtime && (!printfordeadplayers || bNextClientPrintAllForce) && !forcenextclientprintall )
	{
		//Msg( "returned\n" );
		return;
	}

	flNextClientPrintAll = gpGlobals->curtime + 1.0f;
	bNextClientPrintAllForce = forcenextclientprintall;

	CBasePlayer *pPlayer = NULL;

	while( (pPlayer = (CBasePlayer*)gEntList.FindEntityByClassname( pPlayer, "player" )) != NULL )
	{
		if( !pPlayer->IsAlive() && !printfordeadplayers )	//this doesn't concern dead players
			continue;

		ClientPrint( pPlayer, HUD_PRINTCENTER, str );
	}

	//Msg( "done\n" );
}
//#endif


//BG2 - SaintGreg - dynamic flag settings
//input functions
#ifndef CLIENT_DLL
bool CFlag::IsActive( void )
{
	return m_bActive;
}

void CFlag::InputEnable( inputdata_t &inputData )
{
	if( m_bActive )
		return;
	char msg[512];
	Q_snprintf( msg, 512, "flag(%s) has been enabled", STRING( GetEntityName() ) );
	ClientPrintAll( msg, true, true ); // saying whether it is enabled or not is important, so force
	m_bActive = true;
	ChangeTeam( TEAM_UNASSIGNED );
	m_iLastTeam = TEAM_UNASSIGNED;
	SetModel( "models/other/flag_n.mdl" );
	m_OnEnable.FireOutput( inputData.pActivator, this );
	Think(); // think immediately and restart the thinking cycle
}
void CFlag::InputDisable( inputdata_t &inputData )
{
	if( !m_bActive )
		return;
	char msg[512];
	Q_snprintf( msg, 512, "flag(%s) has been disabled", STRING( GetEntityName() ) );
	ClientPrintAll( msg, true, true ); // saying whether it is enabled or not is important, so force
	m_bActive = false;
	if( GetTeamNumber() == TEAM_AMERICANS )
	{
		m_OnAmericanLosePoint.FireOutput( this, this );
		m_OnLosePoint.FireOutput( this, this );
	}
	else if( GetTeamNumber() == TEAM_BRITISH )
	{
		m_OnBritishLosePoint.FireOutput( this, this );
		m_OnLosePoint.FireOutput( this, this );
	}
	ChangeTeam( TEAM_UNASSIGNED );
	m_iLastTeam = TEAM_UNASSIGNED;
	SetModel( "models/other/flag_w.mdl" );
	m_OnDisable.FireOutput( inputData.pActivator, this );
	CFlagHandler::Update();
}
void CFlag::InputToggle( inputdata_t &inputData )
{
	if (m_bActive)
		InputDisable( inputData );
	else
		InputEnable( inputData );
}
#endif // CLIENT_DLL

void CFlag::Spawn( void )
{
	Precache( );

	/*switch( m_iForTeam )
	{
	default:
		SetModel( "models/other/flag_n.mdl" );
#ifndef CLIENT_DLL
		m_iLastTeam = TEAM_UNASSIGNED;
		ChangeTeam( TEAM_UNASSIGNED );
#endif
		break;
	case 1:
		SetModel( "models/other/flag_a.mdl" );
#ifndef CLIENT_DLL
		m_iLastTeam = TEAM_AMERICANS;
		ChangeTeam( TEAM_AMERICANS );
#endif
		break;
	case 2:
		SetModel( "models/other/flag_b.mdl" );
#ifndef CLIENT_DLL
		m_iLastTeam = TEAM_BRITISH;
		ChangeTeam( TEAM_BRITISH );
#endif
		break;
	}*/

	/*switch (m_iForTeam)
	{
		case 1://amer
			SetModel( "models/other/flag_b.mdl" );
			break;
		case 2://brit
			SetModel( "models/other/flag_a.mdl" );
			break;
		default:
			SetModel( "models/other/flag_n.mdl" );
			break;
	}*/

	//m_iLastTeam = TEAM_UNASSIGNED;

	ChangeTeam( TEAM_UNASSIGNED );	//ChangeTeam handles everything..
	m_iRequestingCappers = TEAM_UNASSIGNED;
	m_iLastTeam = TEAM_UNASSIGNED;

#ifndef CLIENT_DLL
	if (HasSpawnFlags( CFlag_START_DISABLED ))
	{
		m_bActive = false;
		SetModel( "models/other/flag_w.mdl" );
	}
	else
	{
		m_bActive = true;
		//SetModel( "models/other/flag_n.mdl" );
	}
#endif // CLIENT_DLL

	/*int m_nIdealSequence = SelectWeightedSequence( ACT_VM_IDLE );
	//SetActivity( ACT_VM_IDLE );
	SetSequence( m_nIdealSequence );	
	//SendViewModelAnim( m_nIdealSequence );
	//SetIdealActivity( ACT_VM_IDLE );
	//SetTouch(&CFlag::MyTouch);*/
	
	//SetSequence( SelectWeightedSequence( ACT_VM_IDLE ) );
	//SetSequence(0);
	
	BaseClass::Spawn( );

	SetThink( &CFlag::Think );
	SetNextThink( gpGlobals->curtime );

	//SetSolid( SOLID_BBOX );
	//SetSolid( SOLID_BBOX );
	//m_iTeam = TEAM_UNASSIGNED;
}
void CFlag::Precache( void )
{
	PrecacheModel ("models/other/flag_n.mdl");
	PrecacheModel ("models/other/flag_a.mdl");
	PrecacheModel ("models/other/flag_b.mdl");
	//BG2 - Tjoppen - temp
	//PrecacheModel ("models/other/flag_w.mdl");  // BG2 - SaintGreg - flag when disabled

	PrecacheScriptSound( "British.win" );
	PrecacheScriptSound( "Americans.win" );
	PrecacheScriptSound( "Flag.capture" );

	//PrecacheScriptSound( "ItemBattery.Touch" );

}
/*void MyTouch( CBaseEntity *pEntity )
{
#ifndef CLIENT_DLL
	CBasePlayer *pPlayer = (CBasePlayer*)pEntity;
	if( !pPlayer )
		return;
	
	if( pPlayer->GetTeamNumber() != GetTeamNumber() )
	{
		Msg( "%s touched the flag\n", pPlayer->PlayerData()->netname );
		ChangeTeam( pPlayer->GetTeamNumber() );
		CFlagHandler::Update();
	}

	//PhysicsRemoveTouchedList( this );

#endif
	/*SetCheckUntouch( true );
	EndTouch( pEntity );
	SetTouch(&CFlag::MyTouch);*//*
}*/

void CFlag::Think( void )
{
	//SetSequence( SelectWeightedSequence( ACT_VM_IDLE ) );
	//StudioFrameAdvance();

#ifndef CLIENT_DLL
	if (!m_bActive)	// if inactive, stop the thinking cycle
		return;
#endif

	CBasePlayer *pPlayer = NULL;

	int americans = 0,
		british = 0;

	switch( GetTeamNumber() )
	{
		case TEAM_AMERICANS:
			if (m_iTeamBonusInterval != 0)
			{
				if (m_flNextTeamBonus <= gpGlobals->curtime)
				{
					g_Teams[TEAM_AMERICANS]->AddScore( m_iTeamBonus );
					m_flNextTeamBonus += m_iTeamBonusInterval;
				}
			}
			break;
		case TEAM_BRITISH:
			if (m_iTeamBonusInterval != 0)
			{
				if (m_flNextTeamBonus <= gpGlobals->curtime)
				{
					g_Teams[TEAM_BRITISH]->AddScore( m_iTeamBonus );
					m_flNextTeamBonus += m_iTeamBonusInterval;
				}
			}
			break;
		default:
			switch (m_iForTeam)
			{
				case 1://amer
					if (m_iTeamBonusInterval != 0)
					{
						if (m_flNextTeamBonus <= gpGlobals->curtime)
						{
							g_Teams[TEAM_BRITISH]->AddScore( m_iTeamBonus );
							m_flNextTeamBonus += m_iTeamBonusInterval;
						}
					}
					break;
				case 2://brit
					if (m_iTeamBonusInterval != 0)
					{
						if (m_flNextTeamBonus <= gpGlobals->curtime)
						{
							g_Teams[TEAM_AMERICANS]->AddScore( m_iTeamBonus );
							m_flNextTeamBonus += m_iTeamBonusInterval;
						}
					}
					break;
			}
	}

	while( (pPlayer = (CBasePlayer*)gEntList.FindEntityByClassnameWithin( pPlayer, "player", GetLocalOrigin(), m_flCaptureRadius )) != NULL )
	{
		if( !pPlayer->IsAlive() )	//dead players don't cap
			continue;

		switch( pPlayer->GetTeamNumber() )
		{
		case TEAM_AMERICANS:
			if ((m_iForTeam == 1) || (m_iForTeam == 0))
			{
				americans++;
			}
			break;
		case TEAM_BRITISH:
			if ((m_iForTeam == 2) || (m_iForTeam == 0))
			{
				british++;
			}
			break;
		default:
			break;
		}
	}

	char *msg = NULL;

	if( americans + british > 0 && (americans <= 0 || british <= 0) )
	{
		//Msg( "\namericans = %i  british = %i\n", americans, british );

		//only americans or british at the flag
		//if we don't already own it, and we've been here for at least three seconds - capture

		if(americans > 0)
		{
			m_iRequestingCappers = TEAM_AMERICANS;
			if (americans >= min( m_iCapturePlayers, g_Teams[TEAM_AMERICANS]->GetNumPlayers() ) && GetTeamNumber() != TEAM_AMERICANS )
			{
				//Msg( "americans\n" );
				if( m_iLastTeam != TEAM_AMERICANS )
				{
					char msg2[512];
					//Msg( "americans are capturing a flag(\"%s\")\n", STRING( m_sFlagName.Get() ) );
					Q_snprintf( msg2, 512, "The americans are capturing a flag(%s)", STRING( m_sFlagName.Get() ) );
					msg = msg2;
					m_iLastTeam = TEAM_AMERICANS;
					m_flNextCapture = gpGlobals->curtime + m_flCaptureTime;

					m_OnAmericanStartCapture.FireOutput( this, this );
					m_OnStartCapture.FireOutput( this, this );
				}
				else if( gpGlobals->curtime >= m_flNextCapture )
				{
					char msg2[512];
					//CFlagHandler::PlayCaptureSound();
					Q_snprintf( msg2, 512, "The americans captured a flag(%s)", STRING( m_sFlagName.Get() ) );
					msg = msg2;
					EmitSound( "Flag.capture" );
					g_Teams[TEAM_AMERICANS]->AddScore( m_iTeamBonus );
					m_flNextTeamBonus = (gpGlobals->curtime + m_iTeamBonusInterval);

					while( (pPlayer = (CBasePlayer*)gEntList.FindEntityByClassnameWithin( pPlayer, "player", GetLocalOrigin(), m_flCaptureRadius )) != NULL )
					{
						if( !pPlayer->IsAlive() )	//dead players don't cap
							continue;

						switch( pPlayer->GetTeamNumber() )
						{
							case TEAM_AMERICANS:
								pPlayer->IncrementFragCount(m_iPlayerBonus);
								CHL2MP_Player *pPlayer2 = ToHL2MPPlayer(pPlayer);
								pPlayer2->IncreaseReward(1);
								break;
						}
					}
					m_iLastTeam = TEAM_UNASSIGNED;
					m_iRequestingCappers = TEAM_UNASSIGNED;

					// before we change team, if they stole the point, fire the output
					if (GetTeamNumber() == TEAM_BRITISH)
					{
						m_OnBritishLosePoint.FireOutput( this, this );
						m_OnLosePoint.FireOutput( this, this );
					}

					ChangeTeam( TEAM_AMERICANS );
					CFlagHandler::Update();

					m_OnAmericanCapture.FireOutput( this, this );
					m_OnCapture.FireOutput( this, this );
				}
			}
		}
		else if(british > 0)
		{
			m_iRequestingCappers = TEAM_BRITISH;
			if (british >= min( m_iCapturePlayers, g_Teams[TEAM_BRITISH]->GetNumPlayers() ) && GetTeamNumber() != TEAM_BRITISH )
			{
				//Msg( "british\n" );
				if( m_iLastTeam != TEAM_BRITISH )
				{
					char msg2[512];
					//Msg( "british are capturing a flag(\"%s\")\n", STRING( m_sFlagName.Get() ) );
					Q_snprintf( msg2, 512, "The british are capturing a flag(%s)", STRING( m_sFlagName.Get() ) );
					msg = msg2;
					m_iLastTeam = TEAM_BRITISH;
					m_flNextCapture = gpGlobals->curtime + m_flCaptureTime;

					m_OnBritishStartCapture.FireOutput( this, this );
					m_OnStartCapture.FireOutput( this, this );
				}
				else if( gpGlobals->curtime >= m_flNextCapture )
				{
					char msg2[512];
					//CFlagHandler::PlayCaptureSound();
					Q_snprintf( msg2, 512, "The british captured a flag(%s)", STRING( m_sFlagName.Get() ) );
					msg = msg2;
					EmitSound( "Flag.capture" );
					g_Teams[TEAM_BRITISH]->AddScore( m_iTeamBonus );
					m_flNextTeamBonus = (gpGlobals->curtime + m_iTeamBonusInterval);

					while( (pPlayer = (CBasePlayer*)gEntList.FindEntityByClassnameWithin( pPlayer, "player", GetLocalOrigin(), m_flCaptureRadius )) != NULL )
					{
						if( !pPlayer->IsAlive() )	//dead players don't cap
							continue;

						switch( pPlayer->GetTeamNumber() )
						{
							case TEAM_BRITISH:
								pPlayer->IncrementFragCount(m_iPlayerBonus);
								CHL2MP_Player *pPlayer2 = ToHL2MPPlayer(pPlayer);
								pPlayer2->IncreaseReward(1);
								break;
						}
					}

					// before we change team, if they stole the point, fire the output
					if (GetTeamNumber() == TEAM_AMERICANS)
					{
						m_OnAmericanLosePoint.FireOutput( this, this );
						m_OnLosePoint.FireOutput( this, this );
					}

					ChangeTeam( TEAM_BRITISH );
					CFlagHandler::Update();
					m_iRequestingCappers = TEAM_UNASSIGNED;
					m_iLastTeam = TEAM_UNASSIGNED;

					m_OnBritishCapture.FireOutput( this, this );
					m_OnCapture.FireOutput( this, this );
				}
			}
		}
	}
	else
	{
		if( m_iLastTeam != TEAM_UNASSIGNED )
		{
			//Msg( "stopped capturing a flag\n" );
			if( m_iLastTeam == TEAM_AMERICANS && GetTeamNumber() != TEAM_AMERICANS )
			{
				char msg2[512];
				Q_snprintf( msg2, 512, "The americans stopped capturing a flag(%s)", STRING( m_sFlagName.Get() ) );
				msg = msg2;

				m_OnAmericanStopCapture.FireOutput( this, this );
				m_OnStopCapture.FireOutput( this, this );
			}
			else if( m_iLastTeam == TEAM_BRITISH && GetTeamNumber() != TEAM_BRITISH )
			{
				char msg2[512];
				Q_snprintf( msg2, 512, "The british stopped capturing a flag(%s)", STRING( m_sFlagName.Get() ) );
				msg = msg2;

				m_OnBritishStopCapture.FireOutput( this, this );
				m_OnStopCapture.FireOutput( this, this );
			}
		}
		//noone here
		m_iLastTeam = TEAM_UNASSIGNED;
		m_iRequestingCappers = TEAM_UNASSIGNED;
		
		m_flNextCapture = 0;
	}

	ClientPrintAll( msg );
//#endif
	SetNextThink( gpGlobals->curtime + 0.5f );
}

void CFlag::ChangeTeam( int iTeamNum )
{
//#ifndef CLIENT_DLL
#ifndef CLIENT_DLL
		if (HasSpawnFlags( CFlag_START_DISABLED ))
		{
			m_bActive = false;
			SetModel( "models/other/flag_w.mdl" );
			return;
		}
		
		m_bActive = true;
#endif // CLIENT_DLL

	CTeam *pAmericans = g_Teams[TEAM_AMERICANS];
	CTeam *pBritish = g_Teams[TEAM_BRITISH];
	
	switch( iTeamNum )
	{
	case TEAM_AMERICANS:
		SetModel( "models/other/flag_a.mdl" );
		pAmericans->AddMorale(m_flMoraleAdd, m_flMoraleAddTime);
		break;
	case TEAM_BRITISH:
		pBritish->AddMorale(m_flMoraleAdd, m_flMoraleAddTime);
		SetModel( "models/other/flag_b.mdl" );
		break;
	default:
		switch( m_iForTeam )
		{
			case 1://amer
				SetModel( "models/other/flag_b.mdl" );
				break;
			case 2://brit
				SetModel( "models/other/flag_a.mdl" );
				break;
			default:
				SetModel( "models/other/flag_n.mdl" );
				break;
		}
		//SetModel( "models/other/flag_n.mdl" );
		break;
	}
//#endif
	//m_iLastTeam = iTeamNum;
	//m_iLastTeam = TEAM_UNASSIGNED;

	BaseClass::ChangeTeam( iTeamNum );
}

int CFlag::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

IMPLEMENT_NETWORKCLASS_ALIASED( Flag, DT_Flag )

BEGIN_NETWORK_TABLE( CFlag, DT_Flag )
	SendPropInt( SENDINFO( m_iLastTeam ), Q_log2(NUM_TEAMS), SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iRequestingCappers ), Q_log2(NUM_TEAMS), SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_flNextCapture ) ),
	SendPropInt( SENDINFO( m_iCapturePlayers ), Q_log2(MAX_PLAYERS), SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iForTeam ), 2, SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_flCaptureTime ) ),
	SendPropStringT( SENDINFO( m_sFlagName ) ),
	SendPropInt( SENDINFO( m_iHUDSlot ), 4, SPROP_UNSIGNED ),	//15 slots.. 0 = sequential tile
	SendPropBool( SENDINFO( m_bActive ) ),
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CFlag )
END_PREDICTION_DATA()

BEGIN_DATADESC( CFlag )

	DEFINE_KEYFIELD( m_iCapturePlayers, FIELD_INTEGER, "CapturePlayers" ),
	DEFINE_KEYFIELD( m_flCaptureRadius, FIELD_FLOAT, "CaptureRadius" ),
	DEFINE_KEYFIELD( m_flCaptureTime, FIELD_FLOAT, "CaptureTime" ),
	DEFINE_KEYFIELD( m_iUncap, FIELD_INTEGER, "Uncap" ),
	DEFINE_KEYFIELD( m_iTeamBonus, FIELD_INTEGER, "TeamBonus" ),
	DEFINE_KEYFIELD( m_iTeamBonusInterval, FIELD_INTEGER, "TeamBonusInterval" ),
	DEFINE_KEYFIELD( m_iPlayerBonus, FIELD_INTEGER, "PlayerBonus" ),
	DEFINE_KEYFIELD( m_iForTeam, FIELD_INTEGER, "ForTeam" ),
	DEFINE_KEYFIELD( m_sFlagName, FIELD_STRING, "FlagName" ),
	DEFINE_KEYFIELD( m_iHUDSlot, FIELD_INTEGER, "HUDSlot" ),
	DEFINE_KEYFIELD( m_flMoraleAdd, FIELD_FLOAT, "MoraleAdd" ),
	DEFINE_KEYFIELD( m_flMoraleAddTime, FIELD_FLOAT, "MoraleAddTime" ),
#ifndef CLIENT_DLL
	DEFINE_THINKFUNC( Think ),
#endif

#ifndef CLIENT_DLL
	//BG2 - SaintGreg - dynamic flags
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

	//BG2 - SaintGreg - Output functions similar to BG's
	DEFINE_OUTPUT( m_OnAmericanStartCapture, "OnAmericanStartCapture" ),
	DEFINE_OUTPUT( m_OnBritishStartCapture, "OnBritishStartCapture" ),
	DEFINE_OUTPUT( m_OnStartCapture, "OnStartCapture" ),
	DEFINE_OUTPUT( m_OnAmericanCapture, "OnAmericanCapture" ),
	DEFINE_OUTPUT( m_OnBritishCapture, "OnBritishCapture" ),
	DEFINE_OUTPUT( m_OnCapture, "OnCapture" ),
	DEFINE_OUTPUT( m_OnAmericanStopCapture, "OnAmericanStopCapture" ),
	DEFINE_OUTPUT( m_OnBritishStopCapture, "OnBritishStopCapture" ),
	DEFINE_OUTPUT( m_OnStopCapture, "OnStopCapture" ),
	DEFINE_OUTPUT( m_OnAmericanLosePoint, "OnAmericanLosePoint" ),
	DEFINE_OUTPUT( m_OnBritishLosePoint, "OnBritishLosePoint" ),
	DEFINE_OUTPUT( m_OnLosePoint, "OnLosePoint" ),
	DEFINE_OUTPUT( m_OnEnable, "OnEnable" ),
	DEFINE_OUTPUT( m_OnDisable, "OnDisable" ),
#endif // CLIENT_DLL

END_DATADESC()

LINK_ENTITY_TO_CLASS(flag, CFlag);
PRECACHE_REGISTER(flag);



bool CMoraleBooster::IsActive( void )
{
	return m_bActive;
}

void CMoraleBooster::InputAmerican( inputdata_t &inputData )
{
	ChangeTeam( TEAM_AMERICAN );
}
void CMoraleBooster::InputBritish( inputdata_t &inputData )
{
	ChangeTeam( TEAM_BRITISH );
}

void CMoraleBooster::InputEnable( inputdata_t &inputData )
{
	if( m_bActive )
		return;
	m_bActive = true;
	ChangeTeam( TEAM_UNASSIGNED );
}
void CMoraleBooster::InputDisable( inputdata_t &inputData )
{
	if( !m_bActive )
		return;
	m_bActive = false;
	ChangeTeam( TEAM_UNASSIGNED );
}
void CMoraleBooster::InputToggle( inputdata_t &inputData )
{
	if (m_bActive)
		InputDisable( inputData );
	else
		InputEnable( inputData );
}

void CMoraleBooster::Spawn( void )
{
	Precache( );

	ChangeTeam( TEAM_UNASSIGNED );	//ChangeTeam handles everything..

	BaseClass::Spawn( );
}
void CMoraleBooster::Precache( void )
{
}

void CMoraleBooster::ChangeTeam( int iTeamNum )
{
	if (GetTeamNumber() != iTeamNum)
	{
		switch(GetTeamNumber())
		{
			case TEAM_AMERICAN:
				if (iTeamNum == TEAM_BRITISH)
				{
					g_Teams[TEAM_BRITISH]->AddMoralePoint(this);
				}
				g_Teams[TEAM_AMERICAN]->RemoveMoralePoint(this);
				break;
			case TEAM_BRITISH:
				if (iTeamNum == TEAM_AMERICAN)
				{
					g_Teams[TEAM_AMERICAN]->AddMoralePoint(this);
				}
				g_Teams[TEAM_AMERICAN]->RemoveMoralePoint(this);
				break;
			default:
				switch (iTeamNum)
				{
					case TEAM_BRITISH:
						g_Teams[TEAM_BRITISH]->AddMoralePoint(this);
						break;
					case TEAM_AMERICAN:
						g_Teams[TEAM_AMERICAN]->AddMoralePoint(this);
						break;
				}
				g_Teams[iTeamNum]->AddMoralePoint(this);
				break;
		}
		BaseClass::ChangeTeam( iTeamNum );
	}
}

IMPLEMENT_NETWORKCLASS_ALIASED( MoraleBooster, DT_MoraleBooster )

BEGIN_NETWORK_TABLE( CMoraleBooster, DT_MoraleBooster )
	SendPropBool( SENDINFO( m_bActive ) ),
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CMoraleBooster )
END_PREDICTION_DATA()

BEGIN_DATADESC( CMoraleBooster )

	DEFINE_KEYFIELD( m_flMoraleAdd, FIELD_FLOAT, "MoraleAdd" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "British", InputBritish ),
	DEFINE_INPUTFUNC( FIELD_VOID, "American", InputAmerican ),

END_DATADESC()

LINK_ENTITY_TO_CLASS(moralebooster, CMoraleBooster);
PRECACHE_REGISTER(moralebooster);




#ifndef CLIENT_DLL
//BG2 - Tjoppen - just make this global so we can reset it
float nextwinsong = 0;	//to prevent win spamming sound issues..

void CFlagHandler::RespawnAll( char *pSound )
{
	if( nextwinsong > gpGlobals->curtime )
		pSound = NULL;	//what a nice hack
	else
		nextwinsong = gpGlobals->curtime + 20;

	bool spawn = true;	//set to false if we ran out of spawn points

	int x;
	for( x = 0; x < g_Teams[TEAM_AMERICANS]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_AMERICANS]->GetPlayer( x ) );

		if( !pPlayer )
			continue;

		//check if we've run out of spawn points. if we alread have, we can skip this step
		if( spawn && !pPlayer->CheckSpawnPoints() )
			spawn = false;

		//CBasePlayer *pPlayer = g_Teams[TEAM_AMERICANS]->GetPlayer( x );
		/*if( pPlayer->IsAlive() )
			pPlayer->Kill();*/

		if( spawn )
			pPlayer->Spawn();
		else if( pPlayer->IsAlive() )
		{
			//if there's no spawn points and the player isn't dead - just kill 'em!
			pPlayer->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
			pPlayer->SetNextThink( gpGlobals->curtime + 3.0f );	//don't respawn in a while
		}

		//BG2 - Tjoppen - remove ragdoll - remember to change this to remove multiple ones if we decide to enable more corpses
		if( pPlayer->m_hRagdoll )
		{
			UTIL_RemoveImmediate( pPlayer->m_hRagdoll );
			pPlayer->m_hRagdoll = NULL;
		}

		if( pSound )
		{
			//pPlayer->EmitSound( pSound );
			/*CPASAttenuationFilter filter( this );
			filter.UsePredictionRules();
			EmitSound( filter, entindex(), "HL2Player.SprintNoPower" );*/
			//C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "HUDQuickInfo.LowHealth" );
			//CPASAttenuationFilter filter( pPlayer, ATTN_NONE );
			CPASAttenuationFilter filter( pPlayer, 10.0f );	//high attenuation so only this player hears it
			filter.UsePredictionRules();
			pPlayer->EmitSound( filter, pPlayer->entindex(), pSound );
		}
	}

	spawn = true;	//reset for other team

	for( x = 0; x < g_Teams[TEAM_BRITISH]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_BRITISH]->GetPlayer( x ) );

		if( !pPlayer )
			continue;

		//check if we've run out of spawn points. if we alread have, we can skip this step
		if( spawn && !pPlayer->CheckSpawnPoints() )
			spawn = false;

		//CBasePlayer *pPlayer = g_Teams[TEAM_BRITISH]->GetPlayer( x );
		/*if( pPlayer->IsAlive() )
			pPlayer->Kill();*/

		if( spawn )
			pPlayer->Spawn();
		else if( pPlayer->IsAlive() )
		{
			//if there's no spawn points and the player isn't dead - just kill 'em!
			pPlayer->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
			pPlayer->SetNextThink( gpGlobals->curtime + 3.0f );	//don't respawn in a while
		}

		//BG2 - Tjoppen - remove ragdoll - remember to change this to remove multiple ones if we decide to enable more corpses
		if( pPlayer->m_hRagdoll )
		{
			UTIL_RemoveImmediate( pPlayer->m_hRagdoll );
			pPlayer->m_hRagdoll = NULL;
		}

		if( pSound )
		{
			//pPlayer->EmitSound( pSound );
			//CPASAttenuationFilter filter( pPlayer, ATTN_NONE );
			CPASAttenuationFilter filter( pPlayer, 10.0f );	//high attenuation so only this player hears it
			filter.UsePredictionRules();
			pPlayer->EmitSound( filter, pPlayer->entindex(), pSound );
		}
	}

	/*HL2MPRules()->m_bIsRestartingRound = false;
	HL2MPRules()->m_flNextRoundRestart = gpGlobals->curtime + 1;*/
}

void CFlagHandler::RespawnWave()
{
	int x;
	for( x = 0; x < g_Teams[TEAM_AMERICANS]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_AMERICANS]->GetPlayer( x ) );
		if( pPlayer && !pPlayer->IsAlive() )
		{
			if( !pPlayer->CheckSpawnPoints() )
				break;

			pPlayer->Spawn();
		}
	}

	for( x = 0; x < g_Teams[TEAM_BRITISH]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_BRITISH]->GetPlayer( x ) );
		if( pPlayer && !pPlayer->IsAlive() )
		{
			if( !pPlayer->CheckSpawnPoints() )
				break;

			pPlayer->Spawn();
		}
	}

	/*for( x = 0; x < g_Teams[TEAM_AMERICANS]->GetNumPlayers(); x++ )
	{
		CBasePlayer *pPlayer = g_Teams[TEAM_AMERICANS]->GetPlayer( x );
		if( !pPlayer->IsAlive() )
		{
			if( !pPlayer->CheckSpawnPoints() )
				break;

			pPlayer->Spawn();
		}
	}

	for( x = 0; x < g_Teams[TEAM_BRITISH]->GetNumPlayers(); x++ )
	{
		CBasePlayer *pPlayer = g_Teams[TEAM_BRITISH]->GetPlayer( x );
		if( !pPlayer->IsAlive() )
		{
			if( !pPlayer->CheckSpawnPoints() )
				break;

			pPlayer->Spawn();
		}
	}*/
}

/*void CFlagHandler::PlayCaptureSound( void )
{
	/*CBasePlayer *pPlayer = NULL;
	while( (pPlayer = (CBasePlayer*)gEntList.FindEntityByClassname( pPlayer, "player" )) != NULL )
		pPlayer->EmitSound( "Flag.capture" );*//*
}*/

void CFlagHandler::ResetFlags( void )
{
	CBaseEntity *pEntity = NULL;

	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "flag" )) != NULL )
	{
		CFlag *pFlag = (CFlag*)pEntity;
		if( !pFlag )
			continue;

		if( pFlag->GetTeamNumber() == TEAM_AMERICANS )
		{
			pFlag->m_OnAmericanLosePoint.FireOutput( pFlag, pFlag );
			pFlag->m_OnLosePoint.FireOutput( pFlag, pFlag );
		}
		else if( pFlag->GetTeamNumber() == TEAM_BRITISH )
		{
			pFlag->m_OnBritishLosePoint.FireOutput( pFlag, pFlag );
			pFlag->m_OnLosePoint.FireOutput( pFlag, pFlag );
		}

		pFlag->ChangeTeam( TEAM_UNASSIGNED );	//ChangeTeam handles everything..
		pFlag->m_iLastTeam = TEAM_UNASSIGNED;
		pFlag->m_iRequestingCappers = TEAM_UNASSIGNED;

#ifndef CLIENT_DLL
		if (pFlag->HasSpawnFlags( CFlag_START_DISABLED ))
		{
			pFlag->m_bActive = false;
			pFlag->SetModel( "models/other/flag_w.mdl" );
		}
		else
		{
			pFlag->m_bActive = true;
			//SetModel( "models/other/flag_n.mdl" );
		}
#endif // CLIENT_DLL
	}

	//BG2 - Tjoppen - reset spawnpoints aswell.. this is a hack. we should really have a proper RoundRestart() function somewhere
	//also, I'm considering removing support for info_player_rebel and info_player_combine..
	pEntity = NULL;
	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "info_player_rebel" )) != NULL )
	{
		CSpawnPoint *pPoint = (CSpawnPoint*)pEntity;
		if( !pPoint )
			continue;

		pPoint->Reset();
	}

	pEntity = NULL;
	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "info_player_combine" )) != NULL )
	{
		CSpawnPoint *pPoint = (CSpawnPoint*)pEntity;
		if( !pPoint )
			continue;

		pPoint->Reset();
	}

	pEntity = NULL;
	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "info_player_american" )) != NULL )
	{
		CSpawnPoint *pPoint = (CSpawnPoint*)pEntity;
		if( !pPoint )
			continue;

		pPoint->Reset();
	}

	pEntity = NULL;
	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "info_player_british" )) != NULL )
	{
		CSpawnPoint *pPoint = (CSpawnPoint*)pEntity;
		if( !pPoint )
			continue;

		pPoint->Reset();
	}
	//end of spawnpoint resettings
}

void CFlagHandler::Update( void )
{
	CBaseEntity *pEntity = NULL;
	
	int	american_flags = 0,
		british_flags = 0,
		neutral_flags = 0;
	int	foramericans = 0;
	int	forbritish = 0;
	//BG2 - Tjoppen - not needed
	/*int iNumFlags = 0;
	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "flag" )) != NULL )
	{
		iNumFlags++;
	}*/

	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "flag" )) != NULL )
	{
		CFlag *pFlag = (CFlag*)pEntity;
		if( !pFlag || !pFlag->IsActive() )
			continue;

		switch( pFlag->GetTeamNumber() )
		{
		case TEAM_AMERICANS:
			american_flags++;
			break;
		case TEAM_BRITISH:
			british_flags++;
			break;
		default:
			neutral_flags++;
			break;
		}
		switch(pFlag->m_iForTeam)
		{
			case 0:
				foramericans++;
				forbritish++;
				break;
			case 1:
				foramericans++;
				break;
			case 2:
				forbritish++;
				break;
			default://assume both
				foramericans++;
				forbritish++;
				break;
		}
	}

	/*Msg( "american_flags = %i\n", american_flags );
	Msg( "british_flags = %i\n", british_flags );
	Msg( "neutral_flags = %i\n", neutral_flags );*/

	if( !american_flags && !british_flags && !neutral_flags )
		return;

	if( neutral_flags > 0 )
	{
		if( (foramericans - american_flags) == 0 && foramericans != 0 )
		{
			ClientPrintAll( "The americans won this round!", true );
			g_Teams[TEAM_AMERICANS]->AddScore( 200 );
			ResetFlags();
			RespawnAll( "Americans.win" );
			//do not cause two simultaneous round restarts..
			HL2MPRules()->m_bIsRestartingRound = false;
			HL2MPRules()->m_flNextRoundRestart = gpGlobals->curtime + 1;
			return;
		}
		if( (forbritish - british_flags) == 0 && forbritish != 0 )
		{
			ClientPrintAll( "The british won this round!", true );
			g_Teams[TEAM_BRITISH]->AddScore( 200 );
			ResetFlags();
			RespawnAll( "British.win" );
			//do not cause two simultaneous round restarts..
			HL2MPRules()->m_bIsRestartingRound = false;
			HL2MPRules()->m_flNextRoundRestart = gpGlobals->curtime + 1;
			return;
		}
	}
	else
	{
		if( american_flags <= 0 && british_flags <= 0 )
		{
			//draw
			//Msg( "draw\n" );
			ClientPrintAll( "This round became a draw", true );
			ResetFlags();
			RespawnAll( NULL );
			//do not cause two simultaneous round restarts..
			HL2MPRules()->m_bIsRestartingRound = false;
			HL2MPRules()->m_flNextRoundRestart = gpGlobals->curtime + 1;
			return;
		}

		if ( american_flags <= 0 )
		{
			//british win
			//Msg( "british win\n" );
			ClientPrintAll( "The british won this round!", true );
			g_Teams[TEAM_BRITISH]->AddScore( 200 );
			ResetFlags();
			RespawnAll( "British.win" );
			//do not cause two simultaneous round restarts..
			HL2MPRules()->m_bIsRestartingRound = false;
			HL2MPRules()->m_flNextRoundRestart = gpGlobals->curtime + 1;
			return;
		}

		if ( british_flags <= 0 )
		{
			//americans win
			//Msg( "americans win\n" );
			ClientPrintAll( "The americans won this round!", true );
			g_Teams[TEAM_AMERICANS]->AddScore( 200 );
			ResetFlags();
			RespawnAll( "Americans.win" );
			//do not cause two simultaneous round restarts..
			HL2MPRules()->m_bIsRestartingRound = false;
			HL2MPRules()->m_flNextRoundRestart = gpGlobals->curtime + 1;
			return;
		}
	}
}
#endif