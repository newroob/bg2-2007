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
		Tomas "Tjoppen" Härdin		tjoppen@gamedtivityList
		ev.se

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

//BG2 - Tjoppen - these constants from weapon_data.h @ BG 1.0F RC14
// damage values for each attack on each weapon
const float BESS_FIRE_DAMAGE = 60.0;
const float BESS_BAYONET_DAMAGE = 43.0;

const float REVOL_FIRE_DAMAGE = 58.0;
const float REVOL_BAYONET_DAMAGE = 41.0;

//BG2 - Tjoppen - roundoff errors makes me have to increase CHARLE_FIRE_DAMAGE by 0.5
const float CHARLE_FIRE_DAMAGE = 58.0;//57.5;
const float CHARLE_BAYONET_DAMAGE = 41.0;

const float PENNY_FIRE_DAMAGE = 60.0;
const float JAEGER_FIRE_DAMAGE = 63.0;

const float PISTOLA_FIRE_DAMAGE = 40.0;
const float PISTOLB_FIRE_DAMAGE = 40.0;

const float KNIFE_DAMAGE = 30.0;
const float SABRE_DAMAGE = 40.0; //38
const float TOMAHAWK_DAMAGE = 43;  //45
const float HIRSCHFAENGER_DAMAGE = KNIFE_DAMAGE;		//same damage as knife

//BG2 - Tjoppen - own constants, interpreted from various places in the BG source
const float BESS_BAYONET_RANGE = 83.0; //90
//const float REVOL_BAYONET_RANGE = 75.0;
const float CHARLE_BAYONET_RANGE = 80.0; //87
const float SABRE_RANGE = 57.0;
const float HIRSCHFAENGER_RANGE = 57.0;
const float KNIFE_RANGE = 52.0;
const float TOMAHAWK_RANGE = 55.0; //66

#ifdef CLIENT_DLL
#define CWeaponbrownbess C_Weaponbrownbess
#endif
DECLARE_BG2_WEAPON( brownbess )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	vecIronsightPosOffset.x		= -17.25; //forward
	vecIronsightPosOffset.y		= -7.75; //right
	vecIronsightPosOffset.z		= 6.8; //up
 
	angIronsightAngOffset[PITCH]	= -0.1;
	angIronsightAngOffset[YAW]		= 0;
	angIronsightAngOffset[ROLL]		= 0;
	flIronsightFOVOffset		= 0;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_flDamage				= BESS_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 12.0f;
	m_Attackinfos[0].m_flCrouchStill		= 3.6f; //2.4
	m_Attackinfos[0].m_flStandMoving		= 13.2f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.6f; //2.4
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.4f;	
	m_Attackinfos[0].m_flStandAimMoving		= 3.0f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.4f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 3.0f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= 20.0 * 36.0;	//20 yards
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_STAB;
	m_Attackinfos[1].m_flDamage				= BESS_BAYONET_DAMAGE;//60;
	m_Attackinfos[1].m_flAttackrate			= 1.0f;//-0.7f;
	m_Attackinfos[1].m_flRange				= BESS_BAYONET_RANGE;
	//m_Attackinfos[1].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[1].m_iAttackActivity		= ACT_VM_SECONDARYATTACK;
	m_Attackinfos[1].m_iAttackActivityEmpty	= ACT_VM_SECONDARYATTACK_EMPTY;

	m_pBayonetDeathNotice = "brownbess_bayonet";

	m_fMinRange2	= 0;
	m_fMaxRange2	= BESS_BAYONET_RANGE;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( brownbess )
#endif

#ifdef CLIENT_DLL
#define CWeaponcharleville C_Weaponcharleville
#endif
DECLARE_BG2_WEAPON( charleville )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	vecIronsightPosOffset.x		= -18.25; //forward
	vecIronsightPosOffset.y		= -7.90; //right
	vecIronsightPosOffset.z		= 7.0; //up
 
	angIronsightAngOffset[PITCH]	= 0;
	angIronsightAngOffset[YAW]		= 0;
	angIronsightAngOffset[ROLL]		= 0;
	flIronsightFOVOffset		= 0;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_flDamage				= CHARLE_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.6;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 11.5f;
	m_Attackinfos[0].m_flCrouchStill		= 3.4f;
	m_Attackinfos[0].m_flStandMoving		= 12.7f; //11.5f;
	m_Attackinfos[0].m_flStandStill			= 3.4f;
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.2f;	
	m_Attackinfos[0].m_flStandAimMoving		= 3.0f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.2f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 3.0f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= 20.0 * 36.0;	//20 yards
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_STAB;
	m_Attackinfos[1].m_flDamage				= CHARLE_BAYONET_DAMAGE;//60;
	m_Attackinfos[1].m_flAttackrate			= 1.0f;//-0.7f;
	m_Attackinfos[1].m_flRange				= CHARLE_BAYONET_RANGE;
	//m_Attackinfos[1].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[1].m_iAttackActivity		= ACT_VM_SECONDARYATTACK;
	m_Attackinfos[1].m_iAttackActivityEmpty	= ACT_VM_SECONDARYATTACK_EMPTY;

	m_pBayonetDeathNotice = "charleville_bayonet";

	m_fMinRange2	= 0;
	m_fMaxRange2	= CHARLE_BAYONET_RANGE;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( charleville )
#endif

//jäger rifle, but spelled jaeger to avoid any charset problems
#ifdef CLIENT_DLL
#define CWeaponjaeger C_Weaponjaeger
#endif
DECLARE_BG2_WEAPON( jaeger )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	vecIronsightPosOffset.x		= -17.55; //forward
	vecIronsightPosOffset.y		= -7.64; //right
	vecIronsightPosOffset.z		= 7.15; //up
 
	angIronsightAngOffset[PITCH]	= 0;
	angIronsightAngOffset[YAW]		= 0.14;
	angIronsightAngOffset[ROLL]		= 0;
	flIronsightFOVOffset		= 0;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_flDamage				= JAEGER_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.9;
	m_Attackinfos[0].m_flRange				= RIFLE_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 10.0f;
	m_Attackinfos[0].m_flCrouchStill		= 2.4f; //0.83
	m_Attackinfos[0].m_flStandMoving		= 11.0f; //8.0f
	m_Attackinfos[0].m_flStandStill			= 3.0f; //2.0
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.0f;	
	m_Attackinfos[0].m_flStandAimMoving		= 4.0f;
	m_Attackinfos[0].m_flCrouchAimStill		= 0.83f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 3.0f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= 30.0 * 36.0;	//25 yards
	m_Attackinfos[0].m_flRelativeDrag		= 0.75;			//rifle
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

	m_fMinRange1	= 0;
	m_fMaxRange1	= RIFLE_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_NONE;
}

#ifndef CLIENT_DLL
RIFLE_ACTTABLE( jaeger )
#endif


#ifdef CLIENT_DLL
#define CWeaponpennsylvania C_Weaponpennsylvania
#endif
DECLARE_BG2_WEAPON( pennsylvania )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	vecIronsightPosOffset.x		= -17.15; //forward
	vecIronsightPosOffset.y		= -8.05; //right
	vecIronsightPosOffset.z		= 6.7; //up
 
	angIronsightAngOffset[PITCH]	= 0;
	angIronsightAngOffset[YAW]		= 0;
	angIronsightAngOffset[ROLL]		= 0;
	flIronsightFOVOffset		= 0;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_flDamage				= PENNY_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 1.0;
	m_Attackinfos[0].m_flRange				= RIFLE_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 10.0f;
	m_Attackinfos[0].m_flCrouchStill		= 2.3f; //0.75
	m_Attackinfos[0].m_flStandMoving		= 12.0f; //8.0f
	m_Attackinfos[0].m_flStandStill			= 3.0f; //1.75
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 1.75f;	
	m_Attackinfos[0].m_flStandAimMoving		= 3.8f;
	m_Attackinfos[0].m_flCrouchAimStill		= 0.75f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 3.0f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= 30.0 * 36.0;	//25 yards
	m_Attackinfos[0].m_flRelativeDrag		= 0.75;			//rifle
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

	m_fMinRange1	= 0;
	m_fMaxRange1	= RIFLE_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_NONE;
}

#ifndef CLIENT_DLL
RIFLE_ACTTABLE( pennsylvania )
#endif

#ifdef CLIENT_DLL
#define CWeaponpistol_a C_Weaponpistol_a
#endif
DECLARE_BG2_WEAPON( pistol_a )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_fHolsterTime = 0.50f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_flDamage				= PISTOLA_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= PISTOL_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 7.5f;
	m_Attackinfos[0].m_flCrouchStill		= 6.0f;
	m_Attackinfos[0].m_flStandMoving		= 9.0f;
	m_Attackinfos[0].m_flStandStill			= 6.0f; 
	m_Attackinfos[0].m_flConstantDamageRange= 15.0 * 36.0;	//15 yards
	m_Attackinfos[0].m_flRelativeDrag		= 1.25;			//pistol
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

	m_fMinRange1	= 0;
	m_fMaxRange1	= PISTOL_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_NONE;
}

#ifndef CLIENT_DLL
PISTOL_ACTTABLE( pistol_a )
#endif

#ifdef CLIENT_DLL
#define CWeaponpistol_b C_Weaponpistol_b
#endif
DECLARE_BG2_WEAPON( pistol_b )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_fHolsterTime = 0.50f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_flDamage				= PISTOLB_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= PISTOL_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 7.5f;
	m_Attackinfos[0].m_flCrouchStill		= 6.0f;
	m_Attackinfos[0].m_flStandMoving		= 9.0f;
	m_Attackinfos[0].m_flStandStill			= 6.0f; 
	m_Attackinfos[0].m_flConstantDamageRange= 15.0 * 36.0;	//15 yards
	m_Attackinfos[0].m_flRelativeDrag		= 1.25;			//pistol
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

	m_fMinRange1	= 0;
	m_fMaxRange1	= PISTOL_RANGE;

	//secondary
	m_Attackinfos[1].m_iAttacktype			= ATTACKTYPE_NONE;
}

#ifndef CLIENT_DLL
PISTOL_ACTTABLE( pistol_b )
#endif

#ifdef CLIENT_DLL
#define CWeaponsabre_a C_Weaponsabre_a
#endif
DECLARE_BG2_WEAPON( sabre_a )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_fHolsterTime = 0.50f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_flDamage				= SABRE_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= SABRE_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

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
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_fHolsterTime = 0.50f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_flDamage				= SABRE_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= SABRE_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

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
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_fHolsterTime = 0.75f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_flDamage				= KNIFE_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.1;//-0.7f;
	m_Attackinfos[0].m_flRange				= KNIFE_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

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
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_fHolsterTime = 0.75f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_flDamage				= HIRSCHFAENGER_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= HIRSCHFAENGER_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

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
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_fHolsterTime = 0.75f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_flDamage				= TOMAHAWK_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= TOMAHAWK_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_ATTACK;

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
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	vecIronsightPosOffset.x		= -19.90; //forward
	vecIronsightPosOffset.y		= -7.90; //right
	vecIronsightPosOffset.z		= 6.72; //up
 
	angIronsightAngOffset[PITCH]	= -0.36;
	angIronsightAngOffset[YAW]		= 0;
	angIronsightAngOffset[ROLL]		= 0;
	flIronsightFOVOffset		= 0;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_flDamage				= REVOL_FIRE_DAMAGE;//75
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 12.0f;
	m_Attackinfos[0].m_flCrouchStill		= 3.4f;  //2.4
	m_Attackinfos[0].m_flStandMoving		= 13.2f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.4f; //2.4
	//Iron Sights. These values will probably be changed.
	m_Attackinfos[0].m_flStandAimStill		= 2.0f;	
	m_Attackinfos[0].m_flStandAimMoving		= 3.5f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.0f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 3.5f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= 25.0 * 36.0;	//20 yards
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( revolutionnaire )
#endif

#ifdef CLIENT_DLL //Basically just a Brownbess without the bayo coded in. The skin for this will be set elsewhere for now.
#define CWeaponbrownbess_nobayo C_Weaponbrownbess_nobayo
#endif
DECLARE_BG2_WEAPON( brownbess_nobayo )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_bCantAbortReload	= true;

	m_fHolsterTime = 0.75f;

	//Iron sights viewmodel settings.
	vecIronsightPosOffset.x		= -17.25; //forward
	vecIronsightPosOffset.y		= -7.75; //right
	vecIronsightPosOffset.z		= 6.8; //up
 
	angIronsightAngOffset[PITCH]	= -0.25;
	angIronsightAngOffset[YAW]		= 0;
	angIronsightAngOffset[ROLL]		= 0;
	flIronsightFOVOffset		= 0;

	m_bWeaponHasSights = true; 
	//

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_FIREARM;
	m_Attackinfos[0].m_flDamage				= BESS_FIRE_DAMAGE;//75;
	m_Attackinfos[0].m_flAttackrate			= 1.0;
	m_Attackinfos[0].m_flRecoil				= 0.7;
	m_Attackinfos[0].m_flRange				= MUSKET_RANGE;
	m_Attackinfos[0].m_flCrouchMoving		= 12.0f;
	m_Attackinfos[0].m_flCrouchStill		= 3.6f; //2.4
	m_Attackinfos[0].m_flStandMoving		= 13.2f; //12.0f
	m_Attackinfos[0].m_flStandStill			= 3.6f; //2.4
	//Iron Sights.
	m_Attackinfos[0].m_flStandAimStill		= 2.4f;	
	m_Attackinfos[0].m_flStandAimMoving		= 3.0f;
	m_Attackinfos[0].m_flCrouchAimStill		= 2.4f;
	m_Attackinfos[0].m_flCrouchAimMoving	= 3.0f;
	//
	m_Attackinfos[0].m_flConstantDamageRange= 20.0 * 36.0;	//20 yards
	m_Attackinfos[0].m_flRelativeDrag		= 1.0;			//musket
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

	m_fMinRange1	= 0;
	m_fMaxRange1	= MUSKET_RANGE;
}

#ifndef CLIENT_DLL
MUSKET_ACTTABLE( brownbess_nobayo )
#endif

#ifdef CLIENT_DLL
#define CWeaponbeltaxe C_Weaponbeltaxe
#endif
DECLARE_BG2_WEAPON( beltaxe )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bDontAutoreload	= true;

	m_fHolsterTime = 0.75f;

	m_bWeaponHasSights = false; 

	//primary
	m_Attackinfos[0].m_iAttacktype			= ATTACKTYPE_SLASH;
	m_Attackinfos[0].m_flDamage				= TOMAHAWK_DAMAGE;//60;
	m_Attackinfos[0].m_flAttackrate			= 1.4;//-0.7f;
	m_Attackinfos[0].m_flRange				= TOMAHAWK_RANGE;
	//m_Attackinfos[0].m_flCosAngleTolerance	= 0.95f;
	m_Attackinfos[0].m_iAttackActivity		= ACT_VM_PRIMARYATTACK;

	m_fMinRange1 = m_fMinRange2	= 0;
	m_fMaxRange1 = m_fMaxRange2 = TOMAHAWK_RANGE;

	//secondary
	m_Attackinfos[1] = m_Attackinfos[0];
}
#ifndef CLIENT_DLL
MELEE_ACTTABLE( beltaxe )
#endif