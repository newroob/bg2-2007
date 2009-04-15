//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "EventLog.h"
#include "team.h"
#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CEventLog::CEventLog()
{
}

CEventLog::~CEventLog()
{
}


void CEventLog::FireGameEvent( IGameEvent *event )
{
	PrintEvent ( event );
}

bool CEventLog::PrintEvent( IGameEvent *event )
{
	const char * name = event->GetName();

	if ( Q_strncmp(name, "server_", strlen("server_")) == 0 )
	{
		return true; // we don't care about server events (engine does)
	}
	else if ( Q_strncmp(name, "player_", strlen("player_")) == 0 )
	{
		return PrintPlayerEvent( event );
	}
	else if ( Q_strncmp(name, "team_", strlen("team_")) == 0 )
	{
		return PrintTeamEvent( event );
	}
	else if ( Q_strncmp(name, "game_", strlen("game_")) == 0 )
	{
		return PrintGameEvent( event );
	}
	else
	{
		return PrintOtherEvent( event ); // bomb_, round_, et al
	}
}

bool CEventLog::PrintGameEvent( IGameEvent *event )
{
//	const char * name = event->GetName() + Q_strlen("game_"); // remove prefix

	return false;
}

bool CEventLog::PrintPlayerEvent( IGameEvent *event )
{
	const char * eventName = event->GetName();
	const int userid = event->GetInt( "userid" );

	if ( !Q_strncmp( eventName, "player_connect", Q_strlen("player_connect") ) ) // player connect is before the CBasePlayer pointer is setup
	{
		const char *name = event->GetString( "name" );
		const char *address = event->GetString( "address" );
		const char *networkid = event->GetString("networkid" );
		UTIL_LogPrintf( "\"%s<%i><%s><>\" connected, address \"%s\"\n", name, userid, networkid, address);
		return true;
	}
	else if ( !Q_strncmp( eventName, "player_disconnect", Q_strlen("player_disconnect")  ) )
	{
		const char *reason = event->GetString("reason" );
		const char *name = event->GetString("name" );
		const char *networkid = event->GetString("networkid" );
		CTeam *team = NULL;
		CBasePlayer *pPlayer = UTIL_PlayerByUserId( userid );

		if ( pPlayer )
		{
			team = pPlayer->GetTeam();
		}

		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" disconnected (reason \"%s\")\n", name, userid, networkid, team ? team->GetName() : "", reason );
		return true;
	}

	CBasePlayer *pPlayer = UTIL_PlayerByUserId( userid );
	if ( !pPlayer)
	{
		DevMsg( "CEventLog::PrintPlayerEvent: Failed to find player (userid: %i, event: %s)\n", userid, eventName );
		return false;
	}

	if ( !Q_strncmp( eventName, "player_team", Q_strlen("player_team") ) )
	{
		const bool bDisconnecting = event->GetBool( "disconnect" );

		if ( !bDisconnecting )
		{
			const int newTeam = event->GetInt( "team" );
			const int oldTeam = event->GetInt( "oldteam" );
			CTeam *team = GetGlobalTeam( newTeam );
			CTeam *oldteam = GetGlobalTeam( oldTeam );
			
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" joined team \"%s\"\n", 
			pPlayer->GetPlayerName(),
			pPlayer->GetUserID(),
			pPlayer->GetNetworkIDString(),
			oldteam->GetName(),
			team->GetName() );
		}

		return true;
	}
	else if ( !Q_strncmp( eventName, "player_death", Q_strlen("player_death") ) )
	{
		const int attackerid = event->GetInt("attacker" );

#ifdef HL2MP
		const char *weapon = event->GetString( "weapon" );
#endif
		
		CBasePlayer *pAttacker = UTIL_PlayerByUserId( attackerid );
		CTeam *team = pPlayer->GetTeam();
		CTeam *attackerTeam = NULL;
		
		if ( pAttacker )
		{
			attackerTeam = pAttacker->GetTeam();
		}
		if ( pPlayer == pAttacker && pPlayer )  
		{  

#ifdef HL2MP
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" committed suicide with \"%s\"\n",  
							pPlayer->GetPlayerName(),
							userid,
							pPlayer->GetNetworkIDString(),
							team ? team->GetName() : "",
							weapon
							);
#else
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" committed suicide with \"%s\"\n",  
							pPlayer->GetPlayerName(),
							userid,
							pPlayer->GetNetworkIDString(),
							team ? team->GetName() : "",
							pAttacker->GetClassname()
							);
#endif
		}
		else if ( pAttacker )
		{
			CTeam *attackerTeam = pAttacker->GetTeam();

#ifdef HL2MP
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" killed \"%s<%i><%s><%s>\" with \"%s\"\n",  
							pAttacker->GetPlayerName(),
							attackerid,
							pAttacker->GetNetworkIDString(),
							attackerTeam ? attackerTeam->GetName() : "",
							pPlayer->GetPlayerName(),
							userid,
							pPlayer->GetNetworkIDString(),
							team ? team->GetName() : "",
							weapon
							);
#else
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" killed \"%s<%i><%s><%s>\"\n",  
							pAttacker->GetPlayerName(),
							attackerid,
							pAttacker->GetNetworkIDString(),
							attackerTeam ? attackerTeam->GetName() : "",
							pPlayer->GetPlayerName(),
							userid,
							pPlayer->GetNetworkIDString(),
							team ? team->GetName() : ""
							);								
#endif
		}
		else
		{  
			// killed by the world
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" committed suicide with \"world\"\n",
							pPlayer->GetPlayerName(),
							userid,
							pPlayer->GetNetworkIDString(),
							team ? team->GetName() : ""
							);
		}
		return true;
	}
	else if ( !Q_strncmp( eventName, "player_activate", Q_strlen("player_activate") ) )
	{
		UTIL_LogPrintf( "\"%s<%i><%s><>\" entered the game\n",  
							pPlayer->GetPlayerName(),
							userid,
							pPlayer->GetNetworkIDString()
							);

		return true;
	}
	else if ( !Q_strncmp( eventName, "player_changename", Q_strlen("player_changename") ) )
	{
		const char *newName = event->GetString( "newname" );
		const char *oldName = event->GetString( "oldname" );
		CTeam *team = pPlayer->GetTeam();
		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed name to \"%s\"\n", 
					oldName,
					userid,
					pPlayer->GetNetworkIDString(),
					team ? team->GetName() : "",
					newName
					);
		return true;
	}

	//BG2 - These are extra events for HLSTATSX Support. As requested by Bonzo. - HairyPotter
	else if ( !Q_strncmp( eventName, "round_win", Q_strlen("round_win")  ) )
	{
		const char *team = event->GetString("team" );

		UTIL_LogPrintf( "The  \"%s\" team has won the round! \n", team );
		return true;
	}
	else if ( !Q_strncmp( eventName, "map_win", Q_strlen("map_win")  ) )
	{
		const char *winteam = event->GetString("winteam" );
		const int winteamscore = event->GetInt("winteamscore" );
		const int winteamdamage = event->GetInt("winteamdamage" );
		const char *loseteam = event->GetString("loseteam" );
		const int loseteamscore = event->GetInt("loseteamscore" );
		const int loseteamdamage = event->GetInt("loseteamdamage" );

		UTIL_LogPrintf( "Team \"%s\" has defeated team \"%s\": \"%i\" to \"%i\". With damages: \"%i\" to \"%i\". \n", winteam, loseteam, winteamscore, loseteamscore, winteamdamage, loseteamdamage );
		return true;
	}
	else if ( !Q_strncmp( eventName, "flag_capture", Q_strlen("flag_capture")  ) )
	{
		const char *team = event->GetString("team" );

		UTIL_LogPrintf( "Team \"%s\" captured a flag. \n", team);
		return true;
	}
	else if ( !Q_strncmp( eventName, "class_change", Q_strlen("class_change")  ) )
	{
		const char *name = event->GetString("name" );
		const char *newclass = event->GetString("newclass" );
		const char *team = event->GetString("team" );

		UTIL_LogPrintf( "\"%s\" is going to fight as \"%s\" for team \"%s\".\n", name, newclass, team);
		return true;
	}
	//
				   
// ignored events
//player_hurt
	return false;
}

bool CEventLog::PrintTeamEvent( IGameEvent *event )
{
//	const char * name = event->GetName() + Q_strlen("team_"); // remove prefix

	return false;
}

bool CEventLog::PrintOtherEvent( IGameEvent *event )
{
	return false;
}


bool CEventLog::Init()
{
	ListenForGameEvent( "player_changename" );
	ListenForGameEvent( "player_activate" );
	ListenForGameEvent( "player_death" );
	ListenForGameEvent( "player_team" );
	ListenForGameEvent( "player_disconnect" );
	ListenForGameEvent( "player_connect" );
	//BG2 - New Events. -HairyPotter
	ListenForGameEvent( "round_win" );
	ListenForGameEvent( "map_win" );
	ListenForGameEvent( "flag_win" );
	ListenForGameEvent( "class_change" );
	//

	return true;
}