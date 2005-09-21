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

#include "weapon_bg2base.h"
#include "ammodef.h"

//BG2 - Draco - no hl2 player stuff on server!
#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
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
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "effect_dispatch_data.h"
#include "bullet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_simulatedbullets( "sv_simulatedbullets", "1", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEMO,
		"EXPERIMENTAL!\nWhen non-zero, makes all firearms shoot \"real\" bullets.");

ConVar sv_simulatedbullets_drag( "sv_simulatedbullets_drag", "0.0003", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEMO,
	   "Tweak this value to affect how fast the speed and thus damage of bullets drop off with distance.\n\tLower values => more damage over distance" );

ConVar sv_infiniteammo( "sv_infiniteammo", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Bullet weapons don\'t use up ammo\n" );
ConVar sv_turboshots( "sv_turboshots", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Turns all guns into machineguns\n" );
ConVar sv_perfectaim( "sv_perfectaim", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "No spread for bullet weapons\n" );
ConVar sv_steadyhand( "sv_steadyhand", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "No recoil for bullet weapons\n" );

ConVar mp_disable_melee( "mp_disable_melee", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "When non-zero, melee weapons are disabled" );
ConVar mp_disable_firearms( "mp_disable_firearms", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "When non-zero, firearms are disabled" );


//-----------------------------------------------------------------------------
// CBaseBG2Weapon
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( BaseBG2Weapon, DT_BaseBG2Weapon )

BEGIN_NETWORK_TABLE( CBaseBG2Weapon, DT_BaseBG2Weapon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CBaseBG2Weapon )
END_PREDICTION_DATA()

/*LINK_ENTITY_TO_CLASS( weapon_revolutionnaire, CBaseBG2Weapon );
PRECACHE_WEAPON_REGISTER( weapon_revolutionnaire );*/

#ifndef CLIENT_DLL
acttable_t CBaseBG2Weapon::m_acttable[] = 
{

	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_PISTOL,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_PISTOL,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_PISTOL,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_PISTOL,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_PISTOL,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_PISTOL,				false },

	{ ACT_RANGE_ATTACK2,				ACT_VM_SECONDARYATTACK,					false },

	//this is to catch the default fire animation...
	//{ ACT_WALK,							ACT_HL2MP_JUMP_PISTOL,					false },
};

IMPLEMENT_ACTTABLE( CBaseBG2Weapon );

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBaseBG2Weapon::CBaseBG2Weapon( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	//m_Swingstate		= ATTACK_NONE;

	m_Attackinfos[0].m_iAttacktype = ATTACKTYPE_NONE;
	m_Attackinfos[1].m_iAttacktype = ATTACKTYPE_NONE;

	//m_nViewModelIndex	= random->RandomInt( 0, 1 );	//test..
	//GetViewModel

	/*m_bInSwing			= false;
	m_bUsesDelay		= false;

	m_flRange			= 90.f;
	m_flFireRate		= 0.8f;
	m_flImpactDelay		= 0.3f;

	m_flShotDamage		= 75.f;
	m_flStabDamage		= 60.f;

	m_vDuckSpread		= VECTOR_CONE_2DEGREES;
	m_vStandSpread		= VECTOR_CONE_5DEGREES;*/
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int CBaseBG2Weapon::Fire( int iAttack )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return 0;
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
			m_flNextPrimaryAttack = 0.15;
		}

		return 0;
	}

	if( pPlayer->GetWaterLevel() == 3 )
	{
		// This weapon doesn't fire underwater
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.2;
		return 0;
	}

	/*WeaponSound( SINGLE );

	if( sv_turboshots.GetInt() == 0 )
	{
		//don't spam sprites with sv_turboshots
		pPlayer->DoMuzzleFlash();
		pPlayer->SetAnimation( PLAYER_ATTACK1 );
	}

	SendWeaponAnim( GetActivity( iAttack ) );*/

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
	
	if( sv_turboshots.GetInt() == 0 )
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetAttackRate( iAttack );
	else
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.1f;

	if( sv_infiniteammo.GetInt() == 0 )
		m_iClip1--;

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );	

	FireBulletsInfo_t info( 1, vecSrc, vecAiming,
							sv_perfectaim.GetInt() == 0 ? GetSpread( iAttack ) : vec3_origin,
							GetRange( iAttack ), m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;
	info.m_iPlayerDamage = GetDamage( iAttack );
	info.m_iDamage = -1;	//ancient chinese secret..
	info.m_iTracerFreq = 1;		//allways to tracers

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );

	//Disorient the player
	QAngle angles = pPlayer->GetLocalAngles();

	if( sv_steadyhand.GetInt() == 0 )
	{
		angles.x += random->RandomInt( -1, 1 ) * GetRecoil(iAttack);
		angles.y += random->RandomInt( -1, 1 ) * GetRecoil(iAttack);
		angles.z = 0;

#ifndef CLIENT_DLL
	pPlayer->SnapEyeAngles( angles );
#endif

		pPlayer->ViewPunch( QAngle( -8, random->RandomFloat( -2, 2 ), 0 ) * GetRecoil(iAttack) );
	}

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); 
	}

	return 15;
}

int CBaseBG2Weapon::FireBullet( int iAttack )
{
	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = 0.15;
		}

		return 0;
	}

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return 0;

	if( pOwner->GetWaterLevel() == 3 )
	{
		// This weapon doesn't fire underwater
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.2;
		return 0;
	}

#ifndef CLIENT_DLL
	Vector vecAiming	= pOwner->GetAutoaimVector( 0 );	
	Vector vecSrc		= pOwner->Weapon_ShootPosition();
	
	/*QAngle angAiming;
	VectorAngles( vecAiming, angAiming );*/

    CShotManipulator Manipulator( vecAiming );
	Vector vecDir		= Manipulator.ApplySpread( GetSpread( iAttack ) );

	QAngle angDir;
	VectorAngles( vecDir, angDir );

	/*CBullet *pBolt =*/ CBullet::BoltCreate( vecSrc, angDir, GetDamage(iAttack), pOwner );

	/*if ( pOwner->GetWaterLevel() == 3 )
	{
		//pBolt->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
		pBolt->SetAbsVelocity( vecDir * BOLT_WATER_VELOCITY );
	}
	else
	{
		//pBolt->SetAbsVelocity( vecAiming * BOLT_AIR_VELOCITY );
		pBolt->SetAbsVelocity( vecDir * BOLT_AIR_VELOCITY );
	}*/

#endif

	WeaponSound( SINGLE );
	//WeaponSound( SPECIAL2 );

	if( sv_turboshots.GetInt() == 0 )
	{
		//pOwner->DoMuzzleFlash();

		//BG2 - Tjoppen -	the weapon animation !!MUST!! play before the player animation or there's
		//					 no muzzle flash. don't forget this! ever! it's annoying as hell!
		//SendWeaponAnim( GetPrimaryAttackActivity() );

		// player "shoot" animation
		//pOwner->SetAnimation( PLAYER_ATTACK1 );
	}
	
	if( sv_turboshots.GetInt() == 0 )
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetAttackRate( iAttack );
	else
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.1f;

	if( sv_infiniteammo.GetInt() == 0 )
		m_iClip1--;

	if( sv_steadyhand.GetInt() == 0 )
		pOwner->ViewPunch( QAngle( -2, 0, 0 ) );

	SendWeaponAnim( GetActivity( iAttack ) );

	if ( !m_iClip1 && pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	/*m_flNextPrimaryAttack = m_flNextSecondaryAttack	= gpGlobals->curtime + 0.75;

	DoLoadEffect();
	SetChargerState( CHARGER_STATE_DISCHARGE );*/

	return 15;
}

//------------------------------------------------------------------------------
// Purpose: Implement impact function
//------------------------------------------------------------------------------
void CBaseBG2Weapon::Hit( trace_t &traceHit, int iAttack )//Activity nHitActivity )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	//Do view kick
//	AddViewKick();

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if ( pHitEntity != NULL )
	{
		Vector hitDirection;
		pPlayer->EyeVectors( &hitDirection, NULL, NULL );
		VectorNormalize( hitDirection );

#ifndef CLIENT_DLL
		float	damage		= GetDamage( iAttack );	

		//relative velocity, projected on aim vector..
		//only for stabbing weapons
		Vector	vecAiming;
		AngleVectors( pPlayer->EyeAngles() + pPlayer->m_Local.m_vecPunchAngle, &vecAiming );

		float	bonus1		= pPlayer->GetLocalVelocity().Dot( vecAiming ) * 0.08f,
				bonus2		= pHitEntity->GetLocalVelocity().Dot( vecAiming ) * -0.08f;

		float	bonus = bonus1 + bonus2;
		bonus *= 0.8f;	//BG2 - Tjoppen - TWEAKME
		if( bonus < 0 ) bonus = 0;

		if( GetAttackType(iAttack) != ATTACKTYPE_STAB )
			bonus = 0;

		//BG2 - Tjoppen - apply force to both attacker and victim
		{
			CTakeDamageInfo info( GetOwner(), GetOwner(), damage + bonus, DMG_CLUB );

			if( pPlayer && pHitEntity->IsNPC() )
			{
				// If bonking an NPC, adjust damage.
				info.AdjustPlayerDamageInflictedForSkillLevel();
			}

			if( GetAttackType(iAttack) == ATTACKTYPE_STAB )
				CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos );
			else
				CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos, 0.1f );
				//info.SetDamageForce( vec3_origin );	//no force in slashing weapons..

			pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit ); 
			ApplyMultiDamage();

			// Now hit all triggers along the ray that... 
			TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, hitDirection );
		}

		if( GetAttackType(iAttack) == ATTACKTYPE_STAB )
		{
			//only for stabbing weapons
			float amount = damage + bonus;
			if( amount > 100 ) amount = 100;

			//pPlayer->VelocityPunch( -hitDirection * (damage + bonus) * ImpulseScale( 75, 1 ) * phys_pushscale.GetFloat() );
			//pPlayer->VPhysicsGetObject()->ApplyForceCenter( -hitDirection * (damage + bonus) * ImpulseScale( 75, 1 ) * phys_pushscale.GetFloat() );
			pPlayer->VelocityPunch( -hitDirection * amount * (pPlayer->GetFlags() & FL_ONGROUND ? 4.0f : 1.0f) * phys_pushscale.GetFloat() );
		}

#endif
		//WeaponSound( MELEE_HIT );
		if( pHitEntity->IsPlayer() )
		{
			//WeaponSound( MELEE_HIT );
			ImpactEffect( traceHit );
		}
		else
		{
			//WeaponSound( MELEE_HIT_WORLD );
			if( GetAttackType(iAttack) != ATTACKTYPE_SLASH )
			{
				//don't make bullet holes in walls for slashing weapons
				ImpactEffect( traceHit );
			}
		}
	}
	else
	{
        // Apply an impact effect
		ImpactEffect( traceHit );
	}
}

Activity CBaseBG2Weapon::ChooseIntersectionPointAndActivity( int iAttack, trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner )
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = {mins.Base(), maxs.Base()};
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	//UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
	if ( tmpTrace.fraction == 1.0 )
	{
		for ( i = 0; i < 2; i++ )
		{
			for ( j = 0; j < 2; j++ )
			{
				for ( k = 0; k < 2; k++ )
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					//UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
					UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
					if ( tmpTrace.fraction < 1.0 )
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if ( thisDistance < distance )
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return GetActivity(iAttack);//ACT_VM_HITCENTER;
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
	UTIL_ImpactTrace( &traceHit, DMG_CLUB );
}

//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
/*void CBaseBG2Weapon::Swing( int iAttack )
{
	//Msg( "CBaseBG2Weapon::Swing() - dmg=%f rng=%f recl=%f rate=%f\n", GetDamage(iAttack), GetRange(iAttack), GetRecoil(iAttack), GetAttackRate(iAttack) );
    
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	WeaponSound( SPECIAL1 );

	Vector vecSrc		= pOwner->Weapon_ShootPosition();
	Vector vecAiming	= pOwner->GetAutoaimVector( AUTOAIM_5DEGREES );	

	float	speed = vecAiming.Dot( pOwner->GetLocalVelocity() ),
			damage = (speed > 0 ? speed : 0)*0.15f + GetDamage(iAttack);

	//Msg( "Swing(): %.2f damage, %.2f speed, %.2f projected speed => %.2f mod damage\n", GetDamage( iAttack ), pOwner->GetLocalVelocity().Length(), speed, damage );

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, VECTOR_CONE_5DEGREES, GetRange( iAttack ), GetAmmoDef()->Index("357") );
	info.m_pAttacker = pOwner;
	info.m_iPlayerDamage = info.m_iDamage = damage;//GetDamage( iAttack );
	info.m_iTracerFreq = 0;		//no tracers

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pOwner->FireBullets( info );

	// Send the anim
	SendWeaponAnim( GetActivity( iAttack ) );

	pOwner->SetAnimation( PLAYER_ATTACK2 );

	//Disorient the player - more than firing
	QAngle angles = pOwner->GetLocalAngles();

	angles.x += random->RandomInt( -3, 3 ) * GetRecoil(iAttack);
	angles.y += random->RandomInt( -3, 3 ) * GetRecoil(iAttack);
	angles.z = 0;

#ifndef CLIENT_DLL
	pOwner->SnapEyeAngles( angles );
#endif

	if( GetAttackType(iAttack) == ATTACKTYPE_STAB )
        pOwner->ViewPunch( QAngle( -8, random->RandomFloat( -2, 2 ), 0 ) * GetRecoil(iAttack) );
	else if( GetAttackType(iAttack) == ATTACKTYPE_SLASH )
		pOwner->ViewPunch( QAngle( random->RandomFloat( -2, 2 ), random->RandomFloat( -8, 8 ), 0 ) * GetRecoil(iAttack) );

	//Setup our next attack times
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetAttackRate( iAttack );
}*/

void CBaseBG2Weapon::PrimaryAttack( void )
{
	int drain = 0;
	if( GetAttackType( ATTACK_PRIMARY ) == ATTACKTYPE_STAB || GetAttackType( ATTACK_PRIMARY ) == ATTACKTYPE_SLASH )
	{
		if( GetOwner() && (GetOwner()->GetFlags() & FL_DUCKING) )
			return;

		if( mp_disable_melee.GetInt() )
			return;

		drain = Swing( ATTACK_PRIMARY );
	}
	else if( GetAttackType( ATTACK_PRIMARY ) == ATTACKTYPE_FIREARM )
	{
		if( mp_disable_firearms.GetInt() )
			return;

		if( sv_simulatedbullets.GetInt() )
			drain = FireBullet( ATTACK_PRIMARY );
		else
			drain = Fire( ATTACK_PRIMARY );
	}
	else
		return;	//don't drain stamina

	//BG2 - Draco - decrease stam when attacking
#ifndef CLIENT_DLL
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( GetOwner() );
#else
	C_HL2MP_Player *pHL2Player = ToHL2MPPlayer( GetOwner() );
#endif

	pHL2Player->m_iStamina -= drain;
	if( pHL2Player->m_iStamina < 0 )
		pHL2Player->m_iStamina = 0;
}

void CBaseBG2Weapon::SecondaryAttack( void )
{
	int drain = 0;
	if( GetAttackType( ATTACK_SECONDARY ) == ATTACKTYPE_STAB || GetAttackType( ATTACK_SECONDARY ) == ATTACKTYPE_SLASH )
	{
		if( GetOwner() && (GetOwner()->GetFlags() & FL_DUCKING) )
			return;

		if( mp_disable_melee.GetInt() )
			return;

		drain = Swing( ATTACK_SECONDARY );
	}
	else if( GetAttackType( ATTACK_SECONDARY ) == ATTACKTYPE_FIREARM )
	{
		if( mp_disable_firearms.GetInt() )
			return;

		if( sv_simulatedbullets.GetInt() )
			drain = FireBullet( ATTACK_SECONDARY );
		else
			drain = Fire( ATTACK_SECONDARY );
	}
	else
		return;	//don't drain stamina

	//BG2 - Draco - decrease stam when attacking
#ifndef CLIENT_DLL
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( GetOwner() );
#else
	C_HL2MP_Player *pHL2Player = ToHL2MPPlayer( GetOwner() );
#endif

	pHL2Player->m_iStamina -= drain;
	if( pHL2Player->m_iStamina < 0 )
		pHL2Player->m_iStamina = 0;

}

int CBaseBG2Weapon::Swing( int iAttack )
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return 0;

	Vector swingStart = pOwner->Weapon_ShootPosition( );
	Vector forward;

	pOwner->EyeVectors( &forward, NULL, NULL );

	Vector swingEnd = swingStart + forward * GetRange(iAttack);
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
	//UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &traceHit );
	Activity nHitActivity = GetActivity(iAttack);

#ifndef CLIENT_DLL
	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), GetDamage(iAttack), DMG_CLUB );

	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, vec3_origin );
#endif

	//slashing weapons get a hit area tolerance thingy..
	//stabbing too, for the moment
	if ( traceHit.fraction == 1.0 )//&& GetAttackType(iAttack) == ATTACKTYPE_SLASH )
	{
		//Msg( "near hit..." );

		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		//we make "near hits" trace against the hull to avoid easy headshots
		UTIL_TraceHull( swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
		//UTIL_TraceHull( swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &traceHit );
		if ( traceHit.fraction < 1.0 && traceHit.m_pEnt )
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize( vecToTarget );

			float dot = vecToTarget.Dot( forward );

			//Msg( "hit? %f ", dot );

			// YWB:  Make sure they are sort of facing the guy at least...
			if ( dot < 0.70721f )
			//if ( dot < GetCosTolerance(iAttack) )//0.70721f )
			{
				// Force amiss
				//Msg( "miss\n" );
				traceHit.fraction = 1.0f;
			}
			else
			{
				//Msg( "hit due to closeness\n" );
				nHitActivity = ChooseIntersectionPointAndActivity( iAttack, traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner );
			}
		}
		/*else
			Msg( "miss\n" );*/
	}
	else
	{
		//we hit the hull, but can we do better?
		UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &traceHit );
		if ( traceHit.fraction == 1.0 )
		{
			//nope, fall back
			UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
		}
	}
	/*else
		Msg( "hit\n" );*/

	//WeaponSound( SINGLE );
	WeaponSound( SPECIAL1 );

	// -------------------------
	//	Miss
	// -------------------------
	if ( traceHit.fraction == 1.0f )
	{
		//nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange(iAttack);
		
		// See if we happened to hit water
		ImpactWater( swingStart, testEnd );
	}
	else
	{
		//Hit( traceHit, nHitActivity );
		Hit( traceHit, iAttack );
	}

	// Send the anim
	/*SendWeaponAnim( nHitActivity );

	pOwner->SetAnimation( PLAYER_ATTACK1 );*/
	SendWeaponAnim( GetActivity( iAttack ) );
	pOwner->SetAnimation( PLAYER_ATTACK2 );

	if( GetAttackType(iAttack) == ATTACKTYPE_STAB )
        pOwner->ViewPunch( QAngle( -8, random->RandomFloat( -2, 2 ), 0 ) * GetRecoil(iAttack) );
	else if( GetAttackType(iAttack) == ATTACKTYPE_SLASH )
		pOwner->ViewPunch( QAngle( random->RandomFloat( -2, 2 ), random->RandomFloat( -8, 8 ), 0 ) * GetRecoil(iAttack) );

	//Setup our next attack times
	//m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	//m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	//m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetAttackRate(iAttack);

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

	return 25;
}

void CBaseBG2Weapon::Drop( const Vector &vecVelocity )
{
#ifndef CLIENT_DLL
	UTIL_Remove( this );
#endif
}