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
//#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//#ifndef CLIENT_DLL
//BG2 - Tjoppen - just make these global so we can reset them
float	flNextClientPrintAll = 0;
bool	bNextClientPrintAllForce = false;

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
	m_iLastTeam = TEAM_UNASSIGNED;

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

//#ifndef CLIENT_DLL
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

		if( americans > 0 &&
			americans >= min( m_iCapturePlayers, g_Teams[TEAM_AMERICANS]->GetNumPlayers() ) &&
			GetTeamNumber() != TEAM_AMERICANS )
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
				ChangeTeam( TEAM_AMERICANS );
				CFlagHandler::Update();
			}
		}
		else if( british > 0 &&
			british >= min( m_iCapturePlayers, g_Teams[TEAM_BRITISH]->GetNumPlayers() ) &&
			GetTeamNumber() != TEAM_BRITISH )
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
				ChangeTeam( TEAM_BRITISH );
				CFlagHandler::Update();
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
			}
			else if( m_iLastTeam == TEAM_BRITISH && GetTeamNumber() != TEAM_BRITISH )
			{
				char msg2[512];
				Q_snprintf( msg2, 512, "The british stopped capturing a flag(%s)", STRING( m_sFlagName.Get() ) );
				msg = msg2;
			}
		}
		//noone here
		m_iLastTeam = TEAM_UNASSIGNED;
		m_flNextCapture = 0;
	}

	ClientPrintAll( msg );
//#endif
	SetNextThink( gpGlobals->curtime + 0.5f );
}

void CFlag::ChangeTeam( int iTeamNum )
{
//#ifndef CLIENT_DLL
	switch( iTeamNum )
	{
	case TEAM_AMERICANS:
		SetModel( "models/other/flag_a.mdl" );
		break;
	case TEAM_BRITISH:
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
	SendPropFloat( SENDINFO( m_flNextCapture ) ),
	SendPropInt( SENDINFO( m_iCapturePlayers ), Q_log2(MAX_PLAYERS), SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iForTeam ), 2, SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_flCaptureTime ) ),
	SendPropStringT( SENDINFO( m_sFlagName ) ),
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

#ifndef CLIENT_DLL
	DEFINE_THINKFUNC( Think ),
#endif

END_DATADESC()

LINK_ENTITY_TO_CLASS(flag, CFlag);
PRECACHE_REGISTER(flag);

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

		if( spawn && !pPlayer->CheckSpawnPoints() )
			spawn = false;

		//CBasePlayer *pPlayer = g_Teams[TEAM_AMERICANS]->GetPlayer( x );
		/*if( pPlayer->IsAlive() )
			pPlayer->Kill();*/

		//BG2 - Tjoppen - remove ragdoll - remember to change this to remove multiple ones if we decide to enable more corpses
		if( pPlayer->m_hRagdoll )
		{
			UTIL_RemoveImmediate( pPlayer->m_hRagdoll );
			pPlayer->m_hRagdoll = NULL;
		}

		if( spawn )
			pPlayer->Spawn();
		else if( pPlayer->IsAlive() )
		{
			//if there's no spawn points and the player isn't dead - just kill 'em!
			pPlayer->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
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

		if( spawn && !pPlayer->CheckSpawnPoints() )
			spawn = false;

		//CBasePlayer *pPlayer = g_Teams[TEAM_BRITISH]->GetPlayer( x );
		/*if( pPlayer->IsAlive() )
			pPlayer->Kill();*/

		if( spawn )
			pPlayer->Spawn();

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

		pFlag->ChangeTeam( TEAM_UNASSIGNED );	//ChangeTeam handles everything..
		pFlag->m_iLastTeam = TEAM_UNASSIGNED;
	}
}

void CFlagHandler::Update( void )
{
	CBaseEntity *pEntity = NULL;
	
	int	american_flags = 0,
		british_flags = 0,
		neutral_flags = 0;
	int	foramericans = 0;
	int	forbritish = 0;
	int iNumFlags = 0;
	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "flag" )) != NULL )
	{
		iNumFlags++;
	}

	/*CBasePlayer *pPlayer = NULL;
	while( (pPlayer = (CBasePlayer*)gEntList.FindEntityByClassname( pPlayer, "player" )) != NULL )
	{
		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();
		UserMessageBegin( user, "flagstatus" );
		WRITE_BYTE(iNumFlags);
		char szTemp[512] = "";
		while( (pEntity = gEntList.FindEntityByClassname( pEntity, "flag" )) != NULL )
		{
			CFlag *pFlag = (CFlag*)pEntity;
			if( !pFlag )
				continue;
			Q_snprintf( szTemp, 512, "%s", STRING( pFlag->m_sFlagName.Get() ));
			WRITE_STRING(szTemp);
			WRITE_BYTE(pFlag->GetTeamNumber());
			if (pFlag->m_flNextCapture <= gpGlobals->curtime)
			{
				WRITE_SHORT(0);
			}
			else
			{
				WRITE_SHORT((int)(pFlag->m_flNextCapture - gpGlobals->curtime));
			}
			WRITE_BYTE(pFlag->m_iCapturePlayers);
			WRITE_BYTE(pFlag->m_iForTeam);
			WRITE_SHORT((int)pFlag->m_flCaptureTime);
			WRITE_BYTE(pFlag->m_iLastTeam);
		}
		MessageEnd();
	}*/

	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "flag" )) != NULL )
	{
		CFlag *pFlag = (CFlag*)pEntity;
		if( !pFlag )
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