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
	#include "mapentities.h"
	#include "in_buttons.h"
	#include <ctype.h>
	#include "voice_gamemgr.h"
	#include "iscorer.h"
	#include "hl2mp_player.h"
	#include "team.h"
	#include "voice_gamemgr.h"
	#include "hl2mp_gameinterface.h"
	#include "hl2mp_cvars.h"
	#include "soundscape_system.h"

//BG2 - Draco
#include "triggers.h"
#include "../bg2/bg2_maptriggers.h"
#include "../bg2/flag.h"
#include "../bg2/ctfflag.h"
#include "bg2/mapfilter.h" 
//BG2 - Tjoppen - #includes
#include "sdk/sdk_bot_temp.h"

/*#ifdef DEBUG	
	#include "hl2mp_bot_temp.h"
#endif*/

extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);

//extern bool FindInList( const char **pStrings, const char *pToFind );

ConVar sv_hl2mp_weapon_respawn_time( "sv_hl2mp_weapon_respawn_time", "20", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_hl2mp_item_respawn_time( "sv_hl2mp_item_respawn_time", "30", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_report_client_settings("sv_report_client_settings", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );

extern ConVar mp_chattime;
//BG2 - Draco - Start
ConVar mp_respawnstyle( "mp_respawnstyle", "1", FCVAR_REPLICATED | FCVAR_NOTIFY );	//0 = regular dm, 1 = waves, 2 = rounds
ConVar mp_respawntime( "mp_respawntime", "14", FCVAR_REPLICATED | FCVAR_NOTIFY );
ConVar sv_restartround( "sv_restartround", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_restartmap( "sv_restartmap", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar mp_americanscore( "mp_americanscore", "0", FCVAR_GAMEDLL /*| FCVAR_NOTIFY*/ | FCVAR_CHEAT );
ConVar mp_britishscore( "mp_britishscore", "0", FCVAR_GAMEDLL /*| FCVAR_NOTIFY*/ | FCVAR_CHEAT  );
ConVar mp_autobalanceteams( "mp_autobalanceteams", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar mp_autobalancetolerance( "mp_autobalancetolerance", "3", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar mp_timeleft( "mp_timeleft", "0", FCVAR_GAMEDLL, "");

//BG2 - Draco - End
//BG2 - Tjoppen - mp_winbonus
ConVar mp_winbonus( "mp_winbonus", "200", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Amount of points awarded to team winning the round" );
//

//BG2 - Tjoppen - away with these
/*extern CBaseEntity	 *g_pLastCombineSpawn;
extern CBaseEntity	 *g_pLastRebelSpawn;*/
//


#define WEAPON_MAX_DISTANCE_FROM_SPAWN 64

#endif

//BG2 - Tjoppen - beautiful defines. you will see another one further down
#ifdef CLIENT_DLL
#define CVAR_FLAGS	(FCVAR_REPLICATED | FCVAR_NOTIFY)
#else
#define CVAR_FLAGS	(FCVAR_GAMEDLL | FCVAR_REPLICATED | FCVAR_NOTIFY)
#endif
#define LIMIT_DEFINES( size, sizename )\
	ConVar mp_limit_inf_a_##size( "mp_limit_inf_a_"#size, "-1", CVAR_FLAGS,\
									"Max number of Continental Soldiers on " sizename " maps" );\
	ConVar mp_limit_off_a_##size( "mp_limit_off_a_"#size, "-1", CVAR_FLAGS,\
									"Max number of Continental Officers on " sizename " maps" );\
	ConVar mp_limit_rif_a_##size( "mp_limit_rif_a_"#size, "-1", CVAR_FLAGS,\
									"Max number of Frontiersmen on " sizename " maps" );\
	ConVar mp_limit_ski_a_##size( "mp_limit_ski_a_"#size, "-1", CVAR_FLAGS,\
									"Max number of Militia on " sizename " maps" );\
	ConVar mp_limit_inf_b_##size( "mp_limit_inf_b_"#size, "-1", CVAR_FLAGS,\
									"Max number of Royal Infantry on " sizename " maps" );\
	ConVar mp_limit_off_b_##size( "mp_limit_off_b_"#size, "-1", CVAR_FLAGS,\
									"Max number of Royal Commanders on " sizename " maps" );\
	ConVar mp_limit_rif_b_##size( "mp_limit_rif_b_"#size, "-1", CVAR_FLAGS,\
									"Max number of J�gers on " sizename " maps" );\
	ConVar mp_limit_ski_b_##size( "mp_limit_ski_b_"#size, "-1", CVAR_FLAGS,\
									"Max number of Natives on " sizename " maps" );\
	ConVar mp_limit_linf_b_##size("mp_limit_linf_b_"#size, "-1", CVAR_FLAGS,\
									"Max number of Light infantry on " sizename " maps" );

//as you can see, the macro is a shorthand and should also help avoid misspellings and such that are
//usually common with repetitive stuff like this
LIMIT_DEFINES( sml, "small" )
LIMIT_DEFINES( med, "medium" )
LIMIT_DEFINES( lrg, "large" )

ConVar mp_limit_mapsize_low( "mp_limit_mapsize_low", "16", CVAR_FLAGS, "Servers with player counts <= this number are small, above it are medium or large" );
ConVar mp_limit_mapsize_high( "mp_limit_mapsize_high", "32", CVAR_FLAGS, "Servers with player counts <= this number are small or medium, above it are large" );



REGISTER_GAMERULES_CLASS( CHL2MPRules );

BEGIN_NETWORK_TABLE_NOBASE( CHL2MPRules, DT_HL2MPRules )

	#ifdef CLIENT_DLL
		//RecvPropBool( RECVINFO( m_bTeamPlayEnabled ) ),
		RecvPropFloat( RECVINFO( m_fLastRespawnWave ) ), //BG2 This needs to be here for the timer to work. -HairyPotter
	#else
		//SendPropBool( SENDINFO( m_bTeamPlayEnabled ) ),
		SendPropFloat( SENDINFO( m_fLastRespawnWave ) ), //BG2 This needs to be here for the timer to work. -HairyPotter
	#endif

END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( hl2mp_gamerules, CHL2MPGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( HL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )

static HL2MPViewVectors g_HL2MPViewVectors(
	Vector( 0, 0, 60 ),       //VEC_VIEW (m_vView) //BG2 - This was 64, but the BG2 models are slightly smaller I guess.. 
							  //so let's lower it slightly. -HairyPotter
							  
	Vector(-16, -16, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  72 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-16, -16, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  36 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 36 ),		  //VEC_DUCK_VIEW		(m_vDuckView) //BG2 - Raise this from 28.. it almost looks like you're proned. -HairyPotter
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-16, -16, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 16,  16,  60 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

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
		virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
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

	//m_bTeamPlayEnabled = teamplay.GetBool();
	m_flIntermissionEndTime = 0.0f;
	m_flGameStartTime = 0;

	m_hRespawnableItemsAndWeapons.RemoveAll();

	m_tmNextPeriodicThink = 0;
	m_flRestartGameTime = 0;
	m_bCompleteReset = false;
	m_bHeardAllPlayersReady = false;
	m_bAwaitingReadyRestart = false;

	//BG2 - Tjoppen - ClientPrintAll()- and bot initialization
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

	extern CBaseEntity *g_pLastIntermission;
	g_pLastIntermission = NULL;
	//
	mp_britishscore.SetValue(0);
	mp_americanscore.SetValue(0);
	//m_fEndRoundTime = -1;
	m_fNextFlagUpdate = 0;
	//m_iWaveTime = 0;
	//m_fEndRoundTime = gpGlobals->curtime + mp_respawntime.GetInt();
	m_fLastRespawnWave = gpGlobals->curtime;
	m_iTDMTeamThatWon = 0;
	m_bHasDoneWinSong = false;
	m_bHasLoggedScores = false;
	m_iAmericanDmg = 0;
	m_iBritishDmg = 0;
	m_fNextWinSong = gpGlobals->curtime;
	m_bServerReady = false; //Do this too, this will make it so map changes with bots work.
//BG2 - Skillet
#else
	m_hRagdollList.RemoveAll();
#endif
}

const CViewVectors* CHL2MPRules::GetViewVectors()const
{
	return &g_HL2MPViewVectors;
}

const HL2MPViewVectors* CHL2MPRules::GetHL2MPViewVectors()const
{
	return &g_HL2MPViewVectors;
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

	// BG2 - Away with these.
	/*g_pLastCombineSpawn = NULL;
	g_pLastRebelSpawn = NULL;*/

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

void CHL2MPRules::ResetMap()
{
#ifndef CLIENT_DLL
	CMapEntityFilter filter;
	
	CBaseEntity *pEnt;
	// find the first entity in the entity list
	pEnt = gEntList.FirstEnt();
	// as long as we've got a valid pointer, keep looping through the list
	while ( pEnt ) 
	{
		if( filter.ShouldCreateEntity( pEnt->GetClassname() ) )
		{ 
			//It's not in the "keep" list, so remove the ent.
			UTIL_Remove (pEnt);
			//Msg("Removed %s \n", pEnt->GetClassname());
		}
		//else
		//	Msg("Did not remove : %s \n", pEnt->GetClassname());

		pEnt = gEntList.NextEnt (pEnt);
	}
    // force the entities we've set to be removed to actually be removed
    gEntList.CleanupDeleteList();
	// with any unrequired entities removed, we use MapEntity_ParseAllEntities to reparse the map entities
	// this in effect causes them to spawn back to their normal position.
	MapEntity_ParseAllEntities( engine->GetMapEntitiesString(), &filter, true);
#endif
}

void CHL2MPRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL
	if ( IsIntermission() )
		return;
	BaseClass::PlayerKilled( pVictim, info );
#endif
}

#ifndef CLIENT_DLL
//BG2 - This should handle all the score settings after each round, and also fire any triggers and play win music. -HairyPotter
void CHL2MPRules::HandleScores( int iTeam, int iScore, int msg_type, bool bRestart )
{
	CMapTrigger *BG2Trigger = NULL;

	while ( (BG2Trigger = static_cast<CMapTrigger*>(gEntList.FindEntityByClassname( BG2Trigger, "bg2_maptrigger" ))) != NULL )
	{
		switch ( iTeam )
		{
		case TEAM_AMERICANS:
			if ( bRestart )
				BG2Trigger->AmericanRoundWin();
			else
				BG2Trigger->AmericanMapWin();
			break;
		case TEAM_BRITISH:
			if ( bRestart )
				BG2Trigger->BritishRoundWin();
			else
				BG2Trigger->BritishMapWin();
			break;
		default: //Assume a draw
			BG2Trigger->Draw();
			break;
		}
	}
	if ( bRestart )
	{
		RestartRound();
		//do not cause two simultaneous round restarts..
		m_bIsRestartingRound = false;
		m_flNextRoundRestart = gpGlobals->curtime + 1;
	}

	if ( iScore > 0 )
		g_Teams[iTeam]->AddScore( iScore );

	WinSong( iTeam, true );

	if ( msg_type )
		ClientPrintAll( msg_type );
}
//
#endif

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
				//British Win
				HandleScores( TEAM_BRITISH, 0, BRITISH_MAP_WIN, false );
			}

			if (pAmericans->GetScore() > pBritish->GetScore())
			{
				//Americans Win
				HandleScores( TEAM_AMERICANS, 0, AMERICAN_MAP_WIN, false );
			}

			if (pAmericans->GetScore() == pBritish->GetScore())
			{
				//Draw!
				HandleScores( -1, 0, MAP_DRAW, false );
			}
		}

		/*for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

			if ( pPlayer && pPlayer->FragCount() >= flFragLimit )
			{
				GoToIntermission();
				return;
			}
		}*/
		if ( !m_bHasLoggedScores )
		{
			//BG2 - Log Damages and Scores. -HairyPotter
			for( int x = 0; x < g_Teams[TEAM_AMERICANS]->GetNumPlayers(); x++ )
			{
				CBasePlayer *pPlayer = g_Teams[TEAM_AMERICANS]->GetPlayer( x );
				if ( !pPlayer )
					continue;
				m_iAmericanDmg += pPlayer->DeathCount();
			}
			for( int x = 0; x < g_Teams[TEAM_BRITISH]->GetNumPlayers(); x++ )
			{
				CBasePlayer *pPlayer = g_Teams[TEAM_BRITISH]->GetPlayer( x );
				if ( !pPlayer )
					continue;
				m_iBritishDmg += pPlayer->DeathCount();
			}

			UTIL_LogPrintf("American Scores: DAMAGE: %i   SCORE: %i   \n", m_iAmericanDmg, mp_americanscore.GetInt() );
			UTIL_LogPrintf("British Scores: DAMAGE: %i   SCORE: %i   \n", m_iBritishDmg, mp_britishscore.GetInt() );

			m_bHasLoggedScores = true; //Don't do it again.
		}

		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			m_bHasDoneWinSong = false;
			ChangeLevel(); // intermission is over
		}

		return;
	}

	float flTimeLimit = GetMapRemainingTime();
	float flFragLimit = fraglimit.GetFloat();

	if ( flTimeLimit != 0 && gpGlobals->curtime >= flTimeLimit )
	{
		GoToIntermission();
		return;
	}

	//BG2 - Draco - Start
	//CTeam *pAmericans = g_Teams[TEAM_AMERICANS];
	//CTeam *pBritish = g_Teams[TEAM_BRITISH];
	//m_iWaveTime = ((m_fLastRespawnWave + mp_respawntime.GetFloat()) - gpGlobals->curtime);
//	CFlagHandler::FlagThink();//make the flags think.
	if (m_fNextFlagUpdate <= gpGlobals->curtime)
	{
		UpdateFlags();
		m_fNextFlagUpdate = gpGlobals->curtime + 1;
	}
	else if (m_fNextFlagUpdate == 0)
	{
		m_fNextFlagUpdate = gpGlobals->curtime + 1;
	}
	//=========================
	//Time Left
	//=========================
	mp_timeleft.SetValue((flTimeLimit - gpGlobals->curtime)); //timeleft works just as well..

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
	if (mp_autobalanceteams.GetInt() == 1) //If we're balancing teams on AttemptJoin... is there really a need to think about it again? -HairyPotter
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
					if (pPlayer && !pPlayer->IsAlive()) //Let's try to avoid null pointers.
					{
						pPlayer->ChangeTeam(TEAM_AMERICANS);
					}
					break;
				case TEAM_AMERICANS:
					iAutoTeamBalancePlayerToSwitchIndex = random->RandomInt( 0, (pAmericans->GetNumPlayers() - 1) );
					pPlayer = pAmericans->GetPlayer(iAutoTeamBalancePlayerToSwitchIndex);
					if (pPlayer && !pPlayer->IsAlive()) //Let's try to avoid null pointers.
					{
						pPlayer->ChangeTeam(TEAM_BRITISH);
					}
					break;
			}
		}
		//well, if we aren't even now, there's always next think...
	}
	//=========================
	//Restart Round/Map
	//=========================
	if (sv_restartmap.GetInt() > 0)
	{
		m_fNextGameReset = gpGlobals->curtime + sv_restartmap.GetInt();
		sv_restartmap.SetValue(0);
	}
	//now if the time was set we can check for it, above zero means we are restarting
	if ((m_fNextGameReset > 0) &&( m_fNextGameReset <= gpGlobals->curtime))
	{
		m_fNextGameReset = 0;//dont reset again

		//reset scores...
		pAmericans->SetScore(0);//...for teams...
		pBritish->SetScore(0);
		int x;
		for( x = 0; x < g_Teams[TEAM_AMERICANS]->GetNumPlayers(); x++ )
		{
			CBasePlayer *pPlayer = g_Teams[TEAM_AMERICANS]->GetPlayer( x );
			if ( !pPlayer )
				continue;

			pPlayer->ResetFragCount();//...for cap points...
			pPlayer->ResetDeathCount();//...and damage
		}
		for( x = 0; x < g_Teams[TEAM_BRITISH]->GetNumPlayers(); x++ )
		{
			CBasePlayer *pPlayer = g_Teams[TEAM_BRITISH]->GetPlayer( x );
			if ( !pPlayer )
				continue;

			pPlayer->ResetFragCount();//...for cap points...
			pPlayer->ResetDeathCount();//...and damage
		}
		RestartRound();	//BG2 - Tjoppen - restart round

		//Reset the map and wave times...
		if (mp_respawntime.GetInt() > 0)
		{
			m_fLastRespawnWave = gpGlobals->curtime;
		}
		m_flGameStartTime = gpGlobals->curtime;
		//
	}

	if (sv_restartround.GetInt() > 0)
	{
		m_fNextRoundReset = gpGlobals->curtime + sv_restartround.GetInt();
		sv_restartround.SetValue(0);
	}
	if ((m_fNextRoundReset > 0) && ( m_fNextRoundReset <= gpGlobals->curtime))
	{
		m_fNextRoundReset = 0;//dont reset again
		RestartRound();	//BG2 - restart round

		if (mp_respawntime.GetInt() > 0)
		{
			m_fLastRespawnWave = gpGlobals->curtime;
		}
	}
	
	//=========================
	//Round systems
	//=========================
	if( mp_respawnstyle.GetInt() >= 2 )//if line battle all at once spawn style - Draco
	{
		/*if (mp_respawntime.GetInt() == 0)
		{
			m_fEndRoundTime = 0;
		}*/
		//Tjoppen - start
		//count alive players in each team
		CTeam *pAmericans = g_Teams[TEAM_AMERICANS];
		CTeam *pBritish = g_Teams[TEAM_BRITISH];

		if( pAmericans->GetNumPlayers() == 0 || pBritish->GetNumPlayers() == 0 )
			return;

		int aliveamericans = 0, x = 0;
		for( ; x < pAmericans->GetNumPlayers(); x++ )
		{
			CBasePlayer *pPlayer = pAmericans->GetPlayer( x );
			if( pPlayer && pPlayer->IsAlive() )
				aliveamericans++;
		}

		int alivebritish = 0;
		for( x = 0; x < pBritish->GetNumPlayers(); x++ )
		{
			CBasePlayer *pPlayer = pBritish->GetPlayer( x );
			if( pPlayer && pPlayer->IsAlive() )
				alivebritish++;
		}

		//Tjoppen - End
		//BG2 - Tjoppen - restart rounds a few seconds after the last person is killed
		//wins
		
		if ((aliveamericans == 0) || (alivebritish == 0) || (m_fLastRespawnWave + mp_respawntime.GetFloat() <= gpGlobals->curtime) || (m_bIsRestartingRound))
		{
			if( !m_bIsRestartingRound )
			{
				m_flNextRoundRestart = gpGlobals->curtime + 5;
				m_bIsRestartingRound = true;
				
				if((aliveamericans > 0) && (alivebritish > 0)) //Both teams have players left alive..
				{
					if (aliveamericans > alivebritish)
					{
						ClientPrintAll( AMERICAN_DEFAULT_WIN );
						m_iTDMTeamThatWon = TEAM_AMERICANS;
					}
					else if (aliveamericans < alivebritish)
					{
						ClientPrintAll( BRITISH_DEFAULT_WIN );
						m_iTDMTeamThatWon = TEAM_BRITISH;
					}
					else
					{
						ClientPrintAll( DEFAULT_DRAW );
						m_iTDMTeamThatWon = -1;
					}
				}

				if( aliveamericans == 0 && alivebritish == 0 )
				{
					//draw
					ClientPrintAll( ROUND_DRAW );
					m_iTDMTeamThatWon = -1;
				}
				else if( aliveamericans == 0 )
				{
					//british
					m_iTDMTeamThatWon = TEAM_BRITISH;
					ClientPrintAll( BRITISH_ROUND_WIN );
				}
				else if( alivebritish == 0 )
				{
					//americans
					m_iTDMTeamThatWon = TEAM_AMERICANS;
					ClientPrintAll( AMERICAN_ROUND_WIN );
				}
			}
			else if( m_flNextRoundRestart < gpGlobals->curtime )
			{			
				switch ( m_iTDMTeamThatWon )
				{
					case TEAM_AMERICANS:
						HandleScores( TEAM_AMERICANS, 1, 0, true );
						break;
					case TEAM_BRITISH:
						HandleScores( TEAM_BRITISH, 1, 0, true );
						break;
					default:
						HandleScores( NULL, 0, 0, true );
						break;
				}

				if (mp_respawntime.GetInt() > 0)
				{
					m_fLastRespawnWave = gpGlobals->curtime;
				}
			}
		}
	}
	else if( mp_respawnstyle.GetInt() == 1 )//wave spawning
	{
		if ((m_fLastRespawnWave + mp_respawntime.GetFloat()) <= gpGlobals->curtime)
		{
			RespawnWave();
			m_fLastRespawnWave = gpGlobals->curtime;
		}
	}
	//BG2 - Draco - End

	if ( flFragLimit )
	{
		CTeam *pAmericans = g_Teams[TEAM_AMERICANS];
		CTeam *pBritish = g_Teams[TEAM_BRITISH];

		if ( pAmericans->GetScore() >= flFragLimit || pBritish->GetScore() >= flFragLimit )
		{
			GoToIntermission();
			return;
		}
	}


	ManageObjectRelocation();

#endif
}

void CHL2MPRules::GoToIntermission( void ) //Just for reference.. this fires whenever the map is won. -HairyPotter
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
	else if ( CWeaponHL2MPBase *pWeapon = IsManagedObjectAWeapon( pObject )) 
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
							//pObject->EmitSound( "AlyxEmp.Charge" );

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
// What angles should this item use to respawn?
//=========================================================
QAngle CHL2MPRules::VecItemRespawnAngles( CItem *pItem )
{
	return pItem->GetOriginalSpawnAngles();
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

	//BG2 - Tjoppen - disconnecting. remove from flags
	CHL2MP_Player *pPlayer2 = dynamic_cast<CHL2MP_Player*>( pPlayer );
	if( pPlayer2 )
		pPlayer2->RemoveSelfFromFlags();
	//

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
	if ( info.GetDamageCustom() )
	{
		killer_weapon_name = GetDamageCustomString( info );
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

		/*if ( strcmp( killer_weapon_name, "prop_combine_ball" ) == 0 )
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
		}*/


	}

	IGameEvent *event = gameeventmanager->CreateEvent( "player_death" );
	if( event )
	{
		event->SetInt("userid", pVictim->GetUserID() );
		event->SetInt("attacker", killer_ID );
		event->SetString("weapon", killer_weapon_name );
		event->SetInt( "priority", 7 );
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

	//BG2 - The only thing cl_playermodel does at this point is change your team when you change the value. What's the point? 
	//Just use the class selection menu like everyone else so we can skim more shit off. -HairyPotter
	/*const char *pCurrentModel = modelinfo->GetModelName( pPlayer->GetModel() );
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
	}*/
	if ( sv_report_client_settings.GetInt() == 1 )
	{
		UTIL_LogPrintf( "\"%s\" cl_cmdrate = \"%s\"\n", pHL2Player->GetPlayerName(), engine->GetClientConVarValue( pHL2Player->entindex(), "cl_cmdrate" ));
	}


	BaseClass::ClientSettingsChanged( pPlayer );
#endif
	
}

int CHL2MPRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
#ifndef CLIENT_DLL
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() )
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
	return "Battle Grounds 2 2.0";
	// 
} 

float CHL2MPRules::GetMapRemainingTime()
{
	// if timelimit is disabled, return 0
	if ( mp_timelimit.GetInt() <= 0 )
		return 0;

	// timelimit is in minutes

	float timeleft = (m_flGameStartTime + mp_timelimit.GetInt() * 60.0f );

	return timeleft;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPRules::Precache( void )
{
	//CBaseEntity::PrecacheScriptSound( "AlyxEmp.Charge" ); //BG2 - Not in scripts, removing entirely. -HairyPotter
}

bool CHL2MPRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		swap(collisionGroup0,collisionGroup1);
	}

	if ( (collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT) &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 

}

bool CHL2MPRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
#ifndef CLIENT_DLL
	if( BaseClass::ClientCommand( pEdict, args ) )
		return true;


	CHL2MP_Player *pPlayer = (CHL2MP_Player *) pEdict;

	if ( pPlayer->ClientCommand( args ) )
		return true;
#endif

	return false;
}

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
//BG2 - Tjoppen - we want more.. realistic forces
//#define BULLET_IMPULSE_EXAGGERATION			3.5
#define BULLET_IMPULSE_EXAGGERATION			0//.8
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;
	
	if ( !bInitted )
	{
		bInitted = true;

		//BG2 - Tjoppen - more ammo..
		//def.AddAmmoType("357",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			12,			BULLET_IMPULSE(800, 5000),	0 );
		def.AddAmmoType("357",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,			36,			BULLET_IMPULSE(800, 5000),	0 );
	}

	return &def;
}

#ifdef CLIENT_DLL

	/*ConVar cl_autowepswitch(
		"cl_autowepswitch",
		"0", //"1"
		FCVAR_ARCHIVE | FCVAR_USERINFO,
		"Automatically switch to picked up weapons (if more powerful)" );*/

#else

	bool CHL2MPRules::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon )
	{		
		//BG2 - Why so serious? -HairyPotter
		/*if ( pPlayer->GetActiveWeapon() && pPlayer->IsNetClient() )
		{
			// Player has an active item, so let's check cl_autowepswitch.
			const char *cl_autowepswitch = engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), "cl_autowepswitch" );
			if ( cl_autowepswitch && atoi( cl_autowepswitch ) <= 0 )
			{
				return false;
			}
		}
		*/
		return BaseClass::FShouldSwitchWeapon( pPlayer, pWeapon );
	}

#endif

#ifndef CLIENT_DLL

void CHL2MPRules::RestartGame()
{
	// bounds check
	if ( mp_timelimit.GetInt() < 0 )
	{
		mp_timelimit.SetValue( 0 );
	}
	m_flGameStartTime = gpGlobals->curtime;
	if ( !IsFinite( m_flGameStartTime.Get() ) )
	{
		Warning( "Trying to set a NaN game start time\n" );
		m_flGameStartTime.GetForModify() = 0.0f;
	}
	
	// now respawn all players
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		if ( pPlayer->GetActiveWeapon() )
		{
			pPlayer->GetActiveWeapon()->Holster();
		}
		pPlayer->RemoveAllItems( true );
		respawn( pPlayer, false );
		pPlayer->Reset();
	}

	// Respawn entities (glass, doors, etc..)

	CTeam *pBritish = GetGlobalTeam( TEAM_BRITISH );
	CTeam *pAmericans = GetGlobalTeam( TEAM_AMERICANS );

	if ( pAmericans )
		pAmericans->SetScore( 0 );

	if ( pBritish )
		pBritish->SetScore( 0 );

	m_flIntermissionEndTime = 0;
	m_flRestartGameTime = 0.0;		
	m_bCompleteReset = false;

	IGameEvent * event = gameeventmanager->CreateEvent( "round_start" );
	if ( event )
	{
		event->SetInt("fraglimit", 0 );
		event->SetInt( "priority", 6 ); // HLTV event priority, not transmitted

		event->SetString("objective","DEATHMATCH");

		gameeventmanager->FireEvent( event );
	}
}

void CHL2MPRules::CheckChatForReadySignal( CHL2MP_Player *pPlayer, const char *chatmsg )
{
	if( m_bAwaitingReadyRestart && FStrEq( chatmsg, mp_ready_signal.GetString() ) )
	{
		if( !pPlayer->IsReady() )
		{
			pPlayer->SetReady( true );
		}		
	}
}

void CHL2MPRules::CheckRestartGame( void )
{
	// Restart the game if specified by the server
	int iRestartDelay = mp_restartgame.GetInt();

	if ( iRestartDelay > 0 )
	{
		if ( iRestartDelay > 60 )
			iRestartDelay = 60;


		// let the players know
		char strRestartDelay[64];
		Q_snprintf( strRestartDelay, sizeof( strRestartDelay ), "%d", iRestartDelay );
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "Game will restart in %s1 %s2", strRestartDelay, iRestartDelay == 1 ? "SECOND" : "SECONDS" );
		UTIL_ClientPrintAll( HUD_PRINTCONSOLE, "Game will restart in %s1 %s2", strRestartDelay, iRestartDelay == 1 ? "SECOND" : "SECONDS" );

		m_flRestartGameTime = gpGlobals->curtime + iRestartDelay;
		m_bCompleteReset = true;
		mp_restartgame.SetValue( 0 );
	}

	if( mp_readyrestart.GetBool() )
	{
		m_bAwaitingReadyRestart = true;
		m_bHeardAllPlayersReady = false;
		

		const char *pszReadyString = mp_ready_signal.GetString();


		// Don't let them put anything malicious in there
		if( pszReadyString == NULL || Q_strlen(pszReadyString) > 16 )
		{
			pszReadyString = "ready";
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "hl2mp_ready_restart" );
		if ( event )
			gameeventmanager->FireEvent( event );

		mp_readyrestart.SetValue( 0 );

		// cancel any restart round in progress
		m_flRestartGameTime = -1;
	}
}

void CHL2MPRules::CheckAllPlayersReady( void )
{
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;
		if ( !pPlayer->IsReady() )
			return;
	}
	m_bHeardAllPlayersReady = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CHL2MPRules::GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer )
{
	//BG2 - Let's remove this huge "Fuck You" that Valve put into their own code. -HairyPotter
	return NULL;
	//

	/*if ( !pPlayer )  // dedicated server output
	{
		return NULL;
	}

	const char *pszFormat = NULL;

	// team only
	if ( bTeamOnly == TRUE )
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		{
			pszFormat = "HL2MP_Chat_Spec";
		}
		else
		{
			const char *chatLocation = GetChatLocation( bTeamOnly, pPlayer );
			if ( chatLocation && *chatLocation )
			{
				pszFormat = "HL2MP_Chat_Team_Loc";
			}
			else
			{
				pszFormat = "HL2MP_Chat_Team";
			}
		}
	}
	// everyone
	else
	{
		if ( pPlayer->GetTeamNumber() != TEAM_SPECTATOR )
		{
			pszFormat = "HL2MP_Chat_All";	
		}
		else
		{
			pszFormat = "HL2MP_Chat_AllSpec";
		}
	}

	return pszFormat;
	*/
}

#endif

#ifndef CLIENT_DLL

void CHL2MPRules::RestartRound()
{
	//restart current round. immediately.
	ResetMap();
	ResetFlags();
	RespawnAll();
}

void CHL2MPRules::RespawnAll()
{
	if( g_Teams.Size() < NUM_TEAMS )	//in case teams haven't been inited or something
		return;

	int x;
	for( x = 0; x < g_Teams[TEAM_AMERICANS]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_AMERICANS]->GetPlayer( x ) );

		if( !pPlayer )
			continue;

		pPlayer->Spawn();

		//BG2 - Tjoppen - remove ragdoll - remember to change this to remove multiple ones if we decide to enable more corpses
		if( pPlayer->m_hRagdoll )
		{
			UTIL_RemoveImmediate( pPlayer->m_hRagdoll );
			pPlayer->m_hRagdoll = NULL;
		}
	}

	for( x = 0; x < g_Teams[TEAM_BRITISH]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_BRITISH]->GetPlayer( x ) );

		if( !pPlayer )
			continue;

		pPlayer->Spawn();

		//BG2 - Tjoppen - remove ragdoll - remember to change this to remove multiple ones if we decide to enable more corpses
		if( pPlayer->m_hRagdoll )
		{
			UTIL_RemoveImmediate( pPlayer->m_hRagdoll );
			pPlayer->m_hRagdoll = NULL;
		}
	}
}

void CHL2MPRules::WinSong( int team, bool m_bWonMap )
{
	if( g_Teams.Size() < NUM_TEAMS )	//in case teams haven't been inited or something
		return;

	const char *w_sTeam = team == TEAM_AMERICANS ? "Americans" : "British";

	//This is a good place to do the round_win log stuff, since this function is pretty much called every time a team wins a map/round. -HairyPotter
	if ( !m_bWonMap ) //So this is just an ordinary round.
	{	
		UTIL_LogPrintf( "Team \"%s\" triggered \"round_win\"\n", w_sTeam );
	}
	else //Or it's the end of a map.
	{
		UTIL_LogPrintf( "Team \"%s\" triggered \"map_win\"\n", w_sTeam );
	}
	//

	if( m_fNextWinSong > gpGlobals->curtime )
		return;
	
	m_fNextWinSong = gpGlobals->curtime + 20;

	CRecipientFilter recpfilter;
	recpfilter.AddAllPlayers();
	recpfilter.MakeReliable();
	
	UserMessageBegin( recpfilter, "WinMusic" );
		WRITE_BYTE( team );
	MessageEnd();
	
	/*int x;
	
	for( x = 0; x < g_Teams[TEAM_AMERICANS]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_AMERICANS]->GetPlayer( x ) );

		if( !pPlayer )
			continue;

		if( pSound )
		{
			CPASAttenuationFilter filter( pPlayer, 10.0f );	//high attenuation so only this player hears it
			filter.UsePredictionRules();
			pPlayer->EmitSound( filter, pPlayer->entindex(), pSound );
		}
	}

	for( x = 0; x < g_Teams[TEAM_BRITISH]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_BRITISH]->GetPlayer( x ) );

		if( !pPlayer )
			continue;

		if( pSound )
		{
			CPASAttenuationFilter filter( pPlayer, 10.0f );	//high attenuation so only this player hears it
			filter.UsePredictionRules();
			pPlayer->EmitSound( filter, pPlayer->entindex(), pSound );
		}
	}*/
}

void CHL2MPRules::RespawnWave()
{
	if( g_Teams.Size() < NUM_TEAMS )	//in case teams haven't been inited or something
		return;

	for( int x = 0; x < g_Teams[TEAM_AMERICANS]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_AMERICANS]->GetPlayer( x ) );

		if( !pPlayer )
			continue;

		if( pPlayer->IsAlive() )
			continue;

		pPlayer->Spawn();
	}

	for( int x = 0; x < g_Teams[TEAM_BRITISH]->GetNumPlayers(); x++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( g_Teams[TEAM_BRITISH]->GetPlayer( x ) );
		
		if( !pPlayer )
			continue;

		if( pPlayer->IsAlive() )
			continue;

		pPlayer->Spawn();
	}
}

/*void CFlagHandler::PlayCaptureSound( void )
{
	/*CBasePlayer *pPlayer = NULL;
	while( (pPlayer = (CBasePlayer*)gEntList.FindEntityByClassname( pPlayer, "player" )) != NULL )
		pPlayer->EmitSound( "Flag.capture" );*//*
}*/

void CHL2MPRules::ResetFlags( void )
{
	CBaseEntity *pEntity = NULL;

	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "flag" )) != NULL )
	{
		CFlag *pFlag = static_cast<CFlag*>(pEntity);
		if( !pFlag )
			continue;

		pFlag->ResetFlag(); //It's just that easy.
	}

	//BG2 - Reset CTF Flags as well. -HairyPotter
	while( (pEntity = gEntList.FindEntityByClassname( pEntity, "ctf_flag" )) != NULL )
	{
		CtfFlag *pFlag = static_cast<CtfFlag*>(pEntity);
		if( !pFlag )
			continue;

		pFlag->ResetFlag(); //It's just that easy.
	}
	//
}

void CHL2MPRules::UpdateFlags( void )
{
	CBaseEntity *pEntity = NULL;

	//CMapTrigger *BG2Trigger = NULL;
	
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
		CFlag *pFlag = dynamic_cast<CFlag*>(pEntity);

		if( !pFlag || !pFlag->IsActive() )
			continue;

		int FullCap = pFlag->m_iFullCap;

		//if ( FullCap < 0 || FullCap == NULL ) //This is needed for backwards compat reasons. -HairyPotter
		//	FullCap = 0;
		//Msg("Full cap for %s = %i \n", pFlag->m_sFlagName, FullCap );

		if ( FullCap == 3) //we're not doing full caps on this flag. 0 = normal, 1 = Americans Fullcap, 2 = Brits Fullcap, 3 = No Fullcap.
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
		

		//So the flag is set to be capped by this team.
		switch(pFlag->m_iForTeam)
		{
			case 0:
				switch( FullCap )
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

				}
				break;
			case 1:
				if ( FullCap == 1 || FullCap == 0 )
					foramericans++;
				break;
			case 2:
				if ( FullCap == 2 || FullCap == 0 )
					forbritish++;
				break;
			default://assume both
				switch( FullCap )
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

				}
				break;
		}
	}

	/*Msg( "american_flags = %i & foramericans flags = %i\n", american_flags, foramericans );
	Msg( "british_flags = %i & forbritish flags = %i\n", british_flags, forbritish );
	Msg( "neutral_flags = %i\n", neutral_flags );*/

	if( !american_flags && !british_flags && !neutral_flags )
		return;

	if( neutral_flags > 0 )
	{
		if( (foramericans - american_flags) == 0 && foramericans != 0 )
		{
			HandleScores( TEAM_AMERICANS, mp_winbonus.GetInt(), AMERICAN_ROUND_WIN, true );
			return;
		}
		if( (forbritish - british_flags) == 0 && forbritish != 0 )
		{
			HandleScores( TEAM_BRITISH, mp_winbonus.GetInt(), BRITISH_ROUND_WIN, true );
			return;
		}
	}
	else
	{
		if( american_flags <= 0 && british_flags <= 0 )
		{
			//draw
			HandleScores( NULL, 0, ROUND_DRAW, true );
			//Msg( "draw\n" );
			return;
		}

		if ( american_flags <= 0 && (forbritish - british_flags) == 0)
		{
			//british win
			//Msg( "british win\n" );
			HandleScores( TEAM_BRITISH, mp_winbonus.GetInt(), BRITISH_ROUND_WIN, true );
			return;
		}

		if ( british_flags <= 0 && (foramericans - american_flags) == 0)
		{
			//americans win
			//Msg( "americans win\n" );
			HandleScores( TEAM_AMERICANS, mp_winbonus.GetInt(), AMERICAN_ROUND_WIN, true );
			return;
		}
	}
}
#endif

int CHL2MPRules::GetLimitTeamClass( int iTeam, int iClass )
{
	//mp_limit_<inf/off/rif>_<a/b>_<sml/med/lrg> - mp_limit_inf_a_sml

	//count players - is there a better way? this looks stupid
	int num = 0;
	for( int x = 1; x <= gpGlobals->maxClients; x++ )
		if( UTIL_PlayerByIndex( x ) )
			num++;

	//BG2 - Tjoppen - more macro goodness
#define LIMIT_SWITCH( size )\
	switch( iTeam ){\
	case TEAM_AMERICANS:\
		switch( iClass ){\
		case CLASS_INFANTRY: return mp_limit_inf_a_##size.GetInt();\
		case CLASS_OFFICER: return mp_limit_off_a_##size.GetInt();\
		case CLASS_SNIPER: return mp_limit_rif_a_##size.GetInt();\
		case CLASS_SKIRMISHER: return mp_limit_ski_a_##size.GetInt();\
		default: return -1;}\
	case TEAM_BRITISH:\
		switch( iClass ){\
		case CLASS_INFANTRY: return mp_limit_inf_b_##size.GetInt();\
		case CLASS_OFFICER: return mp_limit_off_b_##size.GetInt();\
		case CLASS_SNIPER: return mp_limit_rif_b_##size.GetInt();\
		case CLASS_SKIRMISHER: return mp_limit_ski_b_##size.GetInt();\
		case CLASS_LIGHT_INFANTRY: return mp_limit_linf_b_##size.GetInt();\
		default: return -1;}\
	default: return -1;}

	if( num <= mp_limit_mapsize_low.GetInt() )
	{
		//small
		LIMIT_SWITCH( sml )
	}
	else if( num <= mp_limit_mapsize_high.GetInt() )
	{
		//medium
		LIMIT_SWITCH( med )
	}
	else
	{
		//large
		LIMIT_SWITCH( lrg )
	}
}