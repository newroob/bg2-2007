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
		Tomas "Tjoppen" H�rdin		tjoppen@gamedev.se

	You may also contact the (future) team via the Battle Grounds website and/or forum at:
		www.bgmod.com

	 Note that because of the sheer volume of files in the Source SDK this
	notice cannot be put in all of them, but merely the ones that have any
	changes from the original SDK.
	 In order to facilitate easy searching, all changes are and must be
	commented on the following form:

	//BG2 - <name of contributer>[ - <small description>]
*/

//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "gamemovement.h"
#include "in_buttons.h"
#include <stdarg.h>
#include "movevars_shared.h"
#include "engine/IEngineTrace.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "decals.h"

#if defined(HL2_DLL) || defined(HL2_CLIENT_DLL)
#include "hl_movedata.h"
#endif
//BG2 - Draco - no hl2 player stuff on server!
#ifndef CLIENT_DLL
	#include "hl2mp_player.h"
#else
	#include "c_hl2mp_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	STOP_EPSILON		0.1
#define	MAX_CLIP_PLANES		5

#include "filesystem.h"
#include <stdarg.h>

extern IFileSystem *filesystem;

#ifndef CLIENT_DLL
	#include "env_player_surface_trigger.h"
	static ConVar dispcoll_drawplane( "dispcoll_drawplane", "0" );
#endif


// tickcount currently isn't set during prediction, although gpGlobals->curtime and
// gpGlobals->frametime are. We should probably set tickcount (to player->m_nTickBase),
// but we're REALLY close to shipping, so we can change that later and people can use
// player->CurrentCommandNumber() in the meantime.
#define tickcount USE_PLAYER_CURRENT_COMMAND_NUMBER__INSTEAD_OF_TICKCOUNT



// [MD] I'll remove this eventually. For now, I want the ability to A/B the optimizations.
bool g_bMovementOptimizations = true;

// Roughly how often we want to update the info about the ground surface we're on.
// We don't need to do this very often.
#define CATEGORIZE_GROUND_SURFACE_INTERVAL			0.3f
#define CATEGORIZE_GROUND_SURFACE_TICK_INTERVAL   ( (int)( CATEGORIZE_GROUND_SURFACE_INTERVAL / TICK_INTERVAL ) )

#define CHECK_STUCK_INTERVAL			0.4f
#define CHECK_STUCK_TICK_INTERVAL		( (int)( CHECK_STUCK_INTERVAL / TICK_INTERVAL ) )

#define CHECK_LADDER_INTERVAL			0.2f
#define CHECK_LADDER_TICK_INTERVAL		( (int)( CHECK_LADDER_INTERVAL / TICK_INTERVAL ) )

void COM_Log( char *pszFile, char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	FileHandle_t fp;
	char *pfilename;
	
	if ( !pszFile )
	{
		pfilename = "hllog.txt";
	}
	else
	{
		pfilename = pszFile;
	}
	va_start (argptr,fmt);
	Q_vsnprintf(string, sizeof( string ), fmt,argptr);
	va_end (argptr);

	fp = filesystem->Open( pfilename, "a+t");
	if (fp)
	{
		filesystem->FPrintf(fp, "%s", string);
		filesystem->Close(fp);
	}
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Debug - draw the displacement collision plane.
//-----------------------------------------------------------------------------
void DrawDispCollPlane( CBaseTrace *pTrace )
{
	float flLength = 30.0f;

	// Create a basis, based on the impact normal.
	int nMajorAxis = 0;
	Vector vecBasisU, vecBasisV, vecNormal;
	vecNormal = pTrace->plane.normal;
	float flAxisValue = vecNormal[0];
	if ( fabs( vecNormal[1] ) > fabs( flAxisValue ) ) { nMajorAxis = 1; flAxisValue = vecNormal[1]; }
	if ( fabs( vecNormal[2] ) > fabs( flAxisValue ) ) { nMajorAxis = 2; }
	if ( ( nMajorAxis == 1 ) || ( nMajorAxis == 2 ) )
	{
		vecBasisU.Init( 1.0f, 0.0f, 0.0f );
	}
	else
	{
		vecBasisU.Init( 0.0f, 1.0f, 0.0f );
	}

	vecBasisV = vecNormal.Cross( vecBasisU );
	VectorNormalize( vecBasisV );

	vecBasisU = vecBasisV.Cross( vecNormal );
	VectorNormalize( vecBasisU );

	// Create the impact point.  Push off the surface a bit.
	Vector vecImpactPoint = pTrace->startpos + pTrace->fraction * ( pTrace->endpos - pTrace->startpos );
	vecImpactPoint += vecNormal;

	// Generate a quad to represent the plane.
	Vector vecPlanePoints[4];
	vecPlanePoints[0] = vecImpactPoint + ( vecBasisU * -flLength ) + ( vecBasisV * -flLength );
	vecPlanePoints[1] = vecImpactPoint + ( vecBasisU * -flLength ) + ( vecBasisV * flLength );
	vecPlanePoints[2] = vecImpactPoint + ( vecBasisU * flLength ) + ( vecBasisV * flLength );
	vecPlanePoints[3] = vecImpactPoint + ( vecBasisU * flLength ) + ( vecBasisV * -flLength );

#if 0
	// Test facing.
	Vector vecEdges[2];
	vecEdges[0] = vecPlanePoints[1] - vecPlanePoints[0];
	vecEdges[1] = vecPlanePoints[2] - vecPlanePoints[0];
	Vector vecCross = vecEdges[0].Cross( vecEdges[1] );
	if ( vecCross.Dot( vecNormal ) < 0.0f )
	{
		// Reverse winding.
	}
#endif

	// Draw the plane.
	NDebugOverlay::Triangle( vecPlanePoints[0], vecPlanePoints[1], vecPlanePoints[2], 125, 125, 125, 125, false, 5.0f );
	NDebugOverlay::Triangle( vecPlanePoints[0], vecPlanePoints[2], vecPlanePoints[3], 125, 125, 125, 125, false, 5.0f );

	NDebugOverlay::Line( vecPlanePoints[0], vecPlanePoints[1], 255, 255, 255, false, 5.0f );
	NDebugOverlay::Line( vecPlanePoints[1], vecPlanePoints[2], 255, 255, 255, false, 5.0f );
	NDebugOverlay::Line( vecPlanePoints[2], vecPlanePoints[3], 255, 255, 255, false, 5.0f );
	NDebugOverlay::Line( vecPlanePoints[3], vecPlanePoints[0], 255, 255, 255, false, 5.0f );

	// Draw the normal.
	NDebugOverlay::Line( vecImpactPoint, vecImpactPoint + ( vecNormal * flLength ), 255, 0, 0, false, 5.0f );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Constructs GameMovement interface
//-----------------------------------------------------------------------------
CGameMovement::CGameMovement( void )
{
	m_nOldWaterLevel	= WL_NotInWater;
	m_nOnLadder			= 0;

	mv					= NULL;

	memset( m_flStuckCheckTime, 0, sizeof(m_flStuckCheckTime) );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGameMovement::~CGameMovement( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
// Output : int
//-----------------------------------------------------------------------------
int CGameMovement::GetCheckInterval( IntervalType_t type )
{
	int tickInterval = 1;
	switch ( type )
	{
	default:
		tickInterval = 1;
		break;
	case GROUND:
		tickInterval = CATEGORIZE_GROUND_SURFACE_TICK_INTERVAL;
		break;
	case STUCK:
		tickInterval = CHECK_STUCK_TICK_INTERVAL;
		break;
	case LADDER:
		tickInterval = CHECK_LADDER_TICK_INTERVAL;
		break;
	}
	return tickInterval;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CGameMovement::CheckInterval( IntervalType_t type )
{
	int tickInterval = GetCheckInterval( type );

	if ( g_bMovementOptimizations )
	{
		return (player->CurrentCommandNumber() + player->entindex()) % tickInterval == 0;
	}
	else
	{
		return true;
	}
}
	

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : ducked - 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CGameMovement::GetPlayerMins( bool ducked ) const
{
	return ducked ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : ducked - 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CGameMovement::GetPlayerMaxs( bool ducked ) const
{	
	return ducked ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CGameMovement::GetPlayerMins( void ) const
{
	if ( player->IsObserver() )
	{
		return VEC_OBS_HULL_MIN;	
	}
	else
	{
		return player->m_Local.m_bDucked  ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CGameMovement::GetPlayerMaxs( void ) const
{	
	if ( player->IsObserver() )
	{
		return VEC_OBS_HULL_MAX;	
	}
	else
	{
		return player->m_Local.m_bDucked  ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : ducked - 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CGameMovement::GetPlayerViewOffset( bool ducked ) const
{
	return ducked ? VEC_DUCK_VIEW : VEC_VIEW;
}

#if 0
//-----------------------------------------------------------------------------
// Traces player movement + position
//-----------------------------------------------------------------------------
inline void CGameMovement::TracePlayerBBox( const Vector& start, const Vector& end, unsigned int fMask, int collisionGroup, trace_t& pm )
{
	VPROF( "CGameMovement::TracePlayerBBox" );

	Ray_t ray;
	ray.Init( start, end, GetPlayerMins(), GetPlayerMaxs() );
	UTIL_TraceRay( ray, fMask, mv->m_nPlayerHandle.Get(), collisionGroup, &pm );
}
#endif

inline CBaseHandle CGameMovement::TestPlayerPosition( const Vector& pos, int collisionGroup, trace_t& pm )
{
	Ray_t ray;
	ray.Init( pos, pos, GetPlayerMins(), GetPlayerMaxs() );
	UTIL_TraceRay( ray, MASK_PLAYERSOLID, mv->m_nPlayerHandle.Get(), collisionGroup, &pm );
	if ( (pm.contents & MASK_PLAYERSOLID) && pm.m_pEnt )
	{
		return pm.m_pEnt->GetRefEHandle();
	}
	else
	{	
		return INVALID_EHANDLE_INDEX;
	}
}


/*

// FIXME FIXME:  Does this need to be hooked up?
bool CGameMovement::IsWet() const
{
	return ((pev->flags & FL_INRAIN) != 0) || (m_WetTime >= gpGlobals->time);
}

//-----------------------------------------------------------------------------
// Plants player footprint decals
//-----------------------------------------------------------------------------

#define PLAYER_HALFWIDTH 12
void CGameMovement::PlantFootprint( surfacedata_t *psurface )
{
	// Can't plant footprints on fake materials (ladders, wading)
	if ( psurface->gameMaterial != 'X' )
	{
		int footprintDecal = -1;

		// Figure out which footprint type to plant...
		// Use the wet footprint if we're wet...
		if (IsWet())
		{
			footprintDecal = DECAL_FOOTPRINT_WET;
		}
		else
		{	   
			// FIXME: Activate this once we decide to pull the trigger on it.
			// NOTE: We could add in snow, mud, others here
//			switch(psurface->gameMaterial)
//			{
//			case 'D':
//				footprintDecal = DECAL_FOOTPRINT_DIRT;
//				break;
//			}
		}

		if (footprintDecal != -1)
		{
			Vector right;
			AngleVectors( pev->angles, 0, &right, 0 );

			// Figure out where the top of the stepping leg is 
			trace_t tr;
			Vector hipOrigin;
			VectorMA( pev->origin, 
				m_IsFootprintOnLeft ? -PLAYER_HALFWIDTH : PLAYER_HALFWIDTH,
				right, hipOrigin );

			// Find where that leg hits the ground
			UTIL_TraceLine( hipOrigin, hipOrigin + Vector(0, 0, -COORD_EXTENT * 1.74), 
							MASK_SOLID_BRUSHONLY, edict(), COLLISION_GROUP_NONE, &tr);

			unsigned char mType = TEXTURETYPE_Find( &tr );

			// Splat a decal
			CPVSFilter filter( tr.endpos );
			te->FootprintDecal( filter, 0.0f, &tr.endpos, &right, ENTINDEX(tr.u.ent), 
							   gDecals[footprintDecal].index, mType );

		}
	}

	// Switch feet for next time
	m_IsFootprintOnLeft = !m_IsFootprintOnLeft;
}

#define WET_TIME			    5.f	// how many seconds till we're completely wet/dry
#define DRY_TIME			   20.f	// how many seconds till we're completely wet/dry

void CBasePlayer::UpdateWetness()
{
	// BRJ 1/7/01
	// Check for whether we're in a rainy area....
	// Do this by tracing a line straight down with a size guaranteed to
	// be larger than the map
	// Update wetness based on whether we're in rain or not...

	trace_t tr;
	UTIL_TraceLine( pev->origin, pev->origin + Vector(0, 0, -COORD_EXTENT * 1.74), 
					MASK_SOLID_BRUSHONLY, edict(), COLLISION_GROUP_NONE, &tr);
	if (tr.surface.flags & SURF_WET)
	{
		if (! (pev->flags & FL_INRAIN) )
		{
			// Transition...
			// Figure out how wet we are now (we were drying off...)
			float wetness = (m_WetTime - gpGlobals->time) / DRY_TIME;
			if (wetness < 0.0f)
				wetness = 0.0f;

			// Here, wet time represents the time at which we get totally wet
			m_WetTime = gpGlobals->time + (1.0 - wetness) * WET_TIME; 

			pev->flags |= FL_INRAIN;
		}
	}
	else
	{
		if ((pev->flags & FL_INRAIN) != 0)
		{
			// Transition...
			// Figure out how wet we are now (we were getting more wet...)
			float wetness = 1.0f + (gpGlobals->time - m_WetTime) / WET_TIME;
			if (wetness > 1.0f)
				wetness = 1.0f;

			// Here, wet time represents the time at which we get totally dry
			m_WetTime = gpGlobals->time + wetness * DRY_TIME; 

			pev->flags &= ~FL_INRAIN;
		}
	}
}
*/


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::CategorizeGroundSurface( void )
{
	//
	// Find the name of the material that lies beneath the player.
	//
	Vector start, end;
	VectorCopy( mv->m_vecAbsOrigin, start );
	VectorCopy( mv->m_vecAbsOrigin, end );

	// Straight down
	end[2] -= 64;

	// Fill in default values, just in case.
	trace_t	trace;
	TracePlayerBBox( start, end, MASK_PLAYERSOLID_BRUSHONLY, COLLISION_GROUP_PLAYER_MOVEMENT, trace ); 
	IPhysicsSurfaceProps *physprops = MoveHelper()->GetSurfaceProps();
	player->m_surfaceProps = trace.surface.surfaceProps;
	player->m_pSurfaceData = physprops->GetSurfaceData( player->m_surfaceProps );
	physprops->GetPhysicsProperties( player->m_surfaceProps, NULL, NULL, &player->m_surfaceFriction, NULL );
	
	// HACKHACK: Scale this to fudge the relationship between vphysics friction values and player friction values.
	// A value of 0.8f feels pretty normal for vphysics, whereas 1.0f is normal for players.
	// This scaling trivially makes them equivalent.  REVISIT if this affects low friction surfaces too much.
	player->m_surfaceFriction *= 1.25f;
	if ( player->m_surfaceFriction > 1.0f )
		player->m_surfaceFriction = 1.0f;

	player->m_chTextureType = player->m_pSurfaceData->game.material;
}

bool CGameMovement::IsDead( void ) const
{
	return ( player->m_iHealth <= 0 ) ? true : false;
}

//-----------------------------------------------------------------------------
// Figures out how the constraint should slow us down
//-----------------------------------------------------------------------------
float CGameMovement::ComputeConstraintSpeedFactor( void )
{
	// If we have a constraint, slow down because of that too.
	if ( !mv || mv->m_flConstraintRadius == 0.0f )
		return 1.0f;

	float flDistSq = mv->m_vecAbsOrigin.DistToSqr( mv->m_vecConstraintCenter );

	float flOuterRadiusSq = mv->m_flConstraintRadius * mv->m_flConstraintRadius;
	float flInnerRadiusSq = mv->m_flConstraintRadius - mv->m_flConstraintWidth;
	flInnerRadiusSq *= flInnerRadiusSq;

	// Only slow us down if we're inside the constraint ring
	if ((flDistSq <= flInnerRadiusSq) || (flDistSq >= flOuterRadiusSq))
		return 1.0f;

	// Only slow us down if we're running away from the center
	Vector vecDesired;
	VectorMultiply( m_vecForward, mv->m_flForwardMove, vecDesired );
	VectorMA( vecDesired, mv->m_flSideMove, m_vecRight, vecDesired );
	VectorMA( vecDesired, mv->m_flUpMove, m_vecUp, vecDesired );

	Vector vecDelta;
	VectorSubtract( mv->m_vecAbsOrigin, mv->m_vecConstraintCenter, vecDelta );
	VectorNormalize( vecDelta );
	VectorNormalize( vecDesired );
	if (DotProduct( vecDelta, vecDesired ) < 0.0f)
		return 1.0f;

	float flFrac = (sqrt(flDistSq) - (mv->m_flConstraintRadius - mv->m_flConstraintWidth)) / mv->m_flConstraintWidth;

	float flSpeedFactor = Lerp( flFrac, 1.0f, mv->m_flConstraintSpeedFactor ); 
	return flSpeedFactor;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::CheckParameters( void )
{
	QAngle	v_angle;

	if ( player->GetMoveType() != MOVETYPE_ISOMETRIC &&
		 player->GetMoveType() != MOVETYPE_NOCLIP &&
		 player->GetMoveType() != MOVETYPE_OBSERVER )
	{
		float spd;
		float maxspeed;

		spd = ( mv->m_flForwardMove * mv->m_flForwardMove ) +
			  ( mv->m_flSideMove * mv->m_flSideMove ) +
			  ( mv->m_flUpMove * mv->m_flUpMove );

		maxspeed = mv->m_flClientMaxSpeed;
		if ( maxspeed != 0.0 )
		{
			mv->m_flMaxSpeed = min( maxspeed, mv->m_flMaxSpeed );
		}

		// Slow down by the speed factor
		float flSpeedFactor = 1.0f;
		if (player->m_pSurfaceData)
		{
			flSpeedFactor = player->m_pSurfaceData->game.maxSpeedFactor;
		}

		// If we have a constraint, slow down because of that too.
		float flConstraintSpeedFactor = ComputeConstraintSpeedFactor();
		if (flConstraintSpeedFactor < flSpeedFactor)
			flSpeedFactor = flConstraintSpeedFactor;

		mv->m_flMaxSpeed *= flSpeedFactor;

		if ( g_bMovementOptimizations )
		{
			// Same thing but only do the sqrt if we have to.
			if ( ( spd != 0.0 ) && ( spd > mv->m_flMaxSpeed*mv->m_flMaxSpeed ) )
			{
				float fRatio = mv->m_flMaxSpeed / sqrt( spd );
				mv->m_flForwardMove *= fRatio;
				mv->m_flSideMove    *= fRatio;
				mv->m_flUpMove      *= fRatio;
			}
		}
		else
		{
			spd = sqrt( spd );
			if ( ( spd != 0.0 ) && ( spd > mv->m_flMaxSpeed ) )
			{
				float fRatio = mv->m_flMaxSpeed / spd;
				mv->m_flForwardMove *= fRatio;
				mv->m_flSideMove    *= fRatio;
				mv->m_flUpMove      *= fRatio;
			}
		}
	}

	if ( !m_bSpeedCropped && ( mv->m_nButtons & IN_SPEED ) && !( player->m_Local.m_bDucked && !player->m_Local.m_bDucking ))
	{
		float frac = 1.0f; // TODO can we remove this ?
		mv->m_flForwardMove *= frac;
		mv->m_flSideMove    *= frac;
		mv->m_flUpMove      *= frac;
		m_bSpeedCropped = true;
	}

	if ( player->GetFlags() & FL_FROZEN ||
		 player->GetFlags() & FL_ONTRAIN || 
		 IsDead() )
	{
		mv->m_flForwardMove = 0;
		mv->m_flSideMove    = 0;
		mv->m_flUpMove      = 0;
	}

	DecayPunchAngle();

	// Take angles from command.
	if ( !IsDead() )
	{
		v_angle = mv->m_vecAngles;
		v_angle = v_angle + player->m_Local.m_vecPunchAngle;

		// Now adjust roll angle
		if ( player->GetMoveType() != MOVETYPE_ISOMETRIC  &&
			 player->GetMoveType() != MOVETYPE_NOCLIP )
		{
			mv->m_vecAngles[ROLL]  = CalcRoll( v_angle, mv->m_vecVelocity, sv_rollangle.GetFloat(), sv_rollspeed.GetFloat() );
		}
		else
		{
			mv->m_vecAngles[ROLL] = 0.0; // v_angle[ ROLL ];
		}
		mv->m_vecAngles[PITCH] = v_angle[PITCH];
		mv->m_vecAngles[YAW]   = v_angle[YAW];
	}
	else
	{
		mv->m_vecAngles = mv->m_vecOldAngles;
	}

	// Set dead player view_offset
	if ( IsDead() )
	{
		player->SetViewOffset( VEC_DEAD_VIEWHEIGHT );
	}

	// Adjust client view angles to match values used on server.
	if ( mv->m_vecAngles[YAW] > 180.0f )
	{
		mv->m_vecAngles[YAW] -= 360.0f;
	}
}

void CGameMovement::ReduceTimers( void )
{
	float frame_msec = 1000.0f * gpGlobals->frametime;

	if ( player->m_Local.m_flDucktime > 0 )
	{
		player->m_Local.m_flDucktime -= frame_msec;
		if ( player->m_Local.m_flDucktime < 0 )
		{
			player->m_Local.m_flDucktime = 0;
		}
	}
	if ( player->m_Local.m_flDuckJumpTime > 0 )
	{
		player->m_Local.m_flDuckJumpTime -= frame_msec;
		if ( player->m_Local.m_flDuckJumpTime < 0 )
		{
			player->m_Local.m_flDuckJumpTime = 0;
		}
	}
	if ( player->m_Local.m_flJumpTime > 0 )
	{
		player->m_Local.m_flJumpTime -= frame_msec;
		if ( player->m_Local.m_flJumpTime < 0 )
		{
			player->m_Local.m_flJumpTime = 0;
		}
	}
	if ( player->m_flSwimSoundTime > 0 )
	{
		player->m_flSwimSoundTime -= frame_msec;
		if ( player->m_flSwimSoundTime < 0 )
		{
			player->m_flSwimSoundTime = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pMove - 
//-----------------------------------------------------------------------------
void CGameMovement::ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMove )
{
	Assert( pMove && pPlayer );

	float flStoreFrametime = gpGlobals->frametime;

	//!!HACK HACK: Adrian - slow down all player movement by this factor.
	//!!Blame Yahn for this one.
	gpGlobals->frametime *= pPlayer->GetLaggedMovementValue();

	ResetGetPointContentsCache();

	// Cropping movement speed scales mv->m_fForwardSpeed etc. globally
	// Once we crop, we don't want to recursively crop again, so we set the crop
	//  flag globally here once per usercmd cycle.
	m_bSpeedCropped = false;

	player = pPlayer;
	mv = pMove;
	mv->m_flMaxSpeed = sv_maxspeed.GetFloat();

	StartTrackPredictionErrors();

	// Run the command.
	PlayerMove();

	FinishMove();

	FinishTrackPredictionErrors();

	//This is probably not needed, but just in case.
	gpGlobals->frametime = flStoreFrametime;
}


// This code is useful for finding where prediction errors happened.
// Change it to use any variable inside mv or in the player and then look at the 
// client.txt and server.txt files to see where the values went astray.
#if 1
	void CGameMovement::StartTrackPredictionErrors()
	{
	}
	void CGameMovement::FinishTrackPredictionErrors()
	{
	}
#else
	static Vector g_vPredictionErrorTrackStart;

	void CGameMovement::StartTrackPredictionErrors()
	{
		g_vPredictionErrorTrackStart = mv->m_vecVelocity;
	}

	void CGameMovement::FinishTrackPredictionErrors()
	{
		const char *pFilename = "c:\\hl2\\client.txt";
		#ifndef CLIENT_DLL
			pFilename = "c:\\hl2\\server.txt";
		#endif
		
		extern IFileSystem *filesystem;
		static FileHandle_t fp = filesystem->Open( pFilename, "wt" );
		
		filesystem->FPrintf( fp, "%04d: (%f %f %f) - (%f %f %f)\n", 
			pPlayer->m_pCurrentCommand->command_number,
			VectorExpand( vStart ),
			VectorExpand( mv->m_vecVelocity ) );
		
		filesystem->Flush( fp );
	}
#endif


//-----------------------------------------------------------------------------
// Purpose: Sets ground entity
//-----------------------------------------------------------------------------
void CGameMovement::FinishMove( void )
{
	mv->m_nOldButtons = mv->m_nButtons;
}

#define PUNCH_DAMPING		9.0f		// bigger number makes the response more damped, smaller is less damped
										// currently the system will overshoot, with larger damping values it won't
#define PUNCH_SPRING_CONSTANT	65.0f	// bigger number increases the speed at which the view corrects

//-----------------------------------------------------------------------------
// Purpose: Decays the punchangle toward 0,0,0.
//			Modelled as a damped spring
//-----------------------------------------------------------------------------
void CGameMovement::DecayPunchAngle( void )
{
	if ( player->m_Local.m_vecPunchAngle->LengthSqr() > 0.001 || player->m_Local.m_vecPunchAngleVel->LengthSqr() > 0.001 )
	{
		player->m_Local.m_vecPunchAngle += player->m_Local.m_vecPunchAngleVel * gpGlobals->frametime;
		float damping = 1 - (PUNCH_DAMPING * gpGlobals->frametime);
		
		if ( damping < 0 )
		{
			damping = 0;
		}
		player->m_Local.m_vecPunchAngleVel *= damping;
		 
		// torsional spring
		// UNDONE: Per-axis spring constant?
		float springForceMagnitude = PUNCH_SPRING_CONSTANT * gpGlobals->frametime;
		springForceMagnitude = clamp(springForceMagnitude, 0, 2 );
		player->m_Local.m_vecPunchAngleVel -= player->m_Local.m_vecPunchAngle * springForceMagnitude;

		// don't wrap around
		player->m_Local.m_vecPunchAngle.Init( 
			clamp(player->m_Local.m_vecPunchAngle->x, -89, 89 ), 
			clamp(player->m_Local.m_vecPunchAngle->y, -179, 179 ),
			clamp(player->m_Local.m_vecPunchAngle->z, -89, 89 ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::StartGravity( void )
{
	float ent_gravity;
	
	if (player->GetGravity())
		ent_gravity = player->GetGravity();
	else
		ent_gravity = 1.0;

	// Add gravity so they'll be in the correct position during movement
	// yes, this 0.5 looks wrong, but it's not.  
	mv->m_vecVelocity[2] -= (ent_gravity * sv_gravity.GetFloat() * 0.5 * gpGlobals->frametime );
	mv->m_vecVelocity[2] += player->GetBaseVelocity()[2] * gpGlobals->frametime;

	Vector temp = player->GetBaseVelocity();
	temp[ 2 ] = 0;
	player->SetBaseVelocity( temp );

	CheckVelocity();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::CheckWaterJump( void )
{
	Vector	flatforward;
	Vector forward;
	Vector	flatvelocity;
	float curspeed;

	AngleVectors( mv->m_vecViewAngles, &forward );  // Determine movement angles

	// Already water jumping.
	if (player->m_flWaterJumpTime)
		return;

	// Don't hop out if we just jumped in
	if (mv->m_vecVelocity[2] < -180)
		return; // only hop out if we are moving up

	// See if we are backing up
	flatvelocity[0] = mv->m_vecVelocity[0];
	flatvelocity[1] = mv->m_vecVelocity[1];
	flatvelocity[2] = 0;

	// Must be moving
	curspeed = VectorNormalize( flatvelocity );
	
	// see if near an edge
	flatforward[0] = forward[0];
	flatforward[1] = forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	// Are we backing into water from steps or something?  If so, don't pop forward
	if ( curspeed != 0.0 && ( DotProduct( flatvelocity, flatforward ) < 0.0 ) )
		return;

	Vector vecStart;
	// Start line trace at waist height (using the center of the player for this here)
	vecStart= mv->m_vecAbsOrigin + (GetPlayerMins() + GetPlayerMaxs() ) * 0.5;

	Vector vecEnd;
	VectorMA( vecStart, 24.0f, flatforward, vecEnd );
	
	trace_t tr;
	TracePlayerBBox( vecStart, vecEnd, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, tr );
	if ( tr.fraction < 1.0 )		// solid at waist
	{
		IPhysicsObject *pPhysObj = tr.m_pEnt->VPhysicsGetObject();
		if ( pPhysObj )
		{
			if ( pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
				return;
		}

		vecStart.z = mv->m_vecAbsOrigin.z + player->GetViewOffset().z + WATERJUMP_HEIGHT; 
		VectorMA( vecStart, 24.0f, flatforward, vecEnd );
		VectorMA( vec3_origin, -50.0f, tr.plane.normal, player->m_vecWaterJumpVel );

		TracePlayerBBox( vecStart, vecEnd, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, tr );
		if ( tr.fraction == 1.0 )		// open at eye level
		{
			// Now trace down to see if we would actually land on a standable surface.
			VectorCopy( vecEnd, vecStart );
			vecEnd.z -= 1024.0f;
			TracePlayerBBox( vecStart, vecEnd, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, tr );
			if ( ( tr.fraction < 1.0f ) && ( tr.plane.normal.z >= 0.7 ) )
			{
				mv->m_vecVelocity[2] = 256.0f;			// Push up
				mv->m_nOldButtons |= IN_JUMP;		// Don't jump again until released
				player->AddFlag( FL_WATERJUMP );
				player->m_flWaterJumpTime = 2000.0f;	// Do this for 2 seconds
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::WaterJump( void )
{
	if (player->m_flWaterJumpTime > 10000)
		player->m_flWaterJumpTime = 10000;

	if (!player->m_flWaterJumpTime)
		return;

	player->m_flWaterJumpTime -= 1000.0f * gpGlobals->frametime;

	if (player->m_flWaterJumpTime <= 0 || !player->GetWaterLevel())
	{
		player->m_flWaterJumpTime = 0;
		player->RemoveFlag( FL_WATERJUMP );
	}
	
	mv->m_vecVelocity[0] = player->m_vecWaterJumpVel[0];
	mv->m_vecVelocity[1] = player->m_vecWaterJumpVel[1];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::WaterMove( void )
{
	int		i;
	Vector	wishvel;
	float	wishspeed;
	Vector	wishdir;
	Vector	start, dest;
	Vector  temp;
	trace_t	pm;
	float speed, newspeed, addspeed, accelspeed;
	Vector forward, right, up;

	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles

	//
	// user intentions
	//
	for (i=0 ; i<3 ; i++)
	{
		wishvel[i] = forward[i]*mv->m_flForwardMove + right[i]*mv->m_flSideMove;
	}

	// if we have the jump key down, move us up as well
	if (mv->m_nButtons & IN_JUMP)
	{
		wishvel[2] += mv->m_flClientMaxSpeed;
	}
	// Sinking after no other movement occurs
	else if (!mv->m_flForwardMove && !mv->m_flSideMove && !mv->m_flUpMove)
	{
		wishvel[2] -= 60;		// drift towards bottom
	}
	else  // Go straight up by upmove amount.
	{
		wishvel[2] += mv->m_flUpMove;
	}

	// Copy it over and determine speed
	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	// Cap speed.
	if (wishspeed > mv->m_flMaxSpeed)
	{
		VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
		wishspeed = mv->m_flMaxSpeed;
	}

	// Slow us down a bit.
	wishspeed *= 0.8;
	
	// Water friction
	VectorCopy(mv->m_vecVelocity, temp);
	speed = VectorNormalize(temp);
	if (speed)
	{
		newspeed = speed - gpGlobals->frametime * speed * sv_friction.GetFloat() * player->m_surfaceFriction;
		if (newspeed < 0.1f)
		{
			newspeed = 0;
		}

		VectorScale (mv->m_vecVelocity, newspeed/speed, mv->m_vecVelocity);
	}
	else
	{
		newspeed = 0;
	}

	// water acceleration
	if (wishspeed >= 0.1f)  // old !
	{
		addspeed = wishspeed - newspeed;
		if (addspeed > 0)
		{
			VectorNormalize(wishvel);
			accelspeed = sv_accelerate.GetFloat() * wishspeed * gpGlobals->frametime * player->m_surfaceFriction;
			if (accelspeed > addspeed)
			{
				accelspeed = addspeed;
			}

			for (i = 0; i < 3; i++)
			{
				float deltaSpeed = accelspeed * wishvel[i];
				mv->m_vecVelocity[i] += deltaSpeed;
				mv->m_outWishVel[i] += deltaSpeed;
			}
		}
	}

	VectorAdd (mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

	// Now move
	// assume it is a stair or a slope, so press down from stepheight above
	VectorMA (mv->m_vecAbsOrigin, gpGlobals->frametime, mv->m_vecVelocity, dest);
	
	TracePlayerBBox( mv->m_vecAbsOrigin, dest, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
	if ( pm.fraction == 1.0f )
	{
		VectorCopy( dest, start );
		if ( player->m_Local.m_bAllowAutoMovement )
		{
			start[2] += player->m_Local.m_flStepSize + 1;
		}
		
		TracePlayerBBox( start, dest, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
		
		if (!pm.startsolid && !pm.allsolid)
		{	
			float stepDist = pm.endpos.z - mv->m_vecAbsOrigin.z;
			mv->m_outStepHeight += stepDist;
			// walked up the step, so just keep result and exit
			VectorCopy (pm.endpos, mv->m_vecAbsOrigin);
			VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
			return;
		}

		// Try moving straight along out normal path.
		TryPlayerMove();
	}
	else
	{
		if ( !player->GetGroundEntity() )
		{
			TryPlayerMove();
			VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
			return;
		}

		StepMove( dest, pm );
	}
	
	VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: Does the basic move attempting to climb up step heights.  It uses
//          the mv->m_vecAbsOrigin and mv->m_vecVelocity.  It returns a new
//          new mv->m_vecAbsOrigin, mv->m_vecVelocity, and mv->m_outStepHeight.
//-----------------------------------------------------------------------------
void CGameMovement::StepMove( Vector &vecDestination, trace_t &trace )
{
	Vector vecEndPos;
	VectorCopy( vecDestination, vecEndPos );

	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
	Vector vecPos, vecVel;
	VectorCopy( mv->m_vecAbsOrigin, vecPos );
	VectorCopy( mv->m_vecVelocity, vecVel );

	// Slide move down.
	TryPlayerMove( &vecEndPos, &trace );
	
	// Down results.
	Vector vecDownPos, vecDownVel;
	VectorCopy( mv->m_vecAbsOrigin, vecDownPos );
	VectorCopy( mv->m_vecVelocity, vecDownVel );
	
	// Reset original values.
	VectorCopy( vecPos, mv->m_vecAbsOrigin );
	VectorCopy( vecVel, mv->m_vecVelocity );
	
	// Move up a stair height.
	VectorCopy( mv->m_vecAbsOrigin, vecEndPos );
	if ( player->m_Local.m_bAllowAutoMovement )
	{
		vecEndPos.z += player->m_Local.m_flStepSize;
	}
	
	TracePlayerBBox( mv->m_vecAbsOrigin, vecEndPos, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );
	if ( !trace.startsolid && !trace.allsolid )
	{
		VectorCopy( trace.endpos, mv->m_vecAbsOrigin );
	}
	
	// Slide move up.
	TryPlayerMove();
	
	// Move down a stair (attempt to).
	VectorCopy( mv->m_vecAbsOrigin, vecEndPos );
	if ( player->m_Local.m_bAllowAutoMovement )
	{
		vecEndPos.z -= player->m_Local.m_flStepSize;
	}
		
	TracePlayerBBox( mv->m_vecAbsOrigin, vecEndPos, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );
	
	// If we are not on the ground any more then use the original movement attempt.
	if ( trace.plane.normal[2] < 0.7 )
	{
		VectorCopy( vecDownPos, mv->m_vecAbsOrigin );
		VectorCopy( vecDownVel, mv->m_vecVelocity );
		float flStepDist = mv->m_vecAbsOrigin.z - vecPos.z;
		if ( flStepDist > 0.0f )
		{
			mv->m_outStepHeight += flStepDist;
		}
		return;
	}
	
	// If the trace ended up in empty space, copy the end over to the origin.
	if ( !trace.startsolid && !trace.allsolid )
	{
		VectorCopy( trace.endpos, mv->m_vecAbsOrigin );
	}
	
	// Copy this origin to up.
	Vector vecUpPos;
	VectorCopy( mv->m_vecAbsOrigin, vecUpPos );
	
	// decide which one went farther
	float flDownDist = ( vecDownPos.x - vecPos.x ) * ( vecDownPos.x - vecPos.x ) + ( vecDownPos.y - vecPos.y ) * ( vecDownPos.y - vecPos.y );
	float flUpDist = ( vecUpPos.x - vecPos.x ) * ( vecUpPos.x - vecPos.x ) + ( vecUpPos.y - vecPos.y ) * ( vecUpPos.y - vecPos.y );
	if ( flDownDist > flUpDist )
	{
		VectorCopy( vecDownPos, mv->m_vecAbsOrigin );
		VectorCopy( vecDownVel, mv->m_vecVelocity );
	}
	else 
	{
		// copy z value from slide move
		mv->m_vecVelocity.z = vecDownVel.z;
	}
	
	float flStepDist = mv->m_vecAbsOrigin.z - vecPos.z;
	if ( flStepDist > 0 )
	{
		mv->m_outStepHeight += flStepDist;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::Friction( void )
{
	float	speed, newspeed, control;
	float	friction;
	float	drop;
	Vector newvel;

	
	// If we are in water jump cycle, don't apply friction
	if (player->m_flWaterJumpTime)
		return;

	// Calculate speed
	speed = VectorLength( mv->m_vecVelocity );
	
	// If too slow, return
	if (speed < 0.1f)
	{
		return;
	}

	drop = 0;

	// apply ground friction
	if (player->GetGroundEntity() != NULL)  // On an entity that is the ground
	{
		Vector start, stop;
		trace_t pm;

		//
		// NOTE: added a "1.0f" to the player minimum (bbox) value so that the 
		//       trace starts just inside of the bounding box, this make sure
		//       that we don't get any collision epsilon (on surface) errors.
		//		 The significance of the 16 below is this is how many units out front we are checking
		//		 to see if the player box would fall.  The 49 is the number of units down that is required
		//		 to be considered a fall.  49 is derived from 1 (added 1 from above) + 48 the max fall 
		//		 distance a player can fall and still jump back up.
		//
		//		 UNDONE: In some cases there are still problems here.  Specifically, no collision check is
		//		 done so 16 units in front of the player could be inside a volume or past a collision point.
		start[0] = stop[0] = mv->m_vecAbsOrigin[0] + (mv->m_vecVelocity[0]/speed)*16;
		start[1] = stop[1] = mv->m_vecAbsOrigin[1] + (mv->m_vecVelocity[1]/speed)*16;
		start[2] = mv->m_vecAbsOrigin[2] + ( GetPlayerMins()[2] + 1.0f );
		stop[2] = start[2] - 49;

		if ( g_bMovementOptimizations )
		{
			// We don't actually need this trace.
		}
		else
		{
			TracePlayerBBox( start, stop, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm ); 
		}

		friction = sv_friction.GetFloat();

		// Grab friction value.
		friction *= player->m_surfaceFriction;  // player friction?

		// Bleed off some speed, but if we have less than the bleed
		//  threshhold, bleed the theshold amount.
		control = (speed < sv_stopspeed.GetFloat()) ?
			sv_stopspeed.GetFloat() : speed;

		// Add the amount to the drop amount.
		drop += control*friction*gpGlobals->frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	// Determine proportion of old speed we are using.
	newspeed /= speed;

	// Adjust velocity according to proportion.
	newvel[0] = mv->m_vecVelocity[0] * newspeed;
	newvel[1] = mv->m_vecVelocity[1] * newspeed;
	newvel[2] = mv->m_vecVelocity[2] * newspeed;

	VectorCopy( newvel, mv->m_vecVelocity );
 	mv->m_outWishVel -= (1.f-newspeed) * mv->m_vecVelocity;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::FinishGravity( void )
{
	float ent_gravity;

	if ( player->m_flWaterJumpTime )
		return;

	if ( player->GetGravity() )
		ent_gravity = player->GetGravity();
	else
		ent_gravity = 1.0;

	// Get the correct velocity for the end of the dt 
  	mv->m_vecVelocity[2] -= (ent_gravity * sv_gravity.GetFloat() * gpGlobals->frametime * 0.5);

	CheckVelocity();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : wishdir - 
//			accel - 
//-----------------------------------------------------------------------------
void CGameMovement::AirAccelerate( Vector& wishdir, float wishspeed, float accel )
{
	int i;
	float addspeed, accelspeed, currentspeed;
	float wishspd;

	wishspd = wishspeed;
	
	if (player->pl.deadflag)
		return;
	
	if (player->m_flWaterJumpTime)
		return;

	// Cap speed
	if (wishspd > 30)
		wishspd = 30;

	// Determine veer amount
	currentspeed = mv->m_vecVelocity.Dot(wishdir);

	// See how much to add
	addspeed = wishspd - currentspeed;

	// If not adding any, done.
	if (addspeed <= 0)
		return;

	// Determine acceleration speed after acceleration
	accelspeed = accel * wishspeed * gpGlobals->frametime * player->m_surfaceFriction;

	// Cap it
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	// Adjust pmove vel.
	for (i=0 ; i<3 ; i++)
	{
		mv->m_vecVelocity[i] += accelspeed * wishdir[i];
		mv->m_outWishVel[i] += accelspeed * wishdir[i];
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::AirMove( void )
{
	int			i;
	Vector		wishvel;
	float		fmove, smove;
	Vector		wishdir;
	float		wishspeed;
	Vector forward, right, up;

	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles
	
	// Copy movement amounts
	fmove = mv->m_flForwardMove;
	smove = mv->m_flSideMove;
	
	// Zero out z components of movement vectors
	forward[2] = 0;
	right[2]   = 0;
	VectorNormalize(forward);  // Normalize remainder of vectors
	VectorNormalize(right);    // 

	for (i=0 ; i<2 ; i++)       // Determine x and y parts of velocity
		wishvel[i] = forward[i]*fmove + right[i]*smove;
	wishvel[2] = 0;             // Zero out z part of velocity

	VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
	wishspeed = VectorNormalize(wishdir);

	//
	// clamp to server defined max speed
	//
	if ( wishspeed != 0 && (wishspeed > mv->m_flMaxSpeed))
	{
		VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
		wishspeed = mv->m_flMaxSpeed;
	}
	
	AirAccelerate( wishdir, wishspeed, sv_airaccelerate.GetFloat() );

	// Add in any base velocity to the current velocity.
	VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );

	TryPlayerMove();

	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
}


bool CGameMovement::CanAccelerate()
{
	//BG2 - Tjoppen - reenable spectators
#ifndef CLIENT_DLL
	if( player->HasPhysicsFlag( PFLAG_OBSERVER ) )
		return true;
#endif
	if( player->GetTeamNumber() == TEAM_SPECTATOR )
		return true;
	//

	// Dead players don't accelerate.
	if (player->pl.deadflag)
		return false;

	// If waterjumping, don't accelerate
	if (player->m_flWaterJumpTime)
		return false;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : wishdir - 
//			wishspeed - 
//			accel - 
//-----------------------------------------------------------------------------
void CGameMovement::Accelerate( Vector& wishdir, float wishspeed, float accel )
{
	int i;
	float addspeed, accelspeed, currentspeed;

	// This gets overridden because some games (CSPort) want to allow dead (observer) players
	// to be able to move around.
	if ( !CanAccelerate() )
		return;

	// See if we are changing direction a bit
	//currentspeed = mv->m_vecVelocity.Dot(wishdir);
	currentspeed = mv->m_vecVelocity.Length();

	// Reduce wishspeed by the amount of veer.
	addspeed = wishspeed - currentspeed;

	// If not going to add any speed, done.
	if (addspeed <= 0)
		return;

	// Determine amount of accleration.
	accelspeed = accel * gpGlobals->frametime * wishspeed * player->m_surfaceFriction;

	// Cap at addspeed
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	// Adjust velocity.
	for (i=0 ; i<3 ; i++)
	{
		mv->m_vecVelocity[i] += accelspeed * wishdir[i];	
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::WalkMove( void )
{
	int i;

	Vector wishvel;
	float spd;
	float fmove, smove;
	Vector wishdir;
	float wishspeed;

	Vector dest;
	trace_t pm;
	Vector forward, right, up;

	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles

	CHandle< CBaseEntity > oldground;
	oldground = player->GetGroundEntity();
	
	// Copy movement amounts
	fmove = mv->m_flForwardMove;
	smove = mv->m_flSideMove;
	
	// Zero out z components of movement vectors
	if ( g_bMovementOptimizations )
	{
		if ( forward[2] != 0 )
		{
			forward[2] = 0;
			VectorNormalize( forward );
		}

		if ( right[2] != 0 )
		{
			right[2] = 0;
			VectorNormalize( right );
		}
	}
	else
	{
		forward[2] = 0;
		right[2]   = 0;
		
		VectorNormalize (forward);  // Normalize remainder of vectors.
		VectorNormalize (right);    // 
	}

	for (i=0 ; i<2 ; i++)       // Determine x and y parts of velocity
		wishvel[i] = forward[i]*fmove + right[i]*smove;
	
	wishvel[2] = 0;             // Zero out z part of velocity

	VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
	wishspeed = VectorNormalize(wishdir);

	//
	// Clamp to server defined max speed
	//
	if ((wishspeed != 0.0f) && (wishspeed > mv->m_flMaxSpeed))
	{
		VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
		wishspeed = mv->m_flMaxSpeed;
	}

	// Set pmove velocity
	mv->m_vecVelocity[2] = 0;
	Accelerate ( wishdir, wishspeed, sv_accelerate.GetFloat() );
	mv->m_vecVelocity[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd (mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );

	spd = VectorLength( mv->m_vecVelocity );

	if ( spd < 1.0f )
	{
		mv->m_vecVelocity.Init();
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
		return;
	}

	// first try just moving to the destination	
	dest[0] = mv->m_vecAbsOrigin[0] + mv->m_vecVelocity[0]*gpGlobals->frametime;
	dest[1] = mv->m_vecAbsOrigin[1] + mv->m_vecVelocity[1]*gpGlobals->frametime;	
	dest[2] = mv->m_vecAbsOrigin[2];

	// first try moving directly to the next spot
	TracePlayerBBox( mv->m_vecAbsOrigin, dest, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );

	// If we made it all the way, then copy trace end as new player position.
	mv->m_outWishVel += wishdir * wishspeed;

	if ( pm.fraction == 1 )
	{
		VectorCopy( pm.endpos, mv->m_vecAbsOrigin );
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
		return;
	}

	// Don't walk up stairs if not on ground.
	if ( oldground == NULL && player->GetWaterLevel()  == 0 )
	{
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
		return;
	}

	// If we are jumping out of water, don't do anything more.
	if ( player->m_flWaterJumpTime )         
	{
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
		return;
	}

	StepMove( dest, pm );

	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::FullWalkMove( )
{
	if ( !CheckWater() ) 
	{
		StartGravity();
	}

	// If we are leaping out of the water, just update the counters.
	if (player->m_flWaterJumpTime)
	{
		WaterJump();
		TryPlayerMove();
		// See if we are still in water?
		CheckWater();
		return;
	}

	// If we are swimming in the water, see if we are nudging against a place we can jump up out
	//  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
	if ( player->GetWaterLevel() >= WL_Waist ) 
	{
		if ( player->GetWaterLevel() == WL_Waist )
		{
			CheckWaterJump();
		}

			// If we are falling again, then we must not trying to jump out of water any more.
		if ( mv->m_vecVelocity[2] < 0 && 
			 player->m_flWaterJumpTime )
		{
			player->m_flWaterJumpTime = 0;
		}

		// Was jump button pressed?
		if (mv->m_nButtons & IN_JUMP)
		{
			CheckJumpButton();
		}
		else
		{
			mv->m_nOldButtons &= ~IN_JUMP;
		}

		// Perform regular water movement
		WaterMove();

		// Redetermine position vars
		CategorizePosition();

		// If we are on ground, no downward velocity.
		if ( player->GetGroundEntity() != NULL )
		{
			mv->m_vecVelocity[2] = 0;			
		}
	}
	else
	// Not fully underwater
	{
		// Was jump button pressed?
		if (mv->m_nButtons & IN_JUMP)
		{
 			CheckJumpButton();
		}
		else
		{
			mv->m_nOldButtons &= ~IN_JUMP;
		}

		// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor, 
		//  we don't slow when standing still, relative to the conveyor.
		if (player->GetGroundEntity() != NULL)
		{
			mv->m_vecVelocity[2] = 0.0;
			Friction();
		}

		// Make sure velocity is valid.
		CheckVelocity();

		if (player->GetGroundEntity() != NULL)
		{
			WalkMove();
		}
		else
		{
			AirMove();  // Take into account movement when in air.
		}

		// Set final flags.
		CategorizePosition();

		// Make sure velocity is valid.
		CheckVelocity();

		// Add any remaining gravitational component.
		if ( !CheckWater() )
		{
			FinishGravity();
		}

		// If we are on ground, no downward velocity.
		if ( player->GetGroundEntity() != NULL )
		{
			mv->m_vecVelocity[2] = 0;
		}
		CheckFalling();
	}

	if  ( ( m_nOldWaterLevel == WL_NotInWater && player->GetWaterLevel() != WL_NotInWater ) ||
		  ( m_nOldWaterLevel != WL_NotInWater && player->GetWaterLevel() == WL_NotInWater ) )
	{
		PlaySwimSound();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::FullObserverMove( void )
{
	int mode = player->GetObserverMode();

	if ( mode == OBS_MODE_IN_EYE || mode == OBS_MODE_CHASE )
	{
		CBaseEntity * target = player->GetObserverTarget();

		if ( target != NULL )
		{
			mv->m_vecAbsOrigin = target->GetAbsOrigin();
			mv->m_vecViewAngles = target->GetAbsAngles();
			mv->m_vecVelocity = target->GetAbsVelocity();
		}

		return;
	}

	if ( mode != OBS_MODE_ROAMING )
	{
		// don't move in fixed or death cam mode
		return;
	}

	if ( sv_specnoclip.GetBool() )
	{
		// roam in noclip mode
		FullNoClipMove( sv_specspeed.GetFloat(), sv_specaccelerate.GetFloat() );
		return;
	}

	// do a full clipped free roam move:

	Vector wishvel;
	Vector forward, right, up;
	Vector wishdir, wishend;
	float wishspeed;

	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles
	
	// Copy movement amounts

	float factor = sv_specspeed.GetFloat();

	if ( mv->m_nButtons & IN_SPEED )
	{
		factor /= 2.0f;
	}

	float fmove = mv->m_flForwardMove * factor;
	float smove = mv->m_flSideMove * factor;
	
	VectorNormalize (forward);  // Normalize remainder of vectors
	VectorNormalize (right);    // 

	for (int i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
		wishvel[i] = forward[i]*fmove + right[i]*smove;
	wishvel[2] += mv->m_flUpMove;

	VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
	wishspeed = VectorNormalize(wishdir);

	//
	// Clamp to server defined max speed
	//

	float maxspeed = sv_maxvelocity.GetFloat(); 


	if (wishspeed > maxspeed )
	{
		VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
		wishspeed = maxspeed;
	}

	// Set pmove velocity, give observer 50% acceration bonus
	Accelerate ( wishdir, wishspeed, sv_specaccelerate.GetFloat() );

	float spd = VectorLength( mv->m_vecVelocity );
	if (spd < 1.0f)
	{
		mv->m_vecVelocity.Init();
		return;
	}
		
	float friction = sv_friction.GetFloat();
					
	// Add the amount to the drop amount.
	float drop = spd * friction * gpGlobals->frametime;

			// scale the velocity
	float newspeed = spd - drop;

	if (newspeed < 0)
		newspeed = 0;

	// Determine proportion of old speed we are using.
	newspeed /= spd;

	VectorScale( mv->m_vecVelocity, newspeed, mv->m_vecVelocity );

	CheckVelocity();

	TryPlayerMove();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::FullNoClipMove( float factor, float maxacceleration )
{
	Vector wishvel;
	Vector forward, right, up;
	Vector wishdir;
	float wishspeed;
	float maxspeed = sv_maxspeed.GetFloat() * factor;

	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles

	if ( mv->m_nButtons & IN_SPEED )
	{
		factor /= 2.0f;
	}
	
	// Copy movement amounts
	float fmove = mv->m_flForwardMove * factor;
	float smove = mv->m_flSideMove * factor;
	
	VectorNormalize (forward);  // Normalize remainder of vectors
	VectorNormalize (right);    // 

	for (int i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
		wishvel[i] = forward[i]*fmove + right[i]*smove;
	wishvel[2] += mv->m_flUpMove * factor;

	VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
	wishspeed = VectorNormalize(wishdir);

	//
	// Clamp to server defined max speed
	//
	if (wishspeed > maxspeed )
	{
		VectorScale (wishvel, maxspeed/wishspeed, wishvel);
		wishspeed = maxspeed;
	}

	if ( maxacceleration > 0.0 )
	{
		// Set pmove velocity
		Accelerate ( wishdir, wishspeed, maxacceleration );

		float spd = VectorLength( mv->m_vecVelocity );
		if (spd < 1.0f)
		{
			mv->m_vecVelocity.Init();
			return;
		}
		
		// Bleed off some speed, but if we have less than the bleed
		//  threshhold, bleed the theshold amount.
		float control = (spd < maxspeed/4.0) ? maxspeed/4.0 : spd;
		
		float friction = sv_friction.GetFloat() * player->m_surfaceFriction;
				
		// Add the amount to the drop amount.
		float drop = control * friction * gpGlobals->frametime;

		// scale the velocity
		float newspeed = spd - drop;
		if (newspeed < 0)
			newspeed = 0;

		// Determine proportion of old speed we are using.
		newspeed /= spd;
		VectorScale( mv->m_vecVelocity, newspeed, mv->m_vecVelocity );
	}
	else
	{
		VectorCopy( wishvel, mv->m_vecVelocity );
	}

	// Just move ( don't clip or anything )
	VectorMA( mv->m_vecAbsOrigin, gpGlobals->frametime, mv->m_vecVelocity, mv->m_vecAbsOrigin );

	// Zero out velocity if in noaccel mode
	if ( maxacceleration < 0.0f )
	{
		mv->m_vecVelocity.Init();
	}
}


//-----------------------------------------------------------------------------
// Checks to see if we should actually jump 
//-----------------------------------------------------------------------------
void CGameMovement::PlaySwimSound()
{
	MoveHelper()->StartSound( mv->m_vecAbsOrigin, "Player.Swim" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CGameMovement::CheckJumpButton( void )
{
	if (player->pl.deadflag)
	{
		mv->m_nOldButtons |= IN_JUMP ;	// don't jump again until released
		return false;
	}

	// See if we are waterjumping.  If so, decrement count and return.
	if (player->m_flWaterJumpTime)
	{
		player->m_flWaterJumpTime -= gpGlobals->frametime;
		if (player->m_flWaterJumpTime < 0)
			player->m_flWaterJumpTime = 0;
		
		return false;
	}

	// If we are in the water most of the way...
	if ( player->GetWaterLevel() >= 2 )
	{	
		// swimming, not jumping
		SetGroundEntity( (CBaseEntity *)NULL );

		if(player->GetWaterType() == CONTENTS_WATER)    // We move up a certain amount
			mv->m_vecVelocity[2] = 100;
		else if (player->GetWaterType() == CONTENTS_SLIME)
			mv->m_vecVelocity[2] = 80;
		
		// play swiming sound
		if ( player->m_flSwimSoundTime <= 0 )
		{
			// Don't play sound again for 1 second
			player->m_flSwimSoundTime = 1000;
			PlaySwimSound();
		}

		return false;
	}

	// No more effect
 	if (player->GetGroundEntity() == NULL)
	{
		mv->m_nOldButtons |= IN_JUMP;
		return false;		// in air, so no effect
	}

	//BG2 - Tjoppen - don't allow jumping while reloading
	if( player->GetActiveWeapon() && player->GetActiveWeapon()->m_bInReload )
		return false;

#ifndef CLIENT_DLL
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( player );
#else
	C_HL2MP_Player *pHL2Player = (C_HL2MP_Player*)C_HL2MP_Player::GetLocalPlayer();
#endif
	if( pHL2Player->m_iStamina < 50 )
		return false;				//don't drain unless we can cover some height
	//

	// Don't allow jumping when the player is in a stasis field.
	if ( player->m_Local.m_bSlowMovement )
		return false;

	if ( mv->m_nOldButtons & IN_JUMP )
		return false;		// don't pogo stick

	// Cannot jump will in the unduck transition.
	if ( player->m_Local.m_bDucking && (  player->GetFlags() & FL_DUCKING ) )
		return false;

	// Still updating the eye position.
	if ( player->m_Local.m_flDuckJumpTime > 0.0f )
		return false;

	// In the air now.
    SetGroundEntity( (CBaseEntity *)NULL );

	player->PlayStepSound( mv->m_vecAbsOrigin, player->m_pSurfaceData, 1.0, true );
	
	MoveHelper()->PlayerSetAnimation( PLAYER_JUMP );

	float flGroundFactor = 1.0f;
	if (player->m_pSurfaceData)
	{
		flGroundFactor = player->m_pSurfaceData->game.jumpFactor; 
	}

	float flMul;
	if ( g_bMovementOptimizations )
	{
#if defined(HL2_DLL) || defined(HL2_CLIENT_DLL)
		Assert( sv_gravity.GetFloat() == 600.0f );
		flMul = 160.0f;	// approx. 21 units.
#else
		Assert( sv_gravity.GetFloat() == 800.0f );
		flMul = 268.3281572999747f;
#endif

	}
	else
	{
		flMul = sqrt(2 * sv_gravity.GetFloat() * GAMEMOVEMENT_JUMP_HEIGHT);
	}

	// Acclerate upward
	// If we are ducking...
	float startz = mv->m_vecVelocity[2];
	if ( (  player->m_Local.m_bDucking ) || (  player->GetFlags() & FL_DUCKING ) )
	{
		// d = 0.5 * g * t^2		- distance traveled with linear accel
		// t = sqrt(2.0 * 45 / g)	- how long to fall 45 units
		// v = g * t				- velocity at the end (just invert it to jump up that high)
		// v = g * sqrt(2.0 * 45 / g )
		// v^2 = g * g * 2.0 * 45 / g
		// v = sqrt( g * 2.0 * 45 )
		mv->m_vecVelocity[2] = flGroundFactor * flMul;  // 2 * gravity * height
	}
	else
	{
		mv->m_vecVelocity[2] += flGroundFactor * flMul;  // 2 * gravity * height
	}

	// Add a little forward velocity based on your current forward velocity - if you are not sprinting.
#if defined( HL2_DLL ) || defined( HL2_CLIENT_DLL )
	if ( gpGlobals->maxClients == 1 )
	{
		CHLMoveData *pMoveData = ( CHLMoveData* )mv;
		Vector vecForward;
		AngleVectors( mv->m_vecViewAngles, &vecForward );
		vecForward.z = 0;
		VectorNormalize( vecForward );
		if ( !pMoveData->m_bIsSprinting && !player->m_Local.m_bDucked )
		{
			for ( int iAxis = 0; iAxis < 2 ; ++iAxis )
			{
				vecForward[iAxis] *= ( mv->m_flForwardMove * 0.5f );
	//			vecForward[iAxis] *= ( mv->m_flForwardMove * jumpforwardscale.GetFloat() );
			}
		}
		else
		{
			for ( int iAxis = 0; iAxis < 2 ; ++iAxis )
			{
				vecForward[iAxis] *= ( mv->m_flForwardMove * 0.1f );
	//			vecForward[iAxis] *= ( mv->m_flForwardMove * jumpforwardsprintscale.GetFloat() );
			}
		}
		VectorAdd( vecForward, mv->m_vecVelocity, mv->m_vecVelocity );
	}
#endif

	FinishGravity();

	mv->m_outJumpVel.z += mv->m_vecVelocity[2] - startz;
	mv->m_outStepHeight += 0.15f;

	// Set jump time.
	if ( gpGlobals->maxClients == 1 )
	{
		player->m_Local.m_flJumpTime = GAMEMOVEMENT_JUMP_TIME;
		player->m_Local.m_bInDuckJump = true;
	}

	// Flag that we jumped.
	mv->m_nOldButtons |= IN_JUMP;	// don't jump again until released

#ifndef CLIENT_DLL
	//BG2 - Draco - decrease stamina for jumping
	//CHL2MP_Player *pHL2Player = ToHL2MPPlayer( player );
	pHL2Player->m_iStamina -= 50;
	if (pHL2Player->m_iStamina < 0)//clamp to 0
	{
		pHL2Player->m_iStamina = 0;
	}
#endif

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::FullLadderMove()
{
	CheckWater();

	// Was jump button pressed? If so, set velocity to 270 away from ladder.  
	if ( mv->m_nButtons & IN_JUMP )
	{
		CheckJumpButton();
	}
	else
	{
		mv->m_nOldButtons &= ~IN_JUMP;
	}
	
	// Perform the move accounting for any base velocity.
	VectorAdd (mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
	TryPlayerMove();
	VectorSubtract (mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CGameMovement::TryPlayerMove( Vector *pFirstDest, trace_t *pFirstTrace )
{
	int			bumpcount, numbumps;
	Vector		dir;
	float		d;
	int			numplanes;
	Vector		planes[MAX_CLIP_PLANES];
	Vector		primal_velocity, original_velocity;
	Vector      new_velocity;
	int			i, j;
	trace_t	pm;
	Vector		end;
	float		time_left, allFraction;
	int			blocked;		
	
	numbumps  = 4;           // Bump up to four times
	
	blocked   = 0;           // Assume not blocked
	numplanes = 0;           //  and not sliding along any planes

	VectorCopy (mv->m_vecVelocity, original_velocity);  // Store original velocity
	VectorCopy (mv->m_vecVelocity, primal_velocity);
	
	allFraction = 0;
	time_left = gpGlobals->frametime;   // Total time for this movement operation.

	new_velocity.Init();

	for (bumpcount=0 ; bumpcount < numbumps; bumpcount++)
	{
		if ( mv->m_vecVelocity.Length() == 0.0 )
			break;

		// Assume we can move all the way from the current origin to the
		//  end point.
		VectorMA( mv->m_vecAbsOrigin, time_left, mv->m_vecVelocity, end );

		// See if we can make it from origin to end point.
		if ( g_bMovementOptimizations )
		{
			// If their velocity Z is 0, then we can avoid an extra trace here during WalkMove.
			if ( pFirstDest && end == *pFirstDest )
				pm = *pFirstTrace;
			else
				TracePlayerBBox( mv->m_vecAbsOrigin, end, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
		}
		else
		{
			TracePlayerBBox( mv->m_vecAbsOrigin, end, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
		}

		allFraction += pm.fraction;

		// If we started in a solid object, or we were in solid space
		//  the whole way, zero out our velocity and return that we
		//  are blocked by floor and wall.
		if (pm.allsolid)
		{	
			// entity is trapped in another solid
			VectorCopy (vec3_origin, mv->m_vecVelocity);
			return 4;
		}

		// If we moved some portion of the total distance, then
		//  copy the end position into the pmove.origin and 
		//  zero the plane counter.
		if( pm.fraction > 0 )
		{	
			// actually covered some distance
			VectorCopy (pm.endpos, mv->m_vecAbsOrigin);
			VectorCopy (mv->m_vecVelocity, original_velocity);
			numplanes = 0;
		}

		// If we covered the entire distance, we are done
		//  and can return.
		if (pm.fraction == 1)
		{
			 break;		// moved the entire distance
		}

		// Save entity that blocked us (since fraction was < 1.0)
		//  for contact
		// Add it if it's not already in the list!!!
		MoveHelper( )->AddToTouched( pm, mv->m_vecVelocity );

		// If the plane we hit has a high z component in the normal, then
		//  it's probably a floor
		if (pm.plane.normal[2] > 0.7)
		{
			blocked |= 1;		// floor
		}
		// If the plane has a zero z component in the normal, then it's a 
		//  step or wall
		if (!pm.plane.normal[2])
		{
			blocked |= 2;		// step / wall
		}

		// Reduce amount of m_flFrameTime left by total time left * fraction
		//  that we covered.
		time_left -= time_left * pm.fraction;

		// Did we run out of planes to clip against?
		if (numplanes >= MAX_CLIP_PLANES)
		{	
			// this shouldn't really happen
			//  Stop our movement if so.
			VectorCopy (vec3_origin, mv->m_vecVelocity);
			//Con_DPrintf("Too many planes 4\n");

			break;
		}

		// Set up next clipping plane
		VectorCopy (pm.plane.normal, planes[numplanes]);
		numplanes++;

		// modify original_velocity so it parallels all of the clip planes
		//

		// relfect player velocity 
		// Only give this a try for first impact plane because you can get yourself stuck in an acute corner by jumping in place
		//  and pressing forward and nobody was really using this bounce/reflection feature anyway...
		if ( numplanes == 1 &&
			player->GetMoveType() == MOVETYPE_WALK &&
			player->GetGroundEntity() == NULL )	
		{
			for ( i = 0; i < numplanes; i++ )
			{
				if ( planes[i][2] > 0.7  )
				{
					// floor or slope
					ClipVelocity( original_velocity, planes[i], new_velocity, 1 );
					VectorCopy( new_velocity, original_velocity );
				}
				else
				{
					ClipVelocity( original_velocity, planes[i], new_velocity, 1.0 + sv_bounce.GetFloat() * (1 - player->m_surfaceFriction) );
				}
			}

			VectorCopy( new_velocity, mv->m_vecVelocity );
			VectorCopy( new_velocity, original_velocity );
		}
		else
		{
			for (i=0 ; i < numplanes ; i++)
			{
				ClipVelocity (
					original_velocity,
					planes[i],
					mv->m_vecVelocity,
					1);

				for (j=0 ; j<numplanes ; j++)
					if (j != i)
					{
						// Are we now moving against this plane?
						if (mv->m_vecVelocity.Dot(planes[j]) < 0)
							break;	// not ok
					}
				if (j == numplanes)  // Didn't have to clip, so we're ok
					break;
			}
			
			// Did we go all the way through plane set
			if (i != numplanes)
			{	// go along this plane
				// pmove.velocity is set in clipping call, no need to set again.
				;  
			}
			else
			{	// go along the crease
				if (numplanes != 2)
				{
					VectorCopy (vec3_origin, mv->m_vecVelocity);
					break;
				}
				CrossProduct (planes[0], planes[1], dir);
				d = dir.Dot(mv->m_vecVelocity);
				VectorScale (dir, d, mv->m_vecVelocity );
			}

			//
			// if original velocity is against the original velocity, stop dead
			// to avoid tiny occilations in sloping corners
			//
			d = mv->m_vecVelocity.Dot(primal_velocity);
			if (d <= 0)
			{
				//Con_DPrintf("Back\n");
				VectorCopy (vec3_origin, mv->m_vecVelocity);
				break;
			}
		}
	}

	if ( allFraction == 0 )
	{
		VectorCopy (vec3_origin, mv->m_vecVelocity);
	}

	return blocked;
}


//-----------------------------------------------------------------------------
// Purpose: Determine whether or not the player is on a ladder (physprop or world).
//-----------------------------------------------------------------------------
inline bool CGameMovement::OnLadder( trace_t &trace )
{
	if ( trace.contents & CONTENTS_LADDER )
		return true;

	IPhysicsSurfaceProps *pPhysProps = MoveHelper( )->GetSurfaceProps();
	if ( pPhysProps )
	{
		surfacedata_t *pSurfaceData = pPhysProps->GetSurfaceData( trace.surface.surfaceProps );
		if ( pSurfaceData )
		{
			if ( pSurfaceData->game.climbable != 0 )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CGameMovement::LadderMove( void )
{
	trace_t pm;
	bool onFloor;
	Vector floor;
	Vector wishdir;
	Vector end;

	if ( player->GetMoveType() == MOVETYPE_NOCLIP )
		return false;

	// If I'm already moving on a ladder, use the previous ladder direction
	if ( player->GetMoveType() == MOVETYPE_LADDER )
	{
		wishdir = -player->m_vecLadderNormal;
	}
	else
	{
		// otherwise, use the direction player is attempting to move
		if ( mv->m_flForwardMove || mv->m_flSideMove )
		{
			for (int i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
				wishdir[i] = m_vecForward[i]*mv->m_flForwardMove + m_vecRight[i]*mv->m_flSideMove;

			VectorNormalize(wishdir);
		}
		else
		{
			// Player is not attempting to move, no ladder behavior
			return false;
		}
	}

	// wishdir points toward the ladder if any exists
	VectorMA( mv->m_vecAbsOrigin, LadderDistance(), wishdir, end );
	TracePlayerBBox( mv->m_vecAbsOrigin, end, LadderMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm );

	// no ladder in that direction, return
	if ( pm.fraction == 1.0f || !OnLadder( pm ) )
		return false;

	player->SetMoveType( MOVETYPE_LADDER );
	player->SetMoveCollide( MOVECOLLIDE_DEFAULT );

	player->m_vecLadderNormal = pm.plane.normal;

	// On ladder, convert movement to be relative to the ladder

	VectorCopy( mv->m_vecAbsOrigin, floor );
	floor[2] += GetPlayerMins()[2] - 1;

	if( enginetrace->GetPointContents( floor ) == CONTENTS_SOLID )
	{
		onFloor = true;
	}
	else
	{
		onFloor = false;
	}

	player->SetGravity( 0 );
	
	float forwardSpeed = 0, rightSpeed = 0;
	if ( mv->m_nButtons & IN_BACK )
		forwardSpeed -= MAX_CLIMB_SPEED;
	
	if ( mv->m_nButtons & IN_FORWARD )
		forwardSpeed += MAX_CLIMB_SPEED;
	
	if ( mv->m_nButtons & IN_MOVELEFT )
		rightSpeed -= MAX_CLIMB_SPEED;
	
	if ( mv->m_nButtons & IN_MOVERIGHT )
		rightSpeed += MAX_CLIMB_SPEED;

	if ( mv->m_nButtons & IN_JUMP )
	{
		player->SetMoveType( MOVETYPE_WALK );
		player->SetMoveCollide( MOVECOLLIDE_DEFAULT );

		VectorScale( pm.plane.normal, 270, mv->m_vecVelocity );
	}
	else
	{
		if ( forwardSpeed != 0 || rightSpeed != 0 )
		{
			Vector velocity, perp, cross, lateral, tmp;

			//ALERT(at_console, "pev %.2f %.2f %.2f - ",
			//	pev->velocity.x, pev->velocity.y, pev->velocity.z);
			// Calculate player's intended velocity
			//Vector velocity = (forward * gpGlobals->v_forward) + (right * gpGlobals->v_right);
			VectorScale( m_vecForward, forwardSpeed, velocity );
			VectorMA( velocity, rightSpeed, m_vecRight, velocity );

			// Perpendicular in the ladder plane
			VectorCopy( vec3_origin, tmp );
			tmp[2] = 1;
			CrossProduct( tmp, pm.plane.normal, perp );
			VectorNormalize( perp );

			// decompose velocity into ladder plane
			float normal = DotProduct( velocity, pm.plane.normal );

			// This is the velocity into the face of the ladder
			VectorScale( pm.plane.normal, normal, cross );

			// This is the player's additional velocity
			VectorSubtract( velocity, cross, lateral );

			// This turns the velocity into the face of the ladder into velocity that
			// is roughly vertically perpendicular to the face of the ladder.
			// NOTE: It IS possible to face up and move down or face down and move up
			// because the velocity is a sum of the directional velocity and the converted
			// velocity through the face of the ladder -- by design.
			CrossProduct( pm.plane.normal, perp, tmp );
			VectorMA( lateral, -normal, tmp, mv->m_vecVelocity );

			if ( onFloor && normal > 0 )	// On ground moving away from the ladder
			{
				VectorMA( mv->m_vecVelocity, MAX_CLIMB_SPEED, pm.plane.normal, mv->m_vecVelocity );
			}
			//pev->velocity = lateral - (CrossProduct( trace.vecPlaneNormal, perp ) * normal);
		}
		else
		{
			mv->m_vecVelocity.Init();
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : axis - 
// Output : const char
//-----------------------------------------------------------------------------
const char *DescribeAxis( int axis )
{
	static char sz[ 32 ];

	switch ( axis )
	{
	case 0:
		Q_strncpy( sz, "X", sizeof( sz ) );
		break;
	case 1:
		Q_strncpy( sz, "Y", sizeof( sz ) );
		break;
	case 2:
	default:
		Q_strncpy( sz, "Z", sizeof( sz ) );
		break;
	}

	return sz;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::CheckVelocity( void )
{
	int i;

	//
	// bound velocity
	//
	for (i=0; i < 3; i++)
	{
		// See if it's bogus.
		if (IS_NAN(mv->m_vecVelocity[i]))
		{
			DevMsg( 1, "PM  Got a NaN velocity %s\n", DescribeAxis( i ) );
			mv->m_vecVelocity[i] = 0;
		}
		if (IS_NAN(mv->m_vecAbsOrigin[i]))
		{
			DevMsg( 1, "PM  Got a NaN origin on %s\n", DescribeAxis( i ) );
			mv->m_vecAbsOrigin[i] = 0;
		}

		// Bound it.
		if (mv->m_vecVelocity[i] > sv_maxvelocity.GetFloat()) 
		{
			DevMsg( 1, "PM  Got a velocity too high on %s\n", DescribeAxis( i ) );
			mv->m_vecVelocity[i] = sv_maxvelocity.GetFloat();
		}
		else if (mv->m_vecVelocity[i] < -sv_maxvelocity.GetFloat())
		{
			DevMsg( 1, "PM  Got a velocity too low on %s\n", DescribeAxis( i ) );
			mv->m_vecVelocity[i] = -sv_maxvelocity.GetFloat();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::AddGravity( void )
{
	float ent_gravity;

	if ( player->m_flWaterJumpTime )
		return;

	if (player->GetGravity())
		ent_gravity = player->GetGravity();
	else
		ent_gravity = 1.0;

	// Add gravity incorrectly
	mv->m_vecVelocity[2] -= (ent_gravity * sv_gravity.GetFloat() * gpGlobals->frametime);
	mv->m_vecVelocity[2] += player->GetBaseVelocity()[2] * gpGlobals->frametime;
	Vector temp = player->GetBaseVelocity();
	temp[2] = 0;
	player->SetBaseVelocity( temp );
	
	CheckVelocity();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : push - 
// Output : trace_t
//-----------------------------------------------------------------------------
void CGameMovement::PushEntity( Vector& push, trace_t *pTrace )
{
	Vector	end;
		
	VectorAdd (mv->m_vecAbsOrigin, push, end);
	TracePlayerBBox( mv->m_vecAbsOrigin, end, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, *pTrace );
	VectorCopy (pTrace->endpos, mv->m_vecAbsOrigin);

	// So we can run impact function afterwards.
	// If
	if ( pTrace->fraction < 1.0 && !pTrace->allsolid )
	{
		MoveHelper( )->AddToTouched( *pTrace, mv->m_vecVelocity );
	}
}	


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : in - 
//			normal - 
//			out - 
//			overbounce - 
// Output : int
//-----------------------------------------------------------------------------
int CGameMovement::ClipVelocity( Vector& in, Vector& normal, Vector& out, float overbounce )
{
	float	backoff;
	float	change;
	float angle;
	int		i, blocked;
	
	angle = normal[ 2 ];

	blocked = 0x00;         // Assume unblocked.
	if (angle > 0)			// If the plane that is blocking us has a positive z component, then assume it's a floor.
		blocked |= 0x01;	// 
	if (!angle)				// If the plane has no Z, it is vertical (wall/step)
		blocked |= 0x02;	// 
	

	// Determine how far along plane to slide based on incoming direction.
	backoff = DotProduct (in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change; 
		// If out velocity is too small, zero it out.
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
	
#if 0
	// slight adjustment - hopefully to adjust for displacement surfaces
	float adjust = DotProduct( out, normal );
	if( adjust < 0.0f )
	{
		out += ( normal * -adjust );
//		Msg( "Adjustment = %lf\n", adjust );
	}
#endif

	// Return blocking flags.
	return blocked;
}

//-----------------------------------------------------------------------------
// Purpose: Computes roll angle for a certain movement direction and velocity
// Input  : angles - 
//			velocity - 
//			rollangle - 
//			rollspeed - 
// Output : float 
//-----------------------------------------------------------------------------
float CGameMovement::CalcRoll ( const QAngle &angles, const Vector &velocity, float rollangle, float rollspeed )
{
	float   sign;
	float   side;
	float   value;
	Vector  forward, right, up;
	
	AngleVectors (angles, &forward, &right, &up);
	
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);
	value = rollangle;
	if (side < rollspeed)
	{
		side = side * value / rollspeed;
	}
	else
	{
		side = value;
	}
	return side*sign;
}

#define CHECKSTUCK_MINTIME 0.05  // Don't check again too quickly.

static Vector rgv3tStuckTable[54];

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CreateStuckTable( void )
{
	float x, y, z;
	int idx;
	int i;
	float zi[3];
	static int firsttime = 1;

	if ( !firsttime )
		return;

	firsttime = 0;

	memset(rgv3tStuckTable, 0, sizeof(rgv3tStuckTable));

	idx = 0;
	// Little Moves.
	x = y = 0;
	// Z moves
	for (z = -0.125 ; z <= 0.125 ; z += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	x = z = 0;
	// Y moves
	for (y = -0.125 ; y <= 0.125 ; y += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for (x = -0.125 ; x <= 0.125 ; x += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for ( x = - 0.125; x <= 0.125; x += 0.250 )
	{
		for ( y = - 0.125; y <= 0.125; y += 0.250 )
		{
			for ( z = - 0.125; z <= 0.125; z += 0.250 )
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}

	// Big Moves.
	x = y = 0;
	zi[0] = 0.0f;
	zi[1] = 1.0f;
	zi[2] = 6.0f;

	for (i = 0; i < 3; i++)
	{
		// Z moves
		z = zi[i];
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	x = z = 0;

	// Y moves
	for (y = -2.0f ; y <= 2.0f ; y += 2.0)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for (x = -2.0f ; x <= 2.0f ; x += 2.0f)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for (i = 0 ; i < 3; i++)
	{
		z = zi[i];
		
		for (x = -2.0f ; x <= 2.0f ; x += 2.0f)
		{
			for (y = -2.0f ; y <= 2.0f ; y += 2.0)
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}
	Assert( idx < sizeof(rgv3tStuckTable)/sizeof(rgv3tStuckTable[0]));
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nIndex - 
//			server - 
//			offset - 
// Output : int
//-----------------------------------------------------------------------------
int GetRandomStuckOffsets( CBasePlayer *pPlayer, Vector& offset)
{
 // Last time we did a full
	int idx;
	idx = pPlayer->m_StuckLast;

	VectorCopy(rgv3tStuckTable[idx % 54], offset);

	return (idx % 54);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nIndex - 
//			server - 
//-----------------------------------------------------------------------------
void ResetStuckOffsets( CBasePlayer *pPlayer )
{
	pPlayer->m_StuckLast = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &input - 
// Output : int
//-----------------------------------------------------------------------------
int CGameMovement::CheckStuck( void )
{
	Vector base;
	Vector offset;
	Vector test;
	EntityHandle_t hitent;
	int idx;
	float fTime;
	int i;
	trace_t traceresult;

	CreateStuckTable();

	hitent = TestPlayerPosition( mv->m_vecAbsOrigin, COLLISION_GROUP_PLAYER_MOVEMENT, traceresult );
	if ( hitent == INVALID_ENTITY_HANDLE )
	{
		ResetStuckOffsets( player );
		return 0;
	}

	// Deal with stuckness...
#ifndef _LINUX
	if ( developer.GetBool() )
	{
		bool isServer = player->IsServer();
		engine->Con_NPrintf( isServer, "%s stuck on object %i/%s", 
			isServer ? "server" : "client",
			hitent.GetEntryIndex(), MoveHelper()->GetName(hitent) );
	}
#endif

	VectorCopy( mv->m_vecAbsOrigin, base );

	// 
	// Deal with precision error in network.
	// 
	// World or BSP model
	if ( !player->IsServer() )
	{
		if ( MoveHelper()->IsWorldEntity( hitent ) )
		{
			int nReps = 0;
			ResetStuckOffsets( player );
			do 
			{
				i = GetRandomStuckOffsets( player, offset);

				VectorAdd(base, offset, test);
				
				if (TestPlayerPosition( test, COLLISION_GROUP_PLAYER_MOVEMENT, traceresult ) == INVALID_ENTITY_HANDLE)
				{
					ResetStuckOffsets( player );
					VectorCopy(test, mv->m_vecAbsOrigin);
					return 0;
				}
				nReps++;
			} while (nReps < 54);
		}
	}

	// Only an issue on the client.
	idx = player->IsServer() ? 0 : 1;

	fTime = engine->Time();
	// Too soon?
	if ( m_flStuckCheckTime[player->entindex()][idx] >=  fTime - CHECKSTUCK_MINTIME )
	{
		return 1;
	}
	m_flStuckCheckTime[player->entindex()][idx] = fTime;

	MoveHelper( )->AddToTouched( traceresult, mv->m_vecVelocity );

	i = GetRandomStuckOffsets( player, offset);

	VectorAdd(base, offset, test);

	if (TestPlayerPosition( test, COLLISION_GROUP_PLAYER_MOVEMENT, traceresult ) == INVALID_ENTITY_HANDLE)
	{
		ResetStuckOffsets( player );

		if ( i >= 27 )
		{
			VectorCopy(test, mv->m_vecAbsOrigin);
		}
		return 0;
	}

	/*
	// If player is flailing while stuck in another player ( should never happen ), then see
	//  if we can't "unstick" them forceably.
	if ( mv->m_nButtons & ( IN_JUMP | IN_DUCK | IN_ATTACK ) && ( pmv->physents[ hitent ].player != 0 ) )
	{
		float x, y, z;
		float xystep = 8.0;
		float zstep = 18.0;
		float xyminmax = xystep;
		float zminmax = 4 * zstep;
		
		for ( z = 0; z <= zminmax; z += zstep )
		{
			for ( x = -xyminmax; x <= xyminmax; x += xystep )
			{
				for ( y = -xyminmax; y <= xyminmax; y += xystep )
				{
					VectorCopy( base, test );
					test[0] += x;
					test[1] += y;
					test[2] += z;

					if ( pmv->TestPosition ( test, NULL ) == -1 )
					{
						VectorCopy( test, mv->m_vecAbsOrigin );
						return 0;
					}
				}
			}
		}
	}
	*/
	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : bool
//-----------------------------------------------------------------------------
bool CGameMovement::InWater( void )
{
	return ( player->GetWaterLevel() > WL_Feet );
}


void CGameMovement::ResetGetPointContentsCache()
{
	m_CachedGetPointContents = -9999;
}


int CGameMovement::GetPointContentsCached( const Vector &point )
{
	if ( g_bMovementOptimizations ) 
	{
		if ( m_CachedGetPointContents == -9999 || point.DistToSqr( m_CachedGetPointContentsPoint ) > 1 )
		{
			m_CachedGetPointContents = enginetrace->GetPointContents ( point );
			m_CachedGetPointContentsPoint = point;
		}
		
		return m_CachedGetPointContents;
	}
	else
	{
		return enginetrace->GetPointContents ( point );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &input - 
// Output : bool
//-----------------------------------------------------------------------------
bool CGameMovement::CheckWater( void )
{
	Vector	point;
	int		cont;

	// Pick a spot just above the players feet.
	point[0] = mv->m_vecAbsOrigin[0] + (GetPlayerMins()[0] + GetPlayerMaxs()[0]) * 0.5;
	point[1] = mv->m_vecAbsOrigin[1] + (GetPlayerMins()[1] + GetPlayerMaxs()[1]) * 0.5;
	point[2] = mv->m_vecAbsOrigin[2] + GetPlayerMins()[2] + 1;
	
	// Assume that we are not in water at all.
	player->SetWaterLevel( WL_NotInWater );
	player->SetWaterType( CONTENTS_EMPTY );

	// Grab point contents.
	cont = GetPointContentsCached( point );	
	
	// Are we under water? (not solid and not empty?)
	if ( cont & MASK_WATER )
	{
		// Set water type
		player->SetWaterType( cont );

		// We are at least at level one
		player->SetWaterLevel( WL_Feet );

		// Now check a point that is at the player hull midpoint.
		point[2] = mv->m_vecAbsOrigin[2] + (GetPlayerMins()[2] + GetPlayerMaxs()[2])*0.5;
		cont = enginetrace->GetPointContents( point );
		// If that point is also under water...
		if ( cont & MASK_WATER )
		{
			// Set a higher water level.
			player->SetWaterLevel( WL_Waist );

			// Now check the eye position.  (view_ofs is relative to the origin)
			point[2] = mv->m_vecAbsOrigin[2] + player->GetViewOffset()[2];
			cont = enginetrace->GetPointContents( point );
			if ( cont & MASK_WATER )
				player->SetWaterLevel( WL_Eyes );  // In over our eyes
		}

		// Adjust velocity based on water current, if any.
		if ( cont & MASK_CURRENT )
		{
			Vector v;
			VectorClear(v);
			if ( cont & CONTENTS_CURRENT_0 )
				v[0] += 1;
			if ( cont & CONTENTS_CURRENT_90 )
				v[1] += 1;
			if ( cont & CONTENTS_CURRENT_180 )
				v[0] -= 1;
			if ( cont & CONTENTS_CURRENT_270 )
				v[1] -= 1;
			if ( cont & CONTENTS_CURRENT_UP )
				v[2] += 1;
			if ( cont & CONTENTS_CURRENT_DOWN )
				v[2] -= 1;

			// BUGBUG -- this depends on the value of an unspecified enumerated type
			// The deeper we are, the stronger the current.
			Vector temp;
			VectorMA( player->GetBaseVelocity(), 50.0*player->GetWaterLevel(), v, temp );
			player->SetBaseVelocity( temp );
		}
	}

	return ( player->GetWaterLevel() > WL_Feet );
}

void CGameMovement::SetGroundEntity( CBaseEntity *newGround )
{
	CBaseEntity *oldGround = player->GetGroundEntity();
	Vector vecBaseVelocity = player->GetBaseVelocity();

	if ( !oldGround && newGround )
	{
		// Subtract ground velocity at instant we hit ground jumping
		vecBaseVelocity -= newGround->GetAbsVelocity(); 
		vecBaseVelocity.z = newGround->GetAbsVelocity().z;
	}
	else if ( oldGround && !newGround )
	{
		// Add in ground velocity at instant we started jumping
 		vecBaseVelocity += oldGround->GetAbsVelocity();
		vecBaseVelocity.z = oldGround->GetAbsVelocity().z;
	}
	// TODO
	//else if ( oldGround && newGround && oldGround != newGround )
	//{
	//   subtract old and add new ground velocity?  When might his occur, who knows?  ywb 9/24/03
	//}

	player->SetBaseVelocity( vecBaseVelocity );
	player->SetGroundEntity( newGround );
}

//-----------------------------------------------------------------------------
// Traces the player's collision bounds in quadrants, looking for a plane that
// can be stood upon (normal's z >= 0.7f).  Regardless of success or failure,
// replace the fraction and endpos with the original ones, so we don't try to
// move the player down to the new floor and get stuck on a leaning wall that
// the original trace hit first.
//-----------------------------------------------------------------------------
void TracePlayerBBoxForGround( const Vector& start, const Vector& end, const Vector& minsSrc,
							  const Vector& maxsSrc, IHandleEntity *player, unsigned int fMask,
							  int collisionGroup, trace_t& pm )
{
	VPROF( "TracePlayerBBoxForGround" );

	Ray_t ray;
	Vector mins, maxs;

	float fraction = pm.fraction;
	Vector endpos = pm.endpos;

	// Check the -x, -y quadrant
	mins = minsSrc;
	maxs.Init( min( 0, maxsSrc.x ), min( 0, maxsSrc.y ), maxsSrc.z );
	ray.Init( start, end, mins, maxs );
	UTIL_TraceRay( ray, fMask, player, collisionGroup, &pm );
	if ( pm.m_pEnt && pm.plane.normal[2] >= 0.7)
	{
		pm.fraction = fraction;
		pm.endpos = endpos;
		return;
	}

	// Check the +x, +y quadrant
	mins.Init( max( 0, minsSrc.x ), max( 0, minsSrc.y ), minsSrc.z );
	maxs = maxsSrc;
	ray.Init( start, end, mins, maxs );
	UTIL_TraceRay( ray, fMask, player, collisionGroup, &pm );
	if ( pm.m_pEnt && pm.plane.normal[2] >= 0.7)
	{
		pm.fraction = fraction;
		pm.endpos = endpos;
		return;
	}

	// Check the -x, +y quadrant
	mins.Init( minsSrc.x, max( 0, minsSrc.y ), minsSrc.z );
	maxs.Init( min( 0, maxsSrc.x ), maxsSrc.y, maxsSrc.z );
	ray.Init( start, end, mins, maxs );
	UTIL_TraceRay( ray, fMask, player, collisionGroup, &pm );
	if ( pm.m_pEnt && pm.plane.normal[2] >= 0.7)
	{
		pm.fraction = fraction;
		pm.endpos = endpos;
		return;
	}

	// Check the +x, -y quadrant
	mins.Init( max( 0, minsSrc.x ), minsSrc.y, minsSrc.z );
	maxs.Init( maxsSrc.x, min( 0, maxsSrc.y ), maxsSrc.z );
	ray.Init( start, end, mins, maxs );
	UTIL_TraceRay( ray, fMask, player, collisionGroup, &pm );
	if ( pm.m_pEnt && pm.plane.normal[2] >= 0.7)
	{
		pm.fraction = fraction;
		pm.endpos = endpos;
		return;
	}

	pm.fraction = fraction;
	pm.endpos = endpos;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &input - 
//-----------------------------------------------------------------------------
void CGameMovement::CategorizePosition( void )
{
	Vector point;
	trace_t pm;

	// observers don't have a ground entity
	if ( player->IsObserver() )
		return;
	
	// if the player hull point one unit down is solid, the player
	// is on ground
	
	// see if standing on something solid	

	// Doing this before we move may introduce a potential latency in water detection, but
	// doing it after can get us stuck on the bottom in water if the amount we move up
	// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
	// this several times per frame, so we really need to avoid sticking to the bottom of
	// water on each call, and the converse case will correct itself if called twice.
	CheckWater();

	point[0] = mv->m_vecAbsOrigin[0];
	point[1] = mv->m_vecAbsOrigin[1];
//	point[2] = mv->m_vecAbsOrigin[2] - 2;
	point[2] = mv->m_vecAbsOrigin[2] - 2;		// move a total of 4 units to try and avoid some
	                                        // epsilon error

	Vector bumpOrigin;
	bumpOrigin = mv->m_vecAbsOrigin;
	bumpOrigin.z += 2;

	// Shooting up really fast.  Definitely not on ground.
	// On ladder moving up, so not on ground either
	// NOTE: 145 is a jump.
	if ( mv->m_vecVelocity[2] > 140 || 
		( mv->m_vecVelocity[2] > 0.0f && player->GetMoveType() == MOVETYPE_LADDER ) )   
	{
		SetGroundEntity( (CBaseEntity *)NULL );
	}
	else
	{
		// Try and move down.
		TracePlayerBBox( bumpOrigin, point, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
		
		// Moving up two units got us stuck in something, start tracing down exactly at our
		//  current origin (since CheckStuck allowed us to get here, that pos is valid)
		if ( pm.startsolid )
		{
			bumpOrigin = mv->m_vecAbsOrigin;
			TracePlayerBBox( bumpOrigin, point, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
		}

		// If we hit a steep plane, we are not on ground
		if ( pm.plane.normal[2] < 0.7)
		{
			// Test four sub-boxes, to see if any of them would have found shallower slope we could
			// actually stand on
			TracePlayerBBoxForGround( bumpOrigin, point, GetPlayerMins(), GetPlayerMaxs(), mv->m_nPlayerHandle.Get(), MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
			if ( pm.plane.normal[2] < 0.7)
			{

				SetGroundEntity( (CBaseEntity *)NULL );	// too steep
				// probably want to add a check for a +z velocity too!
				if ( ( mv->m_vecVelocity.z > 0.0f ) && ( player->GetMoveType() != MOVETYPE_NOCLIP ) )
				{
					player->m_surfaceFriction = 0.25f;
				}
			}
			else
			{
				SetGroundEntity( pm.m_pEnt );  // Otherwise, point to index of ent under us.
			}
		}
		else
		{
			SetGroundEntity( pm.m_pEnt );  // Otherwise, point to index of ent under us.
		}

		// If we are on something...
		if (player->GetGroundEntity() != NULL)
		{
			// Then we are not in water jump sequence
			player->m_flWaterJumpTime = 0;
			
			// If we could make the move, drop us down that 1 pixel
			if ( player->GetWaterLevel() < WL_Waist && !pm.startsolid && !pm.allsolid )
			{
				// check distance we would like to move -- this is supposed to just keep up
				// "on the ground" surface not stap us back to earth (i.e. on move origin to
				// end position when the ground is within .5 units away) (2 units)
				if( pm.fraction )
//				if( pm.fraction < 0.5)
				{
					VectorCopy(pm.endpos, mv->m_vecAbsOrigin);
				}
			}
		}

#ifndef CLIENT_DLL
		
		//Adrian: vehicle code handles for us.
		if ( player->IsInAVehicle() == false )
		{
			// If our gamematerial has changed, tell any player surface triggers that are watching
			IPhysicsSurfaceProps *physprops = MoveHelper()->GetSurfaceProps();
			surfacedata_t *pSurfaceProp = physprops->GetSurfaceData( pm.surface.surfaceProps );
			char cCurrGameMaterial = pSurfaceProp->game.material;
			if ( !player->GetGroundEntity() )
			{
				cCurrGameMaterial = 0;
			}

			// Changed?
			if ( player->m_chPreviousTextureType != cCurrGameMaterial )
			{
				CEnvPlayerSurfaceTrigger::SetPlayerSurface( player, cCurrGameMaterial );
			}

			player->m_chPreviousTextureType = cCurrGameMaterial;
		}
#endif

		// Standing on an entity other than the world
		if ( pm.m_pEnt && !pm.DidHitWorld() )   // So signal that we are touching something.
		{
			MoveHelper( )->AddToTouched( pm, mv->m_vecVelocity );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Determine if the player has hit the ground while falling, apply
//			damage, and play the appropriate impact sound.
//-----------------------------------------------------------------------------
void CGameMovement::CheckFalling( void )
{
	if ( player->GetGroundEntity() != NULL &&
		 !IsDead() &&
		 player->m_Local.m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHOLD )
	{
		bool bAlive = true;
		float fvol = 0.5;

		if ( player->GetWaterLevel() > 0 )
		{
			// They landed in water.
		}
		else
		{
			// Scale it down if we landed on something that's floating...
			if ( player->GetGroundEntity()->IsFloating() )
			{
				player->m_Local.m_flFallVelocity -= PLAYER_LAND_ON_FLOATING_OBJECT;
			}

			//
			// They hit the ground.
			//
			if ( player->m_Local.m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED )
			{
				//
				// If they hit the ground going this fast they may take damage (and die).
				//
				bAlive = MoveHelper( )->PlayerFallingDamage();
				fvol = 1.0;
			}
			else if ( player->m_Local.m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED / 2 )
			{
				fvol = 0.85;
			}
			else if ( player->m_Local.m_flFallVelocity < PLAYER_MIN_BOUNCE_SPEED )
			{
				fvol = 0;
			}
		}

		if ( fvol > 0.0 )
		{
			//
			// Play landing sound right away.
			//BG2 - Tjoppen - footstep fix
			player->m_flStepSoundTime = gpGlobals->curtime + 0.400f;
			//

			// Play step sound for current texture.
			player->PlayStepSound( mv->m_vecAbsOrigin, player->m_pSurfaceData, fvol, true );

			//
			// Knock the screen around a little bit, temporary effect.
			//
			player->m_Local.m_vecPunchAngle.Set( ROLL, player->m_Local.m_flFallVelocity * 0.013 );

			if ( player->m_Local.m_vecPunchAngle[PITCH] > 8 )
			{
				player->m_Local.m_vecPunchAngle.Set( PITCH, 8 );
			}
		}

		if (bAlive)
		{
			MoveHelper( )->PlayerSetAnimation( PLAYER_WALK );
		}
	}

	//
	// Clear the fall velocity so the impact doesn't happen again.
	//
	if ( player->GetGroundEntity() != NULL ) 
	{		
		player->m_Local.m_flFallVelocity = 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Use for ease-in, ease-out style interpolation (accel/decel)  Used by ducking code.
// Input  : value - 
//			scale - 
// Output : float
//-----------------------------------------------------------------------------
float CGameMovement::SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

//-----------------------------------------------------------------------------
// Purpose: Determine if crouch/uncrouch caused player to get stuck in world
// Input  : direction - 
//-----------------------------------------------------------------------------
void CGameMovement::FixPlayerCrouchStuck( bool upward )
{
	EntityHandle_t hitent;
	int i;
	Vector test;
	trace_t dummy;

	int direction = upward ? 1 : 0;

	hitent = TestPlayerPosition( mv->m_vecAbsOrigin, COLLISION_GROUP_PLAYER_MOVEMENT, dummy );
	if (hitent == INVALID_ENTITY_HANDLE )
		return;
	
	VectorCopy( mv->m_vecAbsOrigin, test );	
	for ( i = 0; i < 36; i++ )
	{
		mv->m_vecAbsOrigin[2] += direction;
		hitent = TestPlayerPosition( mv->m_vecAbsOrigin, COLLISION_GROUP_PLAYER_MOVEMENT, dummy );
		if (hitent == INVALID_ENTITY_HANDLE )
			return;
	}

	VectorCopy( test, mv->m_vecAbsOrigin ); // Failed
}

bool CGameMovement::CanUnduck()
{
	int i;
	trace_t trace;
	Vector newOrigin;

	VectorCopy( mv->m_vecAbsOrigin, newOrigin );

	if ( player->GetGroundEntity() != NULL )
	{
		for ( i = 0; i < 3; i++ )
		{
			newOrigin[i] += ( VEC_DUCK_HULL_MIN[i] - VEC_HULL_MIN[i] );
		}
	}
	else
	{
		// If in air an letting go of croush, make sure we can offset origin to make
		//  up for uncrouching
		Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;
		Vector viewDelta = ( hullSizeNormal - hullSizeCrouch );
		viewDelta.Negate();
		VectorAdd( newOrigin, viewDelta, newOrigin );
	}

	bool saveducked = player->m_Local.m_bDucked;
	player->m_Local.m_bDucked = false;
	TracePlayerBBox( mv->m_vecAbsOrigin, newOrigin, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );
	player->m_Local.m_bDucked = saveducked;
	if ( trace.startsolid || ( trace.fraction != 1.0f ) )
		return false;	

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Stop ducking
//-----------------------------------------------------------------------------
void CGameMovement::FinishUnDuck( void )
{
	int i;
	trace_t trace;
	Vector newOrigin;

	VectorCopy( mv->m_vecAbsOrigin, newOrigin );

	if ( player->GetGroundEntity() != NULL )
	{
		for ( i = 0; i < 3; i++ )
		{
			newOrigin[i] += ( VEC_DUCK_HULL_MIN[i] - VEC_HULL_MIN[i] );
		}
	}
	else
	{
		// If in air an letting go of croush, make sure we can offset origin to make
		//  up for uncrouching
		Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;
		Vector viewDelta = ( hullSizeNormal - hullSizeCrouch );
		viewDelta.Negate();
		VectorAdd( newOrigin, viewDelta, newOrigin );
	}

	player->m_Local.m_bDucked = false;
	player->RemoveFlag( FL_DUCKING );
	player->m_Local.m_bDucking  = false;
	player->SetViewOffset( GetPlayerViewOffset( false ) );
	player->m_Local.m_flDucktime = 0;
	
	VectorCopy( newOrigin, mv->m_vecAbsOrigin );

	// Recategorize position since ducking can change origin
	CategorizePosition();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CGameMovement::UpdateDuckJumpEyeOffset( void )
{
	if ( player->m_Local.m_flDuckJumpTime != 0.0f )
	{
		float flDuckMilliseconds = max( 0.0f, GAMEMOVEMENT_DUCK_TIME - ( float )player->m_Local.m_flDuckJumpTime );
		float flDuckSeconds = flDuckMilliseconds / GAMEMOVEMENT_DUCK_TIME;						
		if ( flDuckSeconds > TIME_TO_UNDUCK )
		{
			player->m_Local.m_flDuckJumpTime = 0.0f;
			return;
		}

		float flDuckFraction = SimpleSpline( 1.0f - ( flDuckSeconds / TIME_TO_UNDUCK ) );
		
		Vector vDuckHullMin = GetPlayerMins( true );
		Vector vStandHullMin = GetPlayerMins( false );
		
		float fMore = ( vDuckHullMin.z - vStandHullMin.z );
		
		Vector vecDuckViewOffset = GetPlayerViewOffset( true );
		Vector vecStandViewOffset = GetPlayerViewOffset( false );
		Vector vecTemp = player->GetViewOffset();
		vecTemp.z = ( ( vecDuckViewOffset.z - fMore ) * flDuckFraction ) + ( vecStandViewOffset.z * ( 1 - flDuckFraction ) );
		player->SetViewOffset( vecTemp );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CGameMovement::FinishUnDuckJump( trace_t &trace )
{
	Vector vecNewOrigin;
	VectorCopy( mv->m_vecAbsOrigin, vecNewOrigin );

	//  Up for uncrouching.
	Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
	Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;
	Vector viewDelta = ( hullSizeNormal - hullSizeCrouch );

	float flDeltaZ = viewDelta.z;
	viewDelta.z *= trace.fraction;
	flDeltaZ -= viewDelta.z;

	player->RemoveFlag( FL_DUCKING );
	player->m_Local.m_bDucked = false;
	player->m_Local.m_bDucking  = false;
	player->m_Local.m_bInDuckJump = false;
	player->m_Local.m_flDucktime = 0.0f;
	player->m_Local.m_flDuckJumpTime = 0.0f;
	player->m_Local.m_flJumpTime = 0.0f;
	
	Vector vecViewOffset = GetPlayerViewOffset( false );
	vecViewOffset.z -= flDeltaZ;
	player->SetViewOffset( vecViewOffset );

	VectorSubtract( vecNewOrigin, viewDelta, vecNewOrigin );
	VectorCopy( vecNewOrigin, mv->m_vecAbsOrigin );

	// Recategorize position since ducking can change origin
	CategorizePosition();
}

//-----------------------------------------------------------------------------
// Purpose: Finish ducking
//-----------------------------------------------------------------------------
void CGameMovement::FinishDuck( void )
{
	int i;

#ifndef CLIENT_DLL
	//BG2 - Draco - decrease stamina for ducking
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( player );
	pHL2Player->m_iStamina -= 15;
	if (pHL2Player->m_iStamina < 0)//clamp to 0
	{
		pHL2Player->m_iStamina = 0;
	}
#endif

	player->AddFlag( FL_DUCKING );
	player->m_Local.m_bDucked = 1;
	player->m_Local.m_bDucking = false;

	player->SetViewOffset( GetPlayerViewOffset( true ) );

	// HACKHACK - Fudge for collision bug - no time to fix this properly
	if ( player->GetGroundEntity() != NULL )
	{
		for ( i = 0; i < 3; i++ )
		{
			mv->m_vecAbsOrigin[i] -= ( VEC_DUCK_HULL_MIN[i] - VEC_HULL_MIN[i] );
		}
	}
	else
	{
		Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;
		Vector viewDelta = ( hullSizeNormal - hullSizeCrouch );
   		VectorAdd( mv->m_vecAbsOrigin, viewDelta, mv->m_vecAbsOrigin );
	}

	// See if we are stuck?
	FixPlayerCrouchStuck( true );

	// Recategorize position since ducking can change origin
	CategorizePosition();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CGameMovement::StartUnDuckJump( void )
{
	player->AddFlag( FL_DUCKING );
	player->m_Local.m_bDucked = true;
	player->m_Local.m_bDucking = false;

	player->SetViewOffset( GetPlayerViewOffset( true ) );

	Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
	Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;
	Vector viewDelta = ( hullSizeNormal - hullSizeCrouch );
	VectorAdd( mv->m_vecAbsOrigin, viewDelta, mv->m_vecAbsOrigin );

	// See if we are stuck?
	FixPlayerCrouchStuck( true );

	// Recategorize position since ducking can change origin
	CategorizePosition();
}

//
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : duckFraction - 
//-----------------------------------------------------------------------------
void CGameMovement::SetDuckedEyeOffset( float duckFraction )
{
	Vector vDuckHullMin = GetPlayerMins( true );
	Vector vStandHullMin = GetPlayerMins( false );

	float fMore = ( vDuckHullMin.z - vStandHullMin.z );

	Vector vecDuckViewOffset = GetPlayerViewOffset( true );
	Vector vecStandViewOffset = GetPlayerViewOffset( false );
	Vector temp = player->GetViewOffset();
	temp.z = ( ( vecDuckViewOffset.z - fMore ) * duckFraction ) +
				( vecStandViewOffset.z * ( 1 - duckFraction ) );
	player->SetViewOffset( temp );
}

//-----------------------------------------------------------------------------
// Purpose: Crop the speed of the player when ducking and on the ground.
//   Input: bInDuck - is the player already ducking
//          bInAir - is the player in air
//    NOTE: Only crop player speed once.
//-----------------------------------------------------------------------------
void CGameMovement::HandleDuckingSpeedCrop( void )
{
	if ( !m_bSpeedCropped && ( player->GetFlags() & FL_DUCKING ) && ( player->GetGroundEntity() != NULL ) )
	{
		float frac = 0.33333333f;
		mv->m_flForwardMove	*= frac;
		mv->m_flSideMove	*= frac;
		mv->m_flUpMove		*= frac;
		m_bSpeedCropped		= true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Check to see if we are in a situation where we can unduck jump.
//-----------------------------------------------------------------------------
bool CGameMovement::CanUnDuckJump( trace_t &trace )
{
	// Trace down to the stand position and see if we can stand.
	Vector vecEnd( mv->m_vecAbsOrigin );
	vecEnd.z -= 36.0f;						// This will have to change if bounding hull change!
	TracePlayerBBox( mv->m_vecAbsOrigin, vecEnd, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );
	if ( trace.fraction < 1.0f )
	{
		// Find the endpoint.
		vecEnd.z = mv->m_vecAbsOrigin.z + ( -36.0f * trace.fraction );

		// Test a normal hull.
		trace_t traceUp;
		bool bWasDucked = player->m_Local.m_bDucked;
		player->m_Local.m_bDucked = false;
		TracePlayerBBox( vecEnd, vecEnd, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, traceUp );
		player->m_Local.m_bDucked = bWasDucked;
		if ( !traceUp.startsolid  )
			return true;	
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: See if duck button is pressed and do the appropriate things
//-----------------------------------------------------------------------------
void CGameMovement::Duck( void )
{
	int buttonsChanged	= ( mv->m_nOldButtons ^ mv->m_nButtons );	// These buttons have changed this frame
	int buttonsPressed	=  buttonsChanged & mv->m_nButtons;			// The changed ones still down are "pressed"
	int buttonsReleased	=  buttonsChanged & mv->m_nOldButtons;		// The changed ones which were previously down are "released"

	// Check to see if we are in the air.
	bool bInAir = ( player->GetGroundEntity() == NULL );
	bool bInDuck = ( player->GetFlags() & FL_DUCKING ) ? true : false;
	bool bDuckJump = ( player->m_Local.m_flJumpTime > 0.0f );
	bool bDuckJumpTime = ( player->m_Local.m_flDuckJumpTime > 0.0f );

	if ( mv->m_nButtons & IN_DUCK )
	{
		mv->m_nOldButtons |= IN_DUCK;
	}
	else
	{
		mv->m_nOldButtons &= ~IN_DUCK;
	}

	// Handle death.
	if ( IsDead() )
		return;

	// Slow down ducked players.
	HandleDuckingSpeedCrop();

	// If the player is holding down the duck button, the player is in duck transition, ducking, or duck-jumping.
	if ( ( mv->m_nButtons & IN_DUCK ) || player->m_Local.m_bDucking  || bInDuck || bDuckJump )
	{
		// DUCK
		if ( ( mv->m_nButtons & IN_DUCK ) || bDuckJump )
		{
			// Have the duck button pressed, but the player currently isn't in the duck position.
			if ( ( buttonsPressed & IN_DUCK ) && !bInDuck && !bDuckJump && !bDuckJumpTime )
			{
				player->m_Local.m_flDucktime = GAMEMOVEMENT_DUCK_TIME;
				player->m_Local.m_bDucking = true;
			}
			
			// The player is in duck transition and not duck-jumping.
			if ( player->m_Local.m_bDucking && !bDuckJump && !bDuckJumpTime )
			{
				float flDuckMilliseconds = max( 0.0f, GAMEMOVEMENT_DUCK_TIME - ( float )player->m_Local.m_flDucktime );
				float flDuckSeconds = flDuckMilliseconds / GAMEMOVEMENT_DUCK_TIME;
				
				// Finish in duck transition when transition time is over, in "duck", in air.
				if ( ( flDuckSeconds > TIME_TO_DUCK ) || bInDuck || bInAir )
				{
					FinishDuck();
				}
				else
				{
					// Calc parametric time
					float flDuckFraction = SimpleSpline( flDuckSeconds / TIME_TO_DUCK );
					SetDuckedEyeOffset( flDuckFraction );
				}
			}

			if ( bDuckJump )
			{
				// Make the bounding box small immediately.
				if ( !bInDuck )
				{
					StartUnDuckJump();
				}
				else
				{
					// Check for a crouch override.
					if ( !( mv->m_nButtons & IN_DUCK ) )
					{
						trace_t trace;
						if ( CanUnDuckJump( trace ) )
						{
							FinishUnDuckJump( trace );
							player->m_Local.m_flDuckJumpTime = ( GAMEMOVEMENT_TIME_TO_UNDUCK * ( 1.0f - trace.fraction ) ) + GAMEMOVEMENT_TIME_TO_UNDUCK_INV;
						}
					}
				}
			}
		}
		// UNDUCK (or attempt to...)
		else
		{
			if ( player->m_Local.m_bInDuckJump )
			{
				// Check for a crouch override.
   				if ( !( mv->m_nButtons & IN_DUCK ) )
				{
					trace_t trace;
					if ( CanUnDuckJump( trace ) )
					{
						FinishUnDuckJump( trace );
					
						if ( trace.fraction < 1.0f )
						{
							player->m_Local.m_flDuckJumpTime = ( GAMEMOVEMENT_TIME_TO_UNDUCK * ( 1.0f - trace.fraction ) ) + GAMEMOVEMENT_TIME_TO_UNDUCK_INV;
						}
					}
				}
				else
				{
					player->m_Local.m_bInDuckJump = false;
				}
			}

			if ( bDuckJumpTime )
				return;

			// Try to unduck unless automovement is not allowed
			// NOTE: When not onground, you can always unduck
			if ( player->m_Local.m_bAllowAutoMovement || bInAir )
			{
				// We released the duck button, we aren't in "duck" and we are not in the air - start unduck transition.
				if ( ( buttonsReleased & IN_DUCK ) && bInDuck && !bDuckJump )
				{
					player->m_Local.m_flDucktime = GAMEMOVEMENT_DUCK_TIME;
				}

				// Check to see if we are capable of unducking.
				if ( CanUnduck() )
				{
					// or unducking
					if ( ( player->m_Local.m_bDucking || player->m_Local.m_bDucked ) )
					{
						float flDuckMilliseconds = max( 0.0f, GAMEMOVEMENT_DUCK_TIME - (float)player->m_Local.m_flDucktime );
						float flDuckSeconds = flDuckMilliseconds / GAMEMOVEMENT_DUCK_TIME;
						
						// Finish ducking immediately if duck time is over or not on ground
						if ( flDuckSeconds > TIME_TO_UNDUCK || ( bInAir && !bDuckJump ) )
						{
							FinishUnDuck();
						}
						else
						{
							// Calc parametric time
							float flDuckFraction = SimpleSpline( 1.0f - ( flDuckSeconds / TIME_TO_UNDUCK ) );
							SetDuckedEyeOffset( flDuckFraction );
							player->m_Local.m_bDucking = true;
						}
					}
				}
				else
				{
					// Still under something where we can't unduck, so make sure we reset this timer so
					//  that we'll unduck once we exit the tunnel, etc.
					player->m_Local.m_flDucktime = GAMEMOVEMENT_DUCK_TIME;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::PlayerMove( void )
{
	VPROF( "CGameMovement::PlayerMove" );

	CheckParameters();
	
	// clear output applied velocity
	mv->m_outWishVel.Init();
	mv->m_outJumpVel.Init();

	MoveHelper( )->ResetTouchList();                    // Assume we don't touch anything

	ReduceTimers();

	AngleVectors (mv->m_vecViewAngles, &m_vecForward, &m_vecRight, &m_vecUp );  // Determine movement angles

	// Always try and unstick us unless we are a couple of the movement modes
	if ( CheckInterval( STUCK ) )
	{
		if ( player->GetMoveType() != MOVETYPE_NOCLIP && 
			 player->GetMoveType() != MOVETYPE_NONE && 		 
			 player->GetMoveType() != MOVETYPE_ISOMETRIC && 
			 player->GetMoveType() != MOVETYPE_OBSERVER )
		{
			if ( CheckStuck() )
			{
				// Can't move, we're stuck
				return;  
			}
		}
	}

	// Now that we are "unstuck", see where we are (player->GetWaterLevel() and type, player->GetGroundEntity()).
	CategorizePosition();

	// Store off the starting water level
	m_nOldWaterLevel = player->GetWaterLevel();

	// If we are not on ground, store off how fast we are moving down
	if ( player->GetGroundEntity() == NULL )
	{
		player->m_Local.m_flFallVelocity = -mv->m_vecVelocity[ 2 ];
	}

	m_nOnLadder = 0;

	if ( CheckInterval( GROUND ) )
	{
		CategorizeGroundSurface();
	}

	player->UpdateStepSound( player->m_pSurfaceData, mv->m_vecAbsOrigin, mv->m_vecVelocity );

	UpdateDuckJumpEyeOffset();
	Duck();

	// Don't run ladder code if dead on on a train
	if ( !player->pl.deadflag && !(player->GetFlags() & FL_ONTRAIN) )
	{
		// If was not on a ladder now, but was on one before, 
		//  get off of the ladder
		
		// TODO: this causes lots of weirdness.
		//bool bCheckLadder = CheckInterval( LADDER );
		//if ( bCheckLadder || player->GetMoveType() == MOVETYPE_LADDER )
		{
			if ( !LadderMove() && 
				( player->GetMoveType() == MOVETYPE_LADDER ) )
			{
				// Clear ladder stuff unless player is dead or riding a train
				// It will be reset immediately again next frame if necessary
				player->SetMoveType( MOVETYPE_WALK );
				player->SetMoveCollide( MOVECOLLIDE_DEFAULT );
			}
		}
	}

	// Handle movement modes.
	switch (player->GetMoveType())
	{
		case MOVETYPE_NONE:
			break;

		case MOVETYPE_NOCLIP:
			FullNoClipMove( sv_noclipspeed.GetFloat(), sv_noclipaccelerate.GetFloat() );
			break;

		case MOVETYPE_FLY:
		case MOVETYPE_FLYGRAVITY:
			FullTossMove();
			break;

		case MOVETYPE_LADDER:
			FullLadderMove();
			break;

		case MOVETYPE_WALK:
			FullWalkMove();
			break;

		case MOVETYPE_ISOMETRIC:
			//IsometricMove();
			// Could also try:  FullTossMove();
			FullWalkMove();
			break;
			
		case MOVETYPE_OBSERVER:
			FullObserverMove(); // clips against world&players
			break;

		default:
			DevMsg( 1, "Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", player->GetMoveType(), player->IsServer());
			break;
	}
}


//-----------------------------------------------------------------------------
// Performs the collision resolution for fliers.
//-----------------------------------------------------------------------------
void CGameMovement::PerformFlyCollisionResolution( trace_t &pm, Vector &move )
{
	Vector base;
	float vel;
	float backoff;

	switch (player->GetMoveCollide())
	{
	case MOVECOLLIDE_FLY_CUSTOM:
		// Do nothing; the velocity should have been modified by touch
		// FIXME: It seems wrong for touch to modify velocity
		// given that it can be called in a number of places
		// where collision resolution do *not* in fact occur

		// Should this ever occur for players!?
		Assert(0);
		break;

	case MOVECOLLIDE_FLY_BOUNCE:	
	case MOVECOLLIDE_DEFAULT:
		{
			if (player->GetMoveCollide() == MOVECOLLIDE_FLY_BOUNCE)
				backoff = 2.0 - player->m_surfaceFriction;
			else
				backoff = 1;

			ClipVelocity (mv->m_vecVelocity, pm.plane.normal, mv->m_vecVelocity, backoff);
		}
		break;

	default:
		// Invalid collide type!
		Assert(0);
		break;
	}

	// stop if on ground
	if (pm.plane.normal[2] > 0.7)
	{		
		base.Init();
		if (mv->m_vecVelocity[2] < sv_gravity.GetFloat() * gpGlobals->frametime)
		{
			// we're rolling on the ground, add static friction.
			SetGroundEntity( pm.m_pEnt ); 
			mv->m_vecVelocity[2] = 0;
		}

		vel = DotProduct( mv->m_vecVelocity, mv->m_vecVelocity );

		// Con_DPrintf("%f %f: %.0f %.0f %.0f\n", vel, trace.fraction, ent->velocity[0], ent->velocity[1], ent->velocity[2] );

		if (vel < (30 * 30) || (player->GetMoveCollide() != MOVECOLLIDE_FLY_BOUNCE))
		{
			SetGroundEntity( pm.m_pEnt ); 
			mv->m_vecVelocity.Init();
		}
		else
		{
			VectorScale (mv->m_vecVelocity, (1.0 - pm.fraction) * gpGlobals->frametime * 0.9, move);
			PushEntity( move, &pm );
		}
		VectorSubtract( mv->m_vecVelocity, base, mv->m_vecVelocity );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameMovement::FullTossMove( void )
{
	trace_t pm;
	Vector move;
	
	CheckWater();

	// add velocity if player is moving 
	if ( (mv->m_flForwardMove != 0.0f) || (mv->m_flSideMove != 0.0f) || (mv->m_flUpMove != 0.0f))
	{
		Vector forward, right, up;
		float fmove, smove;
		Vector wishdir, wishvel;
		float wishspeed;
		int i;

		AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles

		// Copy movement amounts
		fmove = mv->m_flForwardMove;
		smove = mv->m_flSideMove;

		VectorNormalize (forward);  // Normalize remainder of vectors.
		VectorNormalize (right);    // 
		
		for (i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
			wishvel[i] = forward[i]*fmove + right[i]*smove;

		wishvel[2] += mv->m_flUpMove;

		VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
		wishspeed = VectorNormalize(wishdir);

		//
		// Clamp to server defined max speed
		//
		if (wishspeed > mv->m_flMaxSpeed)
		{
			VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
			wishspeed = mv->m_flMaxSpeed;
		}

		// Set pmove velocity
		Accelerate ( wishdir, wishspeed, sv_accelerate.GetFloat() );
	}

	if ( mv->m_vecVelocity[2] > 0 )
	{
		SetGroundEntity( (CBaseEntity *)NULL );
	}

	// If on ground and not moving, return.
	if ( player->GetGroundEntity() != NULL )
	{
		if (VectorCompare(player->GetBaseVelocity(), vec3_origin) &&
		    VectorCompare(mv->m_vecVelocity, vec3_origin))
			return;
	}

	CheckVelocity();

	// add gravity
	if ( player->GetMoveType() == MOVETYPE_FLYGRAVITY )
	{
		AddGravity();
	}

	// move origin
	// Base velocity is not properly accounted for since this entity will move again after the bounce without
	// taking it into account
	VectorAdd (mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
	
	CheckVelocity();

	VectorScale (mv->m_vecVelocity, gpGlobals->frametime, move);
	VectorSubtract (mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

	PushEntity( move, &pm );	// Should this clear basevelocity

	CheckVelocity();

	if (pm.allsolid)
	{	
		// entity is trapped in another solid
		SetGroundEntity( pm.m_pEnt );
		mv->m_vecVelocity.Init();
		return;
	}
	
	if (pm.fraction != 1)
	{
		PerformFlyCollisionResolution( pm, move );
	}
	
	// check for in water
	CheckWater();
}

//-----------------------------------------------------------------------------
// Purpose: TF2 commander mode movement logic
//-----------------------------------------------------------------------------

#pragma warning (disable : 4701)

void CGameMovement::IsometricMove( void )
{
	int i;
	Vector wishvel;
	float fmove, smove;
	Vector forward, right, up;

	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles
	
	// Copy movement amounts
	fmove = mv->m_flForwardMove;
	smove = mv->m_flSideMove;
	
	// No up / down movement
	forward[2] = 0;
	right[2] = 0;

	VectorNormalize (forward);  // Normalize remainder of vectors
	VectorNormalize (right);    // 

	for (i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
		wishvel[i] = forward[i]*fmove + right[i]*smove;
	//wishvel[2] += mv->m_flUpMove;

	VectorMA (mv->m_vecAbsOrigin, gpGlobals->frametime, wishvel, mv->m_vecAbsOrigin);
	
	// Zero out the velocity so that we don't accumulate a huge downward velocity from
	//  gravity, etc.
	mv->m_vecVelocity.Init();
}

#pragma warning (default : 4701)
