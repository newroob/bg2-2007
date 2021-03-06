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
		Tomas "Tjoppen" H�rdin		mail, in reverse: se . gamedev @ tjoppen

	You may also contact the (future) team via the Battle Grounds website and/or forum at:
		www.bgmod.com

	 Note that because of the sheer volume of files in the Source SDK this
	notice cannot be put in all of them, but merely the ones that have any
	changes from the original SDK.
	 In order to facilitate easy searching, all changes are and must be
	commented on the following form:

	//BG2 - <name of contributer>[ - <small description>]
*/

#include "weapon_bg2base.h"
#include "ammodef.h"

//BG2 - Draco - no hl2 player stuff on server!
#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "hl2mp_gamerules.h"
	#include "c_te_effect_dispatch.h"

	#include "cbase.h"
	#include "model_types.h"
	#include "ClientEffectPrecacheSystem.h"
	#include "fx.h"
#else
	#include "hl2mp_player.h"
	#include "te_effect_dispatch.h"
	#include "IEffects.h"
	#include "Sprite.h"
	#include "SpriteTrail.h"
	#include "beam_shared.h"

	#include "shot_manipulator.h"
	#include "ilagcompensationmanager.h"
#endif

#include "takedamageinfo.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "effect_dispatch_data.h"
#include "bullet.h"
#include "shot_manipulator.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_simulatedbullets( "sv_simulatedbullets", "0", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEMO,
		"EXPERIMENTAL!\nWhen non-zero, makes all firearms shoot \"real\" bullets.");

ConVar sv_arcscanbullets( "sv_arcscanbullets", "1", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEMO,
		"Like regular hitscan bullets, only traced along the same arc a sv_simulatedbullets bullet would take.");

ConVar sv_simulatedbullets_drag( "sv_simulatedbullets_drag", "0.00003", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEMO | FCVAR_CHEAT,
		"Tweak this value to affect how fast the speed and thus damage of bullets drop off with distance.\n\tLower values => more damage over distance" );

ConVar sv_simulatedbullets_show_trajectories( "sv_simulatedbullets_show_trajectories", "0", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_CHEAT,
		"Draw trajectories of the bullets? Useful for adjusting their settings" );

ConVar sv_simulatedbullets_show_trajectories_timeout( "sv_simulatedbullets_show_trajectories_timeout", "10", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_CHEAT,
		"How long the trajectories stay, in seconds" );

ConVar sv_bullettracers("sv_bullettracers", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Do bullets draw tracers behind them?" );

ConVar sv_retracing_melee( "sv_retracing_melee", "1", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEMO,
		"Should melee weapons continue tracing a short moment after attacks? At the moment 0.1 seconds. This masks the effects of lag a bit." );

ConVar sv_infiniteammo( "sv_infiniteammo", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Bullet weapons don\'t use up ammo\n" );
ConVar sv_turboshots( "sv_turboshots", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Turns all guns into machineguns\n" );
ConVar sv_perfectaim( "sv_perfectaim", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "No spread for bullet weapons\n" );
ConVar sv_steadyhand( "sv_steadyhand", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "No recoil for bullet weapons\n" );

ConVar mp_disable_melee( "mp_disable_melee", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "When non-zero, melee weapons are disabled" );
ConVar mp_disable_firearms( "mp_disable_firearms", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "When non-zero, firearms are disabled" );

ConVar sv_show_damages("sv_show_damages", "0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Allow people to view enemy damages in scoreboard?");
ConVar sv_show_enemy_names("sv_show_enemy_names", "0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Allow people to view enemy names in crosshair?");

ConVar sv_muzzle_velocity_override("sv_muzzle_velocity_override", "0", FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_CHEAT, "If non-zero, overide muzzle velocities with this value (inch per seconds)");
ConVar sv_flintlock_delay("sv_flintlock_delay", "0.135", FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_CHEAT, "Delay in seconds of the flintlock mechanism (delay bullet firing by this amount)");

//-----------------------------------------------------------------------------
// CBaseBG2Weapon
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( BaseBG2Weapon, DT_BaseBG2Weapon )

BEGIN_NETWORK_TABLE( CBaseBG2Weapon, DT_BaseBG2Weapon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CBaseBG2Weapon )
END_PREDICTION_DATA()

#ifndef CLIENT_DLL

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBaseBG2Weapon::CBaseBG2Weapon( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_bShouldSampleForward = false;
	m_bShouldFireDelayed = false;

	m_Attackinfos[0].m_iAttacktype = ATTACKTYPE_NONE;
	m_Attackinfos[1].m_iAttacktype = ATTACKTYPE_NONE;

	//m_nViewModelIndex	= random->RandomInt( 0, 1 );	//test..
}

bool CBaseBG2Weapon::Deploy( void )
{
	//stop reload
	StopWeaponSound( RELOAD );

	//play "draw" sound, but only for players that are alive
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if( pOwner && pOwner->IsAlive() )
		WeaponSound( DEPLOY );

	return BaseClass::Deploy();
}


//PrimaryAttack
void CBaseBG2Weapon::PrimaryAttack( void )
{
	DoAttack( ATTACK_PRIMARY );
}

//SecondaryAttack
void CBaseBG2Weapon::SecondaryAttack( void )
{
	if ( m_bIsIronsighted )
		return;

	DoAttack( ATTACK_SECONDARY );
}

void CBaseBG2Weapon::DoAttack( int iAttack )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	//disallow holding reload button
	if( pOwner == NULL || pOwner->m_nButtons & IN_RELOAD || m_bInReload )
		return;

	m_iLastAttack = iAttack;

	//store forward eye vector
	AngleVectors( pOwner->EyeAngles(), &m_vLastForward, NULL, NULL );

//BG2 - Pretend this doesn't exist. -HairyPotter
#ifndef CLIENT_DLL
	m_iLastAttackType = GetAttackType(iAttack);
#endif
//
	if( GetAttackType( iAttack ) == ATTACKTYPE_STAB || GetAttackType( iAttack ) == ATTACKTYPE_SLASH )
	{
		if( GetOwner() && (GetOwner()->GetFlags() & FL_DUCKING) )
			return;

		if( mp_disable_melee.GetInt() )
			return;

		//do tracelines for so many seconds
		if( sv_retracing_melee.GetBool() )
		{
			m_flStartDemotingHeadhits = m_flStopAttemptingSwing = gpGlobals->curtime + GetRetraceDuration( iAttack );

			if( GetAttackType( iAttack ) == ATTACKTYPE_SLASH )
				m_flStartDemotingHeadhits = gpGlobals->curtime + 0.1f;
		}

		Swing( iAttack, true );
	}
	else if( GetAttackType( iAttack ) == ATTACKTYPE_FIREARM )
	{
		if( mp_disable_firearms.GetInt() )
			return;

		Fire( iAttack );
	}
	else
		return;	//don't drain stamina

	//BG2 - Draco - decrease stam when attacking
#ifndef CLIENT_DLL
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( GetOwner() );

	if ( pHL2Player )
		pHL2Player->DrainStamina( GetStaminaDrain( iAttack ) );
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

void CBaseBG2Weapon::Fire( int iAttack )
{
	m_bLastAttackStab = false;

	// Only the player fires this way so we can cast
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.15;
		}

		return;
	}

	if( pPlayer->GetWaterLevel() == 3 )
	{
		// This weapon doesn't fire underwater
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.2;
		return;
	}

	if( sv_turboshots.GetInt() == 0 )
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetAttackRate( iAttack );
	else
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.1f;

	WeaponSound(SINGLE);

	if( sv_turboshots.GetInt() == 0 )
	{
		pPlayer->DoMuzzleFlash();

		//BG2 - Tjoppen -	the weapon animation !!MUST!! play before the player animation or there's
		//					 no muzzle flash. don't forget this! ever! it's annoying as hell!
		SendWeaponAnim( GetPrimaryAttackActivity() );

		// player "shoot" animation
		pPlayer->SetAnimation( PLAYER_ATTACK1 );
	}
	
	if( sv_infiniteammo.GetInt() == 0 )
		m_iClip1--;

	m_fNextHolster = gpGlobals->curtime + 0.3f; //Keep people from switching weapons right after shooting.

	//sample eye vector after a short delay, then fire the bullet(s) a short time after that
	m_bShouldSampleForward = true;
	m_flNextSampleForward = gpGlobals->curtime + sv_flintlock_delay.GetFloat();
	m_bShouldFireDelayed = true;
	m_flNextDelayedFire = gpGlobals->curtime + sv_flintlock_delay.GetFloat();
}

void CBaseBG2Weapon::FireBullets( int iAttack )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetOwner() );

	if( !pPlayer )
		return;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	//figure out how many balls we should fire based on the player's current ammo kit
	int numActualShot = 1;
	int iDamage = 0;
	float muzzleVelocity = m_flMuzzleVelocity;
	
	switch( pPlayer->GetCurrentAmmoKit() )
	{
	default:
	case AMMO_KIT_BALL:
		numActualShot = 1;
		iDamage = GetDamage( iAttack );
		break;
	case AMMO_KIT_BUCKSHOT:
		if( m_iNumShot > 0 )
		{
			numActualShot = m_iNumShot;
			iDamage = m_iDamagePerShot;
			muzzleVelocity = m_flShotMuzzleVelocity;
		}
		else
		{
			//just in case we got AMMO_KIT_BUCKSHOT without being able to shoot shot
			numActualShot = 1;
			iDamage = GetDamage( iAttack );
		}
		break;
	}

	//be a bit paranoid in case m_iNumShot is set to some crazy value
	if( numActualShot <= 0 )
		numActualShot = 1;
	else if( numActualShot > 30 )
		numActualShot = 30;

	muzzleVelocity = sv_muzzle_velocity_override.GetFloat() > 0 ? sv_muzzle_velocity_override.GetFloat() : muzzleVelocity;

	if( sv_simulatedbullets.GetBool() )
	{
#ifndef CLIENT_DLL

		//Move bullets up slightly so players can shoot over windowcills etc. properly
		vecSrc.z += 2;

		CShotManipulator manipulator2( m_vLastForward );

		for( int x = 0; x < numActualShot; x++ )
		{
			Vector vecDir = manipulator2.ApplySpread( Cone( GetCurrentAmmoSpread() ) );

			QAngle angDir;
			VectorAngles( vecDir, angDir );

#ifdef USE_ENTITY_BULLET
			CBullet::BulletCreate( vecSrc, angDir, iDamage,
									m_Attackinfos[iAttack].m_flConstantDamageRange,
									m_Attackinfos[iAttack].m_flRelativeDrag, muzzleVelocity,
									pPlayer, pPlayer->GetActiveWeapon() );
#else
			SpawnServerBullet( vecSrc, angDir, iDamage,
									m_Attackinfos[iAttack].m_flConstantDamageRange,
									m_Attackinfos[iAttack].m_flRelativeDrag,
									muzzleVelocity, pPlayer );
#endif
		}
#endif
	}
	else
	{
		FireBulletsInfo_t info( numActualShot, vecSrc, m_vLastForward, Cone( GetCurrentAmmoSpread() ), GetRange( iAttack ), m_iPrimaryAmmoType );
		info.m_pAttacker = pPlayer;
		info.m_iPlayerDamage = iDamage;
		info.m_iDamage = -1;		//ancient chinese secret..
		info.m_iTracerFreq = sv_bullettracers.GetBool();	
		
		//arcscan bullet
		info.m_bArc = sv_arcscanbullets.GetBool();
		info.m_flMuzzleVelocity = muzzleVelocity;
		info.m_flRelativeDrag = m_Attackinfos[iAttack].m_flRelativeDrag;
		info.m_flConstantDamageRange = m_Attackinfos[iAttack].m_flConstantDamageRange;

		// Fire the bullets, and force the first shot to be perfectly accurate
		pPlayer->FireBullets( info );
	}
}

//------------------------------------------------------------------------------
// Purpose: Implement impact function
//------------------------------------------------------------------------------

void CBaseBG2Weapon::Hit( trace_t &traceHit, int iAttack )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if( pHitEntity != NULL )
	{
		Vector hitDirection;
		pPlayer->EyeVectors( &hitDirection, NULL, NULL );
		VectorNormalize( hitDirection );

		int damage		= GetDamage( iAttack );	

		//BG2 - Tjoppen - apply no force
		CTakeDamageInfo info( GetOwner(), GetOwner(), damage, DMG_CLUB | DMG_PREVENT_PHYSICS_FORCE | DMG_NEVERGIB );
		info.SetDamagePosition( traceHit.endpos );

		pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit);	//negative dir for weird reasons
		ApplyMultiDamage();

		//WeaponSound( MELEE_HIT );
		if( pHitEntity->IsPlayer() )
		{
			//play miss sound until client gets HitVerif and overrides with MELEE_HIT
			WeaponSound( SPECIAL1 );
#ifndef CLIENT_DLL
			ImpactEffect( traceHit );
#endif
		}
		else
		{
			WeaponSound( MELEE_HIT_WORLD );
			if( GetAttackType(iAttack) != ATTACKTYPE_SLASH )
				ImpactEffect( traceHit );
		}
	}
	else if( GetAttackType(iAttack) != ATTACKTYPE_SLASH )
	{
		// Apply an impact effect
		ImpactEffect( traceHit );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &traceHit - 
//-----------------------------------------------------------------------------
bool CBaseBG2Weapon::ImpactWater( const Vector &start, const Vector &end )
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...
	
	// We must start outside the water
	if ( UTIL_PointContents( start ) & (CONTENTS_WATER|CONTENTS_SLIME))
		return false;

	// We must end inside of water
	if ( !(UTIL_PointContents( end ) & (CONTENTS_WATER|CONTENTS_SLIME)))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine( start, end, (CONTENTS_WATER|CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace );

	if ( waterTrace.fraction < 1.0f )
	{
#ifndef CLIENT_DLL
		CEffectData	data;

		data.m_fFlags  = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if ( waterTrace.contents & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect( "watersplash", data );			
#endif
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseBG2Weapon::ImpactEffect( trace_t &traceHit )
{
	// See if we hit water (we don't do the other impact effects in this case)
	if ( ImpactWater( traceHit.startpos, traceHit.endpos ) )
		return;

	//FIXME: need new decals
	UTIL_ImpactTrace( &traceHit, DMG_BULLET );	//BG2 - Tjoppen - surface blood
}

void CBaseBG2Weapon::Swing( int iAttack, bool bIsFirstAttempt )
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	if ( m_bIsIronsighted ) //No melee with ironsights.
		return;

#ifndef CLIENT_DLL
	//only the server can do lag compensation
	lagcompensation->StartLagCompensation( pOwner, pOwner->GetCurrentCommand() );
#endif

	if( GetAttackType(iAttack) == ATTACKTYPE_STAB )
		m_bLastAttackStab = true;
	else
		m_bLastAttackStab = false;

	Vector swingStart = pOwner->Weapon_ShootPosition( );
	Vector forward;

	pOwner->EyeVectors( &forward, NULL, NULL );

	Vector swingEnd = swingStart + forward * GetRange(iAttack);
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &traceHit );

#ifndef CLIENT_DLL
	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), GetDamage(iAttack), DMG_CLUB );

	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, vec3_origin );
#endif //Keep the following code with the client.dll

	//only allow slashing weapons to hit to head on the first frame
	//any subsequent hit to the head gets demoted a hit to the chest
	if( GetAttackType( iAttack ) == ATTACKTYPE_SLASH && traceHit.hitgroup == HITGROUP_HEAD &&
			gpGlobals->curtime >= m_flStartDemotingHeadhits && !bIsFirstAttempt )
		traceHit.hitgroup = HITGROUP_CHEST;

	if ( traceHit.fraction == 1.0f )
	{
		//miss, do any waterimpact stuff
		if( bIsFirstAttempt )
		{
			Vector testEnd = swingStart + forward * GetRange(iAttack);
			ImpactWater( swingStart, testEnd );

			WeaponSound( SPECIAL1 );	//miss
		}
	}
	else
	{
		//stop attempting more swings (don't cut through masses of people or hit the same person ten times)
		m_flStopAttemptingSwing = 0;

		Hit( traceHit, iAttack );
	}
	// Send the anim
	if( bIsFirstAttempt )
	{
		SendWeaponAnim( GetActivity( iAttack ) );
		pOwner->SetAnimation( PLAYER_ATTACK2 );

		//BG2 - Draco - you cant stab very fast when your nackered, add quarter of a second
#ifndef CLIENT_DLL
		CHL2MP_Player *pHL2Player = ToHL2MPPlayer( GetOwner() );
#else
		C_HL2MP_Player *pHL2Player = ToHL2MPPlayer( GetOwner() );
#endif

		if (pHL2Player->m_iStamina <= 35)
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetAttackRate(iAttack) + 0.25f;
		else
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetAttackRate(iAttack);
	}

#ifndef CLIENT_DLL
	lagcompensation->FinishLagCompensation( pOwner );
#endif
}

void CBaseBG2Weapon::Drop( const Vector &vecVelocity )
{
/* #ifndef CLIENT_DLL
	UTIL_Remove( this );
#endif*/
	BaseClass::Drop(vecVelocity);
}

//-----------------------------------------------------------------------------
// Purpose: Allows the weapon to choose proper weapon idle animation
//-----------------------------------------------------------------------------
void CBaseBG2Weapon::WeaponIdle( void )
{
	//See if we should idle high or low
	if ( HasWeaponIdleTimeElapsed() && !m_bIsIronsighted ) //Don't do idle anims when aiming. -HairyPotter
	{
		if( m_iClip1 == 0 )
			SendWeaponAnim( ACT_VM_IDLE_EMPTY );
		else
			SendWeaponAnim( ACT_VM_IDLE );
	}
}

/**
 * Returns how many degrees we need to angle the barrel up to be zeroed
 * in at the given range with the given muzzle velocity.
 * Note that this function does not account for drag, since doing so
 * would mean having to solve a rather complicated differential equation.
 * This function is probably good enough for practical purposes.
 * This image is helpful: http://www.frfrogspad.com/traj.gif
 */
static float AngleForZeroedRange( float muzzleVelocity, float zeroRange )
{
	extern ConVar sv_gravity;
	float q = sv_gravity.GetFloat() * zeroRange / (muzzleVelocity*muzzleVelocity);

	//avoid returning NaN in case of bogus range/velocity combination
	if( q < -1 || q > 1 )
		return 0;

	//half of sin^-1(q) in degrees
	return 90.f / M_PI * asinf(q);
}

void CBaseBG2Weapon::Think( void )
{
	CHL2MP_Player *pOwner = ToHL2MPPlayer( GetOwner() );
	int group;

	//set submodels and skins every frame
	//TODO: we don't actually have to do this every frame - only when the animation changes
	SetNextThink( gpGlobals->curtime ); 

	if( pOwner == NULL ){
		BaseClass::Think(); //BG2 - roob - need to call this so dropped weapons get removed.
		return;
	}

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	//only fiddle around if we have a view model and we're the active weapon
	//if we're not the active weapon, we'd mess up the active weapon's submodels and skin!
	if ( !pViewModel || pOwner->GetActiveWeapon() != this )
	{
		//BG2 - Tjoppen - don't forget to call Think() here as well
		BaseClass::Think();
		return;
	}

	//hide bayonet if we don't have a secondary attack
	if ( (group = pViewModel->FindBodygroupByName( "musket_bayonet" )) >= 0)
		pViewModel->SetBodygroup( group, m_Attackinfos[1].m_iAttacktype == ATTACKTYPE_STAB );

	//pick the correct metal
	if ( (group = pViewModel->FindBodygroupByName( "musket_metal" )) >= 0)
		pViewModel->SetBodygroup( group, pOwner->GetTeamNumber() == TEAM_BRITISH );

	//use the correct arms (natives don't have sleeves)
	if ( (group = pViewModel->FindBodygroupByName( "arms" )) >= 0)
		pViewModel->SetBodygroup( group, pOwner->GetTeamNumber() == TEAM_BRITISH &&
		                            pOwner->GetClass() == CLASS_SKIRMISHER );
	
	BaseClass::Think();
}

void CBaseBG2Weapon::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if( pOwner == NULL )
		return;

	if( m_flStopAttemptingSwing > gpGlobals->curtime )
	{
		//we're attempting another swing. do. but don't play any animation or swing sound. just hit sound
		//check to see that we're not flailing around. only allow a few angles of movement
		Vector vForward;
		AngleVectors( pOwner->EyeAngles(), &vForward, NULL, NULL );

		if( m_vLastForward.Dot( vForward ) < GetCosAngleTolerance() )
		{
			//flailing too much. stop
			m_flStopAttemptingSwing = 0;
		}
		else
		{
			Swing( m_iLastAttack, false );
		}
	}

	if( m_bShouldSampleForward && m_flNextSampleForward <= gpGlobals->curtime )
	{
		Vector vecAiming;
		QAngle angles = EyeAngles() + pOwner->m_Local.m_vecPunchAngle;

		//angle the barrel up slightly to account for bullet drop
		angles.x -= AngleForZeroedRange( m_flMuzzleVelocity, m_flZeroRange );

		AngleVectors( angles, &vecAiming );
		CShotManipulator manipulator( vecAiming );

		m_vLastForward = manipulator.ApplySpread( sv_perfectaim.GetInt() == 0 ? Cone( GetAccuracy( m_iLastAttack ) ) : vec3_origin );
		m_bShouldSampleForward = false;

		//Disorient the player
		if( sv_steadyhand.GetInt() == 0 )
		{
			int iSeed = CBaseEntity::GetPredictionRandomSeed() & 255;
			RandomSeed( iSeed );

			//BG2 - Tjoppen - HACKHACK: weapon attacks get called multiple times on client. until we figure out
			//							why, multiple recoils must be supressed.
			if( m_flLastRecoil + 0.1f < gpGlobals->curtime )
				pOwner->ViewPunch( QAngle( -8, random->RandomFloat( -2, 2 ), 0 ) * GetRecoil(m_iLastAttack) );

			m_flLastRecoil = gpGlobals->curtime;
		}
	}

	if( m_bShouldFireDelayed && m_flNextDelayedFire <= gpGlobals->curtime )
	{
		m_bShouldFireDelayed = false;

		FireBullets( m_iLastAttack );
	}

	BaseClass::ItemPostFrame();
}

Activity CBaseBG2Weapon::GetDrawActivity( void )
{
	if( m_iClip1 == 0 )
		return ACT_VM_DRAW_EMPTY;

	return ACT_VM_DRAW;
}

bool CBaseBG2Weapon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	//BG2 - Holster delay. - HairyPotter
	if ( gpGlobals->curtime < m_fNextHolster)
		return false;

	m_fNextHolster = gpGlobals->curtime + m_fHolsterTime;
	//

	//stop reload
	StopWeaponSound( RELOAD );

	return BaseClass::Holster( pSwitchingTo );
}

bool CBaseBG2Weapon::Reload( void )
{
	//BG2 - Tjoppen - disallow reloading until delayed firing has occured
	if( m_bShouldFireDelayed )
		return false;

	return BaseClass::Reload();
}

//roob: set correct skin on equip
void CBaseBG2Weapon::Equip( CBaseCombatCharacter *pOwner )
{

	if( pOwner == NULL )
		return;

	CHL2MP_Player *player = ToHL2MPPlayer( pOwner );

	switch( player->GetTeamNumber() )
	{
		case TEAM_AMERICANS:
		switch( player->GetClass() )
		{
		case CLASS_SNIPER:
			//minuteman
			m_nSkin = 1;
			break;
		case CLASS_SKIRMISHER:
			//militia
			m_nSkin = 2;
			break;
		default:
			//everyone else
			m_nSkin = 0;
			break;
		}
		break;
	case TEAM_BRITISH:
		switch( player->GetClass() )
		{
		case CLASS_SNIPER:
			//j�ger
			m_nSkin = 4;
			break;
		default:
			//everyone else
			m_nSkin = 3;
			break;
		}
		break;
	default:
		m_nSkin = 0;
		break;
	}

	return BaseClass::Equip( pOwner );

}