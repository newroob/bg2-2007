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

const float	LONG_RANGE = 1000 * 3 * 12,		//since damage decreases with range, we can pick a very long range for all weapons, say 1000 yards
			PISTOL_RANGE = LONG_RANGE,		//40 * 3 * 12,		//40 yards
			MUSKET_RANGE = LONG_RANGE,		//175 * 3 * 12,	//175 yards
			RIFLE_RANGE = LONG_RANGE;		//195 * 3 * 12;		//195 yards

const float PISTOL_CONSTANT_DAMAGE_RANGE = 8 * 36,	//8 yards
			MUSKET_CONSTANT_DAMAGE_RANGE = 30 * 36,	//30 yards
			RIFLE_CONSTANT_DAMAGE_RANGE  = 40 * 36;	//40 yards

const int	PISTOL_STAMINA_DRAIN = 0,
			MUSKET_RIFLE_STAMINA_DRAIN = 0,
			MELEE_STAMINA_DRAIN = 25;

//BG2 - Tjoppen - these constants are based on values from weapon_data.h @ BG 1.0F RC14
//they have since been adjusted and normalized to work with a chest damage modifier of 1 instead of 1.85
const int BESS_FIRE_DAMAGE = 111;			//60 * 1.85
const int BESS_BAYONET_DAMAGE = 63;			//43 * 0.8 * 1.85
const int BESS_BAYONET_RANGE = 83;

const int REVOL_FIRE_DAMAGE = 107;			//58 * 1.85
const int REVOL_BAYONET_DAMAGE = 61;		//41 * 0.8 * 1.85 + 1
const int REVOL_BAYONET_RANGE = 75;

const int CHARLE_FIRE_DAMAGE = REVOL_FIRE_DAMAGE;
const int CHARLE_BAYONET_DAMAGE = REVOL_BAYONET_DAMAGE;
const int CHARLE_BAYONET_RANGE = 80;

const int PENNY_FIRE_DAMAGE = BESS_FIRE_DAMAGE;

const int LONGPATTERN_FIRE_DAMAGE = 114;	//62 * 1.85
const int LONGPATTERN_BAYONET_DAMAGE = 66;	//45 * 0.8 * 1.85
const int LONGPATTERN_BAYONET_RANGE = 86;

const int FOWLER_FIRE_DAMAGE = 103;			//damage per ball. 56 * 1.85
const int FOWLER_SHOT_DAMAGE = 18;			//damage per shot. 10 * 1.85 -> 18 -> 8*18 = 144
const int FOWLER_NUM_SHOT = 10;

const int CARBINE_FIRE_DAMAGE = 107;		//damage per ball. between fowler and normal brown bess. 58 * 1.85
const int CARBINE_SHOT_DAMAGE = 20;			//damage per shot. 11 * 1.85 -> 20 -> 8*20 = 160
const int CARBINE_NUM_SHOT = 10;
const int CARBINE_BAYONET_RANGE = 72;

const int JAEGER_FIRE_DAMAGE = 116;			//63 * 1.85

const int PISTOL_FIRE_DAMAGE = 101;			//55 * 1.85

const int KNIFE_DAMAGE = 55;				//30 * 1.85
const int SABRE_DAMAGE = 74;				//40 * 1.85
const int TOMAHAWK_DAMAGE = 79;				//43 * 1.85
const int HIRSCHFAENGER_DAMAGE = KNIFE_DAMAGE;		//same damage as knife

//BG2 - Tjoppen - own constants, interpreted from various places in the BG source
const float SABRE_RANGE = 57.0;
const float HIRSCHFAENGER_RANGE = 57.0;
const float KNIFE_RANGE = 52.0;
const float TOMAHAWK_RANGE = 55.0;

const float BAYONET_COS_TOLERANCE = 0.9961946980917;		//+-5 degrees
const float BAYONET_RETRACE_DURATION = 0.1;

ConVar sv_bayonet_angle_tolerance("sv_bayonet_angle_tolerance", "5", FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_CHEAT, "How many angles the sights are allowed to move before retracing stops");
ConVar sv_bayonet_retrace_duration("sv_bayonet_retrace_duration", "0.1", FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_CHEAT, "How long bayonet melee traces happen for");

const float KNIFE_COS_TOLERANCE				= 0.940f;		//+-20 degrees
const float KNIFE_RETRACE_DURATION			= 0.2f;

const float TOMAHAWK_COS_TOLERANCE			= 0.906f;		//+-25 degrees
const float TOMAHAWK_RETRACE_DURATION		= 0.2f;

const float HIRSCHFAENGER_COS_TOLERANCE		= 0.906f;		//+-25 degrees
const float HIRSCHFAENGER_RETRACE_DURATION	= 0.2f;

const float SABRE_COS_TOLERANCE				= 0.866f;		//+-30 degrees
const float SABRE_RETRACE_DURATION			= 0.25f;

const float MUZZLE_VELOCITY_RIFLE = 20400;
const float MUZZLE_VELOCITY_SMOOTHBORE = 15600;
const float MUZZLE_VELOCITY_BUCKSHOT = 12000;	//TODO: implement different muzzle velocity for buckshot
const float MUZZLE_VELOCITY_PISTOL = 10800;

const float ZERO_RANGE_PISTOL = 10 * 36;	//ten yards
const float ZERO_RANGE_MUSKET = 50 * 36;	//fifty yards
const float ZERO_RANGE_RIFLE = 130 * 36;	//two hundred yards

#ifdef CLIENT_DLL
#define CWeaponbrownbess C_Weaponbrownbess
#endif
DECLARE_BG2_WEAPON( brownbess )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= BESS_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 12.0f;
	m_Attackinfos[0].m_flCrouchStill		= 3.6f; //2.4
	m_Attackinfos[0].m_flStandMoving		= 13.2f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.6f; //2.4
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.4f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.8f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.4f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 8.0f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_STAB;
	m_Attackinfos[1].m_iDamage				= BESS_BAYONET_DAMAGE;//60;
	m_Attackinfos[1].m_flAttackrate			= 1.0f;//-0.7f;
	m_Attackinfos[1].m_flRange				= BESS_BAYONET_RANGE;
	//m_Attackinfos[1].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[1].m_iAttackActivity		= ACT_VM_SECONDARYATTACK;
	m_Attackinfos[1].m_iAttackActivityEmpty	= ACT_VM_SECONDARYATTACK_EMPTY;
	m_Attackinfos[1].m_flCosAngleTolerance  = BAYONET_COS_TOLERANCE;
	m_Attackinfos[1].m_flRetraceDuration    = BAYONET_RETRACE_DURATION;
	m_Attackinfos[1].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_pBayonetDeathNotice = "brownbess_bayonet";

	m_fMinRange2	= 0;
	m_fMaxRange2	= BESS_BAYONET_RANGE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flZeroRange = ZERO_RANGE_MUSKET;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( brownbess )
#endif

#ifdef CLIENT_DLL
#define CWeaponcharleville C_Weaponcharleville
#endif
DECLARE_BG2_WEAPON( charleville )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= CHARLE_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.6;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 11.5f;
	m_Attackinfos[0].m_flCrouchStill		= 3.4f;
	m_Attackinfos[0].m_flStandMoving		= 12.7f; //11.5f;
	m_Attackinfos[0].m_flStandStill			= 3.4f;
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.2f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.5f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.2f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 7.7f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_STAB;
	m_Attackinfos[1].m_iDamage				= CHARLE_BAYONET_DAMAGE;//60;
	m_Attackinfos[1].m_flAttackrate			= 1.0f;//-0.7f;
	m_Attackinfos[1].m_flRange				= CHARLE_BAYONET_RANGE;
	//m_Attackinfos[1].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[1].m_iAttackActivity		= ACT_VM_SECONDARYATTACK;
	m_Attackinfos[1].m_iAttackActivityEmpty	= ACT_VM_SECONDARYATTACK_EMPTY;
	m_Attackinfos[1].m_flCosAngleTolerance  = BAYONET_COS_TOLERANCE;
	m_Attackinfos[1].m_flRetraceDuration    = BAYONET_RETRACE_DURATION;
	m_Attackinfos[1].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_pBayonetDeathNotice = "charleville_bayonet";

	m_fMinRange2	= 0;
	m_fMaxRange2	= CHARLE_BAYONET_RANGE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flZeroRange = ZERO_RANGE_MUSKET;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( charleville )
#endif

//j�ger rifle, but spelled jaeger to avoid any charset problems
#ifdef CLIENT_DLL
#define CWeaponjaeger C_Weaponjaeger
#endif
DECLARE_BG2_WEAPON( jaeger )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= JAEGER_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.9;
	m_Attackinfos[0].m_flRange				= RIFLE_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 10.0f;
	m_Attackinfos[0].m_flCrouchStill		= 2.4f; //0.83
	m_Attackinfos[0].m_flStandMoving		= 11.0f; //8.0f
	m_Attackinfos[0].m_flStandStill			= 3.0f; //2.0
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.0f;	
	m_Attackinfos[0].m_flStandAimMoving		= 7.3f;
	m_Attackinfos[0].m_flCrouchAimStill		= 0.83f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 6.7f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= RIFLE_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 0.75;			//rifle
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= RIFLE_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_NONE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_RIFLE;
	m_flZeroRange = ZERO_RANGE_RIFLE;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
RIFLE_ACTTABLE( jaeger )
#endif


#ifdef CLIENT_DLL
#define CWeaponpennsylvania C_Weaponpennsylvania
#endif
DECLARE_BG2_WEAPON( pennsylvania )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -3;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= PENNY_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 1.0;
	m_Attackinfos[0].m_flRange				= RIFLE_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 10.0f;
	m_Attackinfos[0].m_flCrouchStill		= 2.3f; //0.75
	m_Attackinfos[0].m_flStandMoving		= 12.0f; //8.0f
	m_Attackinfos[0].m_flStandStill			= 3.0f; //1.75
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 1.65f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.0f;
	m_Attackinfos[0].m_flCrouchAimStill		= 0.65f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 6.7f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= RIFLE_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 0.75;			//rifle
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= RIFLE_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_NONE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_RIFLE;
	m_flZeroRange = ZERO_RANGE_RIFLE;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
RIFLE_ACTTABLE( pennsylvania )
#endif

#ifdef CLIENT_DLL
#define CWeaponpistol_a C_Weaponpistol_a
#endif
DECLARE_BG2_WEAPON( pistol_a )
{
	m_fHolsterTime = 0.50f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= PISTOL_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= PISTOL_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 7.5f;
	m_Attackinfos[0].m_flCrouchStill		= 6.0f;
	m_Attackinfos[0].m_flStandMoving		= 9.0f;
	m_Attackinfos[0].m_flStandStill			= 6.0f; 
	m_Attackinfos[0].m_flConstantDamageRange= PISTOL_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.25;			//pistol
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= PISTOL_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= PISTOL_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_NONE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_PISTOL;
	m_flZeroRange = ZERO_RANGE_PISTOL;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
PISTOL_ACTTABLE( pistol_a )
#endif

#ifdef CLIENT_DLL
#define CWeaponpistol_b C_Weaponpistol_b
#endif
DECLARE_BG2_WEAPON( pistol_b )
{
	m_fHolsterTime = 0.50f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= PISTOL_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= PISTOL_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 7.5f;
	m_Attackinfos[0].m_flCrouchStill		= 6.0f;
	m_Attackinfos[0].m_flStandMoving		= 9.0f;
	m_Attackinfos[0].m_flStandStill			= 6.0f; 
	m_Attackinfos[0].m_flConstantDamageRange= PISTOL_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.25;			//pistol
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= PISTOL_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= PISTOL_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_NONE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_PISTOL;
	m_flZeroRange = ZERO_RANGE_PISTOL;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
PISTOL_ACTTABLE( pistol_b )
#endif

#ifdef CLIENT_DLL
#define CWeaponsabre_a C_Weaponsabre_a
#endif
DECLARE_BG2_WEAPON( sabre_a )
{
	m_fHolsterTime = 0.50f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_iDamage				= SABRE_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= SABRE_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_flCosAngleTolerance  = SABRE_COS_TOLERANCE;
	m_Attackinfos[0].m_flRetraceDuration    = SABRE_RETRACE_DURATION;
	m_Attackinfos[0].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_fMinRange1 = m_fMinRange2	= 0;
	m_fMaxRange1 = m_fMaxRange2 = SABRE_RANGE;

	//secondary
	m_Attackinfos[1] = m_Attackinfos[0];
}

#ifndef CLIENT_DLL
MELEE_ACTTABLE( sabre_a )
#endif

#ifdef CLIENT_DLL
#define CWeaponsabre_b C_Weaponsabre_b
#endif
DECLARE_BG2_WEAPON( sabre_b )
{
	m_fHolsterTime = 0.50f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_iDamage				= SABRE_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= SABRE_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_flCosAngleTolerance  = SABRE_COS_TOLERANCE;
	m_Attackinfos[0].m_flRetraceDuration    = SABRE_RETRACE_DURATION;
	m_Attackinfos[0].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_fMinRange1 = m_fMinRange2	= 0;
	m_fMaxRange1 = m_fMaxRange2 = SABRE_RANGE;

	//secondary
	m_Attackinfos[1] = m_Attackinfos[0];
}

#ifndef CLIENT_DLL
MELEE_ACTTABLE( sabre_b )
#endif

#ifdef CLIENT_DLL
#define CWeaponknife C_Weaponknife
#endif
DECLARE_BG2_WEAPON( knife )
{
	m_fHolsterTime = 0.75f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_iDamage				= KNIFE_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.1;//-0.7f;
	m_Attackinfos[0].m_flRange				= KNIFE_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_flCosAngleTolerance  = KNIFE_COS_TOLERANCE;
	m_Attackinfos[0].m_flRetraceDuration    = KNIFE_RETRACE_DURATION;
	m_Attackinfos[0].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_fMinRange1 = m_fMinRange2	= 0;
	m_fMaxRange1 = m_fMaxRange2 = KNIFE_RANGE;

	//secondary
	m_Attackinfos[1] = m_Attackinfos[0];
}

#ifndef CLIENT_DLL
MELEE_ACTTABLE( knife )
#endif

#ifdef CLIENT_DLL
#define CWeaponhirschf C_Weaponhirschf
#endif
DECLARE_BG2_WEAPON( hirschf )
{
	m_fHolsterTime = 0.75f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_iDamage				= HIRSCHFAENGER_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= HIRSCHFAENGER_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_flCosAngleTolerance  = HIRSCHFAENGER_COS_TOLERANCE;
	m_Attackinfos[0].m_flRetraceDuration    = HIRSCHFAENGER_RETRACE_DURATION;
	m_Attackinfos[0].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_fMinRange1 = m_fMinRange2	= 0;
	m_fMaxRange1 = m_fMaxRange2 = HIRSCHFAENGER_RANGE;

	//secondary
	m_Attackinfos[1] = m_Attackinfos[0];
}
#ifndef CLIENT_DLL
MELEE_ACTTABLE( hirschf )
#endif

//Those below are new in 1.3b----------------------

#ifdef CLIENT_DLL
#define CWeapontomahawk C_Weapontomahawk
#endif
DECLARE_BG2_WEAPON( tomahawk )
{
	m_fHolsterTime = 0.75f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_iDamage				= TOMAHAWK_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= TOMAHAWK_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_ATTACK;
	m_Attackinfos[0].m_flCosAngleTolerance  = TOMAHAWK_COS_TOLERANCE;
	m_Attackinfos[0].m_flRetraceDuration    = TOMAHAWK_RETRACE_DURATION;
	m_Attackinfos[0].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_fMinRange1 = m_fMinRange2	= 0;
	m_fMaxRange1 = m_fMaxRange2 = TOMAHAWK_RANGE;

	//secondary
	m_Attackinfos[1] = m_Attackinfos[0];
}

#ifndef CLIENT_DLL
MELEE_ACTTABLE( tomahawk )
#endif

#ifdef CLIENT_DLL
#define CWeaponrevolutionnaire C_Weaponrevolutionnaire
#endif
DECLARE_BG2_WEAPON( revolutionnaire )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= REVOL_FIRE_DAMAGE;//75
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 11.3f;
	m_Attackinfos[0].m_flCrouchStill		= 3.2f;  //2.4
	m_Attackinfos[0].m_flStandMoving		= 12.5f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.2f; //2.4
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.15f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.3f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.15f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 7.5f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_STAB;
	m_Attackinfos[1].m_iDamage				= REVOL_BAYONET_DAMAGE;
	m_Attackinfos[1].m_flAttackrate			= 1.0f;
	m_Attackinfos[1].m_flRange				= REVOL_BAYONET_RANGE;
	//m_Attackinfos[1].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[1].m_iAttackActivity		= ACT_VM_SECONDARYATTACK;
	m_Attackinfos[1].m_iAttackActivityEmpty	= ACT_VM_SECONDARYATTACK_EMPTY;
	m_Attackinfos[1].m_flCosAngleTolerance  = BAYONET_COS_TOLERANCE;
	m_Attackinfos[1].m_flRetraceDuration    = BAYONET_RETRACE_DURATION;
	m_Attackinfos[1].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_pBayonetDeathNotice = "revolutionnaire_bayonet";

	m_fMinRange2	= 0;
	m_fMaxRange2	= REVOL_BAYONET_RANGE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flZeroRange = ZERO_RANGE_MUSKET;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( revolutionnaire )
#endif

#ifdef CLIENT_DLL //Basically just a Brownbess without the bayo coded in. 
#define CWeaponbrownbess_nobayo C_Weaponbrownbess_nobayo
#endif
DECLARE_BG2_WEAPON( brownbess_nobayo )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= BESS_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 12.0f;
	m_Attackinfos[0].m_flCrouchStill		= 3.6f; //2.4
	m_Attackinfos[0].m_flStandMoving		= 13.2f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.6f; //2.4
	//Iron Sights.
	m_Attackinfos[0].m_flStandAimStill		= 2.4f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.8f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.4f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 8.0f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flZeroRange = ZERO_RANGE_MUSKET;

	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( brownbess_nobayo )
#endif

#ifdef CLIENT_DLL
#define CWeaponbeltaxe C_Weaponbeltaxe
#endif
DECLARE_BG2_WEAPON( beltaxe )
{
	m_fHolsterTime = 0.75f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_iDamage				= TOMAHAWK_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= TOMAHAWK_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_flCosAngleTolerance  = TOMAHAWK_COS_TOLERANCE;
	m_Attackinfos[0].m_flRetraceDuration    = TOMAHAWK_RETRACE_DURATION;
	m_Attackinfos[0].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_fMinRange1 = m_fMinRange2	= 0;
	m_fMaxRange1 = m_fMaxRange2 = TOMAHAWK_RANGE;

	//secondary
	m_Attackinfos[1] = m_Attackinfos[0];
}
#ifndef CLIENT_DLL
MELEE_ACTTABLE( beltaxe )
#endif

#ifdef CLIENT_DLL
#define CWeaponfowler C_Weaponfowler
#endif
DECLARE_BG2_WEAPON( fowler )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= FOWLER_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 12.1f;
	m_Attackinfos[0].m_flCrouchStill		= 3.4f;
	m_Attackinfos[0].m_flStandMoving		= 13.3f;
	m_Attackinfos[0].m_flStandStill			= 3.4f;
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.05f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.2f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.05f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 7.4f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	m_flShotAimModifier = -1.0f;
	m_flShotSpread = 6.95f * 0.75f;		//4 m spread at 33 m -> (4 / 2) / 33 / sin(0.5)
	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flShotMuzzleVelocity = MUZZLE_VELOCITY_BUCKSHOT;
	m_flZeroRange = ZERO_RANGE_MUSKET;
	m_iNumShot = FOWLER_NUM_SHOT;
	m_iDamagePerShot = FOWLER_SHOT_DAMAGE;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( fowler )
#endif

#ifdef CLIENT_DLL
#define CWeaponlongpattern C_Weaponlongpattern
#endif
DECLARE_BG2_WEAPON( longpattern )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= LONGPATTERN_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 11.0f;
	m_Attackinfos[0].m_flCrouchStill		= 3.5f; //2.4
	m_Attackinfos[0].m_flStandMoving		= 12.2f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.5f; //2.4
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.3f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.1f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.3f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 7.3f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_STAB;
	m_Attackinfos[1].m_iDamage				= LONGPATTERN_BAYONET_DAMAGE;//60;
	m_Attackinfos[1].m_flAttackrate			= 1.2f;//-0.7f;
	m_Attackinfos[1].m_flRange				= LONGPATTERN_BAYONET_RANGE;
	//m_Attackinfos[1].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[1].m_iAttackActivity		= ACT_VM_SECONDARYATTACK;
	m_Attackinfos[1].m_iAttackActivityEmpty	= ACT_VM_SECONDARYATTACK_EMPTY;
	m_Attackinfos[1].m_flCosAngleTolerance  = BAYONET_COS_TOLERANCE;
	m_Attackinfos[1].m_flRetraceDuration    = BAYONET_RETRACE_DURATION;
	m_Attackinfos[1].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_pBayonetDeathNotice = "longpattern_bayonet";

	m_fMinRange2	= 0;
	m_fMaxRange2	= LONGPATTERN_BAYONET_RANGE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flZeroRange = ZERO_RANGE_MUSKET;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( longpattern )
#endif

#ifdef CLIENT_DLL
#define CWeaponlongpattern_nobayo C_Weaponlongpattern_nobayo
#endif
DECLARE_BG2_WEAPON( longpattern_nobayo )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= LONGPATTERN_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 11.0f;
	m_Attackinfos[0].m_flCrouchStill		= 3.5f; //2.4
	m_Attackinfos[0].m_flStandMoving		= 12.2f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.5f; //2.4
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.3f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.1f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.3f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 7.3f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flZeroRange = ZERO_RANGE_MUSKET;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( longpattern_nobayo )
#endif


#ifdef CLIENT_DLL
#define CWeaponamerican_brownbess C_Weaponamerican_brownbess
#endif
DECLARE_BG2_WEAPON( american_brownbess )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= BESS_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 12.1f;
	m_Attackinfos[0].m_flCrouchStill		= 3.7f; //2.4
	m_Attackinfos[0].m_flStandMoving		= 13.3f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.7f; //2.4
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.5f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.9f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.5f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 8.1f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_STAB;
	m_Attackinfos[1].m_iDamage				= BESS_BAYONET_DAMAGE;//60;
	m_Attackinfos[1].m_flAttackrate			= 1.0f;//-0.7f;
	m_Attackinfos[1].m_flRange				= BESS_BAYONET_RANGE;
	//m_Attackinfos[1].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[1].m_iAttackActivity		= ACT_VM_SECONDARYATTACK;
	m_Attackinfos[1].m_iAttackActivityEmpty	= ACT_VM_SECONDARYATTACK_EMPTY;
	m_Attackinfos[1].m_flCosAngleTolerance  = BAYONET_COS_TOLERANCE;
	m_Attackinfos[1].m_flRetraceDuration    = BAYONET_RETRACE_DURATION;
	m_Attackinfos[1].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_pBayonetDeathNotice = "brownbess_bayonet";

	m_fMinRange2	= 0;
	m_fMaxRange2	= BESS_BAYONET_RANGE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flZeroRange = ZERO_RANGE_MUSKET;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( american_brownbess )
#endif

#ifdef CLIENT_DLL 
#define CWeaponamerican_brownbess_nobayo C_Weaponamerican_brownbess_nobayo
#endif
DECLARE_BG2_WEAPON( american_brownbess_nobayo )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= BESS_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 12.0f;
	m_Attackinfos[0].m_flCrouchStill		= 3.6f; //2.4
	m_Attackinfos[0].m_flStandMoving		= 13.2f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.6f; //2.4
	//Iron Sights.
	m_Attackinfos[0].m_flStandAimStill		= 2.4f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.8f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.4f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 8.0f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flZeroRange = ZERO_RANGE_MUSKET;
	m_iNumShot = 0;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( american_brownbess_nobayo )
#endif

#ifdef CLIENT_DLL 
#define CWeaponbrownbess_carbine C_Weaponbrownbess_carbine
#endif
DECLARE_BG2_WEAPON( brownbess_carbine )
{
	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	flIronsightFOVOffset		= -2.5;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_iDamage				= CARBINE_FIRE_DAMAGE;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 12.0f;
	m_Attackinfos[0].m_flCrouchStill		= 3.6f;
	m_Attackinfos[0].m_flStandMoving		= 13.2f;
	m_Attackinfos[0].m_flStandStill			= 3.6f;
	//Iron Sights.
	m_Attackinfos[0].m_flStandAimStill		= 2.6f;	
	m_Attackinfos[0].m_flStandAimMoving		= 8.1f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.6f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 7.3f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= MUSKET_CONSTANT_DAMAGE_RANGE;
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;
	m_Attackinfos[0].m_iStaminaDrain		= MUSKET_RIFLE_STAMINA_DRAIN;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_STAB;
	m_Attackinfos[1].m_iDamage				= BESS_BAYONET_DAMAGE;//60;
	m_Attackinfos[1].m_flAttackrate			= 1.0f;//-0.7f;
	m_Attackinfos[1].m_flRange				= CARBINE_BAYONET_RANGE;
	//m_Attackinfos[1].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[1].m_iAttackActivity		= ACT_VM_SECONDARYATTACK;
	m_Attackinfos[1].m_iAttackActivityEmpty	= ACT_VM_SECONDARYATTACK_EMPTY;
	m_Attackinfos[1].m_flCosAngleTolerance  = BAYONET_COS_TOLERANCE;
	m_Attackinfos[1].m_flRetraceDuration    = BAYONET_RETRACE_DURATION;
	m_Attackinfos[1].m_iStaminaDrain		= MELEE_STAMINA_DRAIN;

	m_pBayonetDeathNotice = "brownbess_bayonet";

	m_fMinRange2	= 0;
	m_fMaxRange2	= CARBINE_BAYONET_RANGE;

	m_flShotAimModifier = -1.0f;
	m_flShotSpread = 7.64f * 0.75f;		//4 m spread at 30 m -> (4 / 2) / 30 / sin(0.5)
	m_flMuzzleVelocity = MUZZLE_VELOCITY_SMOOTHBORE;
	m_flShotMuzzleVelocity = MUZZLE_VELOCITY_BUCKSHOT;
	m_flZeroRange = ZERO_RANGE_MUSKET;
	m_iNumShot = CARBINE_NUM_SHOT;
	m_iDamagePerShot = CARBINE_SHOT_DAMAGE;
}

#ifndef CLIENT_DLL
CARBINE_ACTTABLE( brownbess_carbine )
#endif
