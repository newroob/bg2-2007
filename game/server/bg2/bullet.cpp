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

#define BOLT_MODEL			"models/game/musket_ball.mdl"

#define BOLT_AIR_VELOCITY	14400
#define BOLT_WATER_VELOCITY	1500

//#define BULLET_TRACER "sprites/laser.vmt" //Pew Pew "sprites/laser.vmt"

/*extern ConVar sv_bullettracers;
ConVar teststart("teststart", "0.8f", 0);
ConVar testend("testend", "0.8f", 0);
ConVar testlife("testlife", "0.07f", 0);*/

#ifndef CLIENT_DLL

//-----------------------------------------------------------------------------
// Crossbow Bolt
//-----------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS( bullet, CBullet );

BEGIN_DATADESC( CBullet )
	// Function Pointers
	DEFINE_FUNCTION( BubbleThink ),
	DEFINE_FUNCTION( BoltTouch ),

	DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CBullet, DT_Bullet )
END_SEND_TABLE()

CBullet *CBullet::BulletCreate( const Vector &vecOrigin, const QAngle &angAngles, int iDamage, float flConstantDamageRange, float flRelativeDrag, CBasePlayer *pentOwner, CBaseCombatWeapon *pWeapon  )
{
	// Create a new entity with CBullet private data
	CBullet *pBullet = (CBullet *)CreateEntityByName( "bullet" );
	UTIL_SetOrigin( pBullet, vecOrigin );
	pBullet->SetAbsAngles( angAngles );
	Vector vecDir;
	AngleVectors( angAngles, &vecDir );
	pBullet->SetAbsVelocity( vecDir * BOLT_AIR_VELOCITY );

	pBullet->Spawn();
	pBullet->SetOwnerEntity( pentOwner );

	pBullet->m_iDamage = iDamage;

#define LIFETIME	3.f //10.f //3 seconds is definately long enough lifetime.
	pBullet->m_flDyingTime = gpGlobals->curtime + LIFETIME;
	pBullet->m_flConstantDamageRange = flConstantDamageRange;
	pBullet->m_flRelativeDrag = flRelativeDrag;
	pBullet->m_vTrajStart = vecOrigin;//pBullet->GetAbsOrigin();
	pBullet->m_bHasPlayedNearmiss = false;
	pBullet->firedWeapon = pWeapon; //Store this off.
	pBullet->pAttacker = pentOwner; //This too.

	return pBullet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBullet::~CBullet( void )
{
	/*if ( m_pGlowTrail )
	{
		UTIL_Remove( m_pGlowTrail );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBullet::CreateVPhysics( void )
{
	// Create the object in the physics system
	//BG2 - Tjoppen - SOLID_VPHYSICS
	VPhysicsInitNormal( SOLID_VPHYSICS, FSOLID_NOT_STANDABLE, false );	

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CBullet::PhysicsSolidMaskForEntity() const
{
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBullet::Spawn( void )
{
	Precache( );

	SetModel( BOLT_MODEL );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	AddEffects( EF_NODRAW ); //We'll try having the bullets be invisible for now. -HairyPotter
	//UTIL_SetSize( this, -Vector(1,1,1), Vector(1,1,1) );
	//SetSolid( SOLID_BBOX );
	SetSolid( SOLID_VPHYSICS );
	SetSolidFlags( FSOLID_NOT_STANDABLE );
	//SetGravity( 0.05f );
	SetGravity( 1.0f );
	//VPhysicsGetObject()->EnableDrag( true );
	/*VPhysicsGetObject()->Wake();
	VPhysicsGetObject()->SetMass( 1 );*/
	//SetFriction( 1000.0f );

	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch( &CBullet::BoltTouch );

	SetThink( &CBullet::BubbleThink );
	SetNextThink( gpGlobals->curtime );//+ 0.01f );

	// Start up the eye trail
	//These tracers work.. sorta.. but we're just going to omit them for now.
	/*if ( sv_bullettracers.GetBool() )
	{
		m_pGlowTrail = CSpriteTrail::SpriteTrailCreate( BULLET_TRACER, GetLocalOrigin(), false );

		if ( m_pGlowTrail != NULL )
		{
			m_pGlowTrail->FollowEntity( this );
			m_pGlowTrail->SetTransparency( kRenderTransAdd, 255, 255, 255, 90, kRenderFxNone );																				
			m_pGlowTrail->SetStartWidth( teststart.GetFloat() );
			m_pGlowTrail->SetEndWidth( testend.GetFloat() );
			m_pGlowTrail->SetLifeTime( testlife.GetFloat() );
		}
	}*/
	
}


void CBullet::Precache( void )
{
	PrecacheModel( BOLT_MODEL );

	//PrecacheModel( BULLET_TRACER );

	//BG2 - Tjoppen - Bullet.Hit*
	//PrecacheScriptSound( "Bullet.HitWorld" );
	//PrecacheScriptSound( "Bullet.HitBody" );
	PrecacheScriptSound( "Bullets.DefaultNearmiss" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CBullet::BoltTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;

	if ( pAttacker == NULL || firedWeapon == NULL ) //One of these were null, die.
	{
		UTIL_Remove( this );
		return;
	}

	if ( pAttacker->GetActiveWeapon() != firedWeapon ) //So the weapon belonging to the person that fired the shot is not the same as the one that fired the shot.
	{
		UTIL_Remove( this );
		return;	
	}

	//StopSound( entindex(), "Bullets.DefaultNearmiss" ); //We've hit something, stop the nearmiss sound.

	if ( pOther->m_takedamage != DAMAGE_NO )
	{
		trace_t	tr;
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize( vecNormalizedVel );

		float	speed = GetAbsVelocity().Length();
		if( speed < 100 )
		{
			//too slow. die
			UTIL_RemoveImmediate( this );
			return;
		}

		float	dmg;
		if( (GetAbsOrigin() - m_vTrajStart).Length() < m_flConstantDamageRange )
			dmg = (float)m_iDamage;
		else
			dmg = floorf( (float)m_iDamage * speed * speed / (float)(BOLT_AIR_VELOCITY*BOLT_AIR_VELOCITY) );

		Vector vForward;

		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize ( vForward );

		//BG2 - Tjoppen - We want musket balls to hit body parts, not just HITGROUP_GENERIC
		UTIL_TraceLine( GetAbsOrigin() - vForward * speed * gpGlobals->frametime, GetAbsOrigin() + vForward * speed * gpGlobals->frametime, MASK_SHOT, GetOwnerEntity(), COLLISION_GROUP_NONE, &tr );
		if( tr.fraction == 1.0 || (tr.hitgroup == HITGROUP_GENERIC && pOther->IsPlayer()) )
		{
            //only hit hull - keep going
			//move forward a bit so we don't get stuck
			SetAbsOrigin( GetAbsOrigin() + vForward * 4.f );
			return;
		}

		UTIL_ImpactTrace( &tr, DMG_BULLET );	//BG2 - Tjoppen - surface blood

		//no force!
		CTakeDamageInfo	dmgInfo( GetOwnerEntity(), GetOwnerEntity(), dmg, /*DMG_BULLET | DMG_PREVENT_PHYSICS_FORCE |*/DMG_CRUSH | DMG_NEVERGIB ); //Changed to avoid asserts. -HairyPotter
		dmgInfo.SetDamagePosition( tr.endpos );
		pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if ( pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
			 return;

		SetAbsVelocity( Vector( 0, 0, 0 ) );

		// play body "thwack" sound
		//BG2 - Tjoppen - no sound..
		//EmitSound( "Bullet.HitBody" );

		UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr );

		if ( tr.fraction != 1.0f )
		{
//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

			if ( tr.m_pEnt == NULL || ( tr.m_pEnt && tr.m_pEnt->GetMoveType() == MOVETYPE_NONE ) )
			{
				CEffectData	data;

				data.m_vOrigin = tr.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = tr.fraction != 1.0f;
			
				DispatchEffect( "Impact", data );
			}
		}
		
		SetTouch( NULL );
		SetThink( NULL );

		//BG2 - Tjoppen - remove bullets immediately before they have time to mess with the target
		//this doesn't seem to completely solve the problem of players flying when being hit by bullets though
		//UTIL_Remove( this );
		UTIL_RemoveImmediate( this );
		//
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		if ( pOther->GetMoveType() == MOVETYPE_NONE && !( tr.surface.flags & SURF_SKY ) )
		{
			//BG2 - Tjoppen - Bullet.HitWorld
			//EmitSound( "Bullet.HitWorld" );

			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize( vecDir );

			// See if we should reflect off this surface
			float hitDot = DotProduct( tr.plane.normal, -vecDir );
			
			// BG2 - BP original was( hitDot < 0.5f ) but a musket ball should not bounce off walls if the angle is too big
			//BG2 - Tjoppen - don't ricochet unless we're hitting a surface at a 60 degree horisontal angle or more
			//					this is a hack so that bullets don't ricochet off the ground
			//if ( ( hitDot < 0.2f ) && ( speed > 100 ) )
			if ( ( hitDot < 0.2f ) && ( speed > 100 ) && tr.plane.normal.z < 0.5f )
			{
				Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;
				
				QAngle reflectAngles;

				VectorAngles( vReflection, reflectAngles );

				SetLocalAngles( reflectAngles );

				SetAbsVelocity( vReflection * speed * 0.75f );

				// Start to sink faster
				SetGravity( 1.0f );
			}
			else
			{
				
				//FIXME: We actually want to stick (with hierarchy) to what we've hit
				SetMoveType( MOVETYPE_NONE );
			
				Vector vForward;

				AngleVectors( GetAbsAngles(), &vForward );
				VectorNormalize ( vForward );

				CEffectData	data;

				data.m_vOrigin = tr.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = 0;
			
				DispatchEffect( "Impact", data ); 
				
				UTIL_ImpactTrace( &tr, DMG_BULLET );

				SetTouch( NULL );

				//Just remove the bullet instantly. -HairyPotter
				/*AddEffects( EF_NODRAW );
				SetThink( &CBullet::SUB_Remove );
				SetNextThink( gpGlobals->curtime + 2.0f );*/
				UTIL_Remove( this );
			}
			
			//BG2 - BP  TODO: musket balls only create sparks on metal surfaces Shoot some sparks
			/*if ( UTIL_PointContents( GetAbsOrigin() ) == CONTENTS_WATER)
			{
				g_pEffects->Sparks( GetAbsOrigin() );
			}*/
		}
		else
		{
			// Put a mark unless we've hit the sky
			if ( ( tr.surface.flags & SURF_SKY ) == false )
			{
				UTIL_ImpactTrace( &tr, DMG_BULLET );
			}

			UTIL_Remove( this );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBullet::BubbleThink( void )
{
	QAngle angNewAngles;

	VectorAngles( GetAbsVelocity(), angNewAngles );
	SetAbsAngles( angNewAngles );

	SetNextThink( gpGlobals->curtime );	//BG2 - Tjoppen - think every frame

	if( gpGlobals->curtime > m_flDyingTime )
	{
		UTIL_Remove( this );
		return;
	}

	if ( GetWaterLevel()  == 0 )
	{
		//apply drag
		Vector	vecDir = GetAbsVelocity();
		extern	ConVar	sv_simulatedbullets_drag,
						sv_simulatedbullets_overshoot_range,
						sv_simulatedbullets_overshoot_force,
						sv_gravity;

		float	speed = vecDir.NormalizeInPlace(),
				//drag = 0.0001f;
				drag = sv_simulatedbullets_drag.GetFloat() * m_flRelativeDrag;

		speed -= drag * speed*speed * gpGlobals->frametime;
		if( speed < 1000 )
			speed = 1000;	//clamp

		SetAbsVelocity( vecDir * speed );

		if( sv_simulatedbullets_overshoot_force.GetFloat() > 0 &&
			sv_simulatedbullets_overshoot_range.GetFloat() > 0 )
		{
			//add lift due to bullet rotation which causes overshoot at medium range
			//this extra force declines exponentially and will equal to gravity at time tmax, thus putting the maximum there
			float	t = gpGlobals->curtime - m_flDyingTime + LIFETIME,	//how long we've existed..
					tmax = sv_simulatedbullets_overshoot_range.GetFloat() * 36.f / BOLT_AIR_VELOCITY,	//how long until max?
					F0 = sv_gravity.GetFloat() * sv_simulatedbullets_overshoot_force.GetFloat(),
					//F = F0 * expf( -logf(sv_simulatedbullets_overshoot_force.GetFloat()) * t / tmax );
					F = F0 * powf( sv_simulatedbullets_overshoot_force.GetFloat(), -t / tmax );

			//approx.
			vecDir = GetAbsVelocity();
			vecDir.z += F * gpGlobals->frametime;
			SetAbsVelocity( vecDir );
		}

		//8 units "safety margin" to make sure we passed our victim's head
		/*float	margin = 8, 
				headTolerance = 32,	//how close to the head must the bullet be?
				desiredBacktrace = margin + speed * gpGlobals->frametime * 2;	//*2 due to frametime variations*/
		if( !m_bHasPlayedNearmiss /*&& (GetAbsOrigin() - m_vTrajStart).LengthSqr() > desiredBacktrace*desiredBacktrace*/ )
		{
			//has gone desiredBacktrace units and not played nearmiss so far. trace backward
			//find any player except the shooter who is within the margin and desiredBacktrace distance when projected
			//onto the ray
			//the ray in this case extends behind the bullet

			//re-normalize because overshoot messed it up
			vecDir.NormalizeInPlace();

			for( int x = 1; x <= gpGlobals->maxClients; x++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( x );

				//only want connected, alive players
				if( !pPlayer || !pPlayer->IsAlive() || x == GetOwnerEntity()->entindex() )
					continue;

				//distance along ray, behind the bullet
				//float d = vecDir.Dot(GetAbsOrigin() - pPlayer->EyePosition());//

				//Msg( "%i: %f < %f < %f\t", x, desiredBacktrace, d, margin );

				//if( d < margin || d > desiredBacktrace )//
				//	continue;//

				//shortest distance to eyes from ray must be less than headTolerance
				//if( (GetAbsOrigin() - pPlayer->EyePosition()).LengthSqr() - d*d < headTolerance*headTolerance )//
				{
					//make sure only this player hears it
					CSingleUserRecipientFilter filter( pPlayer );
					EmitSound( filter, entindex(), "Bullets.DefaultNearmiss" ); //By using the entindex of the bullet itself, you're making IT play the sound, not the player. -HairyPotter
					m_bHasPlayedNearmiss = true;

					//break;
				}
			}

			//Msg( "\n" );
		}

		return;
	}

	UTIL_BubbleTrail( GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5 );
}

#else

IMPLEMENT_CLIENTCLASS_DT( C_Bullet, DT_Bullet, CBullet )
END_RECV_TABLE()


/*CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectCrossbow )
CLIENTEFFECT_MATERIAL( "effects/muzzleflash1" )
CLIENTEFFECT_REGISTER_END()*/

//extern void DrawHalo( IMaterial* pMaterial, const Vector &source, float scale, float const *color );

//
// Crossbow bolt
//


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_Bullet::C_Bullet( void )
{
	//SetModel( BOLT_MODEL );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_Bullet::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_bUpdated = false;
		m_vecLastOrigin = GetAbsOrigin();
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_Bullet::DrawModel( int flags )
{
	// Draw the normal portion
	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Bullet::ClientThink( void )
{
	m_bUpdated = false;
}

#endif