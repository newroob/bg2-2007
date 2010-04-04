//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef AI_ACTIVITY_H
#define AI_ACTIVITY_H
#ifdef _WIN32
#pragma once
#endif

#define ACTIVITY_NOT_AVAILABLE		-1

typedef enum
{
	ACT_INVALID = -1,			// So we have something more succint to check for than '-1'
	ACT_RESET = 0,				// Set m_Activity to this invalid value to force a reset to m_IdealActivity
	ACT_IDLE,
	ACT_TRANSITION,
	ACT_COVER,					// FIXME: obsolete? redundant with ACT_COVER_LOW?
	ACT_COVER_MED,				// FIXME: unsupported?
	ACT_COVER_LOW,				// FIXME: rename ACT_IDLE_CROUCH?
	ACT_WALK,
	ACT_WALK_AIM,
	ACT_WALK_CROUCH,
	ACT_WALK_CROUCH_AIM,
	ACT_RUN,
	ACT_RUN_AIM,
	ACT_RUN_CROUCH,
	ACT_RUN_CROUCH_AIM,
	ACT_RUN_PROTECTED,
	ACT_SCRIPT_CUSTOM_MOVE,
	ACT_RANGE_ATTACK1,
	ACT_RANGE_ATTACK2,
	ACT_RANGE_ATTACK1_LOW,		// FIXME: not used yet, crouched versions of the range attack
	ACT_RANGE_ATTACK2_LOW,		// FIXME: not used yet, crouched versions of the range attack
	ACT_DIESIMPLE,
	ACT_DIEBACKWARD,
	ACT_DIEFORWARD,
	ACT_DIEVIOLENT,
	ACT_DIERAGDOLL,
	ACT_FLY,				// Fly (and flap if appropriate)
	ACT_HOVER,
	ACT_GLIDE,
	ACT_SWIM,
	ACT_JUMP,
	ACT_HOP,				// vertical jump
	ACT_LEAP,				// long forward jump
	ACT_LAND,
	ACT_CLIMB_UP,
	ACT_CLIMB_DOWN,
	ACT_CLIMB_DISMOUNT,
	ACT_SHIPLADDER_UP,
	ACT_SHIPLADDER_DOWN,
	ACT_STRAFE_LEFT,
	ACT_STRAFE_RIGHT,
	ACT_ROLL_LEFT,			// tuck and roll, left
	ACT_ROLL_RIGHT,			// tuck and roll, right
	ACT_TURN_LEFT,			// turn quickly left (stationary)
	ACT_TURN_RIGHT,			// turn quickly right (stationary)
	ACT_CROUCH,				// FIXME: obsolete? only used be soldier (the act of crouching down from a standing position)
	ACT_CROUCHIDLE,			// FIXME: obsolete? only used be soldier (holding body in crouched position (loops))
	ACT_STAND,				// FIXME: obsolete? should be transition (the act of standing from a crouched position)
	ACT_USE,
	ACT_SIGNAL1,
	ACT_SIGNAL2,
	ACT_SIGNAL3,

	ACT_SIGNAL_ADVANCE,		// Squad handsignals, specific.
	ACT_SIGNAL_FORWARD,
	ACT_SIGNAL_GROUP,
	ACT_SIGNAL_HALT,
	ACT_SIGNAL_LEFT,
	ACT_SIGNAL_RIGHT,
	ACT_SIGNAL_TAKECOVER,

	ACT_LOOKBACK_RIGHT,		// look back over shoulder without turning around.
	ACT_LOOKBACK_LEFT,
	ACT_COWER,				// FIXME: unused, should be more extreme version of crouching
	ACT_SMALL_FLINCH,		// FIXME: needed? shouldn't flinching be down with overlays?
	ACT_BIG_FLINCH,		
	ACT_MELEE_ATTACK1,
	ACT_MELEE_ATTACK2,
	ACT_RELOAD,
	ACT_RELOAD_START,
	ACT_RELOAD_FINISH,
	ACT_RELOAD_LOW,
	ACT_ARM,				// pull out gun, for instance
	ACT_DISARM,				// reholster gun
	ACT_DROP_WEAPON,
	ACT_DROP_WEAPON_SHOTGUN,
	ACT_PICKUP_GROUND,		// pick up something in front of you on the ground
	ACT_PICKUP_RACK,		// pick up something from a rack or shelf in front of you.
	ACT_IDLE_ANGRY,			// FIXME: being used as an combat ready idle?  alternate idle animation in which the monster is clearly agitated. (loop)

	ACT_IDLE_RELAXED,
	ACT_IDLE_STIMULATED,
	ACT_IDLE_AGITATED,
	ACT_IDLE_STEALTH,
	ACT_IDLE_HURT,

	ACT_WALK_RELAXED,
	ACT_WALK_STIMULATED,
	ACT_WALK_AGITATED,
	ACT_WALK_STEALTH,

	ACT_RUN_RELAXED,
	ACT_RUN_STIMULATED,
	ACT_RUN_AGITATED,
	ACT_RUN_STEALTH,
		
	ACT_IDLE_AIM_RELAXED,
	ACT_IDLE_AIM_STIMULATED,
	ACT_IDLE_AIM_AGITATED,
	ACT_IDLE_AIM_STEALTH,

	ACT_WALK_AIM_RELAXED,
	ACT_WALK_AIM_STIMULATED,
	ACT_WALK_AIM_AGITATED,
	ACT_WALK_AIM_STEALTH,

	ACT_RUN_AIM_RELAXED,
	ACT_RUN_AIM_STIMULATED,
	ACT_RUN_AIM_AGITATED,
	ACT_RUN_AIM_STEALTH,

	ACT_CROUCHIDLE_STIMULATED,
	ACT_CROUCHIDLE_AIM_STIMULATED,
	ACT_CROUCHIDLE_AGITATED,

	ACT_WALK_HURT,			// limp  (loop)
	ACT_RUN_HURT,			// limp  (loop)
	ACT_SPECIAL_ATTACK1,	// very monster specific special attacks.
	ACT_SPECIAL_ATTACK2,	
	ACT_COMBAT_IDLE,		// FIXME: unused?  agitated idle.
	ACT_WALK_SCARED,
	ACT_RUN_SCARED,
	ACT_VICTORY_DANCE,		// killed a player, do a victory dance.
	ACT_DIE_HEADSHOT,		// die, hit in head. 
	ACT_DIE_CHESTSHOT,		// die, hit in chest
	ACT_DIE_GUTSHOT,		// die, hit in gut
	ACT_DIE_BACKSHOT,		// die, hit in back
	ACT_FLINCH_HEAD,
	ACT_FLINCH_CHEST,
	ACT_FLINCH_STOMACH,
	ACT_FLINCH_LEFTARM,
	ACT_FLINCH_RIGHTARM,
	ACT_FLINCH_LEFTLEG,
	ACT_FLINCH_RIGHTLEG,
	ACT_FLINCH_PHYSICS,

	ACT_IDLE_ON_FIRE,		// ON FIRE animations
	ACT_WALK_ON_FIRE,
	ACT_RUN_ON_FIRE,		

	ACT_RAPPEL_LOOP,		// Rappel down a rope!

	ACT_180_LEFT,			// 180 degree left turn
	ACT_180_RIGHT,

	ACT_90_LEFT,			// 90 degree turns
	ACT_90_RIGHT,

	ACT_STEP_LEFT,			// Single steps
	ACT_STEP_RIGHT,
	ACT_STEP_BACK,
	ACT_STEP_FORE,

	ACT_GESTURE_RANGE_ATTACK1,
	ACT_GESTURE_RANGE_ATTACK2,
	ACT_GESTURE_MELEE_ATTACK1,
	ACT_GESTURE_MELEE_ATTACK2,
	ACT_GESTURE_RANGE_ATTACK1_LOW,	// FIXME: not used yet, crouched versions of the range attack
	ACT_GESTURE_RANGE_ATTACK2_LOW,	// FIXME: not used yet, crouched versions of the range attack

	ACT_MELEE_ATTACK_SWING_GESTURE,

	ACT_GESTURE_SMALL_FLINCH,
	ACT_GESTURE_BIG_FLINCH,
	ACT_GESTURE_FLINCH_BLAST,			// Startled by an explosion
	ACT_GESTURE_FLINCH_BLAST_SHOTGUN,
	ACT_GESTURE_FLINCH_BLAST_DAMAGED,	// Damaged by an explosion
	ACT_GESTURE_FLINCH_BLAST_DAMAGED_SHOTGUN,
	ACT_GESTURE_FLINCH_HEAD,
	ACT_GESTURE_FLINCH_CHEST,
	ACT_GESTURE_FLINCH_STOMACH,
	ACT_GESTURE_FLINCH_LEFTARM,
	ACT_GESTURE_FLINCH_RIGHTARM,
	ACT_GESTURE_FLINCH_LEFTLEG,
	ACT_GESTURE_FLINCH_RIGHTLEG,

	ACT_GESTURE_TURN_LEFT,
	ACT_GESTURE_TURN_RIGHT,
	ACT_GESTURE_TURN_LEFT45,
	ACT_GESTURE_TURN_RIGHT45,
	ACT_GESTURE_TURN_LEFT90,
	ACT_GESTURE_TURN_RIGHT90,
	ACT_GESTURE_TURN_LEFT45_FLAT,
	ACT_GESTURE_TURN_RIGHT45_FLAT,
	ACT_GESTURE_TURN_LEFT90_FLAT,
	ACT_GESTURE_TURN_RIGHT90_FLAT,

	// HALF-LIFE 1 compatability stuff goes here. Temporary!
	ACT_BARNACLE_HIT,		// barnacle tongue hits a monster
	ACT_BARNACLE_PULL,		// barnacle is lifting the monster ( loop )
	ACT_BARNACLE_CHOMP,		// barnacle latches on to the monster
	ACT_BARNACLE_CHEW,		// barnacle is holding the monster in its mouth ( loop )

	// Sometimes, you just want to set an NPC's sequence to a sequence that doesn't actually
	// have an activity. The AI will reset the NPC's sequence to whatever its IDEAL activity
	// is, though. So if you set ideal activity to DO_NOT_DISTURB, the AI will not interfere
	// with the NPC's current sequence. (SJB)
	ACT_DO_NOT_DISTURB,

	// viewmodel (weapon) activities
	// FIXME: move these to the specific viewmodels, no need to make global
	ACT_VM_DRAW,
	ACT_VM_HOLSTER,
	ACT_VM_IDLE,
	ACT_VM_FIDGET,
	ACT_VM_PULLBACK,
	ACT_VM_PULLBACK_HIGH,
	ACT_VM_PULLBACK_LOW,
	ACT_VM_THROW,
	ACT_VM_PULLPIN,
	ACT_VM_ATTACK,				//tomahawk attack
	ACT_VM_PRIMARYATTACK,		// fire
	ACT_VM_SECONDARYATTACK,		// alt. fire
	ACT_VM_RELOAD,			
	ACT_VM_DRYFIRE,				// fire with no ammo loaded.
	ACT_VM_HITLEFT,				// bludgeon, swing to left - hit (primary attk)
	ACT_VM_HITLEFT2,			// bludgeon, swing to left - hit (secondary attk)
	ACT_VM_HITRIGHT,			// bludgeon, swing to right - hit (primary attk)
	ACT_VM_HITRIGHT2,			// bludgeon, swing to right - hit (secondary attk)
	ACT_VM_HITCENTER,			// bludgeon, swing center - hit (primary attk)
	ACT_VM_HITCENTER2,			// bludgeon, swing center - hit (secondary attk)
	ACT_VM_MISSLEFT,			// bludgeon, swing to left - miss (primary attk)
	ACT_VM_MISSLEFT2,			// bludgeon, swing to left - miss (secondary attk)
	ACT_VM_MISSRIGHT,			// bludgeon, swing to right - miss (primary attk)
	ACT_VM_MISSRIGHT2,			// bludgeon, swing to right - miss (secondary attk)
	ACT_VM_MISSCENTER,			// bludgeon, swing center - miss (primary attk)
	ACT_VM_MISSCENTER2,			// bludgeon, swing center - miss (secondary attk)
	ACT_VM_HAULBACK,			// bludgeon, haul the weapon back for a hard strike (secondary attk)
	ACT_VM_SWINGHARD,			// bludgeon, release the hard strike (secondary attk)
	ACT_VM_SWINGMISS,
	ACT_VM_SWINGHIT,
	ACT_VM_IDLE_TO_LOWERED,
	ACT_VM_IDLE_LOWERED,
	ACT_VM_LOWERED_TO_IDLE,
	ACT_VM_RECOIL1,
	ACT_VM_RECOIL2,
	ACT_VM_RECOIL3,
	ACT_VM_PICKUP,
	ACT_VM_RELEASE,
	
	ACT_VM_ATTACH_SILENCER,
	ACT_VM_DETACH_SILENCER,

//===========================
// HL2 Specific Activities
//===========================
	// SLAM	Specialty Activities
	/*ACT_SLAM_STICKWALL_IDLE,
	ACT_SLAM_STICKWALL_ND_IDLE,
	ACT_SLAM_STICKWALL_ATTACH,
	ACT_SLAM_STICKWALL_ATTACH2,
	ACT_SLAM_STICKWALL_ND_ATTACH,
	ACT_SLAM_STICKWALL_ND_ATTACH2,
	ACT_SLAM_STICKWALL_DETONATE,
	ACT_SLAM_STICKWALL_DETONATOR_HOLSTER,
	ACT_SLAM_STICKWALL_DRAW, 
	ACT_SLAM_STICKWALL_ND_DRAW, 
	ACT_SLAM_STICKWALL_TO_THROW, 
	ACT_SLAM_STICKWALL_TO_THROW_ND, 
	ACT_SLAM_STICKWALL_TO_TRIPMINE_ND,
	ACT_SLAM_THROW_IDLE,
	ACT_SLAM_THROW_ND_IDLE,
	ACT_SLAM_THROW_THROW,
	ACT_SLAM_THROW_THROW2,
	ACT_SLAM_THROW_THROW_ND,
	ACT_SLAM_THROW_THROW_ND2,
	ACT_SLAM_THROW_DRAW, 
	ACT_SLAM_THROW_ND_DRAW,
	ACT_SLAM_THROW_TO_STICKWALL, 
	ACT_SLAM_THROW_TO_STICKWALL_ND, 
  	ACT_SLAM_THROW_DETONATE,
  	ACT_SLAM_THROW_DETONATOR_HOLSTER,
	ACT_SLAM_THROW_TO_TRIPMINE_ND,
	ACT_SLAM_TRIPMINE_IDLE,
	ACT_SLAM_TRIPMINE_DRAW, 
	ACT_SLAM_TRIPMINE_ATTACH, 
	ACT_SLAM_TRIPMINE_ATTACH2, 
	ACT_SLAM_TRIPMINE_TO_STICKWALL_ND,
	ACT_SLAM_TRIPMINE_TO_THROW_ND,
	ACT_SLAM_DETONATOR_IDLE, 
	ACT_SLAM_DETONATOR_DRAW, 
	ACT_SLAM_DETONATOR_DETONATE,
	ACT_SLAM_DETONATOR_HOLSTER,
	ACT_SLAM_DETONATOR_STICKWALL_DRAW,
	ACT_SLAM_DETONATOR_THROW_DRAW,*/

	// Shotgun Specialty Activities
	ACT_SHOTGUN_RELOAD_START,
	ACT_SHOTGUN_RELOAD_FINISH,
	ACT_SHOTGUN_PUMP,

	// SMG2 special activities
	/*ACT_SMG2_IDLE2,
	ACT_SMG2_FIRE2,
	ACT_SMG2_DRAW2,
	ACT_SMG2_RELOAD2,
	ACT_SMG2_DRYFIRE2,
	ACT_SMG2_TOAUTO,
	ACT_SMG2_TOBURST,*/
	
	// Physcannon special activities
	ACT_PHYSCANNON_UPGRADE,

	// weapon override activities
	ACT_RANGE_ATTACK_AR1,
	ACT_RANGE_ATTACK_AR2,
	ACT_RANGE_ATTACK_AR2_LOW,
	ACT_RANGE_ATTACK_AR2_GRENADE,
	ACT_RANGE_ATTACK_HMG1,
	ACT_RANGE_ATTACK_ML,
	ACT_RANGE_ATTACK_SMG1,
	ACT_RANGE_ATTACK_SMG1_LOW,
	ACT_RANGE_ATTACK_SMG2,
	ACT_RANGE_ATTACK_SHOTGUN,
	ACT_RANGE_ATTACK_SHOTGUN_LOW,
	ACT_RANGE_ATTACK_PISTOL,
	ACT_RANGE_ATTACK_PISTOL_LOW,
	ACT_RANGE_ATTACK_SLAM,
	ACT_RANGE_ATTACK_TRIPWIRE,
	ACT_RANGE_ATTACK_THROW,
	ACT_RANGE_ATTACK_SNIPER_RIFLE,
	ACT_RANGE_ATTACK_RPG,
	ACT_MELEE_ATTACK_SWING,

	ACT_RANGE_AIM_LOW,
	ACT_RANGE_AIM_SMG1_LOW,
	ACT_RANGE_AIM_PISTOL_LOW,
	ACT_RANGE_AIM_AR2_LOW,

	ACT_COVER_PISTOL_LOW,
	ACT_COVER_SMG1_LOW,

	// weapon override activities
	ACT_GESTURE_RANGE_ATTACK_AR1,
	ACT_GESTURE_RANGE_ATTACK_AR2,
	ACT_GESTURE_RANGE_ATTACK_AR2_GRENADE,
	ACT_GESTURE_RANGE_ATTACK_HMG1,
	ACT_GESTURE_RANGE_ATTACK_ML,
	ACT_GESTURE_RANGE_ATTACK_SMG1,
	ACT_GESTURE_RANGE_ATTACK_SMG1_LOW,
	ACT_GESTURE_RANGE_ATTACK_SMG2,
	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,
	ACT_GESTURE_RANGE_ATTACK_PISTOL,
	ACT_GESTURE_RANGE_ATTACK_PISTOL_LOW,
	ACT_GESTURE_RANGE_ATTACK_SLAM,
	ACT_GESTURE_RANGE_ATTACK_TRIPWIRE,
	ACT_GESTURE_RANGE_ATTACK_THROW,
	ACT_GESTURE_RANGE_ATTACK_SNIPER_RIFLE,
	ACT_GESTURE_MELEE_ATTACK_SWING,

	ACT_IDLE_RIFLE,
	ACT_IDLE_SMG1,
	ACT_IDLE_ANGRY_SMG1,
	ACT_IDLE_PISTOL,
	//BG2 - Added - HairyPotter
	ACT_BG2_IRONSIGHTS_AIM, 
	ACT_BG2_IRONSIGHTS_WALK,
	ACT_BG2_IRONSIGHTS_RECOIL,
	//
	ACT_IDLE_ANGRY_PISTOL,
	ACT_IDLE_ANGRY_SHOTGUN,
	ACT_IDLE_STEALTH_PISTOL,

	ACT_IDLE_PACKAGE,
	ACT_WALK_PACKAGE,
	ACT_IDLE_SUITCASE,
	ACT_WALK_SUITCASE,

	ACT_IDLE_SMG1_RELAXED,
	ACT_IDLE_SMG1_STIMULATED,
	ACT_WALK_RIFLE_RELAXED,
	ACT_RUN_RIFLE_RELAXED,
	ACT_WALK_RIFLE_STIMULATED,
	ACT_RUN_RIFLE_STIMULATED,

	ACT_IDLE_AIM_RIFLE_STIMULATED,
	ACT_WALK_AIM_RIFLE_STIMULATED,
	ACT_RUN_AIM_RIFLE_STIMULATED,

	ACT_IDLE_SHOTGUN_RELAXED,
	ACT_IDLE_SHOTGUN_STIMULATED,
	ACT_IDLE_SHOTGUN_AGITATED,

	// Policing activities
	ACT_WALK_ANGRY,
	ACT_POLICE_HARASS1,
	ACT_POLICE_HARASS2,

	// Manned guns
	ACT_IDLE_MANNEDGUN,
		
	// Melee weapon
	ACT_IDLE_MELEE,
	ACT_IDLE_ANGRY_MELEE,

	// RPG activities
	ACT_IDLE_RPG_RELAXED,
	ACT_IDLE_RPG, 
	ACT_IDLE_ANGRY_RPG,
	ACT_COVER_LOW_RPG, 
	ACT_WALK_RPG,
	ACT_RUN_RPG, 
	ACT_WALK_CROUCH_RPG, 
	ACT_RUN_CROUCH_RPG, 
	ACT_WALK_RPG_RELAXED, 
	ACT_RUN_RPG_RELAXED, 

	ACT_WALK_RIFLE,
	ACT_WALK_AIM_RIFLE,
	ACT_WALK_CROUCH_RIFLE,
	ACT_WALK_CROUCH_AIM_RIFLE,
	ACT_RUN_RIFLE,
	ACT_RUN_AIM_RIFLE,
	ACT_RUN_CROUCH_RIFLE,
	ACT_RUN_CROUCH_AIM_RIFLE,
	ACT_RUN_STEALTH_PISTOL,

	ACT_WALK_AIM_SHOTGUN,
	ACT_RUN_AIM_SHOTGUN,

	ACT_WALK_PISTOL,
	ACT_RUN_PISTOL,
	ACT_WALK_AIM_PISTOL,
	ACT_RUN_AIM_PISTOL,
	ACT_WALK_STEALTH_PISTOL,
	ACT_WALK_AIM_STEALTH_PISTOL,
	ACT_RUN_AIM_STEALTH_PISTOL,

	// Reloads
	ACT_RELOAD_PISTOL,
	ACT_RELOAD_PISTOL_LOW,
	ACT_RELOAD_SMG1,
	ACT_RELOAD_SMG1_LOW,
	ACT_RELOAD_SHOTGUN,
	ACT_RELOAD_SHOTGUN_LOW,

	ACT_GESTURE_RELOAD,
	ACT_GESTURE_RELOAD_PISTOL,
	ACT_GESTURE_RELOAD_SMG1,
	ACT_GESTURE_RELOAD_SHOTGUN,

	// Busy animations
	/*ACT_BUSY_LEAN_LEFT,
	ACT_BUSY_LEAN_LEFT_ENTRY,
	ACT_BUSY_LEAN_LEFT_EXIT,
	ACT_BUSY_LEAN_BACK,
	ACT_BUSY_LEAN_BACK_ENTRY,
	ACT_BUSY_LEAN_BACK_EXIT,
	ACT_BUSY_SIT_GROUND,
	ACT_BUSY_SIT_GROUND_ENTRY,
	ACT_BUSY_SIT_GROUND_EXIT,
	ACT_BUSY_SIT_CHAIR,
	ACT_BUSY_SIT_CHAIR_ENTRY,
	ACT_BUSY_SIT_CHAIR_EXIT,
	ACT_BUSY_STAND,
	ACT_BUSY_QUEUE,*/

	// Dodge animations
	ACT_DUCK_DODGE,

	// For NPCs being lifted/eaten by barnacles:
	// being swallowed by a barnacle
	ACT_DIE_BARNACLE_SWALLOW,  
	 // being lifted by a barnacle
	ACT_GESTURE_BARNACLE_STRANGLE, 

	ACT_PHYSCANNON_DETACH,	// An activity to be played if we're picking this up with the physcannon
	ACT_PHYSCANNON_ANIMATE, // An activity to be played by an object being picked up with the physcannon, but has different behavior to DETACH
	ACT_PHYSCANNON_ANIMATE_PRE,	// An activity to be played by an object being picked up with the physcannon, before playing the ACT_PHYSCANNON_ANIMATE
	ACT_PHYSCANNON_ANIMATE_POST,// An activity to be played by an object being picked up with the physcannon, after playing the ACT_PHYSCANNON_ANIMATE

	ACT_DIE_FRONTSIDE,
	ACT_DIE_RIGHTSIDE,
	ACT_DIE_BACKSIDE,
	ACT_DIE_LEFTSIDE,

	ACT_OPEN_DOOR,

	// Dynamic interactions
	/*ACT_DI_ALYX_ZOMBIE_MELEE,
	ACT_DI_ALYX_ZOMBIE_TORSO_MELEE,
	ACT_DI_ALYX_HEADCRAB_MELEE,
	ACT_DI_ALYX_ANTLION,

	ACT_DI_ALYX_ZOMBIE_SHOTGUN64,
	ACT_DI_ALYX_ZOMBIE_SHOTGUN26,*/

	ACT_READINESS_RELAXED_TO_STIMULATED,
	ACT_READINESS_RELAXED_TO_STIMULATED_WALK,
	ACT_READINESS_AGITATED_TO_STIMULATED,
	ACT_READINESS_STIMULATED_TO_RELAXED,

	ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED,
	ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED_WALK,
	ACT_READINESS_PISTOL_AGITATED_TO_STIMULATED,
	ACT_READINESS_PISTOL_STIMULATED_TO_RELAXED,

	ACT_IDLE_CARRY,
	ACT_WALK_CARRY,

	// turning in place
	ACT_TURNRIGHT45,
	ACT_TURNLEFT45,

	ACT_TURN,

	/*ACT_OBJ_ASSEMBLING,
	ACT_OBJ_DISMANTLING,
	ACT_OBJ_STARTUP,
	ACT_OBJ_RUNNING,
	ACT_OBJ_IDLE,
	ACT_OBJ_PLACING,
	ACT_OBJ_DETERIORATING,
	ACT_OBJ_UPGRADING,

	// Deploy
	ACT_DEPLOY,
	ACT_DEPLOY_IDLE,
	ACT_UNDEPLOY,*/

//===========================
// HL1 Specific Activities
//===========================
	// Grenades
	/*ACT_GRENADE_ROLL,
	ACT_GRENADE_TOSS,

	// Hand grenade
	ACT_HANDGRENADE_THROW1,
	ACT_HANDGRENADE_THROW2,
	ACT_HANDGRENADE_THROW3,

	// Shotgun
	ACT_SHOTGUN_IDLE_DEEP,
	ACT_SHOTGUN_IDLE4,

	// Glock
	ACT_GLOCK_SHOOTEMPTY,
	ACT_GLOCK_SHOOT_RELOAD,

	// RPG
	ACT_RPG_DRAW_UNLOADED,
	ACT_RPG_HOLSTER_UNLOADED,
	ACT_RPG_IDLE_UNLOADED,
	ACT_RPG_FIDGET_UNLOADED,

	// Crossbow
	ACT_CROSSBOW_DRAW_UNLOADED,
	ACT_CROSSBOW_IDLE_UNLOADED,
	ACT_CROSSBOW_FIDGET_UNLOADED,

	// Gauss
	ACT_GAUSS_SPINUP,
	ACT_GAUSS_SPINCYCLE,

	// Tripmine
	ACT_TRIPMINE_GROUND,
	ACT_TRIPMINE_WORLD,*/

//===========================
// CSPort Specific Activities
//===========================

	/*ACT_VM_PRIMARYATTACK_SILENCED,		// fire
	ACT_VM_RELOAD_SILENCED,
	ACT_VM_DRYFIRE_SILENCED,				// fire with no ammo loaded.
	ACT_VM_IDLE_SILENCED,
	ACT_VM_DRAW_SILENCED,
	ACT_VM_IDLE_EMPTY_LEFT,
	ACT_VM_DRYFIRE_LEFT,

	ACT_PLAYER_IDLE_FIRE,
	ACT_PLAYER_CROUCH_FIRE,
	ACT_PLAYER_CROUCH_WALK_FIRE,
	ACT_PLAYER_WALK_FIRE,
	ACT_PLAYER_RUN_FIRE,
	
	ACT_IDLETORUN,
	ACT_RUNTOIDLE,*/
	

//===========================
// DoD Specific Activities
//===========================
	ACT_SPRINT,
	
	ACT_GET_DOWN_STAND,
	ACT_GET_UP_STAND,
	ACT_GET_DOWN_CROUCH,
	ACT_GET_UP_CROUCH,
	ACT_PRONE_FORWARD,
	ACT_PRONE_IDLE,

	ACT_DEEPIDLE1,
	ACT_DEEPIDLE2,
	ACT_DEEPIDLE3,
	ACT_DEEPIDLE4,

	ACT_VM_RELOAD_DEPLOYED, 
	ACT_VM_RELOAD_IDLE,

	ACT_VM_DRAW_DEPLOYED,

	//Weapon is empty activities
	ACT_VM_DRAW_EMPTY,
	ACT_VM_PRIMARYATTACK_EMPTY,
	//BG2 - Tjoppen - ACT_VM_SECONDARYATTACK_EMPTY
	ACT_VM_SECONDARYATTACK_EMPTY,
	//
	ACT_VM_RELOAD_EMPTY,
	ACT_VM_IDLE_EMPTY,
	ACT_VM_IDLE_DEPLOYED_EMPTY,

	/*ACT_VM_IDLE_8,
	ACT_VM_IDLE_7,
	ACT_VM_IDLE_6,
	ACT_VM_IDLE_5,
	ACT_VM_IDLE_4,
	ACT_VM_IDLE_3,
	ACT_VM_IDLE_2,
	ACT_VM_IDLE_1,

	ACT_VM_IDLE_DEPLOYED,
	ACT_VM_IDLE_DEPLOYED_8,
	ACT_VM_IDLE_DEPLOYED_7,
	ACT_VM_IDLE_DEPLOYED_6,
	ACT_VM_IDLE_DEPLOYED_5,
	ACT_VM_IDLE_DEPLOYED_4,
	ACT_VM_IDLE_DEPLOYED_3,
	ACT_VM_IDLE_DEPLOYED_2,
	ACT_VM_IDLE_DEPLOYED_1,

	// Animation from prone idle to standing/crouch idle. Number designates bullets left
	ACT_VM_UNDEPLOY,
	ACT_VM_UNDEPLOY_8,
	ACT_VM_UNDEPLOY_7,
	ACT_VM_UNDEPLOY_6,
	ACT_VM_UNDEPLOY_5,
	ACT_VM_UNDEPLOY_4,
	ACT_VM_UNDEPLOY_3,
	ACT_VM_UNDEPLOY_2,
	ACT_VM_UNDEPLOY_1,
	ACT_VM_UNDEPLOY_EMPTY,

	// Animation from standing/crouch idle to prone idle. Number designates bullets left
	ACT_VM_DEPLOY,
	ACT_VM_DEPLOY_8,
	ACT_VM_DEPLOY_7,
	ACT_VM_DEPLOY_6,
	ACT_VM_DEPLOY_5,
	ACT_VM_DEPLOY_4,
	ACT_VM_DEPLOY_3,
	ACT_VM_DEPLOY_2,
	ACT_VM_DEPLOY_1,
	ACT_VM_DEPLOY_EMPTY,

	// Shooting animations for standing/crouch position.  Number designates bullets left at START of animation
	ACT_VM_PRIMARYATTACK_8,
	ACT_VM_PRIMARYATTACK_7,
	ACT_VM_PRIMARYATTACK_6,
	ACT_VM_PRIMARYATTACK_5,
	ACT_VM_PRIMARYATTACK_4,
	ACT_VM_PRIMARYATTACK_3,
	ACT_VM_PRIMARYATTACK_2,
	ACT_VM_PRIMARYATTACK_1,

	// Shooting animations for prone position. Number designates bullets left at START of animation
	ACT_VM_PRIMARYATTACK_DEPLOYED,
	ACT_VM_PRIMARYATTACK_DEPLOYED_8,
	ACT_VM_PRIMARYATTACK_DEPLOYED_7,
	ACT_VM_PRIMARYATTACK_DEPLOYED_6, 
	ACT_VM_PRIMARYATTACK_DEPLOYED_5,
	ACT_VM_PRIMARYATTACK_DEPLOYED_4,
	ACT_VM_PRIMARYATTACK_DEPLOYED_3,
	ACT_VM_PRIMARYATTACK_DEPLOYED_2,
	ACT_VM_PRIMARYATTACK_DEPLOYED_1,
	ACT_VM_PRIMARYATTACK_DEPLOYED_EMPTY,*/

//BG2
	ACT_HL2MP_GESTURE_ATTACK_MELEE_BAYONET,
	ACT_HL2MP_GESTURE_RELOAD_MUSKET,
//

// HL2MP
	ACT_HL2MP_IDLE,
	ACT_HL2MP_RUN,
	ACT_HL2MP_IDLE_CROUCH,
	ACT_HL2MP_WALK_CROUCH,
	ACT_HL2MP_GESTURE_RANGE_ATTACK,
	ACT_HL2MP_GESTURE_RELOAD,
	ACT_HL2MP_JUMP,
	
	ACT_HL2MP_IDLE_PISTOL,
	ACT_HL2MP_RUN_PISTOL,
	ACT_HL2MP_IDLE_CROUCH_PISTOL,
	ACT_HL2MP_WALK_CROUCH_PISTOL,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,
	ACT_HL2MP_GESTURE_RELOAD_PISTOL,
	ACT_HL2MP_JUMP_PISTOL,

	ACT_HL2MP_IDLE_SMG1,
	ACT_HL2MP_RUN_SMG1,
	ACT_HL2MP_IDLE_CROUCH_SMG1,
	ACT_HL2MP_WALK_CROUCH_SMG1,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,
	ACT_HL2MP_GESTURE_RELOAD_SMG1,
	ACT_HL2MP_JUMP_SMG1,

  	ACT_HL2MP_IDLE_AR2,
	ACT_HL2MP_RUN_AR2,
	ACT_HL2MP_IDLE_CROUCH_AR2,
	ACT_HL2MP_WALK_CROUCH_AR2,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2,
	ACT_HL2MP_GESTURE_RELOAD_AR2,
	ACT_HL2MP_JUMP_AR2,

  	ACT_HL2MP_IDLE_SHOTGUN,
	ACT_HL2MP_RUN_SHOTGUN,
	ACT_HL2MP_IDLE_CROUCH_SHOTGUN,
	ACT_HL2MP_WALK_CROUCH_SHOTGUN,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,
	ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,
	ACT_HL2MP_JUMP_SHOTGUN,

  	ACT_HL2MP_IDLE_RPG,
	ACT_HL2MP_RUN_RPG,
	ACT_HL2MP_IDLE_CROUCH_RPG,
	ACT_HL2MP_WALK_CROUCH_RPG,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,
	ACT_HL2MP_GESTURE_RELOAD_RPG,
	ACT_HL2MP_JUMP_RPG,

  	ACT_HL2MP_IDLE_GRENADE,
	ACT_HL2MP_RUN_GRENADE,
	ACT_HL2MP_IDLE_CROUCH_GRENADE,
	ACT_HL2MP_WALK_CROUCH_GRENADE,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,
	ACT_HL2MP_GESTURE_RELOAD_GRENADE,
	ACT_HL2MP_JUMP_GRENADE,

  	ACT_HL2MP_IDLE_PHYSGUN,
	ACT_HL2MP_RUN_PHYSGUN,
	ACT_HL2MP_IDLE_CROUCH_PHYSGUN,
	ACT_HL2MP_WALK_CROUCH_PHYSGUN,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,
	ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,
	ACT_HL2MP_JUMP_PHYSGUN,

  	ACT_HL2MP_IDLE_CROSSBOW,
	ACT_HL2MP_RUN_CROSSBOW,
	ACT_HL2MP_IDLE_CROUCH_CROSSBOW,
	ACT_HL2MP_WALK_CROUCH_CROSSBOW,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,
	ACT_HL2MP_GESTURE_RELOAD_CROSSBOW,
	ACT_HL2MP_JUMP_CROSSBOW,

  	ACT_HL2MP_IDLE_MELEE,
	ACT_HL2MP_RUN_MELEE,
	ACT_HL2MP_IDLE_CROUCH_MELEE,
	ACT_HL2MP_WALK_CROUCH_MELEE,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,
	ACT_HL2MP_GESTURE_RELOAD_MELEE,
	ACT_HL2MP_JUMP_MELEE,

	ACT_HL2MP_IDLE_SLAM,
	ACT_HL2MP_RUN_SLAM,
	ACT_HL2MP_IDLE_CROUCH_SLAM,
	ACT_HL2MP_WALK_CROUCH_SLAM,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM,
	ACT_HL2MP_GESTURE_RELOAD_SLAM,
	ACT_HL2MP_JUMP_SLAM,

// Portal!
	ACT_VM_FIZZLE,

	// Multiplayer
	ACT_MP_STAND_IDLE,
	ACT_MP_CROUCH_IDLE,
	ACT_MP_CROUCH_DEPLOYED_IDLE,
	ACT_MP_CROUCH_DEPLOYED,
	ACT_MP_DEPLOYED_IDLE,
	ACT_MP_RUN,
	ACT_MP_WALK,
	ACT_MP_AIRWALK,
	ACT_MP_CROUCHWALK,
	ACT_MP_SPRINT,
	ACT_MP_JUMP,
	ACT_MP_JUMP_START,
	ACT_MP_JUMP_FLOAT,
	ACT_MP_JUMP_LAND,
	ACT_MP_DOUBLEJUMP,
	ACT_MP_SWIM,
	ACT_MP_DEPLOYED,
	ACT_MP_SWIM_DEPLOYED,
	ACT_MP_VCD,

	ACT_MP_ATTACK_STAND_PRIMARYFIRE,
	ACT_MP_ATTACK_STAND_PRIMARYFIRE_DEPLOYED,
	ACT_MP_ATTACK_STAND_SECONDARYFIRE,
	ACT_MP_ATTACK_STAND_GRENADE,
	ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,
	ACT_MP_ATTACK_CROUCH_PRIMARYFIRE_DEPLOYED,
	ACT_MP_ATTACK_CROUCH_SECONDARYFIRE,
	ACT_MP_ATTACK_CROUCH_GRENADE,
	ACT_MP_ATTACK_SWIM_PRIMARYFIRE,
	ACT_MP_ATTACK_SWIM_SECONDARYFIRE,
	ACT_MP_ATTACK_SWIM_GRENADE,
	ACT_MP_ATTACK_AIRWALK_PRIMARYFIRE,
	ACT_MP_ATTACK_AIRWALK_SECONDARYFIRE,
	ACT_MP_ATTACK_AIRWALK_GRENADE,
	ACT_MP_RELOAD_STAND,
	ACT_MP_RELOAD_STAND_LOOP,
	ACT_MP_RELOAD_STAND_END,
	ACT_MP_RELOAD_CROUCH,
	ACT_MP_RELOAD_CROUCH_LOOP,
	ACT_MP_RELOAD_CROUCH_END,
	ACT_MP_RELOAD_SWIM,
	ACT_MP_RELOAD_SWIM_LOOP,
	ACT_MP_RELOAD_SWIM_END,
	ACT_MP_RELOAD_AIRWALK,
	ACT_MP_RELOAD_AIRWALK_LOOP,
	ACT_MP_RELOAD_AIRWALK_END,
	ACT_MP_ATTACK_STAND_PREFIRE,
	ACT_MP_ATTACK_STAND_POSTFIRE,
	ACT_MP_ATTACK_STAND_STARTFIRE,
	ACT_MP_ATTACK_CROUCH_PREFIRE,
	ACT_MP_ATTACK_CROUCH_POSTFIRE,
	ACT_MP_ATTACK_SWIM_PREFIRE,
	ACT_MP_ATTACK_SWIM_POSTFIRE,

	// Multiplayer - Primary
	ACT_MP_STAND_PRIMARY,
	ACT_MP_CROUCH_PRIMARY,
	ACT_MP_RUN_PRIMARY,
	ACT_MP_WALK_PRIMARY,
	ACT_MP_AIRWALK_PRIMARY,
	ACT_MP_CROUCHWALK_PRIMARY,
	ACT_MP_JUMP_PRIMARY,
	ACT_MP_JUMP_START_PRIMARY,
	ACT_MP_JUMP_FLOAT_PRIMARY,
	ACT_MP_JUMP_LAND_PRIMARY,
	ACT_MP_SWIM_PRIMARY,
	ACT_MP_DEPLOYED_PRIMARY,
	ACT_MP_SWIM_DEPLOYED_PRIMARY,

	ACT_MP_ATTACK_STAND_PRIMARY,		// RUN, WALK
	ACT_MP_ATTACK_STAND_PRIMARY_DEPLOYED,
	ACT_MP_ATTACK_CROUCH_PRIMARY,		// CROUCHWALK
	ACT_MP_ATTACK_CROUCH_PRIMARY_DEPLOYED,
	ACT_MP_ATTACK_SWIM_PRIMARY,
	ACT_MP_ATTACK_AIRWALK_PRIMARY,

	ACT_MP_RELOAD_STAND_PRIMARY,		// RUN, WALK
	ACT_MP_RELOAD_STAND_PRIMARY_LOOP,
	ACT_MP_RELOAD_STAND_PRIMARY_END,
	ACT_MP_RELOAD_CROUCH_PRIMARY,		// CROUCHWALK
	ACT_MP_RELOAD_CROUCH_PRIMARY_LOOP,
	ACT_MP_RELOAD_CROUCH_PRIMARY_END,
	ACT_MP_RELOAD_SWIM_PRIMARY,
	ACT_MP_RELOAD_SWIM_PRIMARY_LOOP,
	ACT_MP_RELOAD_SWIM_PRIMARY_END,
	ACT_MP_RELOAD_AIRWALK_PRIMARY,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_LOOP,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_END,

	ACT_MP_ATTACK_STAND_GRENADE_PRIMARY,		// RUN, WALK
	ACT_MP_ATTACK_CROUCH_GRENADE_PRIMARY,		// CROUCHWALK
	ACT_MP_ATTACK_SWIM_GRENADE_PRIMARY,
	ACT_MP_ATTACK_AIRWALK_GRENADE_PRIMARY,

	// Secondary
	ACT_MP_STAND_SECONDARY,
	ACT_MP_CROUCH_SECONDARY,
	ACT_MP_RUN_SECONDARY,
	ACT_MP_WALK_SECONDARY,
	ACT_MP_AIRWALK_SECONDARY,
	ACT_MP_CROUCHWALK_SECONDARY,
	ACT_MP_JUMP_SECONDARY,
	ACT_MP_JUMP_START_SECONDARY,
	ACT_MP_JUMP_FLOAT_SECONDARY,
	ACT_MP_JUMP_LAND_SECONDARY,
	ACT_MP_SWIM_SECONDARY,

	ACT_MP_ATTACK_STAND_SECONDARY,		// RUN, WALK
	ACT_MP_ATTACK_CROUCH_SECONDARY,		// CROUCHWALK
	ACT_MP_ATTACK_SWIM_SECONDARY,
	ACT_MP_ATTACK_AIRWALK_SECONDARY,

	ACT_MP_RELOAD_STAND_SECONDARY,		// RUN, WALK
	ACT_MP_RELOAD_STAND_SECONDARY_LOOP,
	ACT_MP_RELOAD_STAND_SECONDARY_END,
	ACT_MP_RELOAD_CROUCH_SECONDARY,		// CROUCHWALK
	ACT_MP_RELOAD_CROUCH_SECONDARY_LOOP,
	ACT_MP_RELOAD_CROUCH_SECONDARY_END,
	ACT_MP_RELOAD_SWIM_SECONDARY,
	ACT_MP_RELOAD_SWIM_SECONDARY_LOOP,
	ACT_MP_RELOAD_SWIM_SECONDARY_END,
	ACT_MP_RELOAD_AIRWALK_SECONDARY,
	ACT_MP_RELOAD_AIRWALK_SECONDARY_LOOP,
	ACT_MP_RELOAD_AIRWALK_SECONDARY_END,

	ACT_MP_ATTACK_STAND_GRENADE_SECONDARY,		// RUN, WALK
	ACT_MP_ATTACK_CROUCH_GRENADE_SECONDARY,		// CROUCHWALK
	ACT_MP_ATTACK_SWIM_GRENADE_SECONDARY,
	ACT_MP_ATTACK_AIRWALK_GRENADE_SECONDARY,

	// Melee
	ACT_MP_STAND_MELEE,
	ACT_MP_CROUCH_MELEE,
	ACT_MP_RUN_MELEE,
	ACT_MP_WALK_MELEE,
	ACT_MP_AIRWALK_MELEE,
	ACT_MP_CROUCHWALK_MELEE,
	ACT_MP_JUMP_MELEE,
	ACT_MP_JUMP_START_MELEE,
	ACT_MP_JUMP_FLOAT_MELEE,
	ACT_MP_JUMP_LAND_MELEE,
	ACT_MP_SWIM_MELEE,

	ACT_MP_ATTACK_STAND_MELEE,		// RUN, WALK
	ACT_MP_ATTACK_STAND_MELEE_SECONDARY,
	ACT_MP_ATTACK_CROUCH_MELEE,		// CROUCHWALK
	ACT_MP_ATTACK_CROUCH_MELEE_SECONDARY,
	ACT_MP_ATTACK_SWIM_MELEE,
	ACT_MP_ATTACK_AIRWALK_MELEE,

	ACT_MP_ATTACK_STAND_GRENADE_MELEE,		// RUN, WALK
	ACT_MP_ATTACK_CROUCH_GRENADE_MELEE,		// CROUCHWALK
	ACT_MP_ATTACK_SWIM_GRENADE_MELEE,
	ACT_MP_ATTACK_AIRWALK_GRENADE_MELEE,

	// Flinches
	ACT_MP_GESTURE_FLINCH,
	ACT_MP_GESTURE_FLINCH_PRIMARY,
	ACT_MP_GESTURE_FLINCH_SECONDARY,
	ACT_MP_GESTURE_FLINCH_MELEE,

	ACT_MP_GESTURE_FLINCH_HEAD,
	ACT_MP_GESTURE_FLINCH_CHEST,
	ACT_MP_GESTURE_FLINCH_STOMACH,
	ACT_MP_GESTURE_FLINCH_LEFTARM,
	ACT_MP_GESTURE_FLINCH_RIGHTARM,
	ACT_MP_GESTURE_FLINCH_LEFTLEG,
	ACT_MP_GESTURE_FLINCH_RIGHTLEG,


	/*ACT_VM_UNUSABLE,
	ACT_VM_UNUSABLE_TO_USABLE,
	ACT_VM_USABLE_TO_UNUSABLE,*/

	// this is the end of the global activities, private per-monster activities start here.
	LAST_SHARED_ACTIVITY,
} Activity;


#endif // AI_ACTIVITY_H
