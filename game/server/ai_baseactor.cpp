//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"

#include "sceneentity.h"
#include "choreoevent.h"
#include "choreoscene.h"
#include "choreoactor.h"
#include "ai_baseactor.h"
#include "ai_navigator.h"
#include "saverestore_utlvector.h"
#include "bone_setup.h"
#include "physics_npc_solver.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar flex_minplayertime( "flex_minplayertime", "5" );
ConVar flex_maxplayertime( "flex_maxplayertime", "7" );
ConVar flex_minawaytime( "flex_minawaytime", "0.5" );
ConVar flex_maxawaytime( "flex_maxawaytime", "1.0" );


BEGIN_DATADESC( CAI_BaseActor )

	DEFINE_FIELD( m_fLatchedPositions, FIELD_INTEGER ),
	DEFINE_FIELD( m_latchedEyeOrigin, FIELD_VECTOR ),
	DEFINE_FIELD( m_latchedEyeDirection, FIELD_VECTOR ),
	DEFINE_FIELD( m_latchedHeadDirection, FIELD_VECTOR ),
	DEFINE_FIELD( m_goalHeadDirection, FIELD_VECTOR ),
	DEFINE_FIELD( m_goalHeadInfluence, FIELD_FLOAT ),
	DEFINE_FIELD( m_goalSpineYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_goalBodyYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_goalHeadCorrection, FIELD_VECTOR ),
	DEFINE_FIELD( m_flBlinktime, FIELD_TIME ),
	DEFINE_FIELD( m_hLookTarget, FIELD_EHANDLE ),
	DEFINE_UTLVECTOR( m_lookQueue,	FIELD_EMBEDDED ), 
	DEFINE_UTLVECTOR( m_randomLookQueue, FIELD_EMBEDDED ),
	DEFINE_UTLVECTOR( m_syntheticLookQueue,	FIELD_EMBEDDED ), 
	DEFINE_FIELD( m_flNextRandomLookTime, FIELD_TIME ),
	DEFINE_FIELD( m_iszExpressionScene, FIELD_STRING ),
	DEFINE_FIELD( m_hExpressionSceneEnt, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flNextRandomExpressionTime, FIELD_TIME ),
	DEFINE_FIELD( m_iszIdleExpression, FIELD_STRING ),
	DEFINE_FIELD( m_iszAlertExpression, FIELD_STRING ),
	DEFINE_FIELD( m_iszCombatExpression, FIELD_STRING ),
	DEFINE_FIELD( m_iszDeathExpression, FIELD_STRING ),
	DEFINE_FIELD( m_ParameterBodyYaw, FIELD_INTEGER ),
	DEFINE_FIELD( m_ParameterSpineYaw, FIELD_INTEGER ),
	DEFINE_FIELD( m_ParameterNeckTrans, FIELD_INTEGER ),
	DEFINE_FIELD( m_ParameterHeadYaw, FIELD_INTEGER ),
	DEFINE_FIELD( m_ParameterHeadPitch, FIELD_INTEGER ),
	DEFINE_FIELD( m_ParameterHeadRoll, FIELD_INTEGER ),
	DEFINE_FIELD( m_FlexweightBodyRightLeft, FIELD_INTEGER ),
	DEFINE_FIELD( m_FlexweightChestRightLeft, FIELD_INTEGER ),
	DEFINE_FIELD( m_FlexweightHeadForwardBack, FIELD_INTEGER ),
	DEFINE_FIELD( m_FlexweightHeadRightLeft, FIELD_INTEGER ),
	DEFINE_FIELD( m_FlexweightHeadUpDown, FIELD_INTEGER ),
	DEFINE_FIELD( m_FlexweightHeadTilt, FIELD_INTEGER ),

	DEFINE_FIELD( m_ParameterGestureHeight, FIELD_INTEGER ),
	DEFINE_FIELD( m_ParameterGestureWidth, FIELD_INTEGER ),
	DEFINE_FIELD( m_FlexweightGestureUpDown, FIELD_INTEGER ),
	DEFINE_FIELD( m_FlexweightGestureRightLeft, FIELD_INTEGER ),
	DEFINE_FIELD( m_flAccumYawDelta, FIELD_FLOAT ),
	DEFINE_FIELD( m_flAccumYawScale, FIELD_FLOAT ),

	DEFINE_ARRAY( m_flextarget, FIELD_FLOAT, 64 ),

	DEFINE_KEYFIELD( m_bDontUseSemaphore, FIELD_BOOLEAN, "DontUseSpeechSemaphore" ),

	DEFINE_KEYFIELD( m_iszExpressionOverride, FIELD_STRING, "ExpressionOverride" ),

	DEFINE_EMBEDDEDBYREF( m_pExpresser ),

	DEFINE_INPUTFUNC( FIELD_STRING,	"SetExpressionOverride",	InputSetExpressionOverride ),

END_DATADESC()


BEGIN_SIMPLE_DATADESC( CAI_InterestTarget_t )
	DEFINE_FIELD( m_eType,		FIELD_INTEGER ),
	DEFINE_FIELD( m_hTarget,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecPosition,	FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flStartTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flEndTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flRamp,		FIELD_FLOAT ),
	DEFINE_FIELD( m_flInterest,	FIELD_FLOAT ),
END_DATADESC()




//-----------------------------------------------------------------------------
// Purpose: clear out latched state
//-----------------------------------------------------------------------------
void CAI_BaseActor::StudioFrameAdvance ()
{
	// clear out head and eye latched values
	m_fLatchedPositions &= ~(HUMANOID_LATCHED_ALL);

	BaseClass::StudioFrameAdvance();
}


void CAI_BaseActor::Precache()
{
	BaseClass::Precache();

	if ( NULL_STRING != m_iszExpressionOverride )
	{
		PrecacheInstancedScene( STRING( m_iszExpressionOverride ) );
	}

	if ( m_iszIdleExpression != NULL_STRING )
	{
		PrecacheInstancedScene( STRING(m_iszIdleExpression ) );
	}

	if ( m_iszCombatExpression != NULL_STRING )
	{
		PrecacheInstancedScene( STRING(m_iszCombatExpression ) );
	}

	if ( m_iszAlertExpression != NULL_STRING )
	{
		PrecacheInstancedScene( STRING(m_iszAlertExpression) );
	}

	if ( m_iszDeathExpression != NULL_STRING )
	{
		PrecacheInstancedScene( STRING(m_iszDeathExpression) );
	}
}

static char const *g_ServerSideFlexControllers[] = 
{
	"body_rightleft",
	//"body_updown",
	//"body_tilt",
	"chest_rightleft",
	//"chest_updown",
	//"chest_tilt",
	"head_forwardback",
	"head_rightleft",
	"head_updown",
	"head_tilt",

	"gesture_updown",
	"gesture_rightleft"
};

//-----------------------------------------------------------------------------
// Purpose: Static method 
// Input  : *szName - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_BaseActor::IsServerSideFlexController( char const *szName )
{
	int c = ARRAYSIZE( g_ServerSideFlexControllers );
	for ( int i = 0; i < c; ++i )
	{
		if ( !Q_stricmp( szName, g_ServerSideFlexControllers[ i ] ) )
			return true;
	}
	return false;
}

void CAI_BaseActor::SetModel( const char *szModelName )
{
	BaseClass::SetModel( szModelName );

	//Init( m_ParameterBodyTransY, "body_trans_Y" );
	//Init( m_ParameterBodyTransX, "body_trans_X" );
	//Init( m_ParameterBodyLift, "body_lift" );
	Init( m_ParameterBodyYaw, "body_yaw" );
	//Init( m_ParameterBodyPitch, "body_pitch" );
	//Init( m_ParameterBodyRoll, "body_roll" );
	Init( m_ParameterSpineYaw, "spine_yaw" );
	//Init( m_ParameterSpinePitch, "spine_pitch" );
	//Init( m_ParameterSpineRoll, "spine_roll" );
	Init( m_ParameterNeckTrans, "neck_trans" );
	Init( m_ParameterHeadYaw, "head_yaw" );
	Init( m_ParameterHeadPitch, "head_pitch" );
	Init( m_ParameterHeadRoll, "head_roll" );

	//Init( m_FlexweightMoveRightLeft, "move_rightleft" );
	//Init( m_FlexweightMoveForwardBack, "move_forwardback" );
	//Init( m_FlexweightMoveUpDown, "move_updown" );
	Init( m_FlexweightBodyRightLeft, "body_rightleft" );
	//Init( m_FlexweightBodyUpDown, "body_updown" );
	//Init( m_FlexweightBodyTilt, "body_tilt" );
	Init( m_FlexweightChestRightLeft, "chest_rightleft" );
	//Init( m_FlexweightChestUpDown, "chest_updown" );
	//Init( m_FlexweightChestTilt, "chest_tilt" );
	Init( m_FlexweightHeadForwardBack, "head_forwardback" );
	Init( m_FlexweightHeadRightLeft, "head_rightleft" );
	Init( m_FlexweightHeadUpDown, "head_updown" );
	Init( m_FlexweightHeadTilt, "head_tilt" );

	Init( m_ParameterGestureHeight, "gesture_height" );
	Init( m_ParameterGestureWidth, "gesture_width" );
	Init( m_FlexweightGestureUpDown, "gesture_updown" );
	Init( m_FlexweightGestureRightLeft, "gesture_rightleft" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

bool CAI_BaseActor::StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget )
{
	Assert( info );
	Assert( info->m_pScene );
	Assert( info->m_pEvent );

	// FIXME: this code looks duplicated
	switch ( info->m_pEvent->GetType() )
	{
	case CChoreoEvent::FACE: 
		{
			return BaseClass::StartSceneEvent( info, scene, event, actor, pTarget );
		}
		break;

	case CChoreoEvent::GENERIC:
		{
			if (stricmp( event->GetParameters(), "AI_BLINK") == 0)
			{
				info->m_nType = SCENE_AI_BLINK;
				// blink eyes
				Blink();
				// don't blink for duration, or next random blink time
				float flDuration = (event->GetEndTime() - scene->GetTime());
				m_flBlinktime = gpGlobals->curtime + max( flDuration, random->RandomFloat( 1.5, 4.5 ) ); 
			}
			else if (stricmp( event->GetParameters(), "AI_HOLSTER") == 0)
			{
				// FIXME: temp code for test
				info->m_nType = SCENE_AI_HOLSTER;
				info->m_iLayer = HolsterWeapon();
				return true;
			}
			else if (stricmp( event->GetParameters(), "AI_UNHOLSTER") == 0)
			{
				// FIXME: temp code for test
				info->m_nType = SCENE_AI_UNHOLSTER;
				info->m_iLayer = UnholsterWeapon();
				return true;
			}
			else if (stricmp( event->GetParameters(), "AI_AIM") == 0)
			{
				info->m_nType = SCENE_AI_AIM;
				info->m_hTarget = pTarget;
			}
			else if (stricmp( event->GetParameters(), "AI_RANDOMLOOK") == 0)
			{
				info->m_nType = SCENE_AI_RANDOMLOOK;
				info->m_flNext = 0.0;
			}
			else if (stricmp( event->GetParameters(), "AI_RANDOMFACEFLEX") == 0)
			{
				info->m_nType = SCENE_AI_RANDOMFACEFLEX;
				info->m_flNext = 0.0;
				info->InitWeight( this );
			}
			else if (stricmp( event->GetParameters(), "AI_RANDOMHEADFLEX") == 0)
			{
				info->m_nType = SCENE_AI_RANDOMHEADFLEX;
				info->m_flNext = 0.0;
			}	
			else if (stricmp( event->GetParameters(), "AI_IGNORECOLLISION") == 0)
			{
				CBaseEntity *pTarget = FindNamedEntity( event->GetParameters2( ) );

				if (pTarget)
				{
					info->m_nType = SCENE_AI_IGNORECOLLISION;
					info->m_hTarget = pTarget;
					float remaining = event->GetEndTime() - scene->GetTime();
					NPCPhysics_CreateSolver( this, pTarget, true, remaining );
					info->m_flNext = gpGlobals->curtime + remaining;
					return true;
				}
				else
				{
					Warning( "CSceneEntity %s unable to find actor named \"%s\"\n", scene->GetFilename(), event->GetParameters2() );
					return false;
				}
			}
			else if (stricmp( event->GetParameters(), "AI_DISABLEAI") == 0)
			{
				info->m_nType = SCENE_AI_DISABLEAI;
			}
			else
			{
				return BaseClass::StartSceneEvent( info, scene, event, actor, pTarget );
			}
			return true;
		}
		break;

	default:
		return BaseClass::StartSceneEvent( info, scene, event, actor, pTarget );
	}
}


bool CAI_BaseActor::ProcessSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event )
{
	Assert( info );
	Assert( info->m_pScene );
	Assert( info->m_pEvent );

	// FIXME: this code looks duplicated
	switch ( info->m_pEvent->GetType() )
	{
	case CChoreoEvent::FACE: 
		{
			// make sure target exists
			if (info->m_hTarget == NULL)
				return false;

			bool bInScene = false;
			
			// lockbodyfacing is designed to run on top of both normal AI and on top of
			// scripted_sequences.  By allowing torso turns during post-idles, pre-idles, 
			// act-busy's, scripted_sequences, normal AI movements, etc., it increases 
			// the functionality of those AI features without breaking their assuptions 
			// that the entity won't be made to "turn" by something outside of those 
			// AI's control.
			// lockbody facing is also usefull when npcs are moving and you want them to turn
			// towards something but still walk in the direction of travel.
			if (!event->IsLockBodyFacing())
				bInScene = EnterSceneSequence( scene, event, true );

			// make sure we're still able to play this command
			if (!info->m_bStarted)
			{
				info->m_flInitialYaw = GetLocalAngles().y;
				info->m_flTargetYaw = info->m_flInitialYaw;
				info->m_flFacingYaw = info->m_flInitialYaw;
				if (IsMoving())
				{
					info->m_flWeight = 1.0;
				}
				else
				{
					info->m_flWeight = 0.0;
				}
			}

			// lock in place if aiming at self
			if (info->m_hTarget == this)
			{
				return true;
			}

			if (!bInScene || info->m_bIsMoving != IsMoving())
			{
				info->m_flInitialYaw = GetLocalAngles().y;
			}
			info->m_bIsMoving = IsMoving();

			// Msg("%f : %f - %f\n", scene->GetTime(), event->GetStartTime(), event->GetEndTime() );
			float flTime = clamp( scene->GetTime(), event->GetStartTime(), event->GetEndTime() - 0.1 );
			float intensity = event->GetIntensity( flTime );

			// clamp in-ramp to 0.5 seconds
			float flDuration = scene->GetTime() - event->GetStartTime();
			float flMaxIntensity = flDuration < 0.5f ? SimpleSpline( flDuration / 0.5f ) : 1.0f;
			intensity = clamp( intensity, 0.0f, flMaxIntensity );

			if (bInScene && info->m_bIsMoving)
			{
				info->m_flInitialYaw = GetLocalAngles().y;
			}

			if (!event->IsLockBodyFacing())
			{
				if (!info->m_bIsMoving && bInScene)
				{
					AccumulateIdealYaw( info->m_flFacingYaw, intensity );
				}
			}

			float diff;
			float dir;
			float flSpineYaw;
			float flBodyYaw;
			
			// move upper body to account for missing body yaw
			diff = UTIL_AngleDiff( info->m_flTargetYaw, GetLocalAngles().y );
			if (diff < 0)
			{
				diff = -diff;
				dir = -1;
			}
			else
			{
				dir = 1;
			}
			flSpineYaw = min( diff, 30 );
			flBodyYaw = min( diff - flSpineYaw, 30 );
			m_goalSpineYaw = m_goalSpineYaw * (1.0 - intensity) + intensity * flSpineYaw * dir;
			m_goalBodyYaw = m_goalBodyYaw * (1.0 - intensity) + intensity * flBodyYaw * dir;

			CAI_BaseNPC *pGoalNpc = info->m_hTarget->MyNPCPointer();

			float goalYaw = GetLocalAngles().y;
			
			if ( pGoalNpc )
			{
				goalYaw = CalcIdealYaw( pGoalNpc->FacingPosition() );
			}
			else
			{
				goalYaw = CalcIdealYaw( info->m_hTarget->EyePosition() );
			}

			diff = UTIL_AngleDiff( goalYaw, info->m_flInitialYaw ) * intensity;
			dir = 1.0;

			// debounce delta a bit
			info->m_flTargetYaw = UTIL_AngleMod( info->m_flInitialYaw + diff );

			if (diff < 0)
			{
				diff = -diff;
				dir = -1;
			}

			// calc how much to use the spine for turning
			float spineintensity = (1.0 - max( 0.0, (intensity - 0.5) / 0.5 ));
			// force spine to full if not in scene or locked
			if (!bInScene || event->IsLockBodyFacing() )
			{
				spineintensity = 1.0;
			}

			flSpineYaw = min( diff * spineintensity, 30 );
			flBodyYaw = min( diff * spineintensity - flSpineYaw, 30 );
			info->m_flFacingYaw = info->m_flInitialYaw + (diff - flBodyYaw - flSpineYaw) * dir;

			if (!event->IsLockBodyFacing())
			{
				AddFacingTarget( info->m_hTarget, intensity, 0.2 ); // facing targets are lagged by one frame
			}
			return true;
		}
	case CChoreoEvent::GENERIC:
		{
			switch(info->m_nType)
			{
				case SCENE_AI_BLINK:
					{
						// keep eyes not blinking for duration
						float flDuration = (event->GetEndTime() - scene->GetTime());
						m_flBlinktime = max( m_flBlinktime, gpGlobals->curtime + flDuration );
					}
					return true;
				case SCENE_AI_HOLSTER:
					{
					}
					return true;
				case SCENE_AI_UNHOLSTER:
					{
					}
					return true;
				case SCENE_AI_AIM:
					{
						if ( info->m_hTarget )
						{
							Vector vecAimTargetLoc = info->m_hTarget->EyePosition();
							Vector vecAimDir = vecAimTargetLoc - EyePosition();

							VectorNormalize( vecAimDir );
							SetAim( vecAimDir);
						}
					}
					return true;
				case SCENE_AI_RANDOMLOOK:
					return true;
				case SCENE_AI_RANDOMFACEFLEX:
					return RandomFaceFlex( info, scene, event );
				case SCENE_AI_RANDOMHEADFLEX:
					return true;
				case SCENE_AI_IGNORECOLLISION:
					if (info->m_hTarget && info->m_flNext < gpGlobals->curtime)
					{
						float remaining = event->GetEndTime() - scene->GetTime();
						NPCPhysics_CreateSolver( this, info->m_hTarget, true, remaining );
						info->m_flNext = gpGlobals->curtime + remaining;
					}

					// FIXME: needs to handle scene pause
					return true;
				case SCENE_AI_DISABLEAI:
					if (!(GetState() == NPC_STATE_SCRIPT  || IsCurSchedule( SCHED_SCENE_GENERIC )) )
					{
						EnterSceneSequence( scene, event );
					}
					return true;
				default:
					return false;
			}
		}
		break;
	default:
		return BaseClass::ProcessSceneEvent( info, scene, event );
	}
}


bool CAI_BaseActor::RandomFaceFlex( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event )
{
	if (info->m_flNext < gpGlobals->curtime)
	{
		const flexsettinghdr_t *pSettinghdr = ( const flexsettinghdr_t * )FindSceneFile( event->GetParameters2() );
		if (pSettinghdr == NULL)
		{
			pSettinghdr = ( const flexsettinghdr_t * )FindSceneFile( "random" );
		}
		if ( pSettinghdr )
		{
			info->m_flNext = gpGlobals->curtime + random->RandomFloat( 0.3, 0.5 ) * (30.0 / pSettinghdr->numflexsettings);

			flexsetting_t const *pSetting = NULL;
			pSetting = pSettinghdr->pSetting( random->RandomInt( 0, pSettinghdr->numflexsettings - 1 ) );

			flexweight_t *pWeights = NULL;
			int truecount = pSetting->psetting( (byte *)pSettinghdr, 0, &pWeights );
			if ( !pWeights )
				return false;

			int i;
			for (i = 0; i < truecount; i++, pWeights++)
			{
				// Translate to local flex controller
				// this is translating from the settings's local index to the models local index
				int index = FlexControllerLocalToGlobal( pSettinghdr, pWeights->key );

				// FIXME: this is supposed to blend based on pWeight->influence, but the order is wrong...
				// float value = GetFlexWeight( index ) * (1 - scale * pWeights->influence) + scale * pWeights->weight;

				// Add scaled weighting in to total
				m_flextarget[ index ] = pWeights->weight;
			}
		}
		else
		{
			return false;
		}
	}

	// adjust intensity if this is a background scene and there's other flex animations playing
	float intensity = info->UpdateWeight( this ) * event->GetIntensity( scene->GetTime() );

	// slide it up.
	for (LocalFlexController_t i = LocalFlexController_t(0); i < GetNumFlexControllers(); i++)
	{
		float weight = GetFlexWeight( i );

		if (weight != m_flextarget[i])
		{
			float delta = (m_flextarget[i] - weight) / random->RandomFloat( 2.0, 4.0 );
			weight = weight + delta * intensity;
		}
		weight = clamp( weight, 0.0f, 1.0f );
		SetFlexWeight( i, weight );
	}

	return true;
}





bool CAI_BaseActor::ClearSceneEvent( CSceneEventInfo *info, bool fastKill, bool canceled )
{
	Assert( info );
	Assert( info->m_pScene );
	Assert( info->m_pEvent );

	// FIXME: this code looks duplicated
	switch ( info->m_pEvent->GetType() )
	{
	case CChoreoEvent::FACE: 
		{
			return BaseClass::ClearSceneEvent( info, fastKill, canceled );
		}
		break;
	default:
		return BaseClass::ClearSceneEvent( info, fastKill, canceled );
	}
}



bool CAI_BaseActor::CheckSceneEventCompletion( CSceneEventInfo *info, float currenttime, CChoreoScene *scene, CChoreoEvent *event )
{
	Assert( info );
	Assert( info->m_pScene );
	Assert( info->m_pEvent );

	switch ( event->GetType() )
	{
	case CChoreoEvent::GENERIC:
		{
			switch( info->m_nType)
			{
			case SCENE_AI_HOLSTER:
			case SCENE_AI_UNHOLSTER:
				{
					if (info->m_iLayer == -1)
					{
						return true;
					}
					float preload = event->GetEndTime() - currenttime;
					if (preload < 0)
					{
						return true;
					}
					float t = (1.0 - GetLayerCycle( info->m_iLayer )) * SequenceDuration( GetLayerSequence( info->m_iLayer ) );

					return (t <= preload);
				}
			}
		}
	}

	return BaseClass::CheckSceneEventCompletion( info, currenttime, scene, event );
}



//-----------------------------------------------------------------------------
// Purpose: clear out latched state
//-----------------------------------------------------------------------------
void CAI_BaseActor::SetViewtarget( const Vector &viewtarget )
{
	// clear out eye latch
	m_fLatchedPositions &= ~HUMANOID_LATCHED_EYE;

	BaseClass::SetViewtarget( viewtarget );
}


//-----------------------------------------------------------------------------
// Purpose: Returns true position of the eyeballs
//-----------------------------------------------------------------------------
void CAI_BaseActor::UpdateLatchedValues( ) 
{ 
	if (!(m_fLatchedPositions & HUMANOID_LATCHED_HEAD))
	{
		// set head latch
		m_fLatchedPositions |= HUMANOID_LATCHED_HEAD;

		if (!HasCondition( COND_IN_PVS ) || !GetAttachment( "eyes", m_latchedEyeOrigin, &m_latchedHeadDirection ))
		{
			m_latchedEyeOrigin = BaseClass::EyePosition( );
			AngleVectors( GetLocalAngles(), &m_latchedHeadDirection );
		}
		// clear out eye latch
		m_fLatchedPositions &= ~(HUMANOID_LATCHED_EYE);
		// DevMsg( "eyeball %4f %4f %4f  : %3f %3f %3f\n", origin.x, origin.y, origin.z, angles.x, angles.y, angles.z );
	}

	/*if (!(m_fLatchedPositions & HUMANOID_LATCHED_EYE))
	{
		m_fLatchedPositions |= HUMANOID_LATCHED_EYE;

		if ( CapabilitiesGet() & bits_CAP_ANIMATEDFACE )
		{
			m_latchedEyeDirection = GetViewtarget() - m_latchedEyeOrigin; 
			VectorNormalize( m_latchedEyeDirection );
		}
		else
		{
			m_latchedEyeDirection = m_latchedHeadDirection;
		}
	}*/
}


//-----------------------------------------------------------------------------
// Purpose: Returns true position of the eyeballs
//-----------------------------------------------------------------------------
Vector CAI_BaseActor::EyePosition( )
{ 
	UpdateLatchedValues();

	return m_latchedEyeOrigin;
}


#define MIN_LOOK_TARGET_DIST 1.0f
#define MAX_FULL_LOOK_TARGET_DIST 10.0f

//-----------------------------------------------------------------------------
// Purpose: Returns true if target is in legal range of eye movement for the current head position
//-----------------------------------------------------------------------------
bool CAI_BaseActor::ValidEyeTarget(const Vector &lookTargetPos)
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Integrate head turn over time
//-----------------------------------------------------------------------------
void CAI_BaseActor::SetHeadDirection( const Vector &vTargetPos, float flInterval)
{
	Assert(0); // Actors shouldn't be calling this, it doesn't do anything
}

float CAI_BaseActor::ClampWithBias( PoseParameter_t index, float value, float base )
{
	return EdgeLimitPoseParameter( (int)index, value, base );
}


//-----------------------------------------------------------------------------
// Purpose: Accumulate all the wanted yaw changes
//-----------------------------------------------------------------------------

void CAI_BaseActor::AccumulateIdealYaw( float flYaw, float flIntensity )
{
	float diff = AngleDiff( flYaw, GetLocalAngles().y );
	m_flAccumYawDelta += diff * flIntensity;
	m_flAccumYawScale += flIntensity;
}


//-----------------------------------------------------------------------------
// Purpose: do any pending yaw movements
//-----------------------------------------------------------------------------

bool CAI_BaseActor::SetAccumulatedYawAndUpdate( void )
{
	if (m_flAccumYawScale > 0.0)
	{
		float diff = m_flAccumYawDelta / m_flAccumYawScale;
		float facing = GetLocalAngles().y + diff;

		m_flAccumYawDelta = 0.0;
		m_flAccumYawScale = 0.0;

		if (IsCurSchedule( SCHED_SCENE_GENERIC ))
		{
			if (!IsMoving())
			{
				GetMotor()->SetIdealYawAndUpdate( facing );
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: match actors "forward" attachment to point in direction of vHeadTarget
//-----------------------------------------------------------------------------

void CAI_BaseActor::UpdateBodyControl( )
{
	// FIXME: only during idle, or in response to accel/decel
	//Set( m_ParameterBodyTransY, Get( m_FlexweightMoveRightLeft ) );
	//Set( m_ParameterBodyTransX, Get( m_FlexweightMoveForwardBack ) );
	//Set( m_ParameterBodyLift, Get( m_FlexweightMoveUpDown ) );
	Set( m_ParameterBodyYaw, Get( m_FlexweightBodyRightLeft ) + m_goalBodyYaw );
	//Set( m_ParameterBodyPitch, Get( m_FlexweightBodyUpDown ) );
	//Set( m_ParameterBodyRoll, Get( m_FlexweightBodyTilt ) );
	Set( m_ParameterSpineYaw, Get( m_FlexweightChestRightLeft ) + m_goalSpineYaw );
	//Set( m_ParameterSpinePitch, Get( m_FlexweightChestUpDown ) );
	//Set( m_ParameterSpineRoll, Get( m_FlexweightChestTilt ) );
	Set( m_ParameterNeckTrans, Get( m_FlexweightHeadForwardBack ) );
}


static ConVar scene_clamplookat( "scene_clamplookat", "1", FCVAR_NONE, "Clamp head turns to a max of 20 degrees per think." );


void CAI_BaseActor::UpdateHeadControl( const Vector &vHeadTarget, float flHeadInfluence )
{
}



//------------------------------------------------------------------------------
// Purpose : Calculate the NPC's eye direction in 2D world space
// Input   :
// Output  :
//------------------------------------------------------------------------------
Vector CAI_BaseActor::EyeDirection2D( void )
{
	Vector vEyeDirection = EyeDirection3D( );
	vEyeDirection.z = 0;

	vEyeDirection.AsVector2D().NormalizeInPlace();

	return vEyeDirection;
}

//------------------------------------------------------------------------------
// Purpose : Calculate the NPC's eye direction in 2D world space
// Input   :
// Output  :
//------------------------------------------------------------------------------
Vector CAI_BaseActor::EyeDirection3D( void )
{
	UpdateLatchedValues( );

	return m_latchedEyeDirection;
}

//------------------------------------------------------------------------------
// Purpose : Calculate the NPC's head direction in 2D world space
// Input   :
// Output  :
//------------------------------------------------------------------------------
Vector CAI_BaseActor::HeadDirection2D( void )
{	
	Vector vHeadDirection = HeadDirection3D();
	vHeadDirection.z = 0;
	vHeadDirection.AsVector2D().NormalizeInPlace();
	return vHeadDirection;
}

//------------------------------------------------------------------------------
// Purpose : Calculate the NPC's head direction in 3D world space
// Input   :
// Output  :
//------------------------------------------------------------------------------
Vector CAI_BaseActor::HeadDirection3D( )
{	
	UpdateLatchedValues( );

	return m_latchedHeadDirection;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CAI_BaseActor::HasActiveLookTargets( void )
{
	return m_lookQueue.Count() != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Make sure we're looking at what we're shooting at
//-----------------------------------------------------------------------------

void CAI_BaseActor::StartTaskRangeAttack1( const Task_t *pTask )
{
	BaseClass::StartTaskRangeAttack1( pTask );
	if (GetEnemy())
	{
		AddLookTarget( GetEnemy(), 1.0, 0.5, 0.2 );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Set direction that the NPC is looking
//-----------------------------------------------------------------------------
void CAI_BaseActor::AddLookTarget( CBaseEntity *pTarget, float flImportance, float flDuration, float flRamp )
{
	m_lookQueue.Add( pTarget, flImportance, flDuration, flRamp );
}


void CAI_BaseActor::AddLookTarget( const Vector &vecPosition, float flImportance, float flDuration, float flRamp )
{
	m_lookQueue.Add( vecPosition, flImportance, flDuration, flRamp );
}

//-----------------------------------------------------------------------------
// Purpose: Maintain eye, head, body postures, etc.
//-----------------------------------------------------------------------------
void CAI_BaseActor::MaintainLookTargets( float flInterval )
{
}

//-----------------------------------------------------------------------------

void CAI_BaseActor::PlayExpressionForState( NPC_STATE state )
{
	// If we have an override expression, use it above everything else
	if ( m_iszExpressionOverride != NULL_STRING && state != NPC_STATE_DEAD )
	{
		SetExpression( STRING(m_iszExpressionOverride) );
		return;
	}

	// If we have a random expression, use that
	const char *pszExpression = SelectRandomExpressionForState( state );
	if ( pszExpression && *pszExpression )
	{
		float flDuration = SetExpression( pszExpression );
		m_flNextRandomExpressionTime = gpGlobals->curtime + flDuration;
		return;
	}
	else
	{
		// Stop looking for random expressions for this state
		m_flNextRandomExpressionTime = 0;
	}

	// Lastly, use the base expression loops
	switch ( state )
	{
	case NPC_STATE_IDLE:
		if ( m_iszIdleExpression != NULL_STRING )
		{
			SetExpression( STRING(m_iszIdleExpression) );
		}
		break;

	case NPC_STATE_COMBAT:
		if ( m_iszCombatExpression != NULL_STRING )
		{
			SetExpression( STRING(m_iszCombatExpression) );
		}
		break;

	case NPC_STATE_ALERT:
		if ( m_iszAlertExpression != NULL_STRING )
		{
			SetExpression( STRING(m_iszAlertExpression) );
		}
		break;

	case NPC_STATE_PLAYDEAD:
	case NPC_STATE_DEAD:
		if ( m_iszDeathExpression != NULL_STRING )
		{
			SetExpression( STRING(m_iszDeathExpression) );
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Return a random expression for the specified state to play over 
//			the state's expression loop.
//-----------------------------------------------------------------------------
const char *CAI_BaseActor::SelectRandomExpressionForState( NPC_STATE state )
{
	return NULL;
}

//-----------------------------------------------------------------------------

void CAI_BaseActor::OnStateChange( NPC_STATE OldState, NPC_STATE NewState )
{
	PlayExpressionForState( NewState );

	BaseClass::OnStateChange( OldState, NewState );
}

//-----------------------------------------------------------------------------

float CAI_BaseActor::SetExpression( const char *pszExpressionScene )
{
	if ( !pszExpressionScene || !*pszExpressionScene )
	{
		ClearExpression();
		return 0;
	}

	if ( m_iszExpressionScene != NULL_STRING && stricmp( STRING(m_iszExpressionScene), pszExpressionScene ) == 0 )
	{
		return 0;
	}

	if ( m_hExpressionSceneEnt != NULL )
	{
		StopScriptedScene( this, m_hExpressionSceneEnt );
	}

	m_iszExpressionScene = NULL_STRING;
	if ( pszExpressionScene )
	{
		float flDuration = InstancedScriptedScene( this, pszExpressionScene, &m_hExpressionSceneEnt, 0.0, true );

		if ( m_hExpressionSceneEnt != NULL )
		{
			m_iszExpressionScene = AllocPooledString( pszExpressionScene );
		}

		return flDuration;
	}

	return 0;
}

//-----------------------------------------------------------------------------

void CAI_BaseActor::ClearExpression()
{
	if ( m_hExpressionSceneEnt != NULL )
	{
		StopScriptedScene( this, m_hExpressionSceneEnt );
	}
	m_iszExpressionScene = NULL_STRING;
}

//-----------------------------------------------------------------------------

const char *CAI_BaseActor::GetExpression()
{
	return STRING(m_iszExpressionScene);
}

//-----------------------------------------------------------------------------

void CAI_BaseActor::InputSetExpressionOverride( inputdata_t &inputdata )
{
	bool fHadOverride = ( m_iszExpressionOverride != NULL_STRING );
	m_iszExpressionOverride = inputdata.value.StringID();
	if (  m_iszExpressionOverride != NULL_STRING )
	{
		SetExpression( STRING(m_iszExpressionOverride) );
	}
	else if ( fHadOverride )
	{
		PlayExpressionForState( GetState() );
	}
}

//-----------------------------------------------------------------------------

bool CAI_BaseActor::UseSemaphore( void )
{
	if ( m_bDontUseSemaphore )
		return false;

	return true;
}

//-----------------------------------------------------------------------------

CAI_Expresser *CAI_BaseActor::CreateExpresser()
{
	m_pExpresser = new CAI_Expresser(this);
	return m_pExpresser;
}

//-----------------------------------------------------------------------------

CAI_Expresser *CAI_BaseActor::GetExpresser() 
{ 
	return m_pExpresser; 
}
	
//-----------------------------------------------------------------------------

bool CAI_BaseActor::CreateComponents()
{
	if ( !BaseClass::CreateComponents() )
		return false;

	m_pExpresser = CreateExpresser();
	if ( !m_pExpresser)
		return false;

	m_pExpresser->Connect(this);

	return true;
}

//-----------------------------------------------------------------------------
