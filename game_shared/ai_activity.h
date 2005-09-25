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
	ACT_RELOAD_LOW,
	ACT_ARM,				// pull out gun, for instance
	ACT_DISARM,				// reholster gun
	ACT_PICKUP_GROUND,		// pick up something in front of you on the ground
	ACT_PICKUP_RACK,		// pick up something from a rack or shelf in front of you.
	ACT_IDLE_ANGRY,			// FIXME: being used as an combat ready idle?  alternate idle animation in which the monster is clearly agitated. (loop)

	ACT_IDLE_RELAXED,
	ACT_IDLE_STIMULATED,
	ACT_IDLE_AGITATED,

	ACT_WALK_RELAXED,
	ACT_WALK_STIMULATED,
	ACT_WALK_AGITATED,

	ACT_RUN_RELAXED,
	ACT_RUN_STIMULATED,
	ACT_RUN_AGITATED,

	ACT_IDLE_AIM_RELAXED,
	ACT_IDLE_AIM_STIMULATED,
	ACT_IDLE_AIM_AGITATED,

	ACT_WALK_AIM_RELAXED,
	ACT_WALK_AIM_STIMULATED,
	ACT_WALK_AIM_AGITATED,

	ACT_RUN_AIM_RELAXED,
	ACT_RUN_AIM_STIMULATED,
	ACT_RUN_AIM_AGITATED,

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
	ACT_GESTURE_FLINCH_BLAST,
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
	
	ACT_VM_ATTACH_SILENCER,
	ACT_VM_DETACH_SILENCER,

//===========================
// HL2 Specific Activities
//===========================
	// SLAM	Specialty Activities
	ACT_SLAM_STICKWALL_IDLE,
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
	ACT_SLAM_DETONATOR_THROW_DRAW,

	// Shotgun Specialty Activities
	ACT_SHOTGUN_RELOAD_START,
	ACT_SHOTGUN_RELOAD_FINISH,
	ACT_SHOTGUN_PUMP,

	// SMG2 special activities
	ACT_SMG2_IDLE2,
	ACT_SMG2_FIRE2,
	ACT_SMG2_DRAW2,
	ACT_SMG2_RELOAD2,
	ACT_SMG2_DRYFIRE2,
	ACT_SMG2_TOAUTO,
	ACT_SMG2_TOBURST,
	
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
	ACT_IDLE_ANGRY_PISTOL,
	ACT_IDLE_ANGRY_SHOTGUN,

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

	ACT_WALK_AIM_SHOTGUN,
	ACT_RUN_AIM_SHOTGUN,

	ACT_WALK_AIM_PISTOL,
	ACT_RUN_AIM_PISTOL,

	// Reloads
	ACT_RELOAD_PISTOL,
	ACT_RELOAD_PISTOL_LOW,
	ACT_RELOAD_SMG1,
	ACT_RELOAD_SMG1_LOW,

	ACT_GESTURE_RELOAD,
	ACT_GESTURE_RELOAD_PISTOL,
	ACT_GESTURE_RELOAD_SMG1,

	// Busy animations
	ACT_BUSY_LEAN_LEFT,
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
	ACT_BUSY_QUEUE,

	// For NPCs being lifted/eaten by barnacles:
	// being swallowed by a barnacle
	ACT_DIE_BARNACLE_SWALLOW,  
	 // being lifted by a barnacle
	ACT_GESTURE_BARNACLE_STRANGLE, 

	ACT_PHYSCANNON_DETACH,	// An activity to be played if we're picking this up with the physcannon

	ACT_DIE_FRONTSIDE,
	ACT_DIE_RIGHTSIDE,
	ACT_DIE_BACKSIDE,
	ACT_DIE_LEFTSIDE,

	ACT_OPEN_DOOR,

//===========================
// TF2 Specific Activities
//===========================
	ACT_STARTDYING,
	ACT_DYINGLOOP,
	ACT_DYINGTODEAD,

	ACT_RIDE_MANNED_GUN,

	// All viewmodels
	ACT_VM_SPRINT_ENTER,
	ACT_VM_SPRINT_IDLE,
	ACT_VM_SPRINT_LEAVE,

	// Looping weapon firing
	ACT_FIRE_START,
	ACT_FIRE_LOOP,
	ACT_FIRE_END,

	ACT_CROUCHING_GRENADEIDLE,
	ACT_CROUCHING_GRENADEREADY,
	ACT_CROUCHING_PRIMARYATTACK,
	ACT_OVERLAY_GRENADEIDLE,
	ACT_OVERLAY_GRENADEREADY,
	ACT_OVERLAY_PRIMARYATTACK,
	ACT_OVERLAY_SHIELD_UP,
	ACT_OVERLAY_SHIELD_DOWN,
	ACT_OVERLAY_SHIELD_UP_IDLE,
	ACT_OVERLAY_SHIELD_ATTACK,
	ACT_OVERLAY_SHIELD_KNOCKBACK,
	ACT_SHIELD_UP,
	ACT_SHIELD_DOWN,
	ACT_SHIELD_UP_IDLE,
	ACT_SHIELD_ATTACK,
	ACT_SHIELD_KNOCKBACK,
	ACT_CROUCHING_SHIELD_UP,
	ACT_CROUCHING_SHIELD_DOWN,
	ACT_CROUCHING_SHIELD_UP_IDLE,
	ACT_CROUCHING_SHIELD_ATTACK,
	ACT_CROUCHING_SHIELD_KNOCKBACK,

	// turning in place
	ACT_TURNRIGHT45,
	ACT_TURNLEFT45,

	ACT_TURN,

	ACT_OBJ_ASSEMBLING,
	ACT_OBJ_DISMANTLING,
	ACT_OBJ_STARTUP,
	ACT_OBJ_RUNNING,
	ACT_OBJ_IDLE,
	ACT_OBJ_PLACING,
	ACT_OBJ_DETERIORATING,

	// Deploy
	ACT_DEPLOY,
	ACT_DEPLOY_IDLE,
	ACT_UNDEPLOY,

//===========================
// HL1 Specific Activities
//===========================
	// Grenades
	ACT_GRENADE_ROLL,
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
	ACT_TRIPMINE_WORLD,

//===========================
// CSPort Specific Activities
//===========================

	ACT_VM_PRIMARYATTACK_SILENCED,		// fire
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
	ACT_RUNTOIDLE,
	

//===========================
// DoD Specific Activities
//===========================
	ACT_SPRINT,
	
	ACT_GET_DOWN,
	ACT_GET_UP,
	ACT_PRONE_FORWARD,
	ACT_PRONE_IDLE,

	ACT_DEEPIDLE1,
	ACT_DEEPIDLE2,
	ACT_DEEPIDLE3,
	ACT_DEEPIDLE4,

	ACT_VM_RELOAD_DEPLOYED, 
	ACT_VM_RELOAD_IDLE,

	//Weapon is empty activities
	ACT_VM_DRAW_EMPTY,
	ACT_VM_PRIMARYATTACK_EMPTY,
	//BG2 - Tjoppen - ACT_VM_SECONDARYATTACK_EMPTY
	ACT_VM_SECONDARYATTACK_EMPTY,
	//
	ACT_VM_RELOAD_EMPTY,
	ACT_VM_IDLE_EMPTY,
	ACT_VM_IDLE_DEPLOYED_EMPTY,

	ACT_VM_IDLE_8,
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
	ACT_VM_PRIMARYATTACK_DEPLOYED_EMPTY,

	// Player anim ACTs
	ACT_DOD_DEPLOYED,
	ACT_DOD_PRONE_DEPLOYED,
	ACT_DOD_IDLE_ZOOMED,
	ACT_DOD_WALK_ZOOMED,
	ACT_DOD_CROUCH_ZOOMED,
	ACT_DOD_CROUCHWALK_ZOOMED,
	ACT_DOD_PRONE_ZOOMED,
	ACT_DOD_PRONE_FORWARD_ZOOMED,
	ACT_DOD_PRIMARYATTACK_DEPLOYED,
	ACT_DOD_PRIMARYATTACK_PRONE_DEPLOYED,
	ACT_DOD_RELOAD_DEPLOYED,
	ACT_DOD_RELOAD_PRONE_DEPLOYED,	
	ACT_DOD_PRIMARYATTACK_PRONE,
	ACT_DOD_SECONDARYATTACK_PRONE,
	ACT_DOD_RELOAD_PRONE,

	// Positions
	ACT_DOD_STAND_AIM_PISTOL,
	ACT_DOD_CROUCH_AIM_PISTOL,
	ACT_DOD_CROUCHWALK_AIM_PISTOL,
	ACT_DOD_WALK_AIM_PISTOL,
	ACT_DOD_RUN_AIM_PISTOL,
	ACT_DOD_PRONE_AIM_PISTOL,

	ACT_DOD_STAND_AIM_RIFLE,
	ACT_DOD_CROUCH_AIM_RIFLE,
	ACT_DOD_CROUCHWALK_AIM_RIFLE,
	ACT_DOD_WALK_AIM_RIFLE,
	ACT_DOD_RUN_AIM_RIFLE,
	ACT_DOD_PRONE_AIM_RIFLE,

	ACT_DOD_STAND_AIM_BOLT,
	ACT_DOD_CROUCH_AIM_BOLT,
	ACT_DOD_CROUCHWALK_AIM_BOLT,
	ACT_DOD_WALK_AIM_BOLT,
	ACT_DOD_RUN_AIM_BOLT,
	ACT_DOD_PRONE_AIM_BOLT,

	ACT_DOD_STAND_AIM_TOMMY,
	ACT_DOD_CROUCH_AIM_TOMMY,
	ACT_DOD_CROUCHWALK_AIM_TOMMY,
	ACT_DOD_WALK_AIM_TOMMY,
	ACT_DOD_RUN_AIM_TOMMY,
	ACT_DOD_PRONE_AIM_TOMMY,

	ACT_DOD_STAND_AIM_MP40,
	ACT_DOD_CROUCH_AIM_MP40,
	ACT_DOD_CROUCHWALK_AIM_MP40,
	ACT_DOD_WALK_AIM_MP40,
	ACT_DOD_RUN_AIM_MP40,
	ACT_DOD_PRONE_AIM_MP40,

	ACT_DOD_STAND_AIM_MP44,
	ACT_DOD_CROUCH_AIM_MP44,
	ACT_DOD_CROUCHWALK_AIM_MP44,
	ACT_DOD_WALK_AIM_MP44,
	ACT_DOD_RUN_AIM_MP44,
	ACT_DOD_PRONE_AIM_MP44,

	ACT_DOD_STAND_AIM_GREASE,
	ACT_DOD_CROUCH_AIM_GREASE,
	ACT_DOD_CROUCHWALK_AIM_GREASE,
	ACT_DOD_WALK_AIM_GREASE,
	ACT_DOD_RUN_AIM_GREASE,
	ACT_DOD_PRONE_AIM_GREASE,

	ACT_DOD_STAND_AIM_MG,
	ACT_DOD_CROUCH_AIM_MG,
	ACT_DOD_CROUCHWALK_AIM_MG,
	ACT_DOD_WALK_AIM_MG,
	ACT_DOD_RUN_AIM_MG,
	ACT_DOD_PRONE_AIM_MG,

	ACT_DOD_STAND_AIM_30CAL,
	ACT_DOD_CROUCH_AIM_30CAL,
	ACT_DOD_CROUCHWALK_AIM_30CAL,
	ACT_DOD_WALK_AIM_30CAL,
	ACT_DOD_RUN_AIM_30CAL,
	ACT_DOD_PRONE_AIM_30CAL,

	ACT_DOD_STAND_AIM_GREN,
	ACT_DOD_CROUCH_AIM_GREN,
	ACT_DOD_CROUCHWALK_AIM_GREN,
	ACT_DOD_WALK_AIM_GREN,
	ACT_DOD_RUN_AIM_GREN,
	ACT_DOD_PRONE_AIM_GREN,

	ACT_DOD_STAND_AIM_KNIFE,
	ACT_DOD_CROUCH_AIM_KNIFE,
	ACT_DOD_CROUCHWALK_AIM_KNIFE,
	ACT_DOD_WALK_AIM_KNIFE,
	ACT_DOD_RUN_AIM_KNIFE,
	ACT_DOD_PRONE_AIM_KNIFE,

	ACT_DOD_STAND_AIM_SPADE,
	ACT_DOD_CROUCH_AIM_SPADE,
	ACT_DOD_CROUCHWALK_AIM_SPADE,
	ACT_DOD_WALK_AIM_SPADE,
	ACT_DOD_RUN_AIM_SPADE,
	ACT_DOD_PRONE_AIM_SPADE,

	ACT_DOD_STAND_AIM_BAZOOKA,
	ACT_DOD_CROUCH_AIM_BAZOOKA,
	ACT_DOD_CROUCHWALK_AIM_BAZOOKA,
	ACT_DOD_WALK_AIM_BAZOOKA,
	ACT_DOD_RUN_AIM_BAZOOKA,
	ACT_DOD_PRONE_AIM_BAZOOKA,

	ACT_DOD_STAND_AIM_PSCHRECK,
	ACT_DOD_CROUCH_AIM_PSCHRECK,
	ACT_DOD_CROUCHWALK_AIM_PSCHRECK,
	ACT_DOD_WALK_AIM_PSCHRECK,
	ACT_DOD_RUN_AIM_PSCHRECK,
	ACT_DOD_PRONE_AIM_PSCHRECK,

	// Zoomed aims
	ACT_DOD_STAND_ZOOM_TOMMY,
	ACT_DOD_CROUCH_ZOOM_TOMMY,
	ACT_DOD_CROUCHWALK_ZOOM_TOMMY,
	ACT_DOD_WALK_ZOOM_TOMMY,
	ACT_DOD_RUN_ZOOM_TOMMY,
	ACT_DOD_PRONE_ZOOM_TOMMY,

	ACT_DOD_STAND_ZOOM_BOLT,
	ACT_DOD_CROUCH_ZOOM_BOLT,
	ACT_DOD_CROUCHWALK_ZOOM_BOLT,
	ACT_DOD_WALK_ZOOM_BOLT,
	ACT_DOD_RUN_ZOOM_BOLT,
	ACT_DOD_PRONE_ZOOM_BOLT,

	ACT_DOD_STAND_ZOOM_BAZOOKA,
	ACT_DOD_CROUCH_ZOOM_BAZOOKA,
	ACT_DOD_CROUCHWALK_ZOOM_BAZOOKA,
	ACT_DOD_WALK_ZOOM_BAZOOKA,
	ACT_DOD_RUN_ZOOM_BAZOOKA,
	ACT_DOD_PRONE_ZOOM_BAZOOKA,

	ACT_DOD_STAND_ZOOM_PSCHRECK,
	ACT_DOD_CROUCH_ZOOM_PSCHRECK,
	ACT_DOD_CROUCHWALK_ZOOM_PSCHRECK,
	ACT_DOD_WALK_ZOOM_PSCHRECK,
	ACT_DOD_RUN_ZOOM_PSCHRECK,
	ACT_DOD_PRONE_ZOOM_PSCHRECK,

	// Deployed Aim
	ACT_DOD_DEPLOY_RIFLE,
	ACT_DOD_DEPLOY_TOMMY,
	ACT_DOD_DEPLOY_MG,
	ACT_DOD_DEPLOY_30CAL,

	// Prone Deployed Aim
	ACT_DOD_PRONE_DEPLOY_RIFLE ,
	ACT_DOD_PRONE_DEPLOY_TOMMY,
	ACT_DOD_PRONE_DEPLOY_MG,
	ACT_DOD_PRONE_DEPLOY_30CAL,

	// Attacks

	// Rifle
	ACT_DOD_PRIMARYATTACK_RIFLE,
	ACT_DOD_SECONDARYATTACK_RIFLE,
	ACT_DOD_PRIMARYATTACK_PRONE_RIFLE,
	ACT_DOD_SECONDARYATTACK_PRONE_RIFLE,
	ACT_DOD_PRIMARYATTACK_PRONE_DEPLOYED_RIFLE,
	ACT_DOD_PRIMARYATTACK_DEPLOYED_RIFLE,

	// Bolt
	ACT_DOD_PRIMARYATTACK_BOLT,
	ACT_DOD_SECONDARYATTACK_BOLT,
	ACT_DOD_PRIMARYATTACK_PRONE_BOLT ,
	ACT_DOD_SECONDARYATTACK_PRONE_BOLT ,

	// Tommy
	ACT_DOD_PRIMARYATTACK_TOMMY,
	ACT_DOD_PRIMARYATTACK_PRONE_TOMMY,
	ACT_DOD_PRIMARYATTACK_DEPLOYED_TOMMY,
	ACT_DOD_PRIMARYATTACK_PRONE_DEPLOYED_TOMMY,

	// MP40
	ACT_DOD_PRIMARYATTACK_MP40,
	ACT_DOD_PRIMARYATTACK_PRONE_MP40 ,

	// MP44
	ACT_DOD_PRIMARYATTACK_MP44,
	ACT_DOD_PRIMARYATTACK_PRONE_MP44 ,

	// Greasegun
	ACT_DOD_PRIMARYATTACK_GREASE,
	ACT_DOD_PRIMARYATTACK_PRONE_GREASE ,

	// Pistols (Colt, Luger)
	ACT_DOD_PRIMARYATTACK_PISTOL,
	ACT_DOD_PRIMARYATTACK_PRONE_PISTOL ,

	// Mgs (mg42, mg34)
	ACT_DOD_PRIMARYATTACK_MG,
	ACT_DOD_PRIMARYATTACK_PRONE_MG ,
	ACT_DOD_PRIMARYATTACK_PRONE_DEPLOYED_MG ,
	ACT_DOD_PRIMARYATTACK_DEPLOYED_MG ,

	// 30cal
	ACT_DOD_PRIMARYATTACK_30CAL,
	ACT_DOD_PRIMARYATTACK_PRONE_30CAL,
	ACT_DOD_PRIMARYATTACK_DEPLOYED_30CAL,
	ACT_DOD_PRIMARYATTACK_PRONE_DEPLOYED_30CAL ,

	// Grenades
	ACT_DOD_PRIMARYATTACK_GREN,
	ACT_DOD_SECONDARYATTACK_GREN,
	ACT_DOD_PRIMARYATTACK_PRONE_GREN,

	// Knife
	ACT_DOD_PRIMARYATTACK_KNIFE,
	ACT_DOD_PRIMARYATTACK_PRONE_KNIFE,

	// Spade
	ACT_DOD_PRIMARYATTACK_SPADE,
	ACT_DOD_PRIMARYATTACK_PRONE_SPADE,

	// Bazooka
	ACT_DOD_PRIMARYATTACK_BAZOOKA,
	ACT_DOD_PRIMARYATTACK_PRONE_BAZOOKA,

	// Pschreck
	ACT_DOD_PRIMARYATTACK_PSCHRECK,
	ACT_DOD_PRIMARYATTACK_PRONE_PSCHRECK ,


	// Reloads
	ACT_DOD_RELOAD_GARAND,
	ACT_DOD_RELOAD_K43,
	ACT_DOD_RELOAD_BAR,
	ACT_DOD_RELOAD_MP40,
	ACT_DOD_RELOAD_MP44,
	ACT_DOD_RELOAD_BOLT,
	ACT_DOD_RELOAD_M1CARBINE,
	ACT_DOD_RELOAD_THOMPSON,
	ACT_DOD_RELOAD_GREASEGUN,
	ACT_DOD_RELOAD_PISTOL,
	ACT_DOD_RELOAD_FG42,

	// Bazookas
	ACT_DOD_RELOAD_BAZOOKA,
	ACT_DOD_ZOOMLOAD_BAZOOKA,
	ACT_DOD_RELOAD_PSCHRECK,
	ACT_DOD_ZOOMLOAD_PSCHRECK,

	// Deployed
	ACT_DOD_RELOAD_DEPLOYED_FG42,
	ACT_DOD_RELOAD_DEPLOYED_30CAL,
	ACT_DOD_RELOAD_DEPLOYED_MG42,
	ACT_DOD_RELOAD_DEPLOYED_MG34,
	ACT_DOD_RELOAD_DEPLOYED_BAR,

	// Prone
	ACT_DOD_RELOAD_PRONE_PISTOL,
	ACT_DOD_RELOAD_PRONE_GARAND,
	ACT_DOD_RELOAD_PRONE_M1CARBINE,
	ACT_DOD_RELOAD_PRONE_BOLT,
	ACT_DOD_RELOAD_PRONE_K43,
	ACT_DOD_RELOAD_PRONE_MP40,
	ACT_DOD_RELOAD_PRONE_MP44,
	ACT_DOD_RELOAD_PRONE_BAR,
	ACT_DOD_RELOAD_PRONE_GREASEGUN,
	ACT_DOD_RELOAD_PRONE_THOMPSON,
	ACT_DOD_RELOAD_PRONE_FG42,

	// Prone bazooka
	ACT_DOD_RELOAD_PRONE_BAZOOKA,
	ACT_DOD_ZOOMLOAD_PRONE_BAZOOKA,
	ACT_DOD_RELOAD_PRONE_PSCHRECK,
	ACT_DOD_ZOOMLOAD_PRONE_PSCHRECK,

	// Prone deployed
	ACT_DOD_RELOAD_PRONE_DEPLOYED_BAR,
	ACT_DOD_RELOAD_PRONE_DEPLOYED_FG42,
	ACT_DOD_RELOAD_PRONE_DEPLOYED_30CAL,
	ACT_DOD_RELOAD_PRONE_DEPLOYED_MG42,
	ACT_DOD_RELOAD_PRONE_DEPLOYED_MG34,

	// Prone zoomed aim
	ACT_DOD_PRONE_ZOOM_FORWARD_TOMMY,
	ACT_DOD_PRONE_ZOOM_FORWARD_BOLT,
	ACT_DOD_PRONE_ZOOM_FORWARD_BAZOOKA,
	ACT_DOD_PRONE_ZOOM_FORWARD_PSCHRECK,


	// Hand Signals
	ACT_DOD_HS_STICKTOGETHER,
	ACT_DOD_HS_FALLBACK,
	ACT_DOD_HS_NO,
	ACT_DOD_HS_YES,
	ACT_DOD_HS_SNIPER,
	ACT_DOD_HS_BACKUP,
	ACT_DOD_HS_ENEMYRIGHT,
	ACT_DOD_HS_ENEMYLEFT,
	ACT_DOD_HS_GRENADE,
	ACT_DOD_HS_FLANKLEFT,
	ACT_DOD_HS_MOVEOUT,
	ACT_DOD_HS_FLANKRIGHT,
	ACT_DOD_HS_AREACLEAR,
	ACT_DOD_HS_COVERINGFIRE,
	ACT_DOD_HS_TAKECOVER,
	ACT_DOD_HS_HOLDPOS,
	ACT_DOD_HS_SPREADOUT,
	ACT_DOD_HS_ENEMYSPOTTED,
	ACT_DOD_HS_DISPLACE,
	ACT_DOD_HS_PREPARE,
	ACT_DOD_HS_GOGOGO,
	ACT_DOD_HS_ENEMYAHEAD,
	ACT_DOD_HS_ENEMYBEHIND,
	ACT_DOD_HS_MGAHEAD,
	ACT_DOD_HS_MOVEUPMG,
	ACT_DOD_HS_CEASEFIRE,
	ACT_DOD_HS_USEGRENS,

	// Prone Hand Signals
	ACT_DOD_HS_PRONE_MOVEOUT,
	ACT_DOD_HS_PRONE_FLANKLEFT,
	ACT_DOD_HS_PRONE_FLANKRIGHT,
	ACT_DOD_HS_PRONE_BACKUP,
	ACT_DOD_HS_PRONE_AREACLEAR,
	ACT_DOD_HS_PRONE_FALLBACK,
	ACT_DOD_HS_PRONE_SNIPER,
	ACT_DOD_HS_PRONE_YES,
	ACT_DOD_HS_PRONE_NO,
	ACT_DOD_HS_PRONE_ENEMYLEFT,
	ACT_DOD_HS_PRONE_ENEMYRIGHT,
	ACT_DOD_HS_PRONE_GRENADE,
	ACT_DOD_HS_PRONE_COVERINGFIRE,
	ACT_DOD_HS_PRONE_HOLDPOS,
	ACT_DOD_HS_PRONE_SPREADOUT,
	ACT_DOD_HS_PRONE_STICKTOGETHER,
	ACT_DOD_HS_PRONE_TAKECOVER,
	ACT_DOD_HS_PRONE_ENEMYSPOTTED,
	ACT_DOD_HS_PRONE_DISPLACE,
	ACT_DOD_HS_PRONE_PREPARE,
	ACT_DOD_HS_PRONE_GOGOGO,
	ACT_DOD_HS_PRONE_ENEMYAHEAD,
	ACT_DOD_HS_PRONE_ENEMYBEHIND,
	ACT_DOD_HS_PRONE_MGAHEAD,
	ACT_DOD_HS_PRONE_MOVEUPMG,
	ACT_DOD_HS_PRONE_USEGRENS,
	ACT_DOD_HS_PRONE_CEASEFIRE,

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

	// this is the end of the global activities, private per-monster activities start here.
	LAST_SHARED_ACTIVITY,
} Activity;


#endif // AI_ACTIVITY_H
