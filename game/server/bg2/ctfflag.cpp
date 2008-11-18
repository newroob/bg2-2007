/*
	Added by HairyPotter for CTF mode. -- Work in progress. 
	And yes, this will only be for flag models. Not various entities and such.
*/

#include "cbase.h"
#include "triggers.h"
#include "ctfflag.h"
#include "hl2mp_gamerules.h"
#include "team.h"
#include "engine/IEngineSound.h"

ConVar sv_ctf_flagweight ("sv_ctf_flagweight", "0", FCVAR_NOTIFY | FCVAR_GAMEDLL, "How much speed does carrying this flag drain?");
ConVar sv_ctf_returnstyle ("sv_ctf_returnstyle", "1", FCVAR_NOTIFY | FCVAR_GAMEDLL, "Which way is a flag returned?");

void CtfFlag::Spawn( void )
{
	Precache( );
	SetModel( "models/other/flag.mdl" ); //Always first.

	m_bActive = true;

	if (HasSpawnFlags( CtfFlag_START_DISABLED ))
	{
		m_bActive = false;
		AddEffects( EF_NODRAW );
	}

	BaseClass::Spawn( );
	//cFlagName = m_strName;
	iTeam = m_iForTeam;

	switch( m_iForTeam )
	{
		case 0:
			m_nSkin = 2;
			cFlagName = "Neutral";
			iTeam = NULL;
			break;
		case 1: //British Flag
			m_nSkin = 0;
			cFlagName = "American"; //This is just for the capture text. (Capped American Flag!)
			iTeam = TEAM_BRITISH; 
			break;
		case 2: //American Flag
			m_nSkin = 1;
			cFlagName = "British"; //This is just for the capture text. (Capped British Flag!)
			iTeam = TEAM_AMERICANS; //This is opposite land.
			break;
	}

	FlagOrigin = GetAbsOrigin();
	FlagAngle = GetAbsAngles();

	int nSequence = LookupSequence( "flag_idle1" );
	if ( nSequence > ACTIVITY_NOT_AVAILABLE )
	{
		SetSequence(nSequence);
		SetCycle( 0 );
		ResetSequence( nSequence );
		ResetClientsideFrame();
	}
	else
	{
		Msg( "Sequence is busted...\n");
		SetSequence( 0 );
	}

	SetThink( &CtfFlag::Think );
	SetNextThink( gpGlobals->curtime );
}
void CtfFlag::Precache( void )
{
	PrecacheModel( "models/other/flag.mdl" );
}
void CtfFlag::Think( void )
{
	if ( !m_bActive ) //If it isn't active, just die here.
	{
		SetNextThink( gpGlobals->curtime + 1.0f ); //Think Less
		return;
	}

	SetNextThink( gpGlobals->curtime + 0.125f );

	StudioFrameAdvance(); //For the flag animation. -HairyPotter

	if ( GetParent() ) //Is a player holding the flag??
	{
		if ( !GetParent()->IsAlive() ) //Did the player die while holding the flag? Otherwise just do nothing.
		{
			CBasePlayer *pPlayer = dynamic_cast< CBasePlayer* >( GetParent() );
			SetParent( NULL );
			SetAbsOrigin( GetAbsOrigin() - Vector( 0,0,60 ) ); //HACKHACK: Bring the flag down to it's original height.
			m_bFlagIsCarried = false;
			m_bFlagIsDropped = true;
			fReturnTime = gpGlobals->curtime + m_fReturnTime;
			Q_snprintf( CTFMsg, 512, "%s Has Dropped The %s Flag!", pPlayer->GetPlayerName(), cFlagName );
			PrintAlert( CTFMsg );
			PlaySound( GetAbsOrigin(), m_iDropSound );
			m_OnDropped.FireOutput( this, this ); //Fire the OnDropped output.
		}
	}
	else //If there isn't someone holding the flag, we need to search for a potential capturer or perhaps return it.
	{
		if ( m_bFlagIsDropped && gpGlobals->curtime > fReturnTime && sv_ctf_returnstyle.GetInt() == 1 ) //Should the flag return when time is up?
			ReturnFlag();

		CBasePlayer *pPlayer = NULL;
		while( (pPlayer = dynamic_cast<CBasePlayer*>(gEntList.FindEntityByClassnameWithin( pPlayer, "player", GetLocalOrigin(), m_flPickupRadius ))) != NULL )
		{
			if ( !pPlayer->IsAlive() )
				continue;
		
			int TeamNumber = pPlayer->GetTeamNumber();
			if ( iTeam != NULL && TeamNumber != iTeam )
			{
				if( sv_ctf_returnstyle.GetInt() == 2 && m_bFlagIsDropped ) //Should the flag return if a friendly touches it?
					ReturnFlag();
	
				continue;
			}

			//Assuming you've passed all the checks, you deserve to pick up the flag. So run the code below.
			pPlayer->CtfFlag = this;	//So the capture trigger will know which flag is being captured.
			SetAbsOrigin( pPlayer->GetAbsOrigin() + Vector( 0,0,60 ) );	//Keeps the flag out of the player's FOV, also raises it so it doesn't look like it's stuck in the player's grill.
			SetParent( pPlayer );	//Attach the entity to the player.
			//pPlayer->iSpeed = pPlayer->iSpeed - sv_ctf_flagweight.GetInt(); //For the player speed difference.
			m_bFlagIsCarried = true;
			m_bFlagIsDropped = false;
			Q_snprintf( CTFMsg, 512, "%s Has Taken The %s Flag!", pPlayer->GetPlayerName(), cFlagName );
			PrintAlert( CTFMsg );
			PlaySound( GetAbsOrigin(), m_iPickupSound );
			m_OnPickedUp.FireOutput( this, this ); //Fire the OnPickedUp output.
		}
	}
}

void CtfFlag::PrintAlert( char *Msg )
{
	/*extern ConVar sv_flagalerts;
	switch( sv_flagalerts.GetInt() )
	{
		case 1:
			UTIL_ClientPrintAll( HUD_PRINTCENTER, Msg );
			break;
		case 2:
			UTIL_ClientPrintAll( HUD_PRINTTALK, Msg );
			break;

	}*/
	UTIL_ClientPrintAll( HUD_PRINTTALK, Msg );
}
void CtfFlag::ReturnFlag( void )
{
	PlaySound( GetAbsOrigin(), m_iReturnSound );
	m_bFlagIsDropped = false;
	Q_snprintf( CTFMsg, 512, "The %s Flag Has Returned!", cFlagName );
	PrintAlert( CTFMsg );
	ResetFlag();
	m_OnReturned.FireOutput( this, this ); //Fire the OnReturned output.
}
void CtfFlag::ResetFlag()
{
	SetAbsOrigin( FlagOrigin );
	SetAbsAngles( FlagAngle );
	SetParent ( NULL );
}
void CtfFlag::PlaySound( Vector origin, int sound )
{
	if ( sound == NULL ) //No sound? Just stop right there.
		return;

	char *SoundFile = (char *)sound;
	CRecipientFilter recpfilter;
	recpfilter.AddAllPlayers();
	recpfilter.MakeReliable();
	UserMessageBegin( recpfilter, "CaptureSounds" );
		WRITE_VEC3COORD( GetAbsOrigin() );
		WRITE_STRING( SoundFile );
	MessageEnd();
}

//Inputs Below ------------------------------------------------------------
void CtfFlag::InputReset( inputdata_t &inputData )
{
	if (GetAbsOrigin() == FlagOrigin)
		return;
	if ( GetParent() )
		m_bFlagIsCarried = false;

	ReturnFlag();
}
void CtfFlag::InputEnable( inputdata_t &inputData )
{
	if ( GetParent() )
		m_bFlagIsCarried = false;

	RemoveEffects( EF_NODRAW );
	m_bActive = true;
	m_OnEnable.FireOutput( inputData.pActivator, this );

	ReturnFlag();
}
void CtfFlag::InputDisable( inputdata_t &inputData )
{
	if ( GetParent() ) //Just in case you disable the flag when someone has it.
		m_bFlagIsCarried = false;

	AddEffects( EF_NODRAW );
	m_bActive = false;
	m_OnDisable.FireOutput( inputData.pActivator, this );

	ReturnFlag();
}
void CtfFlag::InputToggle( inputdata_t &inputData )
{
	if (m_bActive)
		InputDisable( inputData );
	else
		InputEnable( inputData );
}
//-------------------------------------------------------------------------
BEGIN_DATADESC( CtfFlag )

	DEFINE_KEYFIELD( m_flPickupRadius, FIELD_FLOAT, "PickupRadius" ),
	DEFINE_KEYFIELD( m_iForTeam, FIELD_INTEGER, "ForTeam" ),
	DEFINE_KEYFIELD( m_fReturnTime, FIELD_FLOAT, "ReturnTime" ),
	DEFINE_KEYFIELD( m_iPickupSound, FIELD_SOUNDNAME, "PickupSound" ),
	//DEFINE_KEYFIELD( m_iWeight, FIELD_SOUNDNAME, "Weight" ), //Could be used to slow the player down when carrying?
	DEFINE_KEYFIELD( m_iDropSound, FIELD_SOUNDNAME, "DropSound" ),
	DEFINE_KEYFIELD( m_iReturnSound, FIELD_SOUNDNAME, "ReturnSound" ),
	//DEFINE_KEYFIELD( m_strName,	FIELD_STRING,	"Name" ), //I don't know... I guess if you wanted to call it something other than British/American flag?

	DEFINE_THINKFUNC( Think ),

	DEFINE_OUTPUT( m_OnDropped, "OnDropped" ),
	DEFINE_OUTPUT( m_OnPickedUp, "OnPickedUp" ),
	DEFINE_OUTPUT( m_OnReturned, "OnReturned" ),
	DEFINE_OUTPUT( m_OnEnable, "OnEnabled" ),
	DEFINE_OUTPUT( m_OnDisable, "OnDisabled" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Reset", InputReset ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( ctf_flag, CtfFlag);
PRECACHE_REGISTER(ctf_flag);
