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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//#define BOLT_MODEL			"models/crossbow_bolt.mdl"
#define BOLT_MODEL			"models/game/musket_ball.mdl"
//#define BOLT_MODEL	"models/weapons/w_missile_closed.mdl"

//#define BOLT_AIR_VELOCITY	3500
#define BOLT_AIR_VELOCITY	10000
#define BOLT_WATER_VELOCITY	1500
#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1


#ifndef CLIENT_DLL

//-----------------------------------------------------------------------------
// Crossbow Bolt
//-----------------------------------------------------------------------------
class CBullet : public CBaseCombatCharacter
{
	DECLARE_CLASS( CBullet, CBaseCombatCharacter );

public:
	CBullet() { };
	~CBullet();

	Class_T Classify( void ) { return CLASS_NONE; }

public:
	void Spawn( void );
	void Precache( void );
	void BubbleThink( void );
	void BoltTouch( CBaseEntity *pOther );
	bool CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	static CBullet *BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner = NULL );

protected:

	bool	CreateSprites( void );

	CHandle<CSprite>		m_pGlowSprite;
	//CHandle<CSpriteTrail>	m_pGlowTrail;
	
	int		m_iDamage;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
LINK_ENTITY_TO_CLASS( bullet, CBullet );

BEGIN_DATADESC( CBullet )
	// Function Pointers
	DEFINE_FUNCTION( BubbleThink ),
	DEFINE_FUNCTION( BoltTouch ),

	// These are recreated on reload, they don't need storage
	DEFINE_FIELD( m_pGlowSprite, FIELD_EHANDLE ),
	//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CBullet, DT_Bullet )
END_SEND_TABLE()

CBullet *CBullet::BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner )
{
	// Create a new entity with CBullet private data
	CBullet *pBolt = (CBullet *)CreateEntityByName( "bullet" );
	UTIL_SetOrigin( pBolt, vecOrigin );
	pBolt->SetAbsAngles( angAngles );
	Vector vecDir;
	AngleVectors( angAngles, &vecDir );
	pBolt->SetAbsVelocity( vecDir * BOLT_AIR_VELOCITY );

	pBolt->Spawn();
	pBolt->SetOwnerEntity( pentOwner );

	pBolt->m_iDamage = iDamage;

	return pBolt;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBullet::~CBullet( void )
{
	if ( m_pGlowSprite )
	{
		UTIL_Remove( m_pGlowSprite );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBullet::CreateVPhysics( void )
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );

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
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBullet::CreateSprites( void )
{
	// Start up the eye glow
	m_pGlowSprite = CSprite::SpriteCreate( "sprites/light_glow02_noz.vmt", GetLocalOrigin(), false );

	if ( m_pGlowSprite != NULL )
	{
		m_pGlowSprite->FollowEntity( this );
		m_pGlowSprite->SetTransparency( kRenderGlow, 255, 255, 255, 128, kRenderFxNoDissipation );
		m_pGlowSprite->SetScale( 0.2f );
		m_pGlowSprite->TurnOff();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBullet::Spawn( void )
{
	Precache( );

	//SetModel( "models/crossbow_bolt.mdl" );
	SetModel( BOLT_MODEL );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	UTIL_SetSize( this, -Vector(1,1,1), Vector(1,1,1) );
	SetSolid( SOLID_BBOX );
	//SetGravity( 0.05f );
	SetGravity( 1.0f );
	//VPhysicsGetObject()->EnableDrag( true );
	/*VPhysicsGetObject()->Wake();
	VPhysicsGetObject()->SetMass( 1 );*/
	SetFriction( 1000.0f );

	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch( &CBullet::BoltTouch );

	SetThink( &CBullet::BubbleThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
	
	CreateSprites();

	// Make us glow until we've hit the wall
	m_nSkin = BOLT_SKIN_GLOW;
}


void CBullet::Precache( void )
{
	PrecacheModel( BOLT_MODEL );

	// This is used by C_TEStickyBolt, despte being different from above!!!
	//PrecacheModel( "models/crossbow_bolt.mdl" );

	PrecacheModel( "sprites/light_glow02_noz.vmt" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CBullet::BoltTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;

	if ( pOther->m_takedamage != DAMAGE_NO )
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize( vecNormalizedVel );

		float	speed = GetAbsVelocity().Length();
		if( speed < 100 )
			speed = 100;

		float	dmg = (float)m_iDamage * speed * speed / (float)(BOLT_AIR_VELOCITY*BOLT_AIR_VELOCITY),
				dmgforcescale = 100.f / speed;

		Msg( "%f / %f => %f / %f\n", speed, (float)BOLT_AIR_VELOCITY, dmg, (float)m_iDamage );

		if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC() )
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_BULLET | DMG_NEVERGIB );
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), dmg, DMG_BULLET | DMG_NEVERGIB );
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, dmgforcescale );
			dmgInfo.SetDamagePosition( tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}
		else
		{
			//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_BULLET | DMG_NEVERGIB );
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), dmg, DMG_BULLET | DMG_NEVERGIB );
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, dmgforcescale );
			dmgInfo.SetDamagePosition( tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if ( pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
			 return;

		SetAbsVelocity( Vector( 0, 0, 0 ) );

		// play body "thwack" sound
		EmitSound( "Weapon_Pistol.HitBody" );

		Vector vForward;

		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize ( vForward );

		UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2 );

		if ( tr2.fraction != 1.0f )
		{
//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

			if ( tr2.m_pEnt == NULL || ( tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE ) )
			{
				CEffectData	data;

				data.m_vOrigin = tr2.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = tr2.fraction != 1.0f;
			
				//DispatchEffect( "BoltImpact", data );
				//DispatchEffect( "BulletImpact", data );
			}
		}
		
		SetTouch( NULL );
		SetThink( NULL );

		UTIL_Remove( this );
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		if ( pOther->GetMoveType() == MOVETYPE_NONE && !( tr.surface.flags & SURF_SKY ) )
		{
			EmitSound( "Weapon_Pistol.BoltHitWorld" );

			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize( vecDir );

			// See if we should reflect off this surface
			float hitDot = DotProduct( tr.plane.normal, -vecDir );
			
			// BG2 - BP original was( hitDot < 0.5f ) but a musket ball should not bounce off walls if the angle is too big
			if ( ( hitDot < 0.2f ) && ( speed > 100 ) )
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
				SetThink( &CBullet::SUB_Remove );
				SetNextThink( gpGlobals->curtime + 2.0f );
				
				//FIXME: We actually want to stick (with hierarchy) to what we've hit
				SetMoveType( MOVETYPE_NONE );
			
				Vector vForward;

				AngleVectors( GetAbsAngles(), &vForward );
				VectorNormalize ( vForward );

				CEffectData	data;

				data.m_vOrigin = tr.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = 0;
			
				//DispatchEffect( "BoltImpact", data );
				//DispatchEffect( "BulletImpact", data );
				
				UTIL_ImpactTrace( &tr, DMG_BULLET );

				AddEffects( EF_NODRAW );
				SetTouch( NULL );
				SetThink( &CBullet::SUB_Remove );
				SetNextThink( gpGlobals->curtime + 2.0f );

				if ( m_pGlowSprite != NULL )
				{
					m_pGlowSprite->TurnOn();
					m_pGlowSprite->FadeAndDie( 3.0f );
				}
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

	if ( g_pGameRules->IsMultiplayer() )
	{
//		SetThink( &CBullet::ExplodeThink );
//		SetNextThink( gpGlobals->curtime + 0.1f );
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

	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( GetWaterLevel()  == 0 )
	{
		//apply drag
		Vector	vecDir = GetAbsVelocity();
		float	speed = VectorNormalize( vecDir ),
				//drag = 0.0001f;
				drag = 0.002f;

		speed -= drag * speed*speed * gpGlobals->frametime;
		if( speed < 1000 )
			speed = 1000;	//clamp

		SetAbsVelocity( vecDir * speed );

		return;
	}

	UTIL_BubbleTrail( GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5 );
}

#else

/*CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectCrossbow )
CLIENTEFFECT_MATERIAL( "effects/muzzleflash1" )
CLIENTEFFECT_REGISTER_END()*/

extern void DrawHalo( IMaterial* pMaterial, const Vector &source, float scale, float const *color );

//
// Crossbow bolt
//

class C_Bullet : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_Bullet, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();
public:
	
	C_Bullet( void );

	virtual RenderGroup_t GetRenderGroup( void )
	{
		// We want to draw translucent bits as well as our main model
		return RENDER_GROUP_TWOPASS;
	}

	virtual void	ClientThink( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual int		DrawModel( int flags );

private:

	C_Bullet( const C_Bullet & ); // not defined, not accessible

	Vector	m_vecLastOrigin;
	bool	m_bUpdated;
};

IMPLEMENT_CLIENTCLASS_DT( C_Bullet, DT_Bullet, CBullet )
END_RECV_TABLE()

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
	// See if we're drawing the motion blur
	if ( flags & STUDIO_TRANSPARENCY )
	{
		float		color[3];
		IMaterial	*pBlurMaterial = materials->FindMaterial( "effects/muzzleflash1", NULL, false );

		Vector	vecDir = GetAbsOrigin() - m_vecLastOrigin;
		float	speed = VectorNormalize( vecDir );
		
		speed = clamp( speed, 0, 32 );
		
		if ( speed > 0 )
		{
			float	stepSize = min( ( speed * 0.5f ), 4.0f );

			Vector	spawnPos = GetAbsOrigin() + ( vecDir * 24.0f );
			Vector	spawnStep = -vecDir * stepSize;

			materials->Bind( pBlurMaterial );

			float	alpha;

			// Draw the motion blurred trail
			for ( int i = 0; i < 20; i++ )
			{
				spawnPos += spawnStep;

				alpha = RemapValClamped( i, 5, 11, 0.25f, 0.05f );

				color[0] = color[1] = color[2] = alpha;

				DrawHalo( pBlurMaterial, spawnPos, 3.0f, color );
			}
		}

		if ( gpGlobals->frametime > 0.0f && !m_bUpdated)
		{
			m_bUpdated = true;
			m_vecLastOrigin = GetAbsOrigin();
		}

		return 1;
	}

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
static ConVar sv_infiniteammo( "sv_infiniteammo", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Bullet weapons don\'t use up ammo\n" );
static ConVar sv_turboshots( "sv_turboshots", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Turns all guns into machineguns\n" );
static ConVar sv_perfectaim( "sv_perfectaim", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "No spread for bullet weapons\n" );
static ConVar sv_steadyhand( "sv_steadyhand", "0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "No recoil for bullet weapons\n" );

/*int CBaseBG2Weapon::Fire( int iAttack )
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

	SendWeaponAnim( GetActivity( iAttack ) );*//*

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
}*/

int CBaseBG2Weapon::Fire( int iAttack )
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

#ifndef CLIENT_DLL
	Vector vecAiming	= pOwner->GetAutoaimVector( 0 );	
	Vector vecSrc		= pOwner->Weapon_ShootPosition();

	/*QAngle angAiming;
	VectorAngles( vecAiming, angAiming );*/

    CShotManipulator Manipulator( vecAiming );
	Vector vecDir		= Manipulator.ApplySpread( GetSpread( iAttack ) );

	QAngle angDir;
	VectorAngles( vecDir, angDir );

	CBullet *pBolt = CBullet::BoltCreate( vecSrc, angDir, GetDamage(iAttack), pOwner );

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
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.03f;

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
			WeaponSound( MELEE_HIT );
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

		drain = Swing( ATTACK_PRIMARY );
	}
	else if( GetAttackType( ATTACK_PRIMARY ) == ATTACKTYPE_FIREARM )
	{
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

		drain = Swing( ATTACK_SECONDARY );
	}
	else if( GetAttackType( ATTACK_SECONDARY ) == ATTACKTYPE_FIREARM )
	{
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