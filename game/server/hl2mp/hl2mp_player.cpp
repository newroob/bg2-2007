//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "hl2mp_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"
#include "KeyValues.h"
#include "team.h"
#include "weapon_hl2mpbase.h"
//#include "grenade_satchel.h"
#include "eventqueue.h"
//#include "GameStats.h"

//#includes - HairyPotter
#include "ammodef.h"
//

//BG2 - Tjoppen - #includes
#include "triggers.h"
#include "bg2/flag.h"
#include "bg2/vcomm.h"
#include "bg2/spawnpoint.h"
#include "bg2/weapon_bg2base.h"
//

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"
#include "bg2/ctfflag.h"

extern ConVar mp_autobalanceteams;
extern ConVar mp_autobalancetolerance;
//BG2 - Tjoppen - sv_voicecomm_text, set to zero for realism
ConVar sv_voicecomm_text( "sv_voicecomm_text", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Show voicecomm text on clients?" );
ConVar sv_unlag_lmb( "sv_unlag_lmb", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Do unlagging for left mouse button (primary attack)?" );
ConVar sv_unlag_rmb( "sv_unlag_rmb", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Do unlagging for right mouse button (primary attack)?" );
ConVar sv_saferespawntime( "sv_saferespawntime", "2.5f", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Amount of time after respawn when player is immune to damage. (In Seconds)");
//

//BG2 - Spawn point optimization test - HairyPotter
CUtlVector<CBaseEntity *> m_MultiSpawns, m_AmericanSpawns, m_BritishSpawns;
//

int g_iLastCitizenModel = 0;
int g_iLastCombineModel = 0;

//BG2 - Tjoppen - away with these
/*CBaseEntity	 *g_pLastCombineSpawn = NULL;
CBaseEntity	 *g_pLastRebelSpawn = NULL;*/
//extern CBaseEntity				*g_pLastSpawn;
//

#define HL2MP_COMMAND_MAX_RATE 0.3

void ClientKill( edict_t *pEdict );
//BG2 - Tjoppen - don't need this
//void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );
//

LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

//BG2 - Tjoppen - CSpawnPoint
BEGIN_DATADESC( CSpawnPoint )
	DEFINE_KEYFIELD( m_iDefaultTeam, FIELD_INTEGER, "StartingTeam" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

	//Below are for info_player_multi
	DEFINE_INPUTFUNC( FIELD_VOID, "ToggleTeam", InputToggleTeam ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetAmerican", InputAmerican ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetBritish", InputBritish),
	//
END_DATADESC()
//

//BG2 - Tjoppen - info_player_american/british
LINK_ENTITY_TO_CLASS( info_player_american, CSpawnPoint );
LINK_ENTITY_TO_CLASS( info_player_british, CSpawnPoint );
LINK_ENTITY_TO_CLASS( info_player_multispawn, CSpawnPoint );
//

IMPLEMENT_SERVERCLASS_ST(CHL2MP_Player, DT_HL2MP_Player)
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11, SPROP_CHANGES_OFTEN ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11, SPROP_CHANGES_OFTEN ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	//BG2 - Tjoppen - tweaking
	SendPropInt( SENDINFO( m_iSpawnInterpCounter), 2, SPROP_UNSIGNED ), //4 ),
	//BG2 - Tjoppen - don't need this
	//SendPropInt( SENDINFO( m_iPlayerSoundType), 3 ),
	//
	
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	//SendPropExclude( "DT_BaseFlex", "m_viewtarget" ), //BG2 - Not needed anymore. -HairyPotter

	//BG2 - Tjoppen - send stamina via C_HL2MP_Player <=> DT_HL2MP_Player <=> CHL2MP_Player
	SendPropInt( SENDINFO( m_iStamina ), 7, SPROP_UNSIGNED ),	//0 <= stamina <= 100, 7 bits enough
	//
	//BG2 - Tjoppen - m_iClass and m_iCurrentAmmoKit are network vars
	SendPropInt( SENDINFO( m_iClass ), 3, SPROP_UNSIGNED ),			//BG2 - Tjoppen - remember: max eight classes per team or increase this
	SendPropInt( SENDINFO( m_iCurrentAmmoKit ), 2, SPROP_UNSIGNED ),//BG2 - Tjoppen - remember: max four ammo kits or increase this
	SendPropInt( SENDINFO( m_iSpeedModifier ), 9, 0 ),
	//
	//BG2 - Tjoppen - rewards put on hold
	/*SendPropInt( SENDINFO( m_iOfficerReward), 7, SPROP_UNSIGNED ),	//BG2 - Draco - Rewards
	SendPropInt( SENDINFO( m_iSniperReward), 7, SPROP_UNSIGNED ),	//BG2 - Draco - Rewards
	SendPropInt( SENDINFO( m_iInfantryReward), 7, SPROP_UNSIGNED ),	//BG2 - Draco - Rewards*/

//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
	
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
END_DATADESC()

const char *g_ppszRandomCitizenModels[] = 
{
	//BG2 - Tjoppen - models
	"models/player/british/jager/jager.mdl",
	"models/player/british/medium_b/medium_b.mdl",
	"models/player/british/light_b/light_b.mdl",
	"models/player/british/mohawk/mohawk.mdl",
	"models/player/british/loyalist/loyalist.mdl",
};

const char *g_ppszRandomCombineModels[] =
{
	//BG2 - Tjoppen - models
	"models/player/american/heavy_a/heavy_a.mdl",
	"models/player/american/medium_a/medium_a.mdl",
	"models/player/american/light_a/light_a.mdl",
	"models/player/american/militia/militia.mdl",
};


#define MAX_COMBINE_MODELS 4
#define MODEL_CHANGE_INTERVAL 5.0f
#define TEAM_CHANGE_INTERVAL 5.0f

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

CHL2MP_Player::CHL2MP_Player() : m_PlayerAnimState( this )
{
	m_angEyeAngles.Init();

	m_iLastWeaponFireUsercmd = 0;

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	m_iSpawnInterpCounter = 0;

    m_bEnterObserver = false;
	m_bReady = false;

	//BG2 - Default weapon kits. -Hairypotter
	m_iGunKit = 1;
	m_iCurrentAmmoKit = m_iAmmoKit = AMMO_KIT_BALL;
	//

	BaseClass::ChangeTeam( 0 );
	
	//UseClientSideAnimation();

	//BG2 - Tjoppen - don't pick a class..
	m_iClass = m_iNextClass = -1;//RandomInt( 0, 2 );
	//

	//BG2 - Tjoppen - tickets
	m_bDontRemoveTicket = true;
}

CHL2MP_Player::~CHL2MP_Player( void )
{

}

void CHL2MP_Player::UpdateOnRemove( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void CHL2MP_Player::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel ( "sprites/glow01.vmt" );

	//Precache Citizen models
	int nHeads = ARRAYSIZE( g_ppszRandomCitizenModels );
	int i;	

	for ( i = 0; i < nHeads; ++i )
	   	 PrecacheModel( g_ppszRandomCitizenModels[i] );

	//Precache Combine Models
	nHeads = ARRAYSIZE( g_ppszRandomCombineModels );

	for ( i = 0; i < nHeads; ++i )
	   	 PrecacheModel( g_ppszRandomCombineModels[i] );

	PrecacheFootStepSounds();

	//BG2 - Tjoppen - precache sounds
	/*PrecacheScriptSound( "NPC_MetroPolice.Die" );
	PrecacheScriptSound( "NPC_CombineS.Die" );
	PrecacheScriptSound( "NPC_Citizen.die" );*/
	PrecacheScriptSound( "BG2Player.die" );
	//PrecacheScriptSound( "BG2Player.pain" );

	//assume right leg is the biggest
	for( i = 0; i <= HITGROUP_RIGHTLEG; i++ )
	{
		PrecacheScriptSound( GetHitgroupPainSound( i, TEAM_AMERICANS ) );
		PrecacheScriptSound( GetHitgroupPainSound( i, TEAM_BRITISH ) );
	}

	for( i = 0; i < NUM_VOICECOMMS; i++ )
	{
		char name[256];

		Q_snprintf( name, sizeof name, "Voicecomms.American.Inf_%i", i+1 );
		PrecacheScriptSound( name );
		Q_snprintf( name, sizeof name, "Voicecomms.American.Off_%i", i+1 );
		PrecacheScriptSound( name );
		Q_snprintf( name, sizeof name, "Voicecomms.American.Rif_%i", i+1 );
		PrecacheScriptSound( name );

		Q_snprintf( name, sizeof name, "Voicecomms.British.Inf_%i", i+1 );
		PrecacheScriptSound( name );
		Q_snprintf( name, sizeof name, "Voicecomms.British.Off_%i", i+1 );
		PrecacheScriptSound( name );
		Q_snprintf( name, sizeof name, "Voicecomms.British.Rif_%i", i+1 );
		PrecacheScriptSound( name );
		Q_snprintf( name, sizeof name, "Voicecomms.British.Ski_%i", i+1 );
		PrecacheScriptSound( name );
	}
}

void CHL2MP_Player::GiveAllItems( void )
{
	CBasePlayer::GiveAmmo( 32,	"357" );
	
	//BG2 - Tjoppen - impulse 101
	GiveNamedItem( "weapon_brownbess" );
	GiveNamedItem( "weapon_revolutionnaire" );
	GiveNamedItem( "weapon_brownbess_nobayo" );
	GiveNamedItem( "weapon_beltaxe" );
	GiveNamedItem( "weapon_tomahawk" );
	GiveNamedItem( "weapon_charleville" );
	GiveNamedItem( "weapon_pennsylvania" );
	GiveNamedItem( "weapon_jaeger" );
	GiveNamedItem( "weapon_pistol_a" );
	GiveNamedItem( "weapon_pistol_b" );
	GiveNamedItem( "weapon_sabre_a" );
	GiveNamedItem( "weapon_sabre_b" );
	GiveNamedItem( "weapon_hirschf" );
	GiveNamedItem( "weapon_knife" );
}

void CHL2MP_Player::GiveDefaultItems( void )
{
	//EquipSuit();

	//BG2 - Tjoppen - default equipment - also strip old equipment
	/*RemoveAllAmmo();
	Weapon_DropAll( true );*/
	RemoveAllItems( false );

	/*CBasePlayer::GiveAmmo( 255,	"Pistol");
	CBasePlayer::GiveAmmo( 45,	"SMG1");
	CBasePlayer::GiveAmmo( 1,	"grenade" );
	CBasePlayer::GiveAmmo( 6,	"Buckshot");*/
	//CBasePlayer::GiveAmmo( 24,	"357", true );

	//remember which ammo kit we spawned with
	m_iCurrentAmmoKit = m_iAmmoKit;
	
	if( GetTeam()->GetTeamNumber() == TEAM_AMERICANS )	//Americans
	{
		switch( m_iClass )
		{
		default:
		case CLASS_INFANTRY:
			switch ( m_iGunKit )
			{
				default: //In case we get the wrong number or something...
				case 1:
					GiveNamedItem( "weapon_charleville" );
					break;
				case 2:
					GiveNamedItem( "weapon_american_brownbess" );
					break;
				case 3:
					GiveNamedItem( "weapon_revolutionnaire" );
					break;
			}
			CBasePlayer::SetAmmoCount( 36,	GetAmmoDef()->Index("357")); //Default ammo for Infantry. -HairyPotter
			break;
		case CLASS_OFFICER:
			GiveNamedItem( "weapon_pistol_a" );
			GiveNamedItem( "weapon_sabre_a" );
			CBasePlayer::SetAmmoCount( 12,	GetAmmoDef()->Index("357")); //Default ammo for Officers. -HairyPotter
			break;
		case CLASS_SNIPER:
			GiveNamedItem( "weapon_pennsylvania" );
			GiveNamedItem( "weapon_knife" );
			CBasePlayer::SetAmmoCount( 24,	GetAmmoDef()->Index("357")); //Default ammo for Snipers. -HairyPotter
			break;
		case CLASS_SKIRMISHER:
			//GiveNamedItem( "weapon_tomahawk" );
			switch ( m_iGunKit )
			{
				default: //In case we get the wrong number or something...
				case 1:
					GiveNamedItem( "weapon_fowler" );
					break;
				case 2:
					GiveNamedItem( "weapon_american_brownbess_nobayo" );
					break;
			}
			GiveNamedItem( "weapon_beltaxe" );
			CBasePlayer::SetAmmoCount( 24,	GetAmmoDef()->Index("357")); //Default ammo for Skirmishers. -HairyPotter
			break;
		}

		//Weapon_Switch( Weapon_OwnsThisType( "weapon_revolutionnaire" ) );
	}
	else if( GetTeam()->GetTeamNumber() == TEAM_BRITISH )	//british
	{
		switch( m_iClass )
		{
		default:
		case CLASS_INFANTRY:
			switch ( m_iGunKit )
			{
				default: //In case we get the wrong number or something...
				case 1:
					GiveNamedItem( "weapon_brownbess" );
					break;
				case 2:
					GiveNamedItem( "weapon_longpattern" );
					break;
			}
			CBasePlayer::SetAmmoCount( 36,	GetAmmoDef()->Index("357")); //Default ammo for Infantry. -HairyPotter
			break;
		case CLASS_OFFICER:
			GiveNamedItem( "weapon_pistol_b" );
			GiveNamedItem( "weapon_sabre_b" );
			CBasePlayer::SetAmmoCount( 12,	GetAmmoDef()->Index("357")); //Default ammo for Officers. -HairyPotter
			break;
		case CLASS_SNIPER:
			GiveNamedItem( "weapon_jaeger" );
			GiveNamedItem( "weapon_hirschf" );
			CBasePlayer::SetAmmoCount( 24,	GetAmmoDef()->Index("357")); //Default ammo for Snipers. -HairyPotter
			break;
		case CLASS_SKIRMISHER:
			//GiveNamedItem( "weapon_brownbess_nobayo", 1 ); //So the native skin is set to 1 (2 in HLMV since 0 is default in the code)
			switch ( m_iGunKit )
			{
				default: //In case we get the wrong number or something...
				case 1:
					GiveNamedItem( "weapon_brownbess_nobayo" );
					break;
				case 2:
					GiveNamedItem( "weapon_longpattern_nobayo" );
					break;
			}
			GiveNamedItem( "weapon_tomahawk" );
			CBasePlayer::SetAmmoCount( 24,	GetAmmoDef()->Index("357")); //Default ammo for Skirmishers. -HairyPotter
			break;
		case CLASS_LIGHT_INFANTRY:
			GiveNamedItem( "weapon_brownbess_carbine" );
			CBasePlayer::SetAmmoCount( 24,	GetAmmoDef()->Index("357")); //BG2 - Tjoppen - Default ammo for light infantry
		}
		
		//Weapon_Switch( Weapon_OwnsThisType( "weapon_brownbess" ) );
	}
}

//BG2 - Tjoppen - g_pLastIntermission
CBaseEntity	*g_pLastIntermission = NULL;
//

void CHL2MP_Player::PickDefaultSpawnTeam( void )
{

	//BG2 - Tjoppen - make players just joining the game be in intermission..
	//	this is a bit flaky at the moment.. maybe later

	if( GetTeamNumber() != TEAM_UNASSIGNED )
	{
		m_pIntermission = NULL;	//just to be safe
		return;					//we get called on spawn, so anyone that has a team shouldn't get teleported around and stuff
	}

	SetModel( "models/player/british/jager/jager.mdl" );	//shut up about no model already!
	AddEffects( EF_NODRAW );
	//try to find a spot..
	CBaseEntity *pSpot = gEntList.FindEntityByClassname( g_pLastIntermission, "info_intermission");
	if( !pSpot )
	{
		//reached end, start over
		g_pLastIntermission = pSpot = gEntList.FindEntityByClassname( NULL, "info_intermission");

		if( !pSpot )
		{
			Msg( "WARNING: No info_intermission in current map. Tell the mapper!\n" );
			return;	//oh no! no info_intermission
		}
	}
	else
	{
		g_pLastIntermission = pSpot;
	}

	m_pIntermission = pSpot;

	SetAbsOrigin( pSpot->GetAbsOrigin() );
	SnapEyeAngles( pSpot->GetAbsAngles() );

	pl.fixangle = FIXANGLE_ABSOLUTE;

	return;
	//
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2MP_Player::Spawn(void)
{
	//BG2 - Always wait.
	//m_flNextModelChangeTime = 0.0f;
	//m_flNextTeamChangeTime = 0.0f;


	//BG2 - Tjoppen - reenable spectators
	if ( GetTeamNumber() == TEAM_SPECTATOR )
		return;	//we're done
	//

	PickDefaultSpawnTeam(); //All this does is sends the player to info_intermission if they aren't on a team.

	//BG2 - Tjoppen - reenable spectators
	if ( GetTeamNumber() <= TEAM_SPECTATOR )
		return;	//we're done
	//

	//reset speed modifier
	SetSpeedModifier( 0 );

	//BG2 - Tjoppen - pick correct model
	m_iClass = m_iNextClass;				//BG2 - Tjoppen - sometimes these may not match

	PlayermodelTeamClass( GetTeamNumber(), m_iClass ); //BG2 - Just set the player models on spawn. We have the technology. -HairyPotter

	BaseClass::Spawn();

	m_flNextVoicecomm = gpGlobals->curtime;	//BG2 - Tjoppen - reset voicecomm timer
	m_iStamina = 100;						//BG2 - Draco - reset stamina to 100
	m_fNextStamRegen = gpGlobals->curtime;	//BG2 - Draco - regen stam now!

	pl.deadflag = false;
	RemoveSolidFlags( FSOLID_NOT_SOLID );

	RemoveEffects( EF_NODRAW );

	StopObserverMode();

	UpdateWaterState(); //BG2 - This fixes the ammo respawn bug if player dies underwater. -HairyPotter
	
	GiveDefaultItems();

	RemoveEffects( EF_NOINTERP );

	SetNumAnimOverlays( 3 );
	ResetAnimation();

	m_nRenderFX = kRenderNormal;

	m_Local.m_iHideHUD = 0;
	
	AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	m_impactEnergyScale = HL2MPPLAYER_PHYSDAMAGE_SCALE;

	if ( HL2MPRules()->IsIntermission() )
	{
		AddFlag( FL_FROZEN );
	}
	else
	{
		RemoveFlag( FL_FROZEN );
	}

	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	//BG2 - Tjoppen - don't need quite this many bits
	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) & 3; //8;
	//

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);

	//BG2 - Put the speed handler into spawn so flag weight works, among other things. -HairyPotter
	//5 speed added to all classes.
	HandleSpeedChanges();
	//

	//BG2 - Tjoppen - tickets
	if( HL2MPRules()->UsingTickets() && GetTeam() )
	{
		if( m_bDontRemoveTicket )
			m_bDontRemoveTicket = false;
		else
			GetTeam()->RemoveTicket();
	}

	m_bReady = false;
	m_fLastRespawn = gpGlobals->curtime + sv_saferespawntime.GetFloat();
}

void CHL2MP_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	
}

//BG2 - Tjoppen - don't need this
/*void CHL2MP_Player::SetupPlayerSoundsByModel( const char *pModelName )
{
	if ( Q_stristr( pModelName, "models/human") )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_CITIZEN;
	}
	else if ( Q_stristr(pModelName, "police" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_METROPOLICE;
	}
	else if ( Q_stristr(pModelName, "combine" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_COMBINESOLDIER;
	}
}*/

void CHL2MP_Player::ResetAnimation( void )
{
	if ( IsAlive() )
	{
		SetSequence ( -1 );
		SetActivity( ACT_INVALID );

		if (!GetAbsVelocity().x && !GetAbsVelocity().y)
			SetAnimation( PLAYER_IDLE );
		else if ((GetAbsVelocity().x || GetAbsVelocity().y) && ( GetFlags() & FL_ONGROUND ))
			SetAnimation( PLAYER_WALK );
		else if (GetWaterLevel() > 1)
			SetAnimation( PLAYER_WALK );
	}
}


bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	if ( bRet == true )
	{
		ResetAnimation();
	}

	return bRet;
}

//=============================================================
//CHL2MP_Player's IncreaseReward
//increases the points towards a bonus, 1 for class, 2 for team
//=============================================================
//BG2 - Tjoppen - rewards put on hold
/*void CHL2MP_Player::IncreaseReward(int iType)
{
/*	switch (iType)
	{
		case 1://class specific
			switch (m_iClass)
			{
				case CLASS_INFANTRY:
					m_iInfantryReward++;
					switch (m_iInfantryReward)
					{
						case 10:
							m_iInfantryLevel = 2;
							break;
						case 20:
							m_iInfantryLevel = 3;
							break;
						case 30:
							m_iInfantryLevel = 4;
							break;
					}
					break;
				case CLASS_OFFICER:
					m_iOfficerReward++;
					switch (m_iOfficerReward)
					{
						case 10:
							m_iOfficerLevel = 2;
							break;
						case 6:
							m_iOfficerLevel = 3;
							break;
						case 9:
							m_iOfficerLevel = 4;
							break;
					}
					break;
				case CLASS_SNIPER:
					m_iSniperReward++;
					switch (m_iSniperReward)
					{
						case 3:
							m_iSniperLevel = 2;
							break;
						case 6:
							m_iSniperLevel = 3;
							break;
						case 9:
							m_iSniperLevel = 4;
							break;
					}
					break;
			}
			break;
		case 2://team specific
			switch (GetTeamNumber())
			{
				case TEAM_AMERICANS:
					m_iAmericanReward++;
					switch (m_iAmericanReward)
					{
						case 3:
							m_iAmericanLevel = 2;
							break;
						case 6:
							m_iAmericanLevel = 3;
							break;
						case 9:
							m_iAmericanLevel = 4;
							break;
					}
					break;
				case TEAM_BRITISH:
					m_iBritishReward++;
					switch (m_iBritishReward)
					{
						case 3:
							m_iBritishLevel = 2;
							break;
						case 6:
							m_iBritishLevel = 3;
							break;
						case 9:
							m_iBritishLevel = 4;
							break;
					}
					break;
			}
			break;
	}*//*
}*/

void CHL2MP_Player::HandleSpeedChanges( void )
{
	SetMaxSpeed( GetCurrentSpeed() );
}

void CHL2MP_Player::SetSpeedModifier( int iSpeedModifier )
{
	m_iSpeedModifier = iSpeedModifier;
}

void CHL2MP_Player::PreThink( void )
{
	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();

	HandleSpeedChanges();

	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
	SetLocalAngles( vOldAngles );
}

void CHL2MP_Player::PostThink( void )
{
	BaseClass::PostThink();

	//BG2 - Draco - recurring stamina regen
	//BG2 - Tjoppen - update stamina 6 units at a time, at about 3Hz, to reduce network load
	if ( m_fNextStamRegen <= gpGlobals->curtime )
	{
		//this causes stamina regen speed to ramp from 11.7 to 23.3 units per second based on health
		m_iStamina += 3;
		m_fNextStamRegen = gpGlobals->curtime + 0.9 / 700.0f * (200.0f - m_iHealth);
	}

	if( m_iStamina > 100 )
		m_iStamina = 100;	//cap if for some reason it went over 100
	//
	
	if ( GetFlags() & FL_DUCKING )
	{
		SetCollisionBounds( VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX );
	}

	m_PlayerAnimState.Update();

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );

	//BG2 - Tjoppen - follow info_intermission
	if( m_pIntermission && GetTeamNumber() == TEAM_UNASSIGNED )
	{
		SetAbsOrigin( m_pIntermission->GetAbsOrigin() );
		SnapEyeAngles( m_pIntermission->GetAbsAngles() );
		pl.fixangle = FIXANGLE_ABSOLUTE;
	}
}

void CHL2MP_Player::PlayerDeathThink()
{
	if( !IsObserver() )
	{
		BaseClass::PlayerDeathThink();
	}
}

void CHL2MP_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	//lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() ); //BG2 - Looks like this was causing players to "jump" around whenever someone shot them. -HairyPotter

	FireBulletsInfo_t modinfo = info;

	CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		//BG2 - Tjoppen - maintain compatibility with HL2 weapons...
		if( modinfo.m_iDamage == -1 )
			modinfo.m_iDamage = modinfo.m_iPlayerDamage;
		else
		//
		modinfo.m_iPlayerDamage = modinfo.m_iDamage = pWeapon->GetHL2MPWpnData().m_iPlayerDamage;
	}

	NoteWeaponFired();

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	//lagcompensation->FinishLagCompensation( this ); //BG2 - Looks like this was causing players to "jump" around whenever someone shot them. -HairyPotter
}

void CHL2MP_Player::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

extern ConVar sv_maxunlag;

bool CHL2MP_Player::WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	//BG2 - Tjoppen - MELEE FIX! I've never been so relieved! So relieved - and ANGRY!
	//if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
	//if ( !( pCmd->buttons & IN_ATTACK ) && !( pCmd->buttons & IN_ATTACK2 ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )

	if( (!( pCmd->buttons & IN_ATTACK ) || !sv_unlag_lmb.GetBool()) &&	//button not pressed or shouldn't be unlagged
		(!( pCmd->buttons & IN_ATTACK2) || !sv_unlag_rmb.GetBool()) &&	//-"-
		(pCmd->command_number - m_iLastWeaponFireUsercmd > 10) )		//unlag a bit longer
	//
		return false;

	// If this entity hasn't been transmitted to us and acked, then don't bother lag compensating it.
	if ( pEntityTransmitBits && !pEntityTransmitBits->Get( pPlayer->entindex() ) )
		return false;

	const Vector &vMyOrigin = GetAbsOrigin();
	const Vector &vHisOrigin = pPlayer->GetAbsOrigin();

	// get max distance player could have moved within max lag compensation time, 
	// multiply by 1.5 to to avoid "dead zones"  (sqrt(2) would be the exact value)
	float maxDistance = 1.5 * pPlayer->MaxSpeed() * sv_maxunlag.GetFloat();

	// If the player is within this distance, lag compensate them in case they're running past us.
	if ( vHisOrigin.DistTo( vMyOrigin ) < maxDistance )
		return true;

	// If their origin is not within a 45 degree cone in front of us, no need to lag compensate.
	Vector vForward;
	AngleVectors( pCmd->viewangles, &vForward );
	
	Vector vDiff = vHisOrigin - vMyOrigin;
	VectorNormalize( vDiff );

	float flCosAngle = 0.707107f;	// 45 degree angle
	if ( vForward.Dot( vDiff ) < flCosAngle )
		return false;

	return true;
}

Activity CHL2MP_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	if ( m_iModelType == TEAM_BRITISH )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

//extern ConVar hl2_normspeed;

// Set the activity based on an event or current state
void CHL2MP_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;

	float speed;

	speed = GetAbsVelocity().Length2D();

	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	
	// bool bRunning = true;

	//Revisit!
/*	if ( ( m_nButtons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
	{
		if ( speed > 1.0f && speed < hl2_normspeed.GetFloat() - 20.0f )
		{
			bRunning = false;
		}
	}*/

	if ( GetFlags() & ( FL_FROZEN | FL_ATCONTROLS ) )
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	Activity idealActivity = ACT_HL2MP_RUN;

	// This could stand to be redone. Why is playerAnim abstracted from activity? (sjb)
	if ( playerAnim == PLAYER_JUMP )
	{
		idealActivity = ACT_HL2MP_JUMP;
	}
	else if ( playerAnim == PLAYER_DIE )
	{
		if ( m_lifeState == LIFE_ALIVE )
		{
			return;
		}
	}
	else if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( pWeapon->m_bIsIronsighted )
		{
			//play "shoot while aiming" animation instead
			idealActivity = ACT_BG2_IRONSIGHTS_RECOIL;
		}
		else if ( GetActivity( ) == ACT_HOVER	|| 
			 GetActivity( ) == ACT_SWIM		||
			 GetActivity( ) == ACT_HOP		||
			 GetActivity( ) == ACT_LEAP		||
			 GetActivity( ) == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity( );
		}
		else
		{
			idealActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
		}
	}
	//BG2 - Tjoppen - PLAYER_ATTACK2
	else if ( playerAnim == PLAYER_ATTACK2 )
	{
		if ( GetActivity( ) == ACT_HOVER	|| 
			 GetActivity( ) == ACT_SWIM		||
			 GetActivity( ) == ACT_HOP		||
			 GetActivity( ) == ACT_LEAP		||
			 GetActivity( ) == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity( );
		}
		else
		{
			idealActivity = ACT_RANGE_ATTACK2;
		}
	}
	//
	else if ( playerAnim == PLAYER_RELOAD )
	{
			idealActivity = ACT_HL2MP_GESTURE_RELOAD;
	}
	else if ( playerAnim == PLAYER_HOLSTER )
	{
		RemoveAllGestures(); //BG2 - It took me 2 solid hours to come up with this hack. - HairyPotter
	}
	else if ( playerAnim == PLAYER_IDLE || playerAnim == PLAYER_WALK )
	{
		if ( !( GetFlags() & FL_ONGROUND ) && GetActivity( ) == ACT_HL2MP_JUMP )	// Still jumping
		{
			idealActivity = GetActivity( );
		}
		/*
		else if ( GetWaterLevel() > 1 )
		{
			if ( speed == 0 )
				idealActivity = ACT_HOVER;
			else
				idealActivity = ACT_SWIM;
		}
		*/
		else
		{
			if ( !pWeapon )
				return;

			if ( GetFlags() & FL_DUCKING )
			{
				if ( speed > 0 )
				{
					idealActivity = ACT_HL2MP_WALK_CROUCH;
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE_CROUCH;
				}
			}
			else
			{
				if ( speed > 0 )
				{
					if ( pWeapon->m_bIsIronsighted || pWeapon->m_bInReload ) //BG2 - This may need to be changed.. -HairyPotter
						idealActivity = ACT_BG2_IRONSIGHTS_WALK;

					else
						idealActivity = ACT_HL2MP_RUN;
				}
				else
				{
					if ( pWeapon->m_bIsIronsighted ) //BG2 - This may need to be changed.. -HairyPotter
						idealActivity = ACT_BG2_IRONSIGHTS_AIM;

					else
						idealActivity = ACT_HL2MP_IDLE;
				}
			}
		}

		//BG2 - Tjoppen - idle weapons.. but not until smoke and stuff have come out
		if( pWeapon && pWeapon->m_flNextPrimaryAttack < gpGlobals->curtime &&
			pWeapon->m_flNextSecondaryAttack < gpGlobals->curtime )
			Weapon_SetActivity( Weapon_TranslateActivity( ACT_HL2MP_IDLE ), 0 );
		//

		idealActivity = TranslateTeamActivity( idealActivity );
	}
	
	if ( idealActivity == ACT_HL2MP_GESTURE_RANGE_ATTACK || idealActivity == ACT_BG2_IRONSIGHTS_RECOIL )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );

		// FIXME: this seems a bit wacked
		Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0 );

		return;
	}
	//BG2 - Tjoppen - ACT_RANGE_ATTACK2
	else if ( idealActivity == ACT_RANGE_ATTACK2 )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK2 ), 0 );
		return;
	}
	//
	else if ( idealActivity == ACT_HL2MP_GESTURE_RELOAD )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		return;
	}
	else
	{
		SetActivity( idealActivity );

		animDesired = SelectWeightedSequence( Weapon_TranslateActivity ( idealActivity ) );

		if (animDesired == -1)
		{
			animDesired = SelectWeightedSequence( idealActivity );

			if ( animDesired == -1 )
			{
				animDesired = 0;
			}
		}
	
		// Already using the desired animation?
		if ( GetSequence() == animDesired )
			return;

		m_flPlaybackRate = 1.0;
		ResetSequence( animDesired );
		SetCycle( 0 );
		return;
	}

	// Already using the desired animation?
	if ( GetSequence() == animDesired )
		return;

	//Msg( "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	ResetSequence( animDesired );
	SetCycle( 0 );
}


extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

void CHL2MP_Player::ChangeTeam( int iTeam )
{
	bool bKill = false;

	if ( iTeam != GetTeamNumber() && GetTeamNumber() != TEAM_UNASSIGNED )
	{
		bKill = true;
	}

	ChangeTeam( iTeam, bKill );
}

void CHL2MP_Player::ChangeTeam( int iTeam, bool bKill )
{
	//BG2 - Tjoppen - changing team. remove self from flags
	if( iTeam != GetTeamNumber() )
		RemoveSelfFromFlags();
	//

	BaseClass::ChangeTeam( iTeam );

	m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;

	if ( iTeam == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );

		State_Transition( STATE_OBSERVER_MODE );
	}

	if ( bKill == true )
	{
		//BG2 - Tjoppen - teamchange suicides have no time limit...
		SetSuicideTime( gpGlobals->curtime );
		//
		CommitSuicide();
	}
}

//BG2 - Tjoppen - ClientPrinttTalkAll
void ClientPrinttTalkAll( char *str, int type )
{
	CBasePlayer *pPlayer = NULL;
	while( (pPlayer = (CBasePlayer*)gEntList.FindEntityByClassname( pPlayer, "player" )) != NULL )
		ClientPrint( pPlayer, type, str );
}
//

//BG2 - Tjoppen - PlayermodelTeamClass - gives models for specified team/class -- Restructured a bit. -HairyPotter
void CHL2MP_Player::PlayermodelTeamClass( int team, int classid )
{
	switch( team )
	{
	case TEAM_AMERICANS:
		switch( classid )
		{
		default:
		case CLASS_INFANTRY:
			SetModel("models/player/american/heavy_a/heavy_a.mdl");
			m_nSkin = RandomInt(0, 3);
			break;
		case CLASS_OFFICER:
			SetModel("models/player/american/light_a/light_a.mdl");
			break;
		case CLASS_SNIPER:
			SetModel("models/player/american/medium_a/medium_a.mdl");
			m_nSkin = RandomInt(0, 1);
			break;
		case CLASS_SKIRMISHER:
			SetModel("models/player/american/militia/militia.mdl");
			break;
		}
		break;
	case TEAM_BRITISH:
		switch( classid )
		{
		default:
		case CLASS_INFANTRY:
			SetModel("models/player/british/medium_b/medium_b.mdl");
			m_nSkin = RandomInt(0, 8);
			break;
		case CLASS_OFFICER:
			SetModel("models/player/british/light_b/light_b.mdl");
			break;
		case CLASS_SNIPER:
			SetModel("models/player/british/jager/jager.mdl");
			m_nSkin = RandomInt(0, 1);
			break;
		case CLASS_SKIRMISHER:
			SetModel("models/player/british/mohawk/mohawk.mdl"); //CAN WE PLEASE ORGANIZE THE PLAYER MODELS?
			m_nSkin = RandomInt(0, 1);
			break;
		case CLASS_LIGHT_INFANTRY:
			SetModel("models/player/british/loyalist/loyalist.mdl");
		}
		break;
	default: //default model
		SetModel("models/player/british/jager/jager.mdl");
		break;
	}
}

//BG2 - Tjoppen - CHL2MP_Player::MayRespawn()
bool CHL2MP_Player::MayRespawn( void )
{
	//note: this function only checks if we MAY respawn. not if we can(due to crowded spawns perhaps)
	if( GetTeamNumber() <= TEAM_SPECTATOR )
		return false;

	CTeam	*pAmer = g_Teams[TEAM_AMERICANS],
			*pBrit = g_Teams[TEAM_BRITISH];

	if( !pAmer || !pBrit )
		return false;

	if( pAmer->GetNumPlayers() <= 0 || pBrit->GetNumPlayers() <= 0 )
		return true;

	extern ConVar	mp_respawnstyle,
					mp_respawntime;

	switch( mp_respawnstyle.GetInt() )
	{
	case 1:
		//waves - we're not allowed to respawn by ourselves. the gamerules decide for us
		return false;
		break;
	case 2:
		//rounds - we're not allowed to respawn by ourselves. the gamerules decide for us
		return false;
		break;
	case 3:
		//rounds - we're not allowed to respawn by ourselves. the gamerules decide for us
		return false;
		break;
	default:
		if( gpGlobals->curtime > GetDeathTime() + DEATH_ANIMATION_TIME )
			return true;
		else
			return false;
	}
}
//

bool CHL2MP_Player::AttemptJoin( int iTeam, int iClass, const char *pClassName )
{
	//returns true on success, false otherwise
	CTeam	*pAmericans = g_Teams[TEAM_AMERICANS],
			*pBritish = g_Teams[TEAM_BRITISH];
	
	if( GetTeamNumber() == iTeam && m_iNextClass == iClass )
			return true;
	
	//check so we don't ruin the team balance..
	//BG2 - Tjoppen - don't bother with checking balance if we're just changing class, not team.
	//					CHL2MPRules::Think() will make sure the teams are kept balanced.
	//					We want to allow people to change class even if their team is too big.
	if( mp_autobalanceteams.GetInt() == 1 && GetTeamNumber() != iTeam ) //So the team we're attempting to join is different from our current team.
	{

		//Initialize just in case.
		int iAutoTeamBalanceTeamDiff = 0,
			iAutoTeamBalanceBiggerTeam = NULL,
			iNumAmericans = pAmericans->GetNumPlayers(), //
			iNumBritish = pBritish->GetNumPlayers(); //

		switch ( GetTeamNumber () ) //Our current team now, but we're changing teams..
		{
			case TEAM_AMERICANS:
				iNumAmericans -= 1; //-1 because we plan on leaving.
				break;
			case TEAM_BRITISH:
				iNumBritish -= 1;
				break;
		}
		
		//if (pAmericans->GetNumPlayers() > pBritish->GetNumPlayers()) //So there are more Americans than British.
		if ( iNumAmericans > iNumBritish )
		{
			//iAutoTeamBalanceTeamDiff = ((pAmericans->GetNumPlayers() - pBritish->GetNumPlayers()) ); //+ 1
			iAutoTeamBalanceTeamDiff = iNumAmericans - iNumBritish;
			iAutoTeamBalanceBiggerTeam = TEAM_AMERICANS;
		}
		else //More british than Americans.
		{
			//iAutoTeamBalanceTeamDiff = ((pBritish->GetNumPlayers() - pAmericans->GetNumPlayers()) ); //+ 1
			iAutoTeamBalanceTeamDiff = iNumBritish - iNumAmericans;
			iAutoTeamBalanceBiggerTeam = TEAM_BRITISH;
		}
		if ((iAutoTeamBalanceTeamDiff >= mp_autobalancetolerance.GetInt()) && (iAutoTeamBalanceBiggerTeam == iTeam)) 
		{
			//BG2 - Tjoppen - TODO: usermessage this
			//char *sTeamName = iTeam == TEAM_AMERICANS ? "American" : "British";
			ClientPrint( this, HUD_PRINTCENTER, "There are too many players on this team!\n" );
			return false;
		}
	}

	//check if there's a limit for this class, and if it has been exceeded
	//if we're changing back to the class we currently are, because perhaps we regret choosing
	// a new class, skip the limit check
	int limit = HL2MPRules()->GetLimitTeamClass( iTeam, iClass );
	if( limit >= 0 && g_Teams[iTeam]->GetNumOfNextClass(iClass) >= limit &&
		!(GetTeamNumber() == iTeam && m_iNextClass == iClass) ) //BG2 - check against next class, m_iClass would be officer making the if false - Roob
	{
		//BG2 - Tjoppen - TODO: usermessage this
		ClientPrint( this, HUD_PRINTCENTER, "There are too many of this class on your team!\n" );
		return false;
	} 

	//The following line prevents anyone else from stealing our spot..
	//Without this line several teamswitching/new players can pick a free class, so there can be for instance 
	// two loyalists even though the limit is one.
	//This may be slightly unfair since a still living player may "steal" a spot without spawning as that class,
	// since it's possible to switch classes around very fast. a player could block the use of a limited class,
	// though it'd be very tedious. It's a tradeoff.
	m_iNextClass = iClass;

	//m_iClass = m_iNextClass;

	if( GetTeamNumber() != iTeam )
	{
		//let Spawn() figure the model out
		CommitSuicide();

		if( GetTeamNumber() <= TEAM_SPECTATOR )
		{
			ChangeTeam( iTeam );
			if( MayRespawn() )
				Spawn();
		}
		else
			ChangeTeam( iTeam );
	}

	//BG2 - Tjoppen - TODO: usermessage this change 
	//This bit of code needs to be run AFTER the team change... if it changed.
	char str[512];
	CTeam *team = GetTeam();
	Q_snprintf( str, 512, "%s is going to fight as %s for the %s\n", GetPlayerName(), pClassName, team->GetName() );
	ClientPrinttTalkAll( str, HUD_BG2CLASSCHANGE  );
	
	//BG2 - Added for HlstatsX Support. -HairyPotter
	UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed role to \"%s\"\n", 
				GetPlayerName(), 
				GetUserID(), 
				GetNetworkIDString(), 
				team ? team->GetName() : "",
				pClassName);
	//

	return true;
}

/* HandleVoicecomm
 *	plays supplied voicecomm by index
 */
void CHL2MP_Player::HandleVoicecomm( int comm )
{
	bool teamonly = comm != NUM_BATTLECRY;	//battlecries are not team only (global)

	if( //only alive assigned player can do voice comms
		GetTeamNumber() > TEAM_SPECTATOR && IsAlive() && m_flNextVoicecomm <= gpGlobals->curtime &&

		//also make sure index not out of bounds
		comm >= 0 && comm < NUM_VOICECOMMS && //comm != VCOMM1_NUM && comm != VCOMM2_START+VCOMM2_NUM &&

		//make sure global voicecomms are only played ever so often - no FREEDOM! spam
		(teamonly || m_flNextGlobalVoicecomm <= gpGlobals->curtime)  )//&&

		//only officers may use officer-only voicecomms (commmenu2)
		//(comm < VCOMM2_START || m_iClass == CLASS_OFFICER) )
	{
		char snd[512];
		char *pClassString, *pTeamString;

		//BG2 - Make it a switch for great justice. -HairyPotter
		switch ( m_iClass )
		{
			case CLASS_INFANTRY:
				pClassString = "Inf";
				break;
			case CLASS_OFFICER:
				pClassString = "Off";
				break;
			case CLASS_SNIPER:
				pClassString = "Rif";
				break;
			case CLASS_SKIRMISHER: //TODO - Get native sounds in. - HairyPotter
				if ( GetTeamNumber() == TEAM_BRITISH )
					pClassString = "Ski";
				else
					pClassString = "Rif"; //We'll use infantry sounds for the Militia class.. for now.
				break;
			case CLASS_LIGHT_INFANTRY:
				pClassString = "Rif"; //same thing here..
				break;
			default:
				return;
		}

		switch ( GetTeamNumber() )
		{
			case TEAM_AMERICANS:
				pTeamString = "American";
				break;
			case TEAM_BRITISH:
				pTeamString = "British";
				break;
			default:
				return;
		}
		//

		Q_snprintf( snd, sizeof snd, "Voicecomms.%s.%s_%i", pTeamString, pClassString, comm + 1 );

		//play voicecomm, with attenuation
		//EmitSound( snd );

		//BG2 - Voice Comms are now Client-Side -HairyPotter
		CRecipientFilter recpfilter;
		//recpfilter.AddAllPlayers();
		recpfilter.AddRecipientsByPAS( GetAbsOrigin() ); //Instead, let's send this to players that are at least slose enough to hear it.. -HairyPotter
		recpfilter.MakeReliable(); //More network priority.
	
		UserMessageBegin( recpfilter, "VCommSounds" );
			WRITE_BYTE( entindex() );
			WRITE_STRING( snd );
		MessageEnd();
		//

		//done. possibly also tell clients to draw text and stuff if sv_voicecomm_text is true
		m_flNextVoicecomm		= gpGlobals->curtime + 2.0f;

		if( !teamonly )
			m_flNextGlobalVoicecomm	= gpGlobals->curtime + 10.0f;

		if( sv_voicecomm_text.GetBool() )
		{
			//figure out which players should get this message
			CRecipientFilter recpfilter;
			CBasePlayer *client = NULL;

			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				client = UTIL_PlayerByIndex( i );
				if ( !client || !client->edict() )
					continue;

				if ( !(client->IsNetClient()) )	// Not a client ? (should never be true)
					continue;

				if ( teamonly && !g_pGameRules->PlayerCanHearChat( client, this ) )//!= GR_TEAMMATE )
					continue;

				if ( !client->CanHearAndReadChatFrom( this ) )
					continue;

				recpfilter.AddRecipient( client );
			}

			//make reliable and send
			recpfilter.MakeReliable();

			UserMessageBegin( recpfilter, "VoiceComm" );
				WRITE_BYTE( entindex() );	//voicecomm originator
				WRITE_BYTE( comm | (GetTeamNumber() == TEAM_AMERICANS ? 32 : 0) );	//pack comm number and team
				WRITE_BYTE( m_iClass );	//class number
			MessageEnd();
		}
	}
}

bool CHL2MP_Player::ClientCommand( const CCommand &args )
{

	const char *cmd = args[0];
	if ( FStrEq( cmd, "kit" ) && args.ArgC() > 2 )
	{
		m_iGunKit = atoi( args[ 1 ] );
		m_iAmmoKit = atoi( args[ 2 ] );
		return true;
	}
	if ( FStrEq( cmd, "class" ) && args.ArgC() > 2 )
	{
		//Eh.. it's a little messy, but it seems a bit more efficent to just use a switch here rather than comparing strings over and over.
		switch ( atoi( args[ 1 ] ) ) //Team
		{
			case TEAM_BRITISH:
				switch ( atoi( args[ 2 ] ) ) //Class
				{
					case CLASS_INFANTRY:
						AttemptJoin( TEAM_BRITISH, CLASS_INFANTRY, "Royal Infantry" );
						break;
					case CLASS_OFFICER:
						AttemptJoin( TEAM_BRITISH, CLASS_OFFICER, "a Royal Commander" );
						break;
					case CLASS_SNIPER:
						AttemptJoin( TEAM_BRITISH, CLASS_SNIPER, "a Jaeger" );
						break;
					case CLASS_SKIRMISHER:
						AttemptJoin( TEAM_BRITISH, CLASS_SKIRMISHER, "a Native" );
						break;
					case CLASS_LIGHT_INFANTRY:
						AttemptJoin( TEAM_BRITISH, CLASS_LIGHT_INFANTRY, "Light Infantry" );
						break;
					default:
						Msg("Class selection invalid. \n");
						return false;
				}
				break;
			case TEAM_AMERICANS:
				switch ( atoi( args[ 2 ] ) ) //Class
				{
					case CLASS_INFANTRY:
						AttemptJoin( TEAM_AMERICANS, CLASS_INFANTRY, "a Continental Soldier" );
						break;
					case CLASS_OFFICER:
						AttemptJoin( TEAM_AMERICANS, CLASS_OFFICER, "a Continental Officer" );
						break;
					case CLASS_SNIPER:
						AttemptJoin( TEAM_AMERICANS, CLASS_SNIPER, "a Frontiersman" );
						break;
					case CLASS_SKIRMISHER:
						AttemptJoin( TEAM_AMERICANS, CLASS_SKIRMISHER, "Militia" );
						break;
					default:
						Msg("Class selection invalid. \n");
						return false;
				}
				break;
			default:
				Msg("Team selection invalid. \n");
				return false;
		}
		return true;
	}
	//BG2 - Tjoppen - voice comms
	else if( FStrEq( cmd, "voicecomm" ) && args.ArgC() > 1 )
	{
		int comm = atoi( args[ 1 ] );
		HandleVoicecomm( comm );

		return true;
	}
	else if( FStrEq( cmd, "battlecry" ) )
	{
		HandleVoicecomm( NUM_BATTLECRY );

		return true;
	}
	//

	return BaseClass::ClientCommand( args );
}

bool CHL2MP_Player::HandleCommand_JoinTeam( int team )
{
	if ( !GetGlobalTeam( team ) || team == 0 )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

	if ( team == TEAM_SPECTATOR )
	{
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			return false;
		}

		if ( GetTeamNumber() != TEAM_UNASSIGNED && !IsDead() )
		{
			m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work

			CommitSuicide();

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;
	}
	else
	{
		StopObserverMode();
		State_Transition(STATE_ACTIVE);
	}

	// Switch their actual team...
	ChangeTeam( team );

	return true;
}
void CHL2MP_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
		case 101:
			{
				if( sv_cheats->GetBool() )
				{
					GiveAllItems();
				}
			}
			break;

		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CHL2MP_Player::ShouldRunRateLimitedCommand( const CCommand &args )
{
	int i = m_RateLimitLastCommandTimes.Find( args[0] );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( args[0], gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < HL2MP_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

void CHL2MP_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CHL2MP_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CHL2MPRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CHL2MPRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( hl2mp_ragdoll, CHL2MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL2MPRagdoll, DT_HL2MPRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


void CHL2MP_Player::CreateRagdollEntity( void )
{
	//BG2 - Tjoppen - here's where we put code for multiple ragdolls
	if ( m_hRagdoll ) //One already exists.. remove it.
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	//CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( m_hRagdoll.Get() );// This makes no sense? We just removed our ragdoll.. 
																				   // Now we're trying to cast to it?
	
	//BG2 - Tjoppen - here's another place where we put code for multiple ragdolls
	//if ( !pRagdoll )
	//{
		// create a new one
		CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( CreateEntityByName( "hl2mp_ragdoll" ) );
	//}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		//BG2 - Tjoppen - clamp bullet force
		if( m_vecTotalBulletForce.Length() > 512.f )
		{
			VectorNormalize( m_vecTotalBulletForce );
			m_vecTotalBulletForce *= 512.f;
		}
		//
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
	}

	//BG2 - Tjoppen - remember to remove all ragdolls on round restart
	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2MP_Player::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOn( void )
{
	/*if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "HL2Player.FlashlightOn" );
	}*/
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOff( void )
{
	/*RemoveEffects( EF_DIMLIGHT );
	
	if( IsAlive() )
	{
		EmitSound( "HL2Player.FlashlightOff" );
	}*/
}

void CHL2MP_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	//Drop a grenade if it's primed.
	//BG2 - Tjoppen - don't need this
	/*if ( GetActiveWeapon() )
	{
		CBaseCombatWeapon *pGrenade = Weapon_OwnsThisType("weapon_frag");

		if ( GetActiveWeapon() == pGrenade )
		{
			if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) )
			{
				DropPrimedFragGrenade( this, pGrenade );
				return;
			}
		}
	}*/
	//
	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}


//BG2 - Tjoppen - don't need this
/*void CHL2MP_Player::DetonateTripmines( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );
}*/

void CHL2MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	SetNumAnimOverlays( 0 );

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	//BG2 - Tjoppen - reenable spectators - no dead bodies raining from the sky
	if( GetTeamNumber() > TEAM_SPECTATOR )
		CreateRagdollEntity();

	RemoveSelfFromFlags();

	//BG2 - Tjoppen - don't need this
	//DetonateTripmines();

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	CBaseEntity *pAttacker = info.GetAttacker();

	if ( pAttacker )
	{
		int iScoreToAdd = 1;

		if ( pAttacker == this )
		{
			iScoreToAdd = -1;
		}

		//BG2 - Tjoppen - only score by winnings rounds..
		//GetGlobalTeam( pAttacker->GetTeamNumber() )->AddScore( iScoreToAdd );
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	//StopZooming();
}

//BG2 - Tjoppen - GetHitgroupPainSound
const char* CHL2MP_Player::GetHitgroupPainSound( int hitgroup, int team )
{
	if( team == TEAM_AMERICANS )
	{
		switch( hitgroup )
		{
		default:
		case HITGROUP_GENERIC	: return "BG2Player.American.pain_generic";
		case HITGROUP_HEAD		: return "BG2Player.American.pain_head";
		case HITGROUP_CHEST		: return "BG2Player.American.pain_chest";
		case HITGROUP_STOMACH	: return "BG2Player.American.pain_stomach";
		case HITGROUP_LEFTARM	: return "BG2Player.American.pain_arm";
		case HITGROUP_RIGHTARM	: return "BG2Player.American.pain_arm";
		case HITGROUP_LEFTLEG	: return "BG2Player.American.pain_leg";
		case HITGROUP_RIGHTLEG	: return "BG2Player.American.pain_leg";
		}
	}
	else if( team == TEAM_BRITISH )
	{
		switch( hitgroup )
		{
		default:
		case HITGROUP_GENERIC	: return "BG2Player.British.pain_generic";
		case HITGROUP_HEAD		: return "BG2Player.British.pain_head";
		case HITGROUP_CHEST		: return "BG2Player.British.pain_chest";
		case HITGROUP_STOMACH	: return "BG2Player.British.pain_stomach";
		case HITGROUP_LEFTARM	: return "BG2Player.British.pain_arm";
		case HITGROUP_RIGHTARM	: return "BG2Player.British.pain_arm";
		case HITGROUP_LEFTLEG	: return "BG2Player.British.pain_leg";
		case HITGROUP_RIGHTLEG	: return "BG2Player.British.pain_leg";
		}
	}
	else
		return 0;
}
//

int CHL2MP_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//BG2 - Draco
	CBaseEntity * pAttacker = inputInfo.GetInflictor();
	if (pAttacker->IsPlayer())
	{
		CBasePlayer * pAttacker2 = (CBasePlayer *)pAttacker;
		//subtract 3x damage for attacking teammates!
		pAttacker2->IncrementDeathCount( (int)inputInfo.GetDamage()
										* (GetTeamNumber() == pAttacker2->GetTeamNumber() ? -3 : 1) );
	}

	//BG2 - Tjoppen - take damage => lose some stamina
	//have bullets drain a lot more stamina than melee damage
	//this makes firearms more useful, and avoids melee turning too slow due to stamina loss
	//have leg damage drain the most stamina. works as a consolation prize and also makes more sense
	//finally, arms drain the least stamina
	float scale;
	
	if( inputInfo.GetDamageType() & DMG_BULLET )
	{
		//legs = 75%
		//body/head = 50%
		//arms = 30% (keep same as melee for simplicity)
		if( LastHitGroup() == HITGROUP_LEFTLEG || LastHitGroup() == HITGROUP_RIGHTLEG )
			scale = 0.75f;
		else if( LastHitGroup() == HITGROUP_LEFTARM || LastHitGroup() == HITGROUP_RIGHTARM )
			scale = 0.3f;
		else
			scale = 0.5f;
	}
	else
		scale = 0.3f;
	
	DrainStamina( inputInfo.GetDamage() * scale );
	//

	m_vecTotalBulletForce += inputInfo.GetDamageForce();
	
	//BG2 - Tjoppen - pain sound
	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	//if ( GetParametersForSound( "BG2Player.pain", params, pModelName ) == false )
	if( GetParametersForSound( GetHitgroupPainSound( LastHitGroup(), GetTeamNumber() ), params, pModelName ) == false )
		return BaseClass::OnTakeDamage( inputInfo );

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );

	//BG2 - Tjoppen - print hit verification
	if ( inputInfo.GetAttacker()->IsPlayer() )
	{
		CBasePlayer *pVictim = this,
					*pAttacker = ToBasePlayer( inputInfo.GetAttacker() );
		Assert( pAttacker != NULL );

		CBaseBG2Weapon *pWeapon = dynamic_cast<CBaseBG2Weapon*>(pAttacker->GetActiveWeapon());
		int attackType = pWeapon ? pWeapon->m_iLastAttackType : 0;

		//use usermessage instead and let the client figure out what to print. saves precious bandwidth
		//send one to attacker and one to victim
		//the "Damage" usermessage is more detailed, but adds up all damage in the current frame. it is therefore
		//hard to determine who is the attacker since the victim can be hit multiple times
		
		CRecipientFilter recpfilter;
		recpfilter.AddRecipient( pAttacker );
		recpfilter.AddRecipient( pVictim );
		recpfilter.MakeReliable();

		UserMessageBegin( recpfilter, "HitVerif" );
			WRITE_BYTE( pAttacker->entindex() );				//attacker id
			WRITE_BYTE( pVictim->entindex() );					//victim id
			WRITE_BYTE( LastHitGroup() + (attackType << 4) );	//where?
			WRITE_SHORT( inputInfo.GetDamage() );				//damage
		MessageEnd();
	}
	//

	return BaseClass::OnTakeDamage( inputInfo );
}

void CHL2MP_Player::DeathSound( const CTakeDamageInfo &info )
{
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		 return;

	//BG2 - Tjoppen - unassigned/spectator don't make deathsound
	if( GetTeamNumber() <= TEAM_SPECTATOR )
		return;
	//

	char szStepSound[128];

	Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.Die", GetPlayerModelSoundPrefix() );

	//BG2 - Tjoppen - don't care about gender
	//const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( szStepSound, params, NULL/*pModelName*/ ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

CBaseEntity* CHL2MP_Player::HandleSpawnList( const CUtlVector<CBaseEntity *>& spawns )
{
	//BG2 - Tjoppen - the code below was broken out of EntSelectSpawnPoint()
	int offset = RandomInt(0, spawns.Count() - 1);	//start at a random offset into the list

	for ( int x = 0; x < spawns.Count(); x++ ) 
	{
		CSpawnPoint *pSpawn = static_cast<CSpawnPoint*>( spawns[(x + offset) % spawns.Count()] );
		if ( pSpawn && !pSpawn->m_bReserved && pSpawn->IsEnabled() && pSpawn->GetTeam() == GetTeamNumber() )
		{
			CBaseEntity *ent = NULL;
			bool IsTaken = false;
			//32 world units seems... safe.. -HairyPotter
			for ( CEntitySphereQuery sphere( pSpawn->GetAbsOrigin(), 32 ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
			{
				// Check to see if the ent is a player.
				if ( ent->IsPlayer() )
				{
					IsTaken = true;
				}
			}
			if ( IsTaken ) //Retry?
				continue;
		
			pSpawn->m_bReserved = true;
			return pSpawn;
		}
	}

	return NULL;
}

CBaseEntity* CHL2MP_Player::EntSelectSpawnPoint( void )
{
	if( GetTeamNumber() <= TEAM_SPECTATOR )
		return NULL;	//BG2 - Tjoppen - spectators/unassigned don't spawn..

	CBaseEntity *pSpot = NULL;
	CUtlVector<CBaseEntity *> spots;

	if ( GetTeamNumber() == TEAM_AMERICANS )
		spots = m_AmericanSpawns;
	else if ( GetTeamNumber() == TEAM_BRITISH )
		spots = m_BritishSpawns;
	else
		return NULL;

	spots.AddVectorToTail( m_MultiSpawns );
	pSpot = HandleSpawnList( spots );

	if ( pSpot )
		return pSpot;

	switch ( GetTeamNumber() )
	{
	case TEAM_BRITISH:
		Warning( "PutClientInServer: There are too few British spawns or they are too close together, or someone was obstructing them.\n");
		break;
	case TEAM_AMERICANS:
		Warning( "PutClientInServer: There are too few American spawns or they are too close together, or someone was obstructing them.\n");
		break;
	}

	return CBaseEntity::Instance( INDEXENT( 0 ) );

}

CON_COMMAND( timeleft, "Prints the time remaining in the round." )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = ((int)HL2MPRules()->GetMapRemainingTime() - gpGlobals->curtime);
    
	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "This game has no timelimit." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in map: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}


void CHL2MP_Player::Reset()
{	
	ResetDeathCount();
	ResetFragCount();
}

bool CHL2MP_Player::IsReady()
{
	return m_bReady;
}

void CHL2MP_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

void CHL2MP_Player::CheckChatText( char *p, int bufsize )
{
	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

	const char *pReadyCheck = p;

	HL2MPRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CHL2MP_Player::State_Transition( HL2MPPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CHL2MP_Player::State_Enter( HL2MPPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CHL2MP_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CHL2MP_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CHL2MPPlayerStateInfo *CHL2MP_Player::State_LookupInfo( HL2MPPlayerState state )
{
	// This table MUST match the 
	static CHL2MPPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CHL2MP_Player::State_Enter_ACTIVE, NULL, &CHL2MP_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CHL2MP_Player::State_Enter_OBSERVER_MODE,	NULL, &CHL2MP_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

bool CHL2MP_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	/*if ( m_bEnterObserver == true )
	{
		VPhysicsDestroyObject();
		return BaseClass::StartObserverMode( mode );
	}*/
	//BG2 - Tjoppen - reenable spectators
	return BaseClass::StartObserverMode( mode );
	//
	//Do nothing.
	return false;
}

void CHL2MP_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CHL2MP_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CHL2MP_Player::State_PreThink_OBSERVER_MODE()
{
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO ); 
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}


void CHL2MP_Player::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	
	// md 8/15/07 - They'll get set back to solid when they actually respawn. If we set them solid now and mp_forcerespawn
	// is false, then they'll be spectating but blocking live players from moving.
	// RemoveSolidFlags( FSOLID_NOT_SOLID );
	
	m_Local.m_iHideHUD = 0;
}


void CHL2MP_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2MP_Player::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return false;

	return true;
} 

void CHL2MP_Player::RemoveSelfFromFlags( void )
{
	//remove ourself from any overload list on all flags

	CFlag *pFlag = NULL;
	while( (pFlag = static_cast<CFlag*>( gEntList.FindEntityByClassname( pFlag, "flag" ) )) != NULL )
	{
		pFlag->m_vOverloadingPlayers.FindAndRemove( this );
		pFlag->m_vTriggerBritishPlayers.FindAndRemove( this );
		pFlag->m_vTriggerAmericanPlayers.FindAndRemove( this );
	}

	//Also have the player drop CTF Flags if they are holding one. -HairyPotter
	CtfFlag *ctfFlag = NULL;
	while( (ctfFlag = static_cast<CtfFlag*>( gEntList.FindEntityByClassname( ctfFlag, "ctf_flag" ) )) != NULL )
	{
		if ( ctfFlag->GetParent() && ctfFlag->GetParent() == this )
			ctfFlag->DropFlag(); 
	}
}

//BG2 - Tjoppen - HACKHACK: no more weapon_physcannon
void PlayerPickupObject( CBasePlayer *pPlayer, CBaseEntity *pObject ){}
bool PlayerHasMegaPhysCannon( void ){return false;}
void PhysCannonForceDrop( CBaseCombatWeapon *pActiveWeapon, CBaseEntity *pOnlyIfHoldingThis ){}
void PhysCannonBeginUpgrade( CBaseAnimating *pAnim ){}
bool PlayerPickupControllerIsHoldingEntity( CBaseEntity *pPickupControllerEntity, CBaseEntity *pHeldEntity ){return false;}
float PhysCannonGetHeldObjectMass( CBaseCombatWeapon *pActiveWeapon, IPhysicsObject *pHeldObject ){return 0;}
CBaseEntity *PhysCannonGetHeldEntity( CBaseCombatWeapon *pActiveWeapon ){return NULL;}
float PlayerPickupGetHeldObjectMass( CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject ){return 0;}
//
