/*
	Added by HairyPotter for King of the Hill mode. -- Work in progress. 
	This is basically a flag capture system that goes by points depending on how long you hold a point. (Duh) 
	Except this will not use a model or anything like that. It'll pretty much be one big trigger.
	The big difference is that one team cannot win a round by simply "Taking the point". And that by holding
	the point for an extended period of time could perhaps grant more points or a round win.
	Also, maybe this mode could work with progressive spawns. Say a team captures a fortress, then that team spawns
	in said fortress. Trouble is, we can't just have a team "Walk in and take over" when the other team is holding it
	or there are still opposing players.
	Perhaps a time/player ratio system is in order. Or if Americans > British, then Americans keep the point? 
	However, at the same time, The timer counts, but the clock is multiplied by the amount of players - Attacking team.
	So 5 Americans - 3 British attackers = (2 * timelimit) Or a ratio: 15 Americans / 5 British = ( 3 * timelimit ) In the end
	it'll all require playtesting.
	Actually now that I think about it, a ratio makes much more sense. Wtf was I thinking? 
	On the bright side. If this can work as one trigger rather than a trigger talking to an ent, that should save alot of 
	confusion on my end.
*/

#include "cbase.h"
#include "triggers.h"
#include "koth.h"
#include "hl2mp_player.h"
#include "hl2mp_gamerules.h"
#include "gamerules.h"
#include "team.h"
#include "engine/IEngineSound.h"
//#include "bg2/spawnpoint.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//BG2 - SaintGreg - dynamic flag settings
//input functions
bool CKoth::IsActive( void )
{
	return m_bActive;
}

void CKoth::InputEnable( inputdata_t &inputData )
{
	if( m_bActive )
		return;

	m_bActive = true;
	ChangeTeam( TEAM_UNASSIGNED );
	m_iLastTeam = TEAM_UNASSIGNED;
	m_OnEnable.FireOutput( inputData.pActivator, this );
	//Think(); // think immediately and restart the thinking cycle
}
void CKoth::InputDisable( inputdata_t &inputData )
{
	if( !m_bActive )
		return;

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
	m_OnDisable.FireOutput( inputData.pActivator, this );
}
void CKoth::InputToggle( inputdata_t &inputData )
{
	if (m_bActive)
		InputDisable( inputData );
	else
		InputEnable( inputData );
}

void CKoth::Spawn( void )
{

	if (HasSpawnFlags( CKoth_START_DISABLED ))
		m_bActive = false;

	else
	{
		m_bActive = true;
		ChangeTeam( TEAM_UNASSIGNED );	//ChangeTeam handles everything..
	}

	m_iRequestingCappers = TEAM_UNASSIGNED;
	m_iLastTeam = TEAM_UNASSIGNED;

	Msg("Lawl Spawned \n");

	BaseClass::Spawn( );
}
void CKoth::Think( void )
{
	if (!m_bActive)	// if inactive, stop the thinking cycle
	{
		SetNextThink( gpGlobals->curtime + 1.25f ); //Think Less.
		return;
	}

	SetNextThink( gpGlobals->curtime + 0.25f );

	Msg(" %i american(s)and %i british are touching the trigger. \n", m_vTriggerAmericanPlayers.Count(), m_vTriggerBritishPlayers.Count() );

	//award any time bonii
	switch( GetTeamNumber() )
	{
		case TEAM_AMERICANS:
			if (m_iTeamBonusInterval != 0)
			{
				if ((m_flNextTeamBonus <= gpGlobals->curtime) )
				{
					g_Teams[TEAM_AMERICANS]->AddScore( m_iTeamBonus );
					m_flNextTeamBonus += m_iTeamBonusInterval;
				}
			}
			break;
		case TEAM_BRITISH:
			if (m_iTeamBonusInterval != 0)
			{
				if ((m_flNextTeamBonus <= gpGlobals->curtime) )
				{
					g_Teams[TEAM_BRITISH]->AddScore( m_iTeamBonus );
					m_flNextTeamBonus += m_iTeamBonusInterval;
				}
			}
			break;
		default:
			break;
	}

	//safeguard against the possibility that m_flNextTeamBonus is somehow out of bounds after above check
	//could happen if the flag stays uncapped for a long time. if so, the team that caps it would get a lot of "stored up"
	//points.
	if (m_flNextTeamBonus <= gpGlobals->curtime)
		m_flNextTeamBonus = gpGlobals->curtime + m_iTeamBonusInterval;

	if( GetTeamNumber() != TEAM_UNASSIGNED )
		ThinkCapped();	//don't need to check for overloading or uncapping on uncapped flags

	ThinkUncapped();	//always run this, so non-uncappable flags can be taken
}

//this is actually the old think funtion, slightly modified
void CKoth::ThinkUncapped( void )
{	
	americans = 0;
	british = 0;

	americans = m_vTriggerAmericanPlayers.Count();
	british = m_vTriggerBritishPlayers.Count();

	//char *msg = NULL;

	if( americans + british > 0 && (americans <= 0 || british <= 0) )
	{
		//only americans or british at the flag
		//if we don't already own it, and we've been here for at least three seconds - capture

		//Yeah that's right, apparently Linux is anal about Const references. -HairyPotter
		const int BritishTeam = TEAM_BRITISH; 
		const int AmericanTeam = TEAM_AMERICANS; 
		//

		if( americans > 0 && GetTeamNumber() != TEAM_AMERICANS )
		{
			m_iRequestingCappers = AmericanTeam; //FIXME: Linux bitches about this, don't ask me why. -HairyPotter
			m_iNearbyPlayers = americans;

			if (americans >= min( m_iCapturePlayers, g_Teams[TEAM_AMERICANS]->GetNumPlayers() ) )
			{
				//Msg( "americans\n" );
				if( m_iLastTeam != TEAM_AMERICANS )
				{
					m_iLastTeam = AmericanTeam; //FIXME: Linux bitches about this, don't ask me why. -HairyPotter
					m_flNextCapture = gpGlobals->curtime + m_flCaptureTime;

					m_OnAmericanStartCapture.FireOutput( this, this );
					m_OnStartCapture.FireOutput( this, this );
				}
				else if( gpGlobals->curtime >= m_flNextCapture )
				{
					Capture( TEAM_AMERICANS );
					Msg("Americans captured the point! \n");
				}
			}
			else //This simply prevents he one man cap exploit. Sure there may still be an american nearby, but if it's under required amount; why continue the cap?
			{
				m_iLastTeam = TEAM_UNASSIGNED;
				m_flNextCapture = 0;
			} //
		}
		else if( british > 0 && GetTeamNumber() != TEAM_BRITISH )
		{

			m_iRequestingCappers = BritishTeam; //FIXME: Linux bitches about this, don't ask me why. -HairyPotter
			m_iNearbyPlayers = british;

			if (british >= min( m_iCapturePlayers, g_Teams[TEAM_BRITISH]->GetNumPlayers() ) )
			{
				//Msg( "british\n" );
				if( m_iLastTeam != TEAM_BRITISH )
				{
					m_iLastTeam = BritishTeam; //FIXME: Linux bitches about this, don't ask me why. -HairyPotter
					m_flNextCapture = gpGlobals->curtime + m_flCaptureTime;

					m_OnBritishStartCapture.FireOutput( this, this );
					m_OnStartCapture.FireOutput( this, this );
				}
				else if( gpGlobals->curtime >= m_flNextCapture )
				{
					Capture( TEAM_BRITISH );
					Msg("British captured the point! \n");
				}
			}
			else //This simply prevents he one man cap exploit. Sure there may still be a brit nearby, but if it's under required amount; why continue the cap?
			{
				m_iLastTeam = TEAM_UNASSIGNED;
				m_flNextCapture = 0;
			} //
		}
	}
	else
	{
		if( m_iLastTeam != TEAM_UNASSIGNED && GetTeamNumber() != m_iLastTeam )
		{
			//Msg( "stopped capturing a flag\n" );
			if( m_iLastTeam == TEAM_AMERICANS )
			{
				m_OnAmericanStopCapture.FireOutput( this, this );
				m_OnStopCapture.FireOutput( this, this );
			}
			else if( m_iLastTeam == TEAM_BRITISH )
			{
				m_OnBritishStopCapture.FireOutput( this, this );
				m_OnStopCapture.FireOutput( this, this );
			}
		}
		//noone here
		m_iLastTeam = TEAM_UNASSIGNED;
		m_iRequestingCappers = TEAM_UNASSIGNED;
		//m_iNearbyPlayers = 0;
		
		m_flNextCapture = 0;
	}

	//ClientPrintAll( msg );
}

void CKoth::Capture( int iTeam )
{
	//iTeam is either americans or british
	//this function handles a lot of the capture related stuff

	/*CRecipientFilter recpfilter;
	recpfilter.AddAllPlayers();
	recpfilter.MakeReliable();
	
	UserMessageBegin( recpfilter, "CaptureSounds" );
		WRITE_VEC3COORD( GetAbsOrigin() );
		WRITE_STRING( "Flag.capture" );
	MessageEnd();*/

	g_Teams[iTeam]->AddScore( m_iTeamBonus );
	m_flNextTeamBonus = (gpGlobals->curtime + m_iTeamBonusInterval);

	m_iLastTeam = TEAM_UNASSIGNED;
	m_iRequestingCappers = TEAM_UNASSIGNED;

	// before we change team, if they stole the point, fire the output
	if (GetTeamNumber() != iTeam)
	{
		switch( GetTeamNumber() )
		{
			case TEAM_AMERICANS:
				m_OnAmericanLosePoint.FireOutput( this, this );
				break;
			case TEAM_BRITISH:
				m_OnBritishLosePoint.FireOutput( this, this );
				break;
		}

		m_OnLosePoint.FireOutput( this, this );
	}

	ChangeTeam( iTeam );

	switch( iTeam )
	{
		case TEAM_AMERICANS:
			m_OnAmericanCapture.FireOutput( this, this );
			break;
		case TEAM_BRITISH:
			m_OnBritishCapture.FireOutput( this, this );
			break;
	}

	m_OnCapture.FireOutput( this, this );
}

void CKoth::ThinkCapped( void )
{
	//check if anyone's overloading or uncapping this flag
	//start by counting people near the flag

	//CBasePlayer *pPlayer = NULL;

	friendlies = 0,
	enemies = 0;

}

void CKoth::ChangeTeam( int iTeamNum )
{
	//m_iLastTeam = iTeamNum;
	//m_iLastTeam = TEAM_UNASSIGNED;

	BaseClass::ChangeTeam( iTeamNum );
}
void CKoth::StartTouch(CBaseEntity *pOther)
{
	//BaseClass::StartTouch( pOther );

	//Defines
	CBasePlayer *pPlayer = dynamic_cast< CBasePlayer* >( pOther->MyCombatCharacterPointer() );

	if ( !m_bActive ) //Trigger is inactive?
		return;		  //Die here.

	if ( !pOther->IsPlayer() ) //Ask yourself, would anything else be able to take the point? Should bullets be able to capture?
		return;

	if( !pPlayer->IsAlive() )	//dead players don't trigger
		return;

	switch( pPlayer->GetTeamNumber() ) //Let's do most of the sorting work here rather than bunching the teams together.
	{
		case TEAM_AMERICANS:
				m_vTriggerAmericanPlayers.AddToTail( pPlayer ); //Add this player to the american player list.
			break;
		case TEAM_BRITISH:
				m_vTriggerBritishPlayers.AddToTail( pPlayer ); //Add this player to the british player list.
			break;
	}

	Msg("Touched and thinking. \n");

	SetThink( &CKoth::Think ); //We've got at least one player on the trigger now. Go ahead and think.
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - The entity that was touching us.
//-----------------------------------------------------------------------------
void CKoth::EndTouch(CBaseEntity *pOther)
{
	//Defines
	CBasePlayer *pPlayer = dynamic_cast< CBasePlayer* >( pOther->MyCombatCharacterPointer() );

	if ( !m_bActive ) //Trigger is inactive?
		return;		  //Die here.

	if ( !pOther->IsPlayer() ) //Ask yourself, would anything else be able to capture a flag? Should bullets be able to capture?
		return;

	//BaseClass::EndTouch( pOther );

	switch( pPlayer->GetTeamNumber() ) //Let's do most of the sorting work here rather than bunching the teams together.
	{
		case TEAM_AMERICANS:
				m_vTriggerAmericanPlayers.FindAndRemove( pPlayer ); //Remove this player from the american player list.
			break;
		case TEAM_BRITISH:
				m_vTriggerBritishPlayers.FindAndRemove( pPlayer ); //Remove this player from the british player list.
			break;
	}

	if( m_vTriggerBritishPlayers.Count() +  m_vTriggerAmericanPlayers.Count() < 1 ) //Nobody Home, just stop thinking entirely. (Save CPU?)
	{
		SetThink(NULL);
		SetNextThink( TICK_NEVER_THINK );
	}

}

int CKoth::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

IMPLEMENT_NETWORKCLASS_ALIASED( Koth, DT_Koth )

BEGIN_NETWORK_TABLE( CKoth, DT_Koth )
	SendPropInt( SENDINFO( m_iLastTeam ), Q_log2(NUM_TEAMS), SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_flNextCapture ) ),
	SendPropInt( SENDINFO( m_iCapturePlayers ), Q_log2(MAX_PLAYERS), SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iNearbyPlayers ), Q_log2(MAX_PLAYERS), SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_flCaptureTime ) ),
	SendPropStringT( SENDINFO( m_sTriggerName ) ),
	SendPropInt( SENDINFO( m_iHUDSlot ), 5 ),	//15 slots.. 0 = sequential tile, -1 = hidden(don't draw)
	

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CKoth )
END_PREDICTION_DATA()

BEGIN_DATADESC( CKoth )

	DEFINE_KEYFIELD( m_iCapturePlayers, FIELD_INTEGER, "CapturePlayers" ),
	DEFINE_KEYFIELD( m_flCaptureTime, FIELD_FLOAT, "CaptureTime" ),
	DEFINE_KEYFIELD( m_iTeamBonus, FIELD_INTEGER, "TeamBonus" ),
	DEFINE_KEYFIELD( m_iTeamBonusInterval, FIELD_INTEGER, "TeamBonusInterval" ),
	DEFINE_KEYFIELD( m_iPlayerBonus, FIELD_INTEGER, "PlayerBonus" ),
	DEFINE_KEYFIELD( m_sTriggerName, FIELD_STRING, "Name" ),
	DEFINE_FUNCTION( Think ),

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


END_DATADESC()

LINK_ENTITY_TO_CLASS(koth_area, CKoth);
PRECACHE_REGISTER(koth_area);
