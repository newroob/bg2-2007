/*
	Added by HairyPotter for CTF mode.  REVISION UNFINISHED FOR 1.2B! NEEDS WORK!
*/
/*
#include "cbase.h"
#include "triggers.h"
#include "ctfflag.h"
#include "hl2mp_gamerules.h"
#include "team.h"
#include "engine/IEngineSound.h"

ConVar sv_ctf_flagweight ("sv_ctf_flagweight", "0", FCVAR_NOTIFY | FCVAR_GAMEDLL, "How much speed does carrying this flag drain? -HairyPotter");
ConVar sv_ctf_returnstyle ("sv_ctf_returnstyle", "1", FCVAR_NOTIFY | FCVAR_GAMEDLL, "Which way is a flag returned? -HairyPotter");

void CtfFlag::Spawn( void )
{
	Precache( );
	BaseClass::Spawn( );
	SetModel( m_strModelName );
	cName = m_strName;
	iTeam = m_iForTeam;

	FlagOrigin = GetAbsOrigin();
	FlagAngle = GetAbsAngles();

	int nSequence = LookupSequence( "flag_idle1" );
	if ( nSequence )
	{
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
	}

	//SetRenderColor( 10, 10, 255, 255 );
	//SetRenderColor Reference.. RED/GREEN/BLUE/

	SetThink( &CtfFlag::Think );
	SetNextThink( gpGlobals->curtime );
}
void CtfFlag::Precache( void )
{
	PrecacheModel( m_strModelName );
}
void CtfFlag::Think( void )
{
	SetNextThink( gpGlobals->curtime + 0.125f );
	StudioFrameAdvance(); //For the flag animation. -HairyPotter

	if ( GetParent() ) //Is a player holding the flag??
	{
		if ( !GetParent()->IsAlive() ) //Did the player die while holding the flag?
		{
			CBasePlayer *pPlayer = dynamic_cast< CBasePlayer* >( GetParent() );
			SetParent( NULL );
			SetAbsOrigin( GetAbsOrigin() - Vector( 0,0,60 ) ); //HACKHACK Bring the flag down to it's original height.
			pPlayer->bCarryingFlag = false;
			bFlagIsDropped = true;
			fReturnTime = gpGlobals->curtime + m_fFlagReturn;
			Q_snprintf( CTFMsg, 512, "%s Has Dropped The %s!", pPlayer->GetPlayerName(), cName );
			PrintAlert( CTFMsg );
			PlaySound( m_iDropSound );
			m_OnDropped.FireOutput( this, this ); //Fire the OnDropped output.
		}
	}
	else //If there isn't someone holding the flag, we need to search for a potential capturer or perhaps return it.
	{
		if ( bFlagIsDropped && gpGlobals->curtime > fReturnTime && sv_ctf_returnstyle.GetInt() == 1 ) //Should the flag return?
			ReturnFlag();

		CBasePlayer *pPlayer = NULL;
		while( (pPlayer = dynamic_cast<CBasePlayer*>(gEntList.FindEntityByClassnameWithin( pPlayer, "player", GetLocalOrigin(), m_flPickupRadius ))) != NULL )
		{
			if ( !pPlayer->IsAlive() )
				continue;
		
			int TeamNumber = pPlayer->GetTeamNumber();
			if ( iTeam != NULL && TeamNumber != iTeam )
			{
				if( sv_ctf_returnstyle.GetInt() == 2 && bFlagIsDropped )
					ReturnFlag();
	
				continue;
			}

			//Assuming you've passed all the checks, you deserve to pick up the flag. So run the code below.
			pPlayer->CtfFlag = this;	//So the capture trigger will know which flag is being captured.
			SetAbsOrigin( pPlayer->GetAbsOrigin() + Vector( 0,0,60 ) );	//Keeps the flag out of the player's FOV, also raises it so it doesn't look like it's stuck in the player's grill.
			SetParent( pPlayer );	//Attach the entity to the player.
			pPlayer->iSpeed = pPlayer->iSpeed - sv_ctf_flagweight.GetInt(); //For the player speed difference.
			pPlayer->bCarryingFlag = true;
			bFlagIsDropped = false;
			Q_snprintf( CTFMsg, 512, "%s Has Taken The %s Flag!", pPlayer->GetPlayerName(), cFlagName );
			PrintAlert( CTFMsg );
			PlaySound( m_iPickupSound );
			m_OnPickedUp.FireOutput( this, this ); //Fire the OnPickedUp output.
		}
	}
}

void CtfFlag::PrintAlert( char *Msg )
{
	extern ConVar sv_flagalerts;
	switch( sv_flagalerts.GetInt() )
	{
		case 1:
			UTIL_ClientPrintAll( HUD_PRINTCENTER, Msg );
			break;
		case 2:
			UTIL_ClientPrintAll( HUD_PRINTTALK, Msg );
			break;

	}
}
void CtfFlag::ReturnFlag( void )
{
	SetAbsOrigin( FlagOrigin );
	SetAbsAngles( FlagAngle );
	bFlagIsDropped = false;
	Q_snprintf( CTFMsg, 512, "The %s Flag Has Returned!", cFlagName );
	PrintAlert( CTFMsg );
	PlaySound( m_iReturnSound );
	m_OnReturned.FireOutput( this, this ); //Fire the OnReturned output.
}
void CtfFlag::PlaySound( int iSound )
{
	if ( iSound == NULL )
		return;

	char *SoundFile = (char *)iSound;
	PrecacheScriptSound(SoundFile);
	EmitSound( SoundFile );
}
void CtfFlag::InputReset( inputdata_t &inputData )
{
	if (GetAbsOrigin() == FlagOrigin)
		return;
	if ( GetParent() )
	{
		CBasePlayer *pPlayer = dynamic_cast< CBasePlayer* >( GetParent() );
		pPlayer->bCarryingFlag = false; //Little hack to make sure people can't get points.
	}
	ReturnFlag();
}
BEGIN_DATADESC( CtfFlag )

	DEFINE_KEYFIELD( m_flPickupRadius, FIELD_FLOAT, "PickupRadius" ),
	DEFINE_KEYFIELD( m_iForTeam, FIELD_INTEGER, "ForTeam" ),
	DEFINE_KEYFIELD( m_fReturnTime, FIELD_FLOAT, "ReturnTime" ),
	DEFINE_KEYFIELD( m_iPickupSound, FIELD_SOUNDNAME, "PickupSound" ),
	DEFINE_KEYFIELD( m_iWeight, FIELD_SOUNDNAME, "Weight" ),
	DEFINE_KEYFIELD( m_iDropSound, FIELD_SOUNDNAME, "DropSound" ),
	DEFINE_KEYFIELD( m_iReturnSound, FIELD_SOUNDNAME, "ReturnSound" ),
	DEFINE_KEYFIELD( m_strName,	FIELD_STRING,	"Name" ),
	DEFINE_KEYFIELD( m_strModelName,	FIELD_STRING,	"ModelName" ),

	DEFINE_THINKFUNC( Think ),

	DEFINE_OUTPUT( m_OnDropped, "OnDropped" ),
	DEFINE_OUTPUT( m_OnPickedUp, "OnPickedUp" ),
	DEFINE_OUTPUT( m_OnReturned, "OnReturned" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Reset", InputReset ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( ctf_flag, CtfFlag);
PRECACHE_REGISTER(ctf_flag);
*/