//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "ammodef.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "physics_saverestore.h"
#include "datacache/imdlcache.h"


#if !defined( CLIENT_DLL )

// Game DLL Headers
#include "soundent.h"
#include "eventqueue.h"
#include "fmtstr.h"
#include "gameweaponmanager.h"

#ifdef HL2MP
	#include "hl2mp_gamerules.h"
	#include "hl2mp_player.h" //BG2 - HairyPotter
#endif

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// The minimum time a hud hint for a weapon should be on screen. If we switch away before
// this, then teh hud hint counter will be deremented so the hint will be shown again, as
// if it had never been seen. The total display time for a hud hint is specified in client
// script HudAnimations.txt (which I can't read here). 
#define MIN_HUDHINT_DISPLAY_TIME 7.0f

#define HIDEWEAPON_THINK_CONTEXT			"BaseCombatWeapon_HideThink"

//#define TWEAK_IRONSIGHTS //BG2 - Comment this out for final release.

//BG2 -Added for Iron Sights Testing. Credits to z33ky for the code. -HairyPotter
#if defined( TWEAK_IRONSIGHTS )
ConVar viewmodel_adjust_enabled( "viewmodel_adjust_enabled", "0", FCVAR_REPLICATED );
ConVar viewmodel_adjust_forward( "viewmodel_adjust_forward", "0", FCVAR_REPLICATED );
ConVar viewmodel_adjust_right( "viewmodel_adjust_right", "0", FCVAR_REPLICATED );
ConVar viewmodel_adjust_up( "viewmodel_adjust_up", "0", FCVAR_REPLICATED );
ConVar viewmodel_adjust_pitch( "viewmodel_adjust_pitch", "0", FCVAR_REPLICATED );
ConVar viewmodel_adjust_yaw( "viewmodel_adjust_yaw", "0", FCVAR_REPLICATED );
ConVar viewmodel_adjust_roll( "viewmodel_adjust_roll", "0", FCVAR_REPLICATED );
ConVar viewmodel_adjust_fov( "viewmodel_adjust_fov", "0", FCVAR_REPLICATED );
#endif

Vector CBaseCombatWeapon::GetIronsightPositionOffset( void ) const
{
#if defined( TWEAK_IRONSIGHTS )
	if( viewmodel_adjust_enabled.GetBool() ) //Out of the way, you!
		return Vector( viewmodel_adjust_forward.GetFloat(), viewmodel_adjust_right.GetFloat(), viewmodel_adjust_up.GetFloat() );
#endif
	return vecIronsightPosOffset; //Just do what the script says!
}
 
QAngle CBaseCombatWeapon::GetIronsightAngleOffset( void ) const
{
#if defined( TWEAK_IRONSIGHTS )
	if( viewmodel_adjust_enabled.GetBool() ) //Out of the way, you!
		return QAngle( viewmodel_adjust_pitch.GetFloat(), viewmodel_adjust_yaw.GetFloat(), viewmodel_adjust_roll.GetFloat() );
#endif
	return angIronsightAngOffset; //Just do what the script says!
}
 
float CBaseCombatWeapon::GetIronsightFOVOffset( void ) const
{
#if defined( TWEAK_IRONSIGHTS )
	if( viewmodel_adjust_enabled.GetBool() ) //Out of the way, you!
		return viewmodel_adjust_fov.GetFloat();
#endif
	return flIronsightFOVOffset; //Just do what the script says!
}
//

extern bool UTIL_ItemCanBeTouchedByPlayer( CBaseEntity *pItem, CBasePlayer *pPlayer );

CBaseCombatWeapon::CBaseCombatWeapon()
{
	// Constructor must call this
	// CONSTRUCT_PREDICTABLE( CBaseCombatWeapon );

	// Some default values.  There should be set in the particular weapon classes
	m_fMinRange1		= 65;
	m_fMinRange2		= 65;
	m_fMaxRange1		= 1024;
	m_fMaxRange2		= 1024;

	m_bReloadsSingly	= false;
	//BG2 - Tjoppen - default to no automatic reload
	m_bDontAutoreload	= false;

	//BG2 -Added for Iron Sights Testing. Credits to z33ky for the code. -HairyPotter
	m_bIsIronsighted = false;
	m_flIronsightedTime = 0.0f;
	//

	// Defaults to zero
	m_nViewModelIndex	= 0;

#if defined( CLIENT_DLL )
	m_iState = m_iOldState = WEAPON_NOT_CARRIED;
	m_iClip1 = -1;
	m_iClip2 = -1;
	m_iPrimaryAmmoType = -1;
	m_iSecondaryAmmoType = -1;
#endif

#if !defined( CLIENT_DLL )
	m_pConstraint = NULL;
	OnBaseCombatWeaponCreated( this );
#endif

	m_hWeaponFileInfo = GetInvalidWeaponInfoHandle();

#if defined( TF_DLL )
	UseClientSideAnimation();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CBaseCombatWeapon::~CBaseCombatWeapon( void )
{
#if !defined( CLIENT_DLL )
	//Remove our constraint, if we have one
	if ( m_pConstraint != NULL )
	{
		physenv->DestroyConstraint( m_pConstraint );
		m_pConstraint = NULL;
	}
	OnBaseCombatWeaponDestroyed( this );
#endif
}

void CBaseCombatWeapon::Activate( void )
{
	BaseClass::Activate();

#ifndef CLIENT_DLL
	if ( GetOwnerEntity() )
		return;

	if ( g_pGameRules->IsAllowedToSpawn( this ) == false )
	{
		UTIL_Remove( this );
		return;
	}
#endif

}
void CBaseCombatWeapon::GiveDefaultAmmo( void )
{
	// If I use clips, set my clips to the default
	if ( UsesClipsForAmmo1() )
	{
		m_iClip1 = GetDefaultClip1();
	}
	else
	{
		SetPrimaryAmmoCount( GetDefaultClip1() );
		m_iClip1 = WEAPON_NOCLIP;
	}
	if ( UsesClipsForAmmo2() )
	{
		m_iClip2 = GetDefaultClip2();
	}
	else
	{
		SetSecondaryAmmoCount( GetDefaultClip2() );
		m_iClip2 = WEAPON_NOCLIP;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set mode to world model and start falling to the ground
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BBOX );
	m_flNextEmptySoundTime = 0.0f;

	// Weapons won't show up in trace calls if they are being carried...
	RemoveEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );

	m_iState = WEAPON_NOT_CARRIED;
	// Assume 
	m_nViewModelIndex = 0;

	GiveDefaultAmmo();

	SetModel( GetWorldModel() );

#if !defined( CLIENT_DLL )

	FallInit();
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	m_takedamage = DAMAGE_EVENTS_ONLY;

	SetBlocksLOS( false );

	// Default to non-removeable, because we don't want the
	// game_weapon_manager entity to remove weapons that have
	// been hand-placed by level designers. We only want to remove
	// weapons that have been dropped by NPC's.
	SetRemoveable( false );
#endif

	// Bloat the box for player pickup
	CollisionProp()->UseTriggerBounds( true, 36 );

	// Use more efficient bbox culling on the client. Otherwise, it'll setup bones for most
	// characters even when they're not in the frustum.
	AddEffects( EF_BONEMERGE_FASTCULL );

	m_iReloadHudHintCount = 0;
	m_iAltFireHudHintCount = 0;
	m_flHudHintMinDisplayTime = 0;
}

//-----------------------------------------------------------------------------
// Purpose: get this game's encryption key for decoding weapon kv files
// Output : virtual const unsigned char
//-----------------------------------------------------------------------------
const unsigned char *CBaseCombatWeapon::GetEncryptionKey( void ) 
{ 
	return g_pGameRules->GetEncryptionKey(); 
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Precache( void )
{
#if defined( CLIENT_DLL )
	Assert( Q_strlen( GetClassname() ) > 0 );
	// Msg( "Client got %s\n", GetClassname() );
#endif
	m_iPrimaryAmmoType = m_iSecondaryAmmoType = -1;

	// Add this weapon to the weapon registry, and get our index into it
	// Get weapon data from script file
	if ( ReadWeaponDataFromFileForSlot( filesystem, GetClassname(), &m_hWeaponFileInfo, GetEncryptionKey() ) )
	{
		// Get the ammo indexes for the ammo's specified in the data file
		if ( GetWpnData().szAmmo1[0] )
		{
			m_iPrimaryAmmoType = GetAmmoDef()->Index( GetWpnData().szAmmo1 );
			if (m_iPrimaryAmmoType == -1)
			{
				Msg("ERROR: Weapon (%s) using undefined primary ammo type (%s)\n",GetClassname(), GetWpnData().szAmmo1);
			}
		}
		if ( GetWpnData().szAmmo2[0] )
		{
			m_iSecondaryAmmoType = GetAmmoDef()->Index( GetWpnData().szAmmo2 );
			if (m_iSecondaryAmmoType == -1)
			{
				Msg("ERROR: Weapon (%s) using undefined secondary ammo type (%s)\n",GetClassname(),GetWpnData().szAmmo2);
			}

		}
#if defined( CLIENT_DLL )
		gWR.LoadWeaponSprites( GetWeaponFileInfoHandle() );
#endif
		// Precache models (preload to avoid hitch)
		m_iViewModelIndex = 0;
		m_iWorldModelIndex = 0;
		if ( GetViewModel() && GetViewModel()[0] )
		{
			m_iViewModelIndex = CBaseEntity::PrecacheModel( GetViewModel() );
		}
		if ( GetWorldModel() && GetWorldModel()[0] )
		{
			m_iWorldModelIndex = CBaseEntity::PrecacheModel( GetWorldModel() );
		}

		// Precache sounds, too
		for ( int i = 0; i < NUM_SHOOT_SOUND_TYPES; ++i )
		{
			const char *shootsound = GetShootSound( i );
			if ( shootsound && shootsound[0] )
			{
				CBaseEntity::PrecacheScriptSound( shootsound );
			}
		}
	}
	else
	{
		// Couldn't read data file, remove myself
		Warning( "Error reading weapon data file for: %s\n", GetClassname() );
	//	Remove( );	//don't remove, this gets released soon!
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get my data in the file weapon info array
//-----------------------------------------------------------------------------
const FileWeaponInfo_t &CBaseCombatWeapon::GetWpnData( void ) const
{
	return *GetFileWeaponInfoFromHandle( m_hWeaponFileInfo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetViewModel( int /*viewmodelindex = 0 -- this is ignored in the base class here*/ ) const
{
	return GetWpnData().szViewModel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetWorldModel( void ) const
{
	return GetWpnData().szWorldModel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetAnimPrefix( void ) const
{
	return GetWpnData().szAnimationPrefix;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetPrintName( void ) const
{
	return GetWpnData().szPrintName;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetMaxClip1( void ) const
{
	return GetWpnData().iMaxClip1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetMaxClip2( void ) const
{
	return GetWpnData().iMaxClip2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetDefaultClip1( void ) const
{
	return GetWpnData().iDefaultClip1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetDefaultClip2( void ) const
{
	return GetWpnData().iDefaultClip2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesClipsForAmmo1( void ) const
{
	return ( GetMaxClip1() != WEAPON_NOCLIP );
}

bool CBaseCombatWeapon::IsMeleeWeapon() const
{
	return GetWpnData().m_bMeleeWeapon;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesClipsForAmmo2( void ) const
{
	return ( GetMaxClip2() != WEAPON_NOCLIP );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetWeight( void ) const
{
	return GetWpnData().iWeight;
}

//-----------------------------------------------------------------------------
// Purpose: Whether this weapon can be autoswitched to when the player runs out
//			of ammo in their current weapon or they pick this weapon up.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::AllowsAutoSwitchTo( void ) const
{
	return GetWpnData().bAutoSwitchTo;
}

//-----------------------------------------------------------------------------
// Purpose: Whether this weapon can be autoswitched away from when the player
//			runs out of ammo in this weapon or picks up another weapon or ammo.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::AllowsAutoSwitchFrom( void ) const
{
	return GetWpnData().bAutoSwitchFrom;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetWeaponFlags( void ) const
{
	return GetWpnData().iFlags;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetSlot( void ) const
{
	return GetWpnData().iSlot;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetPosition( void ) const
{
	return GetWpnData().iPosition;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetName( void ) const
{
	return GetWpnData().szClassName;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
//BG2 - Removing Hl2 Hud Stuff. -HairyPotter
/*CHudTexture const *CBaseCombatWeapon::GetSpriteActive( void ) const
{
	return GetWpnData().iconActive;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteInactive( void ) const
{
	return GetWpnData().iconInactive;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteAmmo( void ) const
{
	return GetWpnData().iconAmmo;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteAmmo2( void ) const
{
	return GetWpnData().iconAmmo2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteCrosshair( void ) const
{
	return GetWpnData().iconCrosshair;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteAutoaim( void ) const
{
	return GetWpnData().iconAutoaim;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteZoomedCrosshair( void ) const
{
	return GetWpnData().iconZoomedCrosshair;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudTexture const *CBaseCombatWeapon::GetSpriteZoomedAutoaim( void ) const
{
	return GetWpnData().iconZoomedAutoaim;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CBaseCombatWeapon::GetShootSound( int iIndex ) const
{
	return GetWpnData().aShootSounds[ iIndex ];
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetRumbleEffect() const
{
	//return GetWpnData().iRumbleEffect;
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseCombatCharacter	*CBaseCombatWeapon::GetOwner() const
{
	return ToBaseCombatCharacter( m_hOwner.Get() );
}	

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : BaseCombatCharacter - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetOwner( CBaseCombatCharacter *owner )
{
	m_hOwner = owner;
	
#ifndef CLIENT_DLL
	DispatchUpdateTransmitState();
#else
	UpdateVisibility();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Return false if this weapon won't let the player switch away from it
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsAllowedToSwitch( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon can be selected via the weapon selection
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::CanBeSelected( void )
{
	if ( !VisibleInWeaponSelection() )
		return false;

	return HasAmmo();
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon has some ammo
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasAmmo( void )
{
	// Weapons with no ammo types can always be selected
	if ( m_iPrimaryAmmoType == -1 && m_iSecondaryAmmoType == -1  )
		return true;
	if ( GetWeaponFlags() & ITEM_FLAG_SELECTONEMPTY )
		return true;

	CBasePlayer *player = ToBasePlayer( GetOwner() );
	if ( !player )
		return false;
	return ( m_iClip1 > 0 || player->GetAmmoCount( m_iPrimaryAmmoType ) || m_iClip2 > 0 || player->GetAmmoCount( m_iSecondaryAmmoType ) );
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon should be seen, and hence be selectable, in the weapon selection
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::VisibleInWeaponSelection( void )
{
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasWeaponIdleTimeElapsed( void )
{
	if ( gpGlobals->curtime > m_flTimeWeaponIdle )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : time - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetWeaponIdleTime( float time )
{
	m_flTimeWeaponIdle = time;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBaseCombatWeapon::GetWeaponIdleTime( void )
{
	return m_flTimeWeaponIdle;
}

//BG2 -Added for Iron Sights Testing. Credits to z33ky for the code. -HairyPotter
bool CBaseCombatWeapon::IsIronsighted( void )
{
	return ( m_bIsIronsighted  );
}

void CBaseCombatWeapon::EnableIronsights( void )
{
	if( m_bIsIronsighted )
		return;
 
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
 
	if( !pOwner )
		return;
 
#ifndef CLIENT_DLL
	if( !pOwner->SetFOV( this, pOwner->GetDefaultFOV()+GetIronsightFOVOffset(), 1.0f, 0.3f ) ) //modify these values to adjust how fast the fov is applied
		return;

	if ( !m_iClip1 )
		SendWeaponAnim( ACT_FORCE_STABLE_EMPTY ); //Dummy animation that will cancel out the idle anim.
	else
		SendWeaponAnim( ACT_FORCE_STABLE ); //Dummy animation that will cancel out the idle anim.

	m_bIsIronsighted = true;
	m_flIronsightedTime = gpGlobals->curtime;
#endif

	//delay both attacks by 450 ms, but make sure we don't roll back the attack times
	m_flNextPrimaryAttack   = max(m_flNextPrimaryAttack,   gpGlobals->curtime + 0.45f);
	m_flNextSecondaryAttack = max(m_flNextSecondaryAttack, gpGlobals->curtime + 0.45f);
}
 
void CBaseCombatWeapon::DisableIronsights( void )
{
	if( !m_bIsIronsighted )
		return;
 
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
 
	if( !pOwner )
		return;
 
#ifndef CLIENT_DLL
	if( !pOwner->SetFOV( this, 0, 0.4f, 0.1f ) ) //modify these values to adjust how fast the fov is applied
		return;

	m_bIsIronsighted = false;
	m_flIronsightedTime = gpGlobals->curtime;
#endif

	//delay both attacks by 450 ms, but make sure we don't roll back the attack times
	m_flNextPrimaryAttack   = max(m_flNextPrimaryAttack,   gpGlobals->curtime + 0.45f);
	m_flNextSecondaryAttack = max(m_flNextSecondaryAttack, gpGlobals->curtime + 0.45f);
}

//-----------------------------------------------------------------------------
// Purpose: Drop/throw the weapon with the given velocity.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Drop( const Vector &vecVelocity )
{
#if !defined( CLIENT_DLL )

	// Once somebody drops a gun, it's fair game for removal when/if
	// a game_weapon_manager does a cleanup on surplus weapons in the
	// world.
	SetRemoveable( true );
	WeaponManager_AmmoMod( this );

	//If it was dropped then there's no need to respawn it.
	AddSpawnFlags( SF_NORESPAWN );

	StopAnimation();
	StopFollowingEntity( );
	SetMoveType( MOVETYPE_FLYGRAVITY );
	// clear follow stuff, setup for collision
	SetGravity(1.0);
	m_iState = WEAPON_NOT_CARRIED;
	RemoveEffects( EF_NODRAW );
	FallInit();
	SetGroundEntity( NULL );
	SetThink( &CBaseCombatWeapon::SetPickupTouch );
	SetTouch(NULL);

	if( hl2_episodic.GetBool() )
	{
		RemoveSpawnFlags( SF_WEAPON_NO_PLAYER_PICKUP );
	}

	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj != NULL )
	{
		AngularImpulse	angImp( 200, 200, 200 );
		pObj->AddVelocity( &vecVelocity, &angImp );
	}
	else
	{
		SetAbsVelocity( vecVelocity );
	}

	CBaseEntity *pOwner = GetOwnerEntity();

	SetNextThink( gpGlobals->curtime + 1.0f );
	SetOwnerEntity( NULL );
	SetOwner( NULL );

	// If we're not allowing to spawn due to the gamerules,
	// remove myself when I'm dropped by an NPC.
	if ( pOwner && pOwner->IsNPC() )
	{
		if ( g_pGameRules->IsAllowedToSpawn( this ) == false )
		{
			UTIL_Remove( this );
			return;
		}
	}
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPicker - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::OnPickedUp( CBaseCombatCharacter *pNewOwner )
{
#if !defined( CLIENT_DLL )
	RemoveEffects( EF_ITEM_BLINK );

	if( pNewOwner->IsPlayer() )
	{
		m_OnPlayerPickup.FireOutput(pNewOwner, this);

		// Play the pickup sound for 1st-person observers
		CRecipientFilter filter;
		for ( int i=1; i <= gpGlobals->maxClients; ++i )
		{
			CBasePlayer *player = UTIL_PlayerByIndex(i);
			if ( player && !player->IsAlive() && player->GetObserverMode() == OBS_MODE_IN_EYE )
			{
				filter.AddRecipient( player );
			}
		}
		if ( filter.GetRecipientCount() )
		{
			CBaseEntity::EmitSound( filter, pNewOwner->entindex(), "Player.PickupWeapon" );
		}

		// Robin: We don't want to delete weapons the player has picked up, so 
		// clear the name of the weapon. This prevents wildcards that are meant 
		// to find NPCs finding weapons dropped by the NPCs as well.
		SetName( NULL_STRING );
	}
	else
	{
		m_OnNPCPickup.FireOutput(pNewOwner, this);
	}

#ifdef HL2MP
	HL2MPRules()->RemoveLevelDesignerPlacedObject( this );
#endif

	// Someone picked me up, so make it so that I can't be removed.
	SetRemoveable( false );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecTracerSrc - 
//			&tr - 
//			iTracerType - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	CBaseEntity *pOwner = GetOwner();

	if ( pOwner == NULL )
	{
		BaseClass::MakeTracer( vecTracerSrc, tr, iTracerType );
		return;
	}

	const char *pszTracerName = GetTracerType();

	Vector vNewSrc = vecTracerSrc;
	int iEntIndex = pOwner->entindex();

	if ( g_pGameRules->IsMultiplayer() )
	{
		iEntIndex = entindex();
	}

	int iAttachment = GetTracerAttachment();

	switch ( iTracerType )
	{
	case TRACER_LINE:
		UTIL_Tracer( vNewSrc, tr.endpos, iEntIndex, iAttachment, 0.0f, true, pszTracerName );
		break;

	case TRACER_LINE_AND_WHIZ:
		UTIL_Tracer( vNewSrc, tr.endpos, iEntIndex, iAttachment, 0.0f, true, pszTracerName );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Default Touch function for player picking up a weapon (not AI)
// Input  : pOther - the entity that touched me
// Output :
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::DefaultTouch( CBaseEntity *pOther )
{
//BG2 - Tjoppen - no weapon pickup
/*#if !defined( CLIENT_DLL )
	// Can't pick up dissolving weapons
	if ( IsDissolving() )
		return;

	// if it's not a player, ignore
	CBasePlayer *pPlayer = ToBasePlayer(pOther);
	if ( !pPlayer )
				return;
		
	if (pPlayer->BumpWeapon(this))
	{
		OnPickedUp( pPlayer );
	}
#endif*/
//
}

//---------------------------------------------------------
// It's OK for base classes to override this completely 
// without calling up. (sjb)
//---------------------------------------------------------
bool CBaseCombatWeapon::ShouldDisplayAltFireHUDHint()
{
	if( m_iAltFireHudHintCount >= WEAPON_RELOAD_HUD_HINT_COUNT )
		return false;

	if( UsesSecondaryAmmo() && HasSecondaryAmmo() )
	{
		return true;
	}

	if( !UsesSecondaryAmmo() && HasPrimaryAmmo() )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::DisplayAltFireHudHint()
{
#if !defined( CLIENT_DLL )
	CFmtStr hint;
	hint.sprintf( "#valve_hint_alt_%s", GetClassname() );
	UTIL_HudHintText( GetOwner(), hint.Access() );
	m_iAltFireHudHintCount++;
	m_bAltFireHudHintDisplayed = true;
	m_flHudHintMinDisplayTime = gpGlobals->curtime + MIN_HUDHINT_DISPLAY_TIME;
#endif//CLIENT_DLL
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::RescindAltFireHudHint()
{
#if !defined( CLIENT_DLL )
	Assert(m_bAltFireHudHintDisplayed);
	
	UTIL_HudHintText( GetOwner(), "" );
	--m_iAltFireHudHintCount;
	m_bAltFireHudHintDisplayed = false;
#endif//CLIENT_DLL
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::ShouldDisplayReloadHUDHint()
{
	if( m_iReloadHudHintCount >= WEAPON_RELOAD_HUD_HINT_COUNT )
		return false;

	CBaseCombatCharacter *pOwner = GetOwner();

	if( pOwner != NULL && pOwner->IsPlayer() && UsesClipsForAmmo1() && m_iClip1 < (GetMaxClip1() / 2) )
	{
		// I'm owned by a player, I use clips, I have less then half a clip loaded. Now, does the player have more ammo?
		if ( pOwner )
		{
			if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) > 0 ) 
				return true;
		}
	}

	return false;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::DisplayReloadHudHint()
{
#if !defined( CLIENT_DLL )
	UTIL_HudHintText( GetOwner(), "valve_hint_reload" );
	m_iReloadHudHintCount++;
	m_bReloadHudHintDisplayed = true;
	m_flHudHintMinDisplayTime = gpGlobals->curtime + MIN_HUDHINT_DISPLAY_TIME;
#endif//CLIENT_DLL
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::RescindReloadHudHint()
{
#if !defined( CLIENT_DLL )
	Assert(m_bReloadHudHintDisplayed);

	UTIL_HudHintText( GetOwner(), "" );
	--m_iReloadHudHintCount;
	m_bReloadHudHintDisplayed = false;
#endif//CLIENT_DLL
}


void CBaseCombatWeapon::SetPickupTouch( void )
{
#if !defined( CLIENT_DLL )
	SetTouch(&CBaseCombatWeapon::DefaultTouch);

	if ( gpGlobals->maxClients > 1 )
	{
		if ( GetSpawnFlags() & SF_NORESPAWN )
		{
			SetThink( &CBaseEntity::SUB_Remove );
			SetNextThink( gpGlobals->curtime + 30.0f );
		}
	}

#endif
}


//-----------------------------------------------------------------------------
// Purpose: Become a child of the owner (MOVETYPE_FOLLOW)
//			disables collisions, touch functions, thinking
// Input  : *pOwner - new owner/operator
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Equip( CBaseCombatCharacter *pOwner )
{
	// Attach the weapon to an owner
	SetAbsVelocity( vec3_origin );
	RemoveSolidFlags( FSOLID_TRIGGER );
	FollowEntity( pOwner );
	SetOwner( pOwner );
	SetOwnerEntity( pOwner );

	// Break any constraint I might have to the world.
	RemoveEffects( EF_ITEM_BLINK );

#if !defined( CLIENT_DLL )
	if ( m_pConstraint != NULL )
	{
		RemoveSpawnFlags( SF_WEAPON_START_CONSTRAINED );
		physenv->DestroyConstraint( m_pConstraint );
		m_pConstraint = NULL;
	}
#endif


	m_flNextPrimaryAttack		= gpGlobals->curtime;
	m_flNextSecondaryAttack		= gpGlobals->curtime;
	SetTouch( NULL );
	SetThink( NULL );
#if !defined( CLIENT_DLL )
	VPhysicsDestroyObject();
#endif

	if ( pOwner->IsPlayer() )
	{
		SetModel( GetViewModel() );
	}
	else
	{
		// Make the weapon ready as soon as any NPC picks it up.
		m_flNextPrimaryAttack = gpGlobals->curtime;
		m_flNextSecondaryAttack = gpGlobals->curtime;
		SetModel( GetWorldModel() );
	}
}

void CBaseCombatWeapon::SetActivity( Activity act, float duration ) 
{ 
	//Adrian: Oh man...
#if !defined( CLIENT_DLL ) && (defined( HL2MP ) || defined( PORTAL ))
	SetModel( GetWorldModel() );
#endif
	
	int sequence = SelectWeightedSequence( act ); 
	
	// FORCE IDLE on sequences we don't have (which should be many)
	if ( sequence == ACTIVITY_NOT_AVAILABLE )
		sequence = SelectWeightedSequence( ACT_VM_IDLE );

	//Adrian: Oh man again...
#if !defined( CLIENT_DLL ) && (defined( HL2MP ) || defined( PORTAL ))
	SetModel( GetViewModel() );
#endif

	if ( sequence != ACTIVITY_NOT_AVAILABLE )
	{
		SetSequence( sequence );
		SetActivity( act ); 
		SetCycle( 0 );
		ResetSequenceInfo( );

		if ( duration > 0 )
		{
			// FIXME: does this even make sense in non-shoot animations?
			m_flPlaybackRate = SequenceDuration( sequence ) / duration;
			m_flPlaybackRate = min( m_flPlaybackRate, 12.0);  // FIXME; magic number!, network encoding range
		}
		else
		{
			m_flPlaybackRate = 1.0;
		}
	}
}

//====================================================================================
// WEAPON CLIENT HANDLING
//====================================================================================
int CBaseCombatWeapon::UpdateClientData( CBasePlayer *pPlayer )
{
	int iNewState = WEAPON_IS_CARRIED_BY_PLAYER;

	if ( pPlayer->GetActiveWeapon() == this )
	{
		if ( pPlayer->m_fOnTarget ) 
		{
			iNewState = WEAPON_IS_ONTARGET;
		}
		else
		{
			iNewState = WEAPON_IS_ACTIVE;
		}
	}
	else
	{
		iNewState = WEAPON_IS_CARRIED_BY_PLAYER;
	}

	if ( m_iState != iNewState )
	{
		m_iState = iNewState;
	}
	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetViewModelIndex( int index )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );
	m_nViewModelIndex = index;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iActivity - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SendViewModelAnim( int nSequence )
{
#if defined( CLIENT_DLL )
	if ( !IsPredicted() )
		return;
#endif
	
	if ( nSequence < 0 )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;
	
	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
	
	if ( vm == NULL )
		return;

	SetViewModel();
	Assert( vm->ViewModelIndex() == m_nViewModelIndex );
	vm->SendViewModelMatchingSequence( nSequence );
}

float CBaseCombatWeapon::GetViewModelSequenceDuration()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
	{
		Assert( false );
		return 0;
	}
	
	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
	if ( vm == NULL )
	{
		Assert( false );
		return 0;
	}

	SetViewModel();
	Assert( vm->ViewModelIndex() == m_nViewModelIndex );
	return vm->SequenceDuration();
}

bool CBaseCombatWeapon::IsViewModelSequenceFinished( void )
{
	// These are not valid activities and always complete immediately
	if ( GetActivity() == ACT_RESET || GetActivity() == ACT_INVALID )
		return true;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
	{
		Assert( false );
		return false;
	}
	
	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
	if ( vm == NULL )
	{
		Assert( false );
		return false;
	}

	return vm->IsSequenceFinished();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetViewModel()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
		return;
	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
	int WeaponSkin = pOwner->GetActiveWeapon()->m_nSkin; //BG2 - HACKHACK Sets the weapon skin for viewmodel -HairyPotter

	if ( vm == NULL )
		return;
	Assert( vm->ViewModelIndex() == m_nViewModelIndex );
	vm->SetWeaponModel( GetViewModel( m_nViewModelIndex ), this );
	vm->m_nSkin = WeaponSkin; //BG2 - HACKHACK Sets the weapon skin for viewmodel - HairyPotter
}

//-----------------------------------------------------------------------------
// Purpose: Set the desired activity for the weapon and its viewmodel counterpart
// Input  : iActivity - activity to play
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::SendWeaponAnim( int iActivity )
{
	//For now, just set the ideal activity and be done with it
	return SetIdealActivity( (Activity) iActivity );
}

//====================================================================================
// WEAPON SELECTION
//====================================================================================

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasAnyAmmo( void )
{
	// If I don't use ammo of any kind, I can always fire
	if ( !UsesPrimaryAmmo() && !UsesSecondaryAmmo() )
		return true;

	// Otherwise, I need ammo of either type
	return ( HasPrimaryAmmo() || HasSecondaryAmmo() );
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasPrimaryAmmo( void )
{
	// If I use a clip, and have some ammo in it, then I have ammo
	if ( UsesClipsForAmmo1() )
	{
		if ( m_iClip1 > 0 )
			return true;
	}

	// Otherwise, I have ammo if I have some in my ammo counts
	CBaseCombatCharacter		*pOwner = GetOwner();
	if ( pOwner )
	{
		if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) > 0 ) 
			return true;
	}
	else
	{
		// No owner, so return how much primary ammo I have along with me.
		if( GetPrimaryAmmoCount() > 0 )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::HasSecondaryAmmo( void )
{
	// If I use a clip, and have some ammo in it, then I have ammo
	if ( UsesClipsForAmmo2() )
	{
		if ( m_iClip2 > 0 )
			return true;
	}

	// Otherwise, I have ammo if I have some in my ammo counts
	CBaseCombatCharacter		*pOwner = GetOwner();
	if ( pOwner )
	{
		if ( pOwner->GetAmmoCount( m_iSecondaryAmmoType ) > 0 ) 
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the weapon actually uses primary ammo
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesPrimaryAmmo( void )
{
	if ( m_iPrimaryAmmoType < 0 )
		return false;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the weapon actually uses secondary ammo
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::UsesSecondaryAmmo( void )
{
	if ( m_iSecondaryAmmoType < 0 )
		return false;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Show/hide weapon and corresponding view model if any
// Input  : visible - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::SetWeaponVisible( bool visible )
{
	CBaseViewModel *vm = NULL;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		vm = pOwner->GetViewModel( m_nViewModelIndex );
	}

	if ( visible )
	{
		RemoveEffects( EF_NODRAW );
		if ( vm )
		{
			vm->RemoveEffects( EF_NODRAW );
		}
	}
	else
	{
		AddEffects( EF_NODRAW );
		if ( vm )
		{
			vm->AddEffects( EF_NODRAW );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsWeaponVisible( void )
{
	CBaseViewModel *vm = NULL;
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		vm = pOwner->GetViewModel( m_nViewModelIndex );
		if ( vm )
			return ( !vm->IsEffectActive(EF_NODRAW) );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: If the current weapon has more ammo, reload it. Otherwise, switch 
//			to the next best weapon we've got. Returns true if it took any action.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::ReloadOrSwitchWeapons( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	Assert( pOwner );

	m_bFireOnEmpty = false;

	// If we don't have any ammo, switch to the next best weapon
	if ( !HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime && m_flNextSecondaryAttack < gpGlobals->curtime )
	{
		// weapon isn't useable, switch.
		if ( ( (GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) == false ) && ( g_pGameRules->SwitchToNextBestWeapon( pOwner, this ) ) )
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
			return true;
		}
	}
	else
	{
		// Weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		//BG2 - Tjoppen - don't automatically reload
		/*if ( UsesClipsForAmmo1() && 
			 (m_iClip1 == 0) && 
			 (GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) == false && 
			 m_flNextPrimaryAttack < gpGlobals->curtime && 
			 m_flNextSecondaryAttack < gpGlobals->curtime )*/
		if ( UsesClipsForAmmo1() && 
			 (m_iClip1 == 0) && 
			 (GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) == false && 
			 m_flNextPrimaryAttack < gpGlobals->curtime && 
			 m_flNextSecondaryAttack < gpGlobals->curtime &&
			 !m_bDontAutoreload )
		{
			// if we're successfully reloading, we're done
			if ( Reload() )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *szViewModel - 
//			*szWeaponModel - 
//			iActivity - 
//			*szAnimExt - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::DefaultDeploy( char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt )
{
	// Msg( "deploy %s at %f\n", GetClassname(), gpGlobals->curtime );

	// Weapons that don't autoswitch away when they run out of ammo 
	// can still be deployed when they have no ammo.
	if ( !HasAnyAmmo() && AllowsAutoSwitchFrom() )
		return false;

	//BG2 - Tjoppen - make sure guns aren't deployed with ironsights enabled and that m_bInReload is false
	//                the point of this is to make sure players never spawn unable to use the sights
	DisableIronsights();
	m_bInReload = false;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		// Dead men deploy no weapons
		if ( pOwner->IsAlive() == false )
			return false;

		pOwner->SetAnimationExtension( szAnimExt );

		SetViewModel();
		SendWeaponAnim( iActivity );

		pOwner->SetNextAttack( gpGlobals->curtime + SequenceDuration() );
	}

	// Can't shoot again until we've finished deploying
	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();
	m_flHudHintMinDisplayTime = 0;

	m_bAltFireHudHintDisplayed = false;
	m_bReloadHudHintDisplayed = false;
	m_flHudHintPollTime = gpGlobals->curtime + 5.0f;
	
	SetWeaponVisible( true );

/*

This code is disabled for now, because moving through the weapons in the carousel 
selects and deploys each weapon as you pass it. (sjb)

*/

	SetContextThink( NULL, 0, HIDEWEAPON_THINK_CONTEXT );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Deploy( )
{
	MDLCACHE_CRITICAL_SECTION();

#if defined( TWEAK_IRONSIGHTS )
	//BG2 - This is here to help test the viewmodel settings for Ironsights. -HairyPotter
	viewmodel_adjust_forward.SetValue( vecIronsightPosOffset.x ); //Forward
	viewmodel_adjust_right.SetValue( vecIronsightPosOffset.y ); //Right
	viewmodel_adjust_up.SetValue( vecIronsightPosOffset.z ); //Up
	viewmodel_adjust_pitch.SetValue( angIronsightAngOffset[PITCH] );
	viewmodel_adjust_yaw.SetValue( angIronsightAngOffset[YAW] );
	viewmodel_adjust_roll.SetValue( angIronsightAngOffset[ROLL] );
	viewmodel_adjust_fov.SetValue( flIronsightFOVOffset );
	//
#endif

	return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), GetDrawActivity(), (char*)GetAnimPrefix() );
}

Activity CBaseCombatWeapon::GetDrawActivity( void )
{
	return ACT_VM_DRAW;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Holster( CBaseCombatWeapon *pSwitchingTo )
{ 
	//BG2 -Added for Iron Sights Testing. Credits to z33ky for the code. -HairyPotter
	DisableIronsights();
	//

	CBaseCombatCharacter *pOwner = GetOwner();

	//BG2 - Tjoppen - m_bCantAbortReload
	if( m_bCantAbortReload && m_bInReload )
		return false;
	else if ( pOwner && m_bInReload )
		pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType ); //Let's just remove that shot from our ammo box. -HairyPotter
	//

	MDLCACHE_CRITICAL_SECTION();

	// cancel any reload in progress.
	m_bInReload = false; 

	// kill any think functions
	SetThink(NULL);

	// Send holster animation
	SendWeaponAnim( ACT_VM_HOLSTER );

	// Some weapon's don't have holster anims yet, so detect that
	float flSequenceDuration = 0;
	if ( GetActivity() == ACT_VM_HOLSTER )
	{
		flSequenceDuration = SequenceDuration();
	}

#ifndef CLIENT_DLL
	//BG2 - Play the player's holster animation - HairyPotter
	if ( pOwner )
	{
		( ( CBasePlayer * )pOwner)->SetAnimation( PLAYER_HOLSTER );
	}
#endif
	
	if (pOwner)
	{
		pOwner->SetNextAttack( gpGlobals->curtime + flSequenceDuration );
	}

	// If we don't have a holster anim, hide immediately to avoid timing issues
	if ( !flSequenceDuration )
	{
		SetWeaponVisible( false );
	}
	else
	{
		// Hide the weapon when the holster animation's finished
		SetContextThink( &CBaseCombatWeapon::HideThink, gpGlobals->curtime + flSequenceDuration, HIDEWEAPON_THINK_CONTEXT );
	}

	// if we were displaying a hud hint, squelch it.
	if (m_flHudHintMinDisplayTime && gpGlobals->curtime < m_flHudHintMinDisplayTime)
	{
		if( m_bAltFireHudHintDisplayed )
			RescindAltFireHudHint();

		if( m_bReloadHudHintDisplayed )
			RescindReloadHudHint();
	}

	return true;
}

#ifdef CLIENT_DLL

	void CBaseCombatWeapon::BoneMergeFastCullBloat( Vector &localMins, Vector &localMaxs, const Vector &thisEntityMins, const Vector &thisEntityMaxs ) const
	{
		// The default behavior pushes it out by BONEMERGE_FASTCULL_BBOX_EXPAND in all directions, but we can do better
		// since we know the weapon will never point behind him.

		localMaxs.x += 20;	// Leaves some space in front for long weapons.
		
		localMins.y -= 20;	// Fatten it to his left and right since he can rotate that way.
		localMaxs.y += 20;	

		localMaxs.z += 15;	// Leave some space at the top.
	}

#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::InputHideWeapon( inputdata_t &inputdata )
{
	// Only hide if we're still the active weapon. If we're not the active weapon
	if ( GetOwner() && GetOwner()->GetActiveWeapon() == this )
	{
		SetWeaponVisible( false );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::HideThink( void )
{
	// Only hide if we're still the active weapon. If we're not the active weapon
	if ( GetOwner() && GetOwner()->GetActiveWeapon() == this )
	{
		SetWeaponVisible( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::ItemPreFrame( void )
{
	MaintainIdealActivity();

#ifndef CLIENT_DLL
#ifndef HL2_EPISODIC
	if ( IsX360() )
#endif
	{
		// If we haven't displayed the hint enough times yet, it's time to try to 
		// display the hint, and the player is not standing still, try to show a hud hint.
		// If the player IS standing still, assume they could change away from this weapon at
		// any second.
		if( (!m_bAltFireHudHintDisplayed || !m_bReloadHudHintDisplayed) && gpGlobals->curtime > m_flHudHintMinDisplayTime && gpGlobals->curtime > m_flHudHintPollTime && GetOwner() && GetOwner()->IsPlayer() )
		{
			CBasePlayer *pPlayer = (CBasePlayer*)(GetOwner());

			if( pPlayer && pPlayer->GetStickDist() > 0.0f )
			{
				// If the player is moving, they're unlikely to switch away from the current weapon
				// the moment this weapon displays its HUD hint.
				if( ShouldDisplayReloadHUDHint() )
				{
					DisplayReloadHudHint();
				}
				else if( ShouldDisplayAltFireHUDHint() )
				{
					DisplayAltFireHudHint();
				}
			}
			else
			{
				m_flHudHintPollTime = gpGlobals->curtime + 2.0f;
			}
		}
	}
#endif
}

//====================================================================================
// WEAPON BEHAVIOUR
//====================================================================================
void CBaseCombatWeapon::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
		return;

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = ( pOwner->m_nButtons & IN_ATTACK ) ? ( m_fFireDuration + gpGlobals->frametime ) : 0.0f;

	if ( UsesClipsForAmmo1() )
	{
		CheckReload();
	}

	bool bFired = false;

	// Secondary attack has priority
	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		if (UsesSecondaryAmmo() && pOwner->GetAmmoCount(m_iSecondaryAmmoType)<=0 )
		{
			if (m_flNextEmptySoundTime < gpGlobals->curtime)
			{
				WeaponSound(EMPTY);
				m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
			}
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// FIXME: This isn't necessarily true if the weapon doesn't have a secondary fire!
			// For instance, the crossbow doesn't have a 'real' secondary fire, but it still 
			// stops the crossbow from firing on the 360 if the player chooses to hold down their
			// zoom button. (sjb) Orange Box 7/25/2007
#if !defined(CLIENT_DLL)
			if( !IsX360() || !ClassMatches("weapon_crossbow") )
#endif
			{
				bFired = true;
			}

			SecondaryAttack();

			// Secondary ammo doesn't have a reload animation
			if ( UsesClipsForAmmo2() )
			{
				// reload clip2 if empty
				if (m_iClip2 < 1)
				{
					pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );
					m_iClip2 = m_iClip2 + 1;
				}
			}
		}
	}
	
	if ( !bFired && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if ( !IsMeleeWeapon() &&  
			//BG2 - Tjoppen - IsMeleeWeapon doesn't work for some reason, so assume melee doesn't use ammo
			m_iPrimaryAmmoType > -1 &&
			// 
			(( UsesClipsForAmmo1() && m_iClip1 <= 0) || ( !UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType)<=0 )) )
		{
			HandleFireOnEmpty();
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
			//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
			//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
			//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
			//			first shot.  Right now that's too much of an architecture change -- jdw
			
			// If the firing button was just pressed, or the alt-fire just released, reset the firing time
			if ( ( pOwner->m_afButtonPressed & IN_ATTACK ) || ( pOwner->m_afButtonReleased & IN_ATTACK2 ) )
			{
				 m_flNextPrimaryAttack = gpGlobals->curtime;
			}

			PrimaryAttack();
		}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if ( pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload ) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
	}

	//only toggling ironsights only if we have them, we're not in a reload and we're not too soon after the last attack
	if ( m_bWeaponHasSights && !m_bInReload && gpGlobals->curtime > m_flNextPrimaryAttack && gpGlobals->curtime > m_flNextSecondaryAttack )
	{
		if ( pOwner->m_nButtons & IN_ZOOM )
		{
			EnableIronsights();
		}
		else
		{
			DisableIronsights();
		}
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (pOwner->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) )
		{
			WeaponIdle();
		}
	}
}

void CBaseCombatWeapon::HandleFireOnEmpty()
{
	// If we're already firing on empty, reload if we can
	if ( m_bFireOnEmpty )
	{
		ReloadOrSwitchWeapons();
		m_fFireDuration = 0.0f;
	}
	else
	{
		if (m_flNextEmptySoundTime < gpGlobals->curtime)
		{
			WeaponSound(EMPTY);
			m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
		}
		m_bFireOnEmpty = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called each frame by the player PostThink, if the player's not ready to attack yet
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::ItemBusyFrame( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for getting bullet type
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::GetBulletType( void )
{
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for getting spread
// Input  :
// Output :
//-----------------------------------------------------------------------------
const Vector& CBaseCombatWeapon::GetBulletSpread( void )
{
	static Vector cone = VECTOR_CONE_15DEGREES;
	return cone;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CBaseCombatWeapon::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t defaultWeaponProficiencyTable[] =
	{
		{ 1.0, 1.0	},
		{ 1.0, 1.0	},
		{ 1.0, 1.0	},
		{ 1.0, 1.0	},
		{ 1.0, 1.0	},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(defaultWeaponProficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);
	return defaultWeaponProficiencyTable;
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for getting firerate
// Input  :
// Output :
//-----------------------------------------------------------------------------
float CBaseCombatWeapon::GetFireRate( void )
{
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Base class default for playing shoot sound
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::WeaponSound( WeaponSound_t sound_type, float soundtime /* = 0.0f */ )
{
	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetShootSound( sound_type );
	if ( !shootsound || !shootsound[0] )
		return;

	CSoundParameters params;
	
	if ( !GetParametersForSound( shootsound, params, NULL ) )
		return;

	if ( params.play_to_owner_only )
	{
		// Am I only to play to my owner?
		if ( GetOwner() && GetOwner()->IsPlayer() )
		{
			CSingleUserRecipientFilter filter( ToBasePlayer( GetOwner() ) );
			if ( IsPredicted() )
			{
				filter.UsePredictionRules();
			}
			EmitSound( filter, GetOwner()->entindex(), shootsound, NULL, soundtime );
		}
	}
	else
	{
		// Play weapon sound from the owner
		if ( GetOwner() )
		{
			CPASAttenuationFilter filter( GetOwner(), params.soundlevel );
			if ( IsPredicted() )
			{
				filter.UsePredictionRules();
			}
			EmitSound( filter, GetOwner()->entindex(), shootsound, NULL, soundtime ); 

#if !defined( CLIENT_DLL )
			if( sound_type == EMPTY )
			{
				CSoundEnt::InsertSound( SOUND_COMBAT, GetOwner()->GetAbsOrigin(), SOUNDENT_VOLUME_EMPTY, 0.2, GetOwner() );
			}
#endif
		}
		// If no owner play from the weapon (this is used for thrown items)
		else
		{
			CPASAttenuationFilter filter( this, params.soundlevel );
			if ( IsPredicted() )
			{
				filter.UsePredictionRules();
			}
			EmitSound( filter, entindex(), shootsound, NULL, soundtime ); 
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop a sound played by this weapon.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::StopWeaponSound( WeaponSound_t sound_type )
{
	//if ( IsPredicted() )
	//	return;

	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetShootSound( sound_type );
	if ( !shootsound || !shootsound[0] )
		return;
	
	CSoundParameters params;
	if ( !GetParametersForSound( shootsound, params, NULL ) )
		return;

	// Am I only to play to my owner?
	if ( params.play_to_owner_only )
	{
		if ( GetOwner() )
		{
			StopSound( GetOwner()->entindex(), shootsound );
		}
	}
	else
	{
		// Play weapon sound from the owner
		if ( GetOwner() )
		{
			StopSound( GetOwner()->entindex(), shootsound );
		}
		// If no owner play from the weapon (this is used for thrown items)
		else
		{
			StopSound( entindex(), shootsound );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::DefaultReload( int iClipSize1, int iClipSize2, int iActivity )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	// If I don't have any spare ammo, I can't reload
	if ( pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
		return false;

	//BG2 - Tjoppen - don't reload under water
	if( pOwner->GetWaterLevel() > 2 )
		return false;
	//

	bool bReload = false;

	// If you don't have clips, then don't try to reload them.
	if ( UsesClipsForAmmo1() )
	{
		// need to reload primary clip?
		int primary	= min(iClipSize1 - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));
		if ( primary != 0 )
		{
			bReload = true;
		}
	}

	if ( UsesClipsForAmmo2() )
	{
		// need to reload secondary clip?
		int secondary = min(iClipSize2 - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
		if ( secondary != 0 )
		{
			bReload = true;
		}
	}

	if ( !bReload )
		return false;

	//BG2 - Tjoppen - server side reload sound
//#ifdef CLIENT_DLL
	// Play reload
	WeaponSound( RELOAD );
//#endif
	SendWeaponAnim( iActivity );


	// Play the player's reload animation
	if ( pOwner->IsPlayer() )
	{
		( ( CBasePlayer * )pOwner)->SetAnimation( PLAYER_RELOAD );
	}

	MDLCACHE_CRITICAL_SECTION();
	float flSequenceEndTime = gpGlobals->curtime + SequenceDuration();
	pOwner->SetNextAttack( flSequenceEndTime );
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;

	m_bInReload = true;

	//BG2 - Disable Iron sights on reload. -HairyPotter
	if( m_bIsIronsighted )
		DisableIronsights();
	//

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Reload( void )
{
	return DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
}

//=========================================================
void CBaseCombatWeapon::WeaponIdle( void )
{
	//Idle again if we've finished
	if ( HasWeaponIdleTimeElapsed() )
	{
		SendWeaponAnim( ACT_VM_IDLE );
	}
}


//=========================================================
Activity CBaseCombatWeapon::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

//=========================================================
Activity CBaseCombatWeapon::GetSecondaryAttackActivity( void )
{
	return ACT_VM_SECONDARYATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: Adds in view kick and weapon accuracy degradation effect
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::AddViewKick( void )
{
	//NOTENOTE: By default, weapon will not kick up (defined per weapon)
}

//-----------------------------------------------------------------------------
// Purpose: Get the string to print death notices with
//-----------------------------------------------------------------------------
char *CBaseCombatWeapon::GetDeathNoticeName( void )
{
#if !defined( CLIENT_DLL )
	return (char*)STRING( m_iszName );
#else
	return "GetDeathNoticeName not implemented on client yet";
#endif
}

//====================================================================================
// WEAPON RELOAD TYPES
//====================================================================================
void CBaseCombatWeapon::CheckReload( void )
{
	if ( m_bReloadsSingly )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		if ( !pOwner )
			return;

		if ((m_bInReload) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			if ( pOwner->m_nButtons & (IN_ATTACK | IN_ATTACK2) && m_iClip1 > 0 )
			{
				m_bInReload = false;
				return;
			}

			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <=0)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			else if (m_iClip1 < GetMaxClip1())
			{
				// Add them to the clip
				m_iClip1 += 1;
				pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				m_flNextPrimaryAttack	= gpGlobals->curtime;
				m_flNextSecondaryAttack = gpGlobals->curtime;
				return;
			}
		}
	}
	else
	{
		if ( (m_bInReload) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			FinishReload();
			m_flNextPrimaryAttack	= gpGlobals->curtime;
			m_flNextSecondaryAttack = gpGlobals->curtime;
			m_bInReload = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reload has finished.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::FinishReload( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner)
	{
		// If I use primary clips, reload primary
		if ( UsesClipsForAmmo1() )
		{
			int primary	= min( GetMaxClip1() - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));	
			m_iClip1 += primary;
			pOwner->RemoveAmmo( primary, m_iPrimaryAmmoType);
		}

		// If I use secondary clips, reload secondary
		if ( UsesClipsForAmmo2() )
		{
			int secondary = min( GetMaxClip2() - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
			m_iClip2 += secondary;
			pOwner->RemoveAmmo( secondary, m_iSecondaryAmmoType );
		}

		if ( m_bReloadsSingly )
		{
			m_bInReload = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Abort any reload we have in progress
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::AbortReload( void )
{
#ifdef CLIENT_DLL
	StopWeaponSound( RELOAD ); 
#endif
	m_bInReload = false;
}

//-----------------------------------------------------------------------------
// Purpose: Primary fire button attack
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::PrimaryAttack( void )
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		Reload();
		return;
	}

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	FireBulletsInfo_t info;
	info.m_vecSrc	 = pPlayer->Weapon_ShootPosition( );
	
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;
	float fireRate = GetFireRate();

	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		info.m_iShots++;
		if ( !fireRate )
			break;
	}

	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
	{
		info.m_iShots = min( info.m_iShots, m_iClip1 );
		m_iClip1 -= info.m_iShots;
	}
	else
	{
		info.m_iShots = min( info.m_iShots, pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) );
		pPlayer->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );
	}

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;

#if !defined( CLIENT_DLL )
	// Fire the bullets
	info.m_vecSpread = pPlayer->GetAttackSpread( this );
#else
	//!!!HACKHACK - what does the client want this function for? 
	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

	pPlayer->FireBullets( info );

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	//Add our view kick in
	AddViewKick();
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame to check if the weapon is going through transition animations
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::MaintainIdealActivity( void )
{
	// Must be transitioning
	if ( GetActivity() != ACT_TRANSITION )
		return;

	// Must not be at our ideal already 
	if ( ( GetActivity() == m_IdealActivity ) && ( GetSequence() == m_nIdealSequence ) )
		return;
	
	// Must be finished with the current animation
	if ( IsViewModelSequenceFinished() == false )
		return;

	// Move to the next animation towards our ideal
	SendWeaponAnim( m_IdealActivity );
}

//-----------------------------------------------------------------------------
// Purpose: Sets the ideal activity for the weapon to be in, allowing for transitional animations inbetween
// Input  : ideal - activity to end up at, ideally
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::SetIdealActivity( Activity ideal )
{
	MDLCACHE_CRITICAL_SECTION();
	int	idealSequence = SelectWeightedSequence( ideal );

	if ( idealSequence == -1 )
		return false;

	//Take the new activity
	m_IdealActivity	 = ideal;
	m_nIdealSequence = idealSequence;

	//Find the next sequence in the potential chain of sequences leading to our ideal one
	int nextSequence = FindTransitionSequence( GetSequence(), m_nIdealSequence, NULL );

	// Don't use transitions when we're deploying
	//BG2 - Tjoppen - fixed for empty draw anim
	//if ( ideal != ACT_VM_DRAW && IsWeaponVisible() && nextSequence != m_nIdealSequence )
	if ( ideal != GetDrawActivity() && IsWeaponVisible() && nextSequence != m_nIdealSequence )
	{
		//Set our activity to the next transitional animation
		SetActivity( ACT_TRANSITION );
		SetSequence( nextSequence );	
		SendViewModelAnim( nextSequence );
	}
	else
	{
		//Set our activity to the ideal
		SetActivity( m_IdealActivity );
		SetSequence( m_nIdealSequence );	
		SendViewModelAnim( m_nIdealSequence );
	}

	//Set the next time the weapon will idle
	//BG2 - Tjoppen - don't idle immediately a.k.a. behave like HL1
	//SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() + random->RandomFloat( 5.f, 15.f ) );
	return true;
}

//-----------------------------------------------------------------------------
// Returns information about the various control panels
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::GetControlPanelInfo( int nPanelIndex, const char *&pPanelName )
{
	pPanelName = NULL;
}

//-----------------------------------------------------------------------------
// Returns information about the various control panels
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::GetControlPanelClassName( int nPanelIndex, const char *&pPanelName )
{
	pPanelName = "vgui_screen";
}


//-----------------------------------------------------------------------------
// Locking a weapon is an exclusive action. If you lock a weapon, that means 
// you are preventing others from doing so for themselves.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Lock( float lockTime, CBaseEntity *pLocker )
{
	m_flUnlockTime = gpGlobals->curtime + lockTime;
	m_hLocker.Set( pLocker );
}

//-----------------------------------------------------------------------------
// If I'm still locked for a period of time, tell everyone except the person
// that locked me that I'm not available. 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::IsLocked( CBaseEntity *pAsker )
{
	return ( m_flUnlockTime > gpGlobals->curtime && m_hLocker != pAsker );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
Activity CBaseCombatWeapon::ActivityOverride( Activity baseAct, bool *pRequired )
{
	acttable_t *pTable = ActivityList();
	int actCount = ActivityListCount();

	for ( int i = 0; i < actCount; i++, pTable++ )
	{
		if ( baseAct == pTable->baseAct )
		{
			if (pRequired)
			{
				*pRequired = pTable->required;
			}
			return (Activity)pTable->weaponAct;
		}
	}
	return baseAct;
}


#if defined( CLIENT_DLL )

BEGIN_PREDICTION_DATA( CBaseCombatWeapon )

	DEFINE_PRED_FIELD( m_nNextThinkTick, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	// Networked
	DEFINE_PRED_FIELD( m_hOwner, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	// DEFINE_FIELD( m_hWeaponFileInfo, FIELD_SHORT ),
	DEFINE_PRED_FIELD( m_iState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),			 
	DEFINE_PRED_FIELD( m_iViewModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	DEFINE_PRED_FIELD( m_iWorldModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	DEFINE_PRED_FIELD_TOL( m_flNextPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),	
	DEFINE_PRED_FIELD_TOL( m_flNextSecondaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
	DEFINE_PRED_FIELD_TOL( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),

	DEFINE_PRED_FIELD( m_iPrimaryAmmoType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iSecondaryAmmoType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iClip1, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),			
	DEFINE_PRED_FIELD( m_iClip2, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),			

	DEFINE_PRED_FIELD( m_nViewModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),

	// Not networked

	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_FIELD( m_bInReload, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFireOnEmpty, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flNextEmptySoundTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_Activity, FIELD_INTEGER ),
	DEFINE_FIELD( m_fFireDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_iszName, FIELD_INTEGER ),		
	DEFINE_FIELD( m_bFiresUnderwater, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAltFiresUnderwater, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fMinRange1, FIELD_FLOAT ),		
	DEFINE_FIELD( m_fMinRange2, FIELD_FLOAT ),		
	DEFINE_FIELD( m_fMaxRange1, FIELD_FLOAT ),		
	DEFINE_FIELD( m_fMaxRange2, FIELD_FLOAT ),		
	DEFINE_FIELD( m_bReloadsSingly, FIELD_BOOLEAN ),	
	DEFINE_FIELD( m_bRemoveable, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iPrimaryAmmoCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSecondaryAmmoCount, FIELD_INTEGER ),

	//DEFINE_PHYSPTR( m_pConstraint ),

	// DEFINE_FIELD( m_iOldState, FIELD_INTEGER ),
	// DEFINE_FIELD( m_bJustRestored, FIELD_BOOLEAN ),

	// DEFINE_FIELD( m_OnPlayerPickup, COutputEvent ),
	// DEFINE_FIELD( m_pConstraint, FIELD_INTEGER ),

END_PREDICTION_DATA()

#endif	// ! CLIENT_DLL

// Special hack since we're aliasing the name C_BaseCombatWeapon with a macro on the client
IMPLEMENT_NETWORKCLASS_ALIASED( BaseCombatWeapon, DT_BaseCombatWeapon )

#if !defined( CLIENT_DLL )
//-----------------------------------------------------------------------------
// Purpose: Save Data for Base Weapon object
//-----------------------------------------------------------------------------// 
BEGIN_DATADESC( CBaseCombatWeapon )

	DEFINE_FIELD( m_flNextPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flNextSecondaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeWeaponIdle, FIELD_TIME ),

	DEFINE_FIELD( m_bInReload, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFireOnEmpty, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),

	DEFINE_FIELD( m_iState, FIELD_INTEGER ),
	DEFINE_FIELD( m_iszName, FIELD_STRING ),
	DEFINE_FIELD( m_iPrimaryAmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSecondaryAmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( m_iClip1, FIELD_INTEGER ),
	DEFINE_FIELD( m_iClip2, FIELD_INTEGER ),
	DEFINE_FIELD( m_bFiresUnderwater, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAltFiresUnderwater, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fMinRange1, FIELD_FLOAT ),
	DEFINE_FIELD( m_fMinRange2, FIELD_FLOAT ),
	DEFINE_FIELD( m_fMaxRange1, FIELD_FLOAT ),
	DEFINE_FIELD( m_fMaxRange2, FIELD_FLOAT ),

	DEFINE_FIELD( m_iPrimaryAmmoCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSecondaryAmmoCount, FIELD_INTEGER ),

	DEFINE_FIELD( m_nViewModelIndex, FIELD_INTEGER ),

// don't save these, init to 0 and regenerate
//	DEFINE_FIELD( m_flNextEmptySoundTime, FIELD_TIME ),
//	DEFINE_FIELD( m_Activity, FIELD_INTEGER ),
 	DEFINE_FIELD( m_nIdealSequence, FIELD_INTEGER ),
	DEFINE_FIELD( m_IdealActivity, FIELD_INTEGER ),

	DEFINE_FIELD( m_fFireDuration, FIELD_FLOAT ),

	DEFINE_FIELD( m_bReloadsSingly, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iSubType, FIELD_INTEGER ),
 	DEFINE_FIELD( m_bRemoveable, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_flUnlockTime,		FIELD_TIME ),
	DEFINE_FIELD( m_hLocker,			FIELD_EHANDLE ),

	//	DEFINE_FIELD( m_iViewModelIndex, FIELD_INTEGER ),
	//	DEFINE_FIELD( m_iWorldModelIndex, FIELD_INTEGER ),
	//  DEFINE_FIELD( m_hWeaponFileInfo, ???? ),

	DEFINE_PHYSPTR( m_pConstraint ),

	DEFINE_FIELD( m_iReloadHudHintCount,	FIELD_INTEGER ),
	DEFINE_FIELD( m_iAltFireHudHintCount,	FIELD_INTEGER ),
	DEFINE_FIELD( m_bReloadHudHintDisplayed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAltFireHudHintDisplayed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flHudHintPollTime, FIELD_TIME ),
	DEFINE_FIELD( m_flHudHintMinDisplayTime, FIELD_TIME ),

	// Just to quiet classcheck.. this field exists only on the client
//	DEFINE_FIELD( m_iOldState, FIELD_INTEGER ),
//	DEFINE_FIELD( m_bJustRestored, FIELD_BOOLEAN ),

	// Function pointers
	DEFINE_FUNCTION( DefaultTouch ),
	DEFINE_FUNCTION( FallThink ),
	DEFINE_FUNCTION( Materialize ),
	DEFINE_FUNCTION( AttemptToMaterialize ),
	DEFINE_FUNCTION( DestroyItem ),
	DEFINE_FUNCTION( SetPickupTouch ),

	DEFINE_FUNCTION( HideThink ),
	DEFINE_INPUTFUNC( FIELD_VOID, "HideWeapon", InputHideWeapon ),

	// Outputs
	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse"),
	DEFINE_OUTPUT( m_OnPlayerPickup, "OnPlayerPickup"),
	DEFINE_OUTPUT( m_OnNPCPickup, "OnNPCPickup"),
	DEFINE_OUTPUT( m_OnCacheInteraction, "OnCacheInteraction" ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Only send to local player if this weapon is the active weapon
// Input  : *pStruct - 
//			*pVarData - 
//			*pRecipients - 
//			objectID - 
// Output : void*
//-----------------------------------------------------------------------------
void* SendProxy_SendActiveLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	// Get the weapon entity
	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pVarData;
	if ( pWeapon )
	{
		// Only send this chunk of data to the player carrying this weapon
		CBasePlayer *pPlayer = ToBasePlayer( pWeapon->GetOwner() );
		if ( pPlayer /*&& pPlayer->GetActiveWeapon() == pWeapon*/ )
		{
			pRecipients->SetOnly( pPlayer->GetClientIndex() );
			return (void*)pVarData;
		}
	}
	
	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendActiveLocalWeaponDataTable );

//-----------------------------------------------------------------------------
// Purpose: Only send the LocalWeaponData to the player carrying the weapon
//-----------------------------------------------------------------------------
void* SendProxy_SendLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	// Get the weapon entity
	CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon*)pVarData;
	if ( pWeapon )
	{
		// Only send this chunk of data to the player carrying this weapon
		CBasePlayer *pPlayer = ToBasePlayer( pWeapon->GetOwner() );
		if ( pPlayer )
		{
			pRecipients->SetOnly( pPlayer->GetClientIndex() );
			return (void*)pVarData;
		}
	}
	
	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendLocalWeaponDataTable );
#endif

#if PREDICTION_ERROR_CHECK_LEVEL > 1
#define SendPropTime SendPropFloat
#define RecvPropTime RecvPropFloat
#endif

//-----------------------------------------------------------------------------
// Purpose: Propagation data for weapons. Only sent when a player's holding it.
//-----------------------------------------------------------------------------
BEGIN_NETWORK_TABLE_NOBASE( CBaseCombatWeapon, DT_LocalActiveWeaponData )
#if !defined( CLIENT_DLL )
	SendPropTime( SENDINFO( m_flNextPrimaryAttack ) ),
	SendPropTime( SENDINFO( m_flNextSecondaryAttack ) ),
	SendPropInt( SENDINFO( m_nNextThinkTick ) ),
	SendPropTime( SENDINFO( m_flTimeWeaponIdle ) ),

#if defined( TF_DLL )
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
#endif

#else
	RecvPropTime( RECVINFO( m_flNextPrimaryAttack ) ),
	RecvPropTime( RECVINFO( m_flNextSecondaryAttack ) ),
	RecvPropInt( RECVINFO( m_nNextThinkTick ) ),
	RecvPropTime( RECVINFO( m_flTimeWeaponIdle ) ),
#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Propagation data for weapons. Only sent when a player's holding it.
//-----------------------------------------------------------------------------
BEGIN_NETWORK_TABLE_NOBASE( CBaseCombatWeapon, DT_LocalWeaponData )
#if !defined( CLIENT_DLL )
	SendPropIntWithMinusOneFlag( SENDINFO(m_iClip1 ), 8 ),
	SendPropIntWithMinusOneFlag( SENDINFO(m_iClip2 ), 8 ),
	SendPropInt( SENDINFO(m_iPrimaryAmmoType ), 8 ),
	SendPropInt( SENDINFO(m_iSecondaryAmmoType ), 8 ),

	SendPropInt( SENDINFO( m_nViewModelIndex ), VIEWMODEL_INDEX_BITS, SPROP_UNSIGNED ),

#if defined( TF_DLL )
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
#endif

#else
	RecvPropIntWithMinusOneFlag( RECVINFO(m_iClip1 )),
	RecvPropIntWithMinusOneFlag( RECVINFO(m_iClip2 )),
	RecvPropInt( RECVINFO(m_iPrimaryAmmoType )),
	RecvPropInt( RECVINFO(m_iSecondaryAmmoType )),

	RecvPropInt( RECVINFO( m_nViewModelIndex ) ),

#endif
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE(CBaseCombatWeapon, DT_BaseCombatWeapon)
#if !defined( CLIENT_DLL )
	SendPropDataTable("LocalWeaponData", 0, &REFERENCE_SEND_TABLE(DT_LocalWeaponData), SendProxy_SendLocalWeaponDataTable ),
	SendPropDataTable("LocalActiveWeaponData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponData), SendProxy_SendActiveLocalWeaponDataTable ),
	SendPropModelIndex( SENDINFO(m_iViewModelIndex) ),
	SendPropModelIndex( SENDINFO(m_iWorldModelIndex) ),
	SendPropInt( SENDINFO(m_iState ), 8, SPROP_UNSIGNED ),
	SendPropEHandle( SENDINFO(m_hOwner) ),
	//BG2 -Added for Iron Sights Testing. Credits to z33ky for the code. -HairyPotter
	SendPropBool( SENDINFO( m_bIsIronsighted ) ),
	SendPropFloat( SENDINFO( m_flIronsightedTime ) ),
	//
#else
	RecvPropDataTable("LocalWeaponData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalWeaponData)),
	RecvPropDataTable("LocalActiveWeaponData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponData)),
	RecvPropInt( RECVINFO(m_iViewModelIndex)),
	RecvPropInt( RECVINFO(m_iWorldModelIndex)),
	RecvPropInt( RECVINFO(m_iState )),
	RecvPropEHandle( RECVINFO(m_hOwner ) ),
	//BG2 -Added for Iron Sights Testing. Credits to z33ky for the code. -HairyPotter
	RecvPropBool( RECVINFO( m_bIsIronsighted ) ),
	RecvPropFloat( RECVINFO( m_flIronsightedTime ) ),
	//
#endif
END_NETWORK_TABLE()
