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
		Tomas "Tjoppen" H�rdin		tjoppen@gamedev.se

	You may also contact the (future) team via the Battle Grounds website and/or forum at:
		www.bgmod.com

	 Note that because of the sheer volume of files in the Source SDK this
	notice cannot be put in all of them, but merely the ones that have any
	changes from the original SDK.
	 In order to facilitate easy searching, all changes are and must be
	commented on the following form:

	//BG2 - <name of contributer>[ - <small description>]
*/

//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hl2mp_gamerules.h"
#include "viewport_panel_names.h"
#include "gameeventdefs.h"
#include <KeyValues.h>
#include "ammodef.h"



#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else

	#include "eventqueue.h"
	#include "player.h"
	#include "gamerules.h"
	#include "game.h"
	#include "items.h"
	#include "entitylist.h"
	#include "in_buttons.h"
	#include <ctype.h>
	#include "voice_gamemgr.h"
	#include "iscorer.h"
	#include "hl2mp_player.h"
	#include "weapon_hl2mpbasehlmpcombatweapon.h"
	#include "team.h"
	#include "voice_gamemgr.h"

//BG2 - Draco
#include "../bg2/flag.h"
//BG2 - Tjoppen - #includes
#include "sdk/sdk_bot_temp.h"

ConVar sv_hl2mp_weapon_respawn_time( "sv_hl2mp_weapon_respawn_time", "20", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_hl2mp_item_respawn_time( "sv_hl2mp_item_respawn_time", "30", FCVAR_GAMEDLL | FCVAR_NOTIFY );

extern ConVar mp_chattime;
//BG2 - Draco - Start
ConVar mp_respawnstyle( "mp_respawnstyle", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar mp_respawntime( "mp_respawntime", "5", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_restartround( "sv_restartround", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar mp_americanscore( "mp_americanscore", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar mp_britishscore( "mp_britishscore", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar mp_autobalanceteams( "mp_autobalanceteams", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar mp_autobalancetolerance( "mp_autobalancetolerance", "3", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar mp_timeleft( "mp_timeleft", "1200", FCVAR_GAMEDLL);
//BG2 - Draco - End


extern CBaseEntity	 *g_pLastCombineSpawn;
extern CBaseEntity	 *g_pLastRebelSpawn;

#define WEAPON_MAX_DISTANCE_FROM_SPAWN 64

#endif


REGISTER_GAMERULES_CLASS( CHL2MPRules );

BEGIN_NETWORK_TABLE_NOBASE( CHL2MPRules, DT_HL2MPRules )

	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bTeamPlayEnabled ) ),
		RecvPropInt( RECVINFO( m_iWaveTime ) ),
	#else
		SendPropBool( SENDINFO( m_bTeamPlayEnabled ) ),
		SendPropInt( SENDINFO( m_iWaveTime ) ),
	#endif

END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( hl2mp_gamerules, CHL2MPGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( HL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )


#ifdef CLIENT_DLL
	void RecvProxy_HL2MPRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CHL2MPRules *pRules = HL2MPRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CHL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )
		RecvPropDataTable( "hl2mp_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_HL2MPRules ), RecvProxy_HL2MPRules )
	END_RECV_TABLE()
#else
	void* SendProxy_HL2MPRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CHL2MPRules *pRules = HL2MPRules();
		Assert( pRules );
		return pRules;
	}

	BEGIN_SEND_TABLE( CHL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )
		SendPropDataTable( "hl2mp_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_HL2MPRules ), SendProxy_HL2MPRules )
	END_SEND_TABLE()
#endif

#ifndef CLIENT_DLL

	class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
	{
	public:
		virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker )
		{
			return ( pListener->GetTeamNumber() == pTalker->GetTeamNumber() );
		}
	};
	CVoiceGameMgrHelper g_VoiceGameMgrHelper;
	IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

#endif

// NOTE: the indices here must match TEAM_TERRORIST, TEAM_CT, TEAM_SPECTATOR, etc.
char *sTeamNames[] =
{
	"Unassigned",
	"Spectator",
	//BG2 - Tjoppen - team names...
	"Americans",	//combine team color is blue..
	"British",		//.. and rebel is red. no need to mess around with that
	/*"Combine",
	"Rebels",*/
};

CHL2MPRules::CHL2MPRules()
{
#ifndef CLIENT_DLL
	// Create the team managers
	for ( int i = 0; i < ARRAYSIZE( sTeamNames ); i++ )
	{
		CTeam *pTeam = static_cast<CTeam*>(CreateEntityByName( "team_manager" ));
		pTeam->Init( sTeamNames[i], i );

		g_Teams.AddToTail( pTeam );
	}

	m_bTeamPlayEnabled = teamplay.GetBool();
	m_flIntermissionEndTime = 0.0f;

	m_hRespawnableItemsAndWeapons.RemoveAll();

	//BG2 - Tjoppen - m_fLastRespawnWave, ClientPrintAll()- and bot initialization
	m_fLastRespawnWave = 0;

	g_CurBotNumber = 1;
	for( int x = 0; x < MAX_PLAYERS; x++ )
	{
		gBots[x].m_pPlayer = NULL;
		gBots[x].m_bInuse = false;
	}

	extern float flNextClientPrintAll;
	extern bool bNextClientPrintAllForce;

	flNextClientPrintAll = 0;
	bNextClientPrintAllForce = false;

	extern float nextwinsong;
	nextwinsong = 0;

	extern CBaseEntity *g_pLastIntermission;
	g_pLastIntermission = NULL;
	//
	mp_britishscore.SetValue(0);
	mp_americanscore.SetValue(0);
	m_fAdditionTime = 0;
	m_fEndRoundTime = -1;
	m_fNextFlagUpdate = 0;
	m_iWaveTime = 0;
	m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
	m_fLastRespawnWave = gpGlobals->curtime;
	m_iTDMTeamThatWon = 0;
	m_bHasDoneWinSong = false;
#endif
}
	
CHL2MPRules::~CHL2MPRules( void )
{
#ifndef CLIENT_DLL
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();
#endif
}

void CHL2MPRules::CreateStandardEntities( void )
{

#ifndef CLIENT_DLL
	// Create the entity that will send our data to the client.

	BaseClass::CreateStandardEntities();

	g_pLastCombineSpawn = NULL;
	g_pLastRebelSpawn = NULL;

#ifdef _DEBUG
	CBaseEntity *pEnt = 
#endif
	CBaseEntity::Create( "hl2mp_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
#endif
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CHL2MPRules::FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( weaponstay.GetInt() > 0 )
	{
		// make sure it's only certain weapons
		if ( !(pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
		{
			return 0;		// weapon respawns almost instantly
		}
	}

	return sv_hl2mp_weapon_respawn_time.GetFloat();
#endif

	return 0;		// weapon respawns almost instantly
}


bool CHL2MPRules::IsIntermission( void )
{
#ifndef CLIENT_DLL
	return m_flIntermissionEndTime > gpGlobals->curtime;
#endif

	return false;
}

void CHL2MPRules::Think( void )
{

#ifndef CLIENT_DLL

	CGameRules::Think();
	CTeam *pAmericans = g_Teams[TEAM_AMERICANS];
	CTeam *pBritish = g_Teams[TEAM_BRITISH];
	void ClientPrintAll( char *str, bool printfordeadplayers, bool forcenextclientprintall );
	if ( g_fGameOver )   // someone else quit the game already
	{
		if (!m_bHasDoneWinSong)
		{
			m_bHasDoneWinSong = true;
			if (pAmericans->GetScore() < pBritish->GetScore())
			{
				ClientPrintAll( "British win!", true, true );
				CFlagHandler::WinSong("British.win");
			}

			if (pAmericans->GetScore() > pBritish->GetScore())
			{
				ClientPrintAll( "Americans win!", true, true );
				CFlagHandler::WinSong("Americans.win");
			}

			if (pAmericans->GetScore() == pBritish->GetScore())
			{
				ClientPrintAll( "Draw!", true, true );
			}
		}
		
		// check to see if we should change levels now
		if ( (m_flIntermissionEndTime + m_fAdditionTime) < gpGlobals->curtime )
		{
			m_bHasDoneWinSong = false;
			ChangeLevel(); // intermission is over
		}

		return;
	}

	float flTimeLimit = mp_timelimit.GetFloat() * 60;
	float flFragLimit = fraglimit.GetFloat();

	if ( flTimeLimit != 0 && gpGlobals->curtime >= flTimeLimit )
	{
		GoToIntermission();
		return;
	}

	//BG2 - Draco - Start
	//CTeam *pAmericans = g_Teams[TEAM_AMERICANS];
	//CTeam *pBritish = g_Teams[TEAM_BRITISH];
	m_iWaveTime = ((m_fLastRespawnWave + mp_respawntime.GetFloat()) - gpGlobals->curtime);
//	CFlagHandler::FlagThink();//make the flags think.
	if (m_fNextFlagUpdate <= gpGlobals->curtime)
	{
		CFlagHandler::Update();
		m_fNextFlagUpdate = gpGlobals->curtime + 1;
	}
	else if (m_fNextFlagUpdate == 0)
	{
		m_fNextFlagUpdate = gpGlobals->curtime + 1;
	}
	//=========================
	//Time Left
	//=========================
	mp_timeleft.SetValue(((flTimeLimit + m_fAdditionTime) - gpGlobals->curtime));

	//=========================
	//Score Cvars
	//=========================
	if (mp_britishscore.GetInt() != pBritish->GetScore())
	{
		g_Teams[TEAM_BRITISH]->SetScore(mp_britishscore.GetInt());
	}
	if (mp_americanscore.GetInt() != pAmericans->GetScore())
	{
		g_Teams[TEAM_AMERICANS]->SetScore(mp_americanscore.GetInt());
	}
	//=========================
	//Auto Team Balance
	//=========================
	if (mp_autobalanceteams.GetInt() == 1)
	{
		//use the right sum to find diff, I don't like negative numbers...
		int iAutoTeamBalanceTeamDiff = 0;
		int iAutoTeamBalanceBiggerTeam = TEAM_BRITISH;
		if (pAmericans->GetNumPlayers() > pBritish->GetNumPlayers())
		{
			iAutoTeamBalanceTeamDiff = (pAmericans->GetNumPlayers() - pBritish->GetNumPlayers());
			iAutoTeamBalanceBiggerTeam = TEAM_AMERICANS;
		}
		else
		{
			iAutoTeamBalanceTeamDiff = (pBritish->GetNumPlayers() - pAmericans->GetNumPlayers());
			iAutoTeamBalanceBiggerTeam = TEAM_BRITISH;
		}

		if (iAutoTeamBalanceTeamDiff >= mp_autobalancetolerance.GetInt())
		{
			//here comes the tricky part, who to swap, how to swap?
			//meh, go random for now, maybe lowest scorer later...
			int iAutoTeamBalancePlayerToSwitchIndex = 0;
			CBasePlayer *pPlayer = NULL;
			switch (iAutoTeamBalanceBiggerTeam)
			{
				case TEAM_BRITISH:
					iAutoTeamBalancePlayerToSwitchIndex = random->RandomInt( 0, (pBritish->GetNumPlayers() - 1) );
					pPlayer = pBritish->GetPlayer(iAutoTeamBalancePlayerToSwitchIndex);
					if (!pPlayer->IsAlive())
					{
						pPlayer->ChangeTeam(TEAM_AMERICANS);
					}
					break;
				case TEAM_AMERICANS:
					iAutoTeamBalancePlayerToSwitchIndex = random->RandomInt( 0, (pAmericans->GetNumPlayers() - 1) );
					pPlayer = pAmericans->GetPlayer(iAutoTeamBalancePlayerToSwitchIndex);
					if (!pPlayer->IsAlive())
					{
						pPlayer->ChangeTeam(TEAM_BRITISH);
					}
					break;
			}
		}
		//well, if we aren't even now, there's always next think...
	}
	//=========================
	//Restart Round
	//=========================
	if (sv_restartround.GetInt() > 0)
	{
		m_fNextGameReset = gpGlobals->curtime + sv_restartround.GetInt();
		sv_restartround.SetValue(0);
	}
	//now if the time was set we can check for it, above zero means we are restarting
	if ((m_fNextGameReset > 0) &&(m_fNextGameReset <= gpGlobals->curtime))
	{
		m_fNextGameReset = 0;//dont reset again
		//reset scores...
		pAmericans->SetScore(0);//...for teams...
		pBritish->SetScore(0);
		int x;
		for( x = 0; x < g_Teams[TEAM_AMERICANS]->GetNumPlayers(); x++ )
		{
			CBasePlayer *pPlayer = g_Teams[TEAM_AMERICANS]->GetPlayer( x );
			pPlayer->ResetFragCount();//...for cap points...
			pPlayer->ResetDeathCount();//...and damage
		}
		for( x = 0; x < g_Teams[TEAM_BRITISH]->GetNumPlayers(); x++ )
		{
			CBasePlayer *pPlayer = g_Teams[TEAM_BRITISH]->GetPlayer( x );
			pPlayer->ResetFragCount();//...for cap points...
			pPlayer->ResetDeathCount();//...and damage
		}
		m_fAdditionTime += gpGlobals->curtime;
		CFlagHandler::ResetFlags();
        CFlagHandler::RespawnAll( NULL );//and respawn! done.
	}
	
	//=========================
	//Round systems
	//=========================
	if( mp_respawnstyle.GetInt() == 2 )//if line battle all at once spawn style - Draco
	{
		if (mp_respawntime.GetInt() == 0)
		{
			m_fEndRoundTime = 0;
		}
		//Tjoppen - start
		//count alive players in each team
		CTeam *pAmericans = g_Teams[TEAM_AMERICANS];
		CTeam *pBritish = g_Teams[TEAM_BRITISH];

		if( pAmericans->GetNumPlayers() == 0 )
			return;

		int aliveamericans = 0, x = 0;
		for( ; x < pAmericans->GetNumPlayers(); x++ )
			if( pAmericans->GetPlayer(x)->IsAlive() )
				aliveamericans++;

		if (pBritish->GetNumPlayers() == 0)
			return;

		int alivebritish = 0;
		for( x = 0; x < pBritish->GetNumPlayers(); x++ )
			if( pBritish->GetPlayer(x)->IsAlive() )
				alivebritish++;
		//Tjoppen - End
		//BG2 - Tjoppen - restart rounds a few seconds after the last person is killed
		//wins
		
		if ((aliveamericans == 0) || (alivebritish == 0) || (m_fEndRoundTime <= gpGlobals->curtime) || (m_bIsRestartingRound))
		{
			if( !m_bIsRestartingRound )
			{
				m_flNextRoundRestart = gpGlobals->curtime + 5;
				m_bIsRestartingRound = true;
				
				if((aliveamericans > 0) && (alivebritish > 0))
				{
					if (aliveamericans > alivebritish)
					{
						ClientPrintAll( "Out of time! Americans win!", true, true );
						m_iTDMTeamThatWon = 1;
						pAmericans->AddScore( 1 );
						m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
					}
					else if (aliveamericans < alivebritish)
					{
						ClientPrintAll( "Out of time! British win!", true, true );
						m_iTDMTeamThatWon = 2;
						pBritish->AddScore( 1 );
						m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
					}
					else
					{
						ClientPrintAll( "Out of time! Draw!", true, true );
						m_iTDMTeamThatWon = 0;
						m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
					}
				}

				if( aliveamericans == 0 && alivebritish == 0 )
				{
					//draw
					ClientPrintAll( "This round became a draw", true, true );
					m_iTDMTeamThatWon = 0;
					m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
				}
				else if( aliveamericans == 0 )
				{
					//british
					pBritish->AddScore( 1 );
					m_iTDMTeamThatWon = 2;
					ClientPrintAll( "The british won this round!", true, true );
					m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
				}
				else if( alivebritish == 0 )
				{
					//americans
					pAmericans->AddScore( 1 );
					m_iTDMTeamThatWon = 1;
					ClientPrintAll( "The americans won this round!", true, true );
					m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
				}
			}
			else if( m_flNextRoundRestart < gpGlobals->curtime )
			{
				m_bIsRestartingRound = false;
				m_flNextRoundRestart = gpGlobals->curtime + 1;	//don't check for dead players for
																//one second to debounce with respect
																//to any non-spawned players

				CFlagHandler::ResetFlags();		//reset spawns and flags before respawning anyone..


				if( m_iTDMTeamThatWon == 0 )
				{
					//draw
					CFlagHandler::RespawnAll( NULL );
					if (mp_respawntime.GetInt() > 0)
					{
						m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
						m_fLastRespawnWave = gpGlobals->curtime;
					}
				}
				else if( m_iTDMTeamThatWon == 2 )
				{
					//british
					CFlagHandler::RespawnAll("British.win");
					if (mp_respawntime.GetInt() > 0)
					{
						m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
						m_fLastRespawnWave = gpGlobals->curtime;
					}
				}
				else if( m_iTDMTeamThatWon == 1 )
				{
					//americans
					CFlagHandler::RespawnAll("Americans.win");
					if (mp_respawntime.GetInt() > 0)
					{
						m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
						m_fLastRespawnWave = gpGlobals->curtime;
					}
				}
			}
		}
	}
	else if( mp_respawnstyle.GetInt() == 1 )//wave spawning
	{
		if ((m_fLastRespawnWave + mp_respawntime.GetFloat()) <= gpGlobals->curtime)
		{
			CFlagHandler::RespawnWave();
			m_fLastRespawnWave = gpGlobals->curtime;
		}
	}
	//BG2 - Draco - End

	if ( flFragLimit )
	{
		if( IsTeamplay() == true )
		{
			CTeam *pCombine = g_Teams[TEAM_AMERICANS];
			CTeam *pRebels = g_Teams[TEAM_BRITISH];

			if ( pCombine->GetScore() >= flFragLimit || pRebels->GetScore() >= flFragLimit )
			{
				GoToIntermission();
				return;
			}
		}
		else
		{
			// check if any player is over the frag limit
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

				if ( pPlayer && pPlayer->FragCount() >= flFragLimit )
				{
					GoToIntermission();
					return;
				}
			}
		}
	}

	ManageObjectRelocation();

#endif
}

void CHL2MPRules::GoToIntermission( void )
{
#ifndef CLIENT_DLL
	if ( g_fGameOver )
		return;

	g_fGameOver = true;

	m_flIntermissionEndTime = gpGlobals->curtime + mp_chattime.GetInt();

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		pPlayer->ShowViewPortPanel( PANEL_SCOREBOARD );
		pPlayer->AddFlag( FL_FROZEN );
	}
#endif
	
}

bool CHL2MPRules::CheckGameOver()
{
#ifndef CLIENT_DLL
	if ( g_fGameOver )   // someone else quit the game already
	{
		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			ChangeLevel(); // intermission is over
		}

		return true;
	}
#endif

	return false;
}

// when we are within this close to running out of entities,  items 
// marked with the ITEM_FLAG_LIMITINWORLD will delay their respawn
#define ENTITY_INTOLERANCE	100

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CHL2MPRules::FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( pWeapon && (pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
	{
		if ( gEntList.NumberOfEntities() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE) )
			return 0;

		// we're past the entity tolerance level,  so delay the respawn
		return FlWeaponRespawnTime( pWeapon );
	}
#endif
	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHL2MPRules::VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	CWeaponHL2MPBase *pHL2Weapon = dynamic_cast< CWeaponHL2MPBase*>( pWeapon );

	if ( pHL2Weapon )
	{
		return pHL2Weapon->GetOriginalSpawnOrigin();
	}
#endif
	
	return pWeapon->GetAbsOrigin();
}

#ifndef CLIENT_DLL

CItem* IsManagedObjectAnItem( CBaseEntity *pObject )
{
	return dynamic_cast< CItem*>( pObject );
}

CWeaponHL2MPBase* IsManagedObjectAWeapon( CBaseEntity *pObject )
{
	return dynamic_cast< CWeaponHL2MPBase*>( pObject );
}

bool GetObjectsOriginalParameters( CBaseEntity *pObject, Vector &vOriginalOrigin, QAngle &vOriginalAngles )
{
	if ( CItem *pItem = IsManagedObjectAnItem( pObject ) )
	{
		if ( pItem->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;
		
		vOriginalOrigin = pItem->GetOriginalSpawnOrigin();
		vOriginalAngles = pItem->GetOriginalSpawnAngles();

		pItem->m_flNextResetCheckTime = gpGlobals->curtime + sv_hl2mp_item_respawn_time.GetFloat();
		return true;
	}
	else if ( CWeaponHL2MPBase *pWeapon = IsManagedObjectAWeapon( pObject ) )
	{
		if ( pWeapon->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;

		vOriginalOrigin = pWeapon->GetOriginalSpawnOrigin();
		vOriginalAngles = pWeapon->GetOriginalSpawnAngles();

		pWeapon->m_flNextResetCheckTime = gpGlobals->curtime + sv_hl2mp_weapon_respawn_time.GetFloat();
		return true;
	}

	return false;
}

void CHL2MPRules::ManageObjectRelocation( void )
{
	int iTotal = m_hRespawnableItemsAndWeapons.Count();

	if ( iTotal > 0 )
	{
		for ( int i = 0; i < iTotal; i++ )
		{
			CBaseEntity *pObject = m_hRespawnableItemsAndWeapons[i].Get();
			
			if ( pObject )
			{
				Vector vSpawOrigin;
				QAngle vSpawnAngles;

				if ( GetObjectsOriginalParameters( pObject, vSpawOrigin, vSpawnAngles ) == true )
				{
					float flDistanceFromSpawn = (pObject->GetAbsOrigin() - vSpawOrigin ).Length();

					if ( flDistanceFromSpawn > WEAPON_MAX_DISTANCE_FROM_SPAWN )
					{
						bool shouldReset = false;
						IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();

						if ( pPhysics )
						{
							shouldReset = pPhysics->IsAsleep();
						}
						else
						{
							shouldReset = (pObject->GetFlags() & FL_ONGROUND) ? true : false;
						}

						if ( shouldReset )
						{
							pObject->Teleport( &vSpawOrigin, &vSpawnAngles, NULL );
							pObject->EmitSound( "AlyxEmp.Charge" );

							IPhysicsObject *pPhys = pObject->VPhysicsGetObject();

							if ( pPhys )
							{
								pPhys->Wake();
							}
						}
					}
				}
			}
		}
	}
}

//=========================================================
//AddLevelDesignerPlacedWeapon
//=========================================================
void CHL2MPRules::AddLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) == -1 )
	{
		m_hRespawnableItemsAndWeapons.AddToTail( pEntity );
	}
}

//=========================================================
//RemoveLevelDesignerPlacedWeapon
//=========================================================
void CHL2MPRules::RemoveLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) != -1 )
	{
		m_hRespawnableItemsAndWeapons.FindAndRemove( pEntity );
	}
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHL2MPRules::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->GetOriginalSpawnOrigin();
}

//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CHL2MPRules::FlItemRespawnTime( CItem *pItem )
{
	return sv_hl2mp_item_respawn_time.GetFloat();
}


//=========================================================
// CanHaveWeapon - returns false if the player is not allowed
// to pick up this weapon
//=========================================================
bool CHL2MPRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem )
{
	if ( weaponstay.GetInt() > 0 )
	{
		if ( pPlayer->Weapon_OwnsThisType( pItem->GetClassname(), pItem->GetSubType() ) )
			 return false;
	}

	return BaseClass::CanHavePlayerItem( pPlayer, pItem );
}

#endif

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CHL2MPRules::WeaponShouldRespawn( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( pWeapon->HasSpawnFlags( SF_NORESPAWN ) )
	{
		return GR_WEAPON_RESPAWN_NO;
	}
#endif

	return GR_WEAPON_RESPAWN_YES;
}

//-----------------------------------------------------------------------------
// Purpose: Player has just left the game
//-----------------------------------------------------------------------------
void CHL2MPRules::ClientDisconnected( edict_t *pClient )
{
#ifndef CLIENT_DLL
	// Msg( "CLIENT DISCONNECTED, REMOVING FROM TEAM.\n" );

	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );
	if ( pPlayer )
	{
		// Remove the player from his team
		if ( pPlayer->GetTeam() )
		{
			pPlayer->GetTeam()->RemovePlayer( pPlayer );
		}
	}

	BaseClass::ClientDisconnected( pClient );

#endif
}

//=========================================================
// Deathnotice. 
//=========================================================
void CHL2MPRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL
	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_ID = 0;

	// Find the killer & the scorer
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = GetDeathScorer( pKiller, pInflictor );

	// Custom kill type?
	if ( info.GetCustomKill() )
	{
		killer_weapon_name = GetCustomKillString( info );
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
		}
	}
	else
	{
		// Is the killer a client?
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
			
			if ( pInflictor )
			{
				if ( pInflictor == pScorer )
				{
					// If the inflictor is the killer,  then it must be their current weapon doing the damage
					if ( pScorer->GetActiveWeapon() )
					{
						//BG2 - Tjoppen - GetDeathNoticeName!
						killer_weapon_name = pScorer->GetActiveWeapon()->GetDeathNoticeName();//GetClassname();
					}
				}
				else
				{
					killer_weapon_name = pInflictor->GetClassname();  // it's just that easy
				}
			}
		}
		else
		{
			killer_weapon_name = pInflictor->GetClassname();
		}

		// strip the NPC_* or weapon_* from the inflictor's classname
		if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
		{
			killer_weapon_name += 7;
		}
		else if ( strncmp( killer_weapon_name, "npc_", 4 ) == 0 )
		{
			killer_weapon_name += 4;
		}
		else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
		{
			killer_weapon_name += 5;
		}
		else if ( strstr( killer_weapon_name, "physics" ) )
		{
			killer_weapon_name = "physics";
		}

		if ( strcmp( killer_weapon_name, "prop_combine_ball" ) == 0 )
		{
			killer_weapon_name = "combine_ball";
		}
		else if ( strcmp( killer_weapon_name, "grenade_ar2" ) == 0 )
		{
			killer_weapon_name = "smg1_grenade";
		}
		else if ( strcmp( killer_weapon_name, "satchel" ) == 0 || strcmp( killer_weapon_name, "tripmine" ) == 0)
		{
			killer_weapon_name = "slam";
		}


	}

	IGameEvent *event = gameeventmanager->CreateEvent( "player_death" );
	if( event )
	{
		event->SetInt("userid", pVictim->GetUserID() );
		event->SetInt("attacker", killer_ID );
		event->SetString("weapon", killer_weapon_name );
		gameeventmanager->FireEvent( event );
	}
#endif

}

void CHL2MPRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( pPlayer );

	if ( pHL2Player == NULL )
		return;

	const char *pCurrentModel = modelinfo->GetModelName( pPlayer->GetModel() );
	const char *szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), "cl_playermodel" );

	//If we're different.
	if ( stricmp( szModelName, pCurrentModel ) )
	{
		//Too soon, set the cvar back to what it was.
		//Note: this will make this function be called again
		//but since our models will match it'll just skip this whole dealio.
		if ( pHL2Player->GetNextModelChangeTime() >= gpGlobals->curtime )
		{
			char szReturnString[512];

			Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", pCurrentModel );
			engine->ClientCommand ( pHL2Player->edict(), szReturnString );

			Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch.\n", (int)(pHL2Player->GetNextModelChangeTime() - gpGlobals->curtime) );
			ClientPrint( pHL2Player, HUD_PRINTTALK, szReturnString );
			return;
		}

		if ( HL2MPRules()->IsTeamplay() == false )
		{
			pHL2Player->SetPlayerModel();

			const char *pszCurrentModelName = modelinfo->GetModelName( pHL2Player->GetModel() );

			char szReturnString[128];
			Q_snprintf( szReturnString, sizeof( szReturnString ), "Your player model is: %s\n", pszCurrentModelName );

			ClientPrint( pHL2Player, HUD_PRINTTALK, szReturnString );
		}
		else
		{
			//BG2 - Tjoppen - don't designate team!
			if( pHL2Player->GetClass() == -1 )
			{
				BaseClass::ClientSettingsChanged( pPlayer );
				return;
			}
			//BG2 - Tjoppen - player models
			//if ( Q_stristr( szModelName, "models/human") )
			if ( Q_stristr( szModelName, "models/player/american") )
			//
			{
				pHL2Player->ChangeTeam( TEAM_AMERICANS );
			}
			else
			{
				pHL2Player->ChangeTeam( TEAM_BRITISH );
			}
		}
	}

	BaseClass::ClientSettingsChanged( pPlayer );
#endif
	
}

int CHL2MPRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
#ifndef CLIENT_DLL
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() || IsTeamplay() == false )
		return GR_NOTTEAMMATE;

	if ( (*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp( GetTeamID(pPlayer), GetTeamID(pTarget) ) )
	{
		return GR_TEAMMATE;
	}
#endif

	return GR_NOTTEAMMATE;
}

const char *CHL2MPRules::GetGameDescription( void )
{ 
	//BG2 - Tjoppen - our game descriptions - putting the current version number in these might be a good idea
	if( IsTeamplay() )
		return "Battle Grounds 2 0.15";

	return "Battle Grounds 2 0.15 - free for all";
	/*if ( IsTeamplay() )
		return "Team Deathmatch"; 

	return "Deathmatch"; */
	//
} 

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPRules::Precache( void )
{
	CBaseEntity::PrecacheScriptSound( "AlyxEmp.Charge" );
}

bool CHL2MPRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( (collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT) &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	if ( collisionGroup0 == COLLISION_GROUP_DEBRIS && collisionGroup1 == COLLISION_GROUP_PUSHAWAY )
	{
		// let debris and multiplayer objects collide
		return true;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 

}

bool CHL2MPRules::ClientCommand(const char *pcmd, CBaseEntity *pEdict )
{

#ifndef CLIENT_DLL
	if( BaseClass::ClientCommand(pcmd, pEdict) )
		return true;


	CHL2MP_Player *pPlayer = (CHL2MP_Player *) pEdict;

	if ( pPlayer->ClientCommand( pcmd ) )
		return true;
#endif

	return false;
}

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;
	
	if ( !bInitted )
	{
		bInitted = true;

		def.AddAmmoType("AR2",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			60,			BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("AR2AltFire",		DMG_DISSOLVE,				TRACER_NONE,			0,			0,			3,			0,							0 );
		def.AddAmmoType("Pistol",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			150,		BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("SMG1",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			225,		BULLET_IMPULSE(200, 1225),	0 );
		//BG2 - Tjoppen - more ammo..
		//def.AddAmmoType("357",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			12,			BULLET_IMPULSE(800, 5000),	0 );
		def.AddAmmoType("357",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			24,			BULLET_IMPULSE(800, 5000),	0 );
		//
		def.AddAmmoType("XBowBolt",			DMG_BULLET,					TRACER_LINE,			0,			0,			10,			BULLET_IMPULSE(800, 8000),	0 );
		def.AddAmmoType("Buckshot",			DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE,			0,			0,			30,			BULLET_IMPULSE(400, 1200),	0 );
		def.AddAmmoType("RPG_Round",		DMG_BURN,					TRACER_NONE,			0,			0,			3,			0,							0 );
		def.AddAmmoType("SMG1_Grenade",		DMG_BURN,					TRACER_NONE,			0,			0,			3,			0,							0 );
		def.AddAmmoType("Grenade",			DMG_BURN,					TRACER_NONE,			0,			0,			5,			0,							0 );
		def.AddAmmoType("slam",				DMG_BURN,					TRACER_NONE,			0,			0,			5,			0,							0 );
	}

	return &def;
}