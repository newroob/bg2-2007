//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hud_crosshair.h"
#include "iclientmode.h"
#include "view.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"
#include "IVRenderView.h"

//BG2 - Tjoppen - #includes
#include "../game/server/bg2/weapon_bg2base.h"
//

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar crosshair( "crosshair", "1", FCVAR_ARCHIVE );
ConVar cl_observercrosshair( "cl_observercrosshair", "1", FCVAR_ARCHIVE );

//BG2 - Tjoppen - cl_crosshair*
ConVar cl_crosshair( "cl_crosshair", "14", FCVAR_ARCHIVE, "Bitmask describing how to draw the crosshair\n  1 = dynamic circular\n  2 = static three-lined\n  4 = dot(square) in the middle\n  8 = hud/crosshair.vtf" );
ConVar cl_crosshair_scale( "cl_crosshair_scale", "1", FCVAR_ARCHIVE, "Scale of cl_crosshairstyle 1, 2 and 4" );

ConVar cl_crosshair_r( "cl_crosshair_r", "197", FCVAR_ARCHIVE, "Crosshair redness. 0-255" );
ConVar cl_crosshair_g( "cl_crosshair_g", "149", FCVAR_ARCHIVE, "Crosshair greenness. 0-255" );
ConVar cl_crosshair_b( "cl_crosshair_b", "105", FCVAR_ARCHIVE, "Crosshair blueness. 0-255" );
ConVar cl_crosshair_a( "cl_crosshair_a", "167", FCVAR_ARCHIVE, "Crosshair opacity(alpha). 0-255" );
//

using namespace vgui;

int ScreenTransform( const Vector& point, Vector& screen );

DECLARE_HUDELEMENT( CHudCrosshair );

CHudCrosshair::CHudCrosshair( const char *pElementName ) :
  CHudElement( pElementName ), BaseClass( NULL, "HudCrosshair" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	//m_pCrosshair = 0;

	m_clrCrosshair = Color( 0, 0, 0, 0 );

	m_vecCrossHairOffsetAngle.Init();

	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_CROSSHAIR );
}

void CHudCrosshair::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	//BG2 - Commented -HairyPotter
	m_pCrosshair = gHUD.GetIcon( "hud_crosshair" );
	//m_pDefaultCrosshair = gHUD.GetIcon("crosshair_default");
	//
	SetPaintBackgroundEnabled( false );

    SetSize( ScreenWidth(), ScreenHeight() );
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudCrosshair::ShouldDraw( void )
{
	bool bNeedsDraw;

	if ( m_bHideCrosshair )
		return false;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if ( pWeapon )
	{
		extern ConVar viewmodel_adjust_enabled;

		if ( pWeapon->m_bIsIronsighted ) //No crosshair in Iron Sights. -HairyPotter
			return viewmodel_adjust_enabled.GetBool();	//BG2 - Tjoppen - unless we're adjusting the sights

		if ( !pWeapon->ShouldDrawCrosshair() )
			return false;
	}

	/* disabled to avoid assuming it's an HL2 player.
	// suppress crosshair in zoom.
	if ( pPlayer->m_HL2Local.m_bZooming )
		return false;
	*/

	// draw a crosshair only if alive or spectating in eye
	bNeedsDraw = m_pCrosshair && 
		crosshair.GetInt() &&
		!engine->IsDrawingLoadingImage() &&
		!engine->IsPaused() && 
		g_pClientMode->ShouldDrawCrosshair() &&
		!( pPlayer->GetFlags() & FL_FROZEN ) &&
		( pPlayer->entindex() == render->GetViewEntity() ) &&
		!pPlayer->IsInVGuiInputMode() &&
		( pPlayer->IsAlive() ||	( pPlayer->GetObserverMode() == OBS_MODE_IN_EYE ) || ( cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING ) );

	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}

void CHudCrosshair::Paint( void )
{
	if ( !m_pCrosshair )
		return;

	if ( !IsCurrentViewAccessAllowed() )
		return;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	m_curViewAngles = CurrentViewAngles();
	m_curViewOrigin = CurrentViewOrigin();

	float x, y;
	x = ScreenWidth()/2;
	y = ScreenHeight()/2;

	// MattB - m_vecCrossHairOffsetAngle is the autoaim angle.
	// if we're not using autoaim, just draw in the middle of the 
	// screen
	if ( m_vecCrossHairOffsetAngle != vec3_angle )
	{
		QAngle angles;
		Vector forward;
		Vector point, screen;

		// this code is wrong
		angles = m_curViewAngles + m_vecCrossHairOffsetAngle;
		AngleVectors( angles, &forward );
		VectorAdd( m_curViewOrigin, forward, point );
		ScreenTransform( point, screen );

		x += 0.5f * screen[0] * ScreenWidth() + 0.5f;
		y += 0.5f * screen[1] * ScreenHeight() + 0.5f;
	}

	//BG2 - Tjoppen - other crosshair..
	/*int max = 0.02f * pPlayer->GetLocalVelocity().Length(),
		duck = pPlayer->GetFlags() & FL_DUCKING ? 0 : 2;

    for( int i = 3 + duck; i < 5 + max + duck; i++ )
	{
		int c = (i-3-duck)*10;
		if( c > 255 )
			c = 255;
		surface()->DrawSetColor( Color( c, c, c, 255 - c ) );
		surface()->DrawOutlinedRect( x - i - max/2, y - i - max/2, x + i + max/2, y + i + max/2 );
	}*/

	C_BaseBG2Weapon *weapon = dynamic_cast<C_BaseBG2Weapon*>( pPlayer->GetActiveWeapon() );

	if( !weapon )
		return;

	if( weapon )
	{
		/*Msg( "%f %f %f\n", weapon->GetSpread( C_BaseBG2Weapon::ATTACK_PRIMARY ).x,
							weapon->GetSpread( C_BaseBG2Weapon::ATTACK_PRIMARY ).y,
							weapon->GetSpread( C_BaseBG2Weapon::ATTACK_PRIMARY ).z );*/

		int w = ScreenWidth(),
			h = ScreenHeight();
		
		float	cx = w / 2,
				cy = h / 2,
				r = max( w, h ) / 2;

		//only draw the circle if we're holding a firearm that is ready to fire
		//when no, the circle will fade out. once holding a loaded firearm again, it will fade back in
		bool drawCircle = false;

		if( weapon->GetAttackType( C_BaseBG2Weapon::ATTACK_PRIMARY ) == C_BaseBG2Weapon::ATTACKTYPE_FIREARM )
		{
			drawCircle = weapon->m_iClip1;

			//add both spreads (accuracy and internal ballistics) to give a more accurate circle
			r *= weapon->GetAccuracy( C_BaseBG2Weapon::ATTACK_PRIMARY ) + weapon->GetCurrentAmmoSpread();
			r *= 0.008725f;
		}
		else
			r *= 0.05f;
	
		//r *= 1.333f; // bp - this is the constant 
		
		/*
		//BG2 - BP - crosshair cirlce actually showing the correct spread cone. bullet always goes inside the circle you see.
		// all this code is not needed to be executed it was rather added to calculate constants.
		QAngle angles;
		Vector forward, up, endpos;
		
		AngleVectors( m_curViewAngles, &forward );
		QAngle up_angle = m_curViewAngles;
		up_angle.x += 90.0f;
		AngleVectors( up_angle, &up );
		VectorMA( m_curViewOrigin, 4096.0f, forward, endpos );
		
		trace_t	trace;
		
		UTIL_TraceLine( m_curViewOrigin, endpos, CONTENTS_SOLID, NULL, COLLISION_GROUP_NONE, &trace );

		float r_plus = 1.0f;

		if (trace.fraction != 1.0)
		{
			Vector difference = trace.endpos - m_curViewOrigin;
			float distance = difference.Length();
	
			// calculate spread cone at this distance
			float spreaddistance = distance * tan(weapon->GetSpread( C_BaseBG2Weapon::ATTACK_PRIMARY ).x);
			Msg( "distance = %f spreadunits = %f spreadangle = %f\n", distance, spreaddistance, weapon->GetSpread( C_BaseBG2Weapon::ATTACK_PRIMARY ).x );

			// Transform point into screen space
			Vector screen, spread_origin;
			int ix,iy;
			spread_origin = trace.endpos; trace.
			VectorMA(spread_origin, spreaddistance, up, spread_origin);
			GetVectorInScreenSpace(spread_origin, ix, iy, NULL);

			r = iy - ScreenHeight()/2;


			//NDebugOverlay::Cross3D( spread_origin, -Vector(4,4,4), Vector(4,4,4), 255, 0, 0, true, 0.5f );
			
			//Msg( "Radius: %f \n", r);
			//Msg( "screen: %f %f\n", ix, iy);
			//Msg( "spreadend: %f %f %f\n", spread_origin.x, spread_origin.y, spread_origin.z);
		}
		*/

		//Msg( "%f\n", gpGlobals->frametime );
		
		static float lastr = 0;
		static float circlealpha = 1;
		//Msg( "Radius: %f \n", r);
		//r = lastr = r * 5.0f * gpGlobals->frametime + lastr * (1.f - 5.0f * gpGlobals->frametime);

		r = lastr = r + (lastr - r) * expf( -13.0f * gpGlobals->frametime );

		//lerp circle alpha up or down
		circlealpha += (drawCircle ? 6 : -6) * gpGlobals->frametime;

		if( circlealpha < 0 ) circlealpha = 0;
		if( circlealpha > 1 ) circlealpha = 1;

		//Msg( "%f %f %f\n", cx, cy, r );
		if( cl_crosshair.GetInt() & 4 )
		{
			float scale = cl_crosshair_scale.GetFloat() * min( w, h ) / 300;

			surface()->DrawSetColor( Color( cl_crosshair_r.GetInt(), cl_crosshair_g.GetInt(),
											cl_crosshair_b.GetInt(), cl_crosshair_a.GetInt() ) );
			//surface()->DrawFilledRect( cx-1, cy-1, cx, cy );	//dot in the middle

			//dot in the middle
			surface()->DrawFilledRect( cx - scale * 0.25f, cy - scale * 0.25f,
										cx + scale * 0.25f, cy + scale * 0.25f );
		}
		
		if( cl_crosshair.GetInt() & 2 )
		{
			float	scale = cl_crosshair_scale.GetFloat() * min( w, h ) / 300.f;/*,
					expand = r * 0.25f;*/

			surface()->DrawSetColor( Color( cl_crosshair_r.GetInt(), cl_crosshair_g.GetInt(),
											cl_crosshair_b.GetInt(), cl_crosshair_a.GetInt() ) );
			/*int size = r < 6 ? r - 2 : 4;
			if( size > 0 )
			{
				//DrawLine() is slightly broken, thus the strange values here
				surface()->DrawLine( cx - size - 4, cy, cx - 3, cy );
				surface()->DrawLine( cx + size + 2, cy, cx + 2, cy );
				surface()->DrawLine( cx, cy + 2, cx, cy + 2 + size );
			}*/

			//without expansion
			//left
			surface()->DrawFilledRect( cx - scale * 3.f, cy - scale * 0.25f,
										cx - scale, cy + scale * 0.25f );
			//right
			surface()->DrawFilledRect( cx + scale, cy - scale * 0.25f,
										cx + scale * 3.f, cy + scale * 0.25f );
			//bottom
			surface()->DrawFilledRect( cx - scale * 0.25f, cy + scale, 
										cx + scale * 0.25f, cy + scale * 3.f );

			//with expansion
			//left
			/*surface()->DrawFilledRect( cx - scale * 3.f - expand, cy - scale * 0.25f,
										cx - scale - expand, cy + scale * 0.25f );
			//right
			surface()->DrawFilledRect( cx + scale + expand, cy - scale * 0.25f,
										cx + scale * 3.f + expand, cy + scale * 0.25f );
			//bottom
			surface()->DrawFilledRect( cx - scale * 0.25f, cy + scale + expand, 
										cx + scale * 0.25f, cy + scale * 3.f + expand );*/
		}

		if( circlealpha > 0 && cl_crosshair.GetInt() & 1 )
		{
			int step = 10;

			surface()->DrawSetColor( Color( (cl_crosshair_r.GetInt()*2)/3, (cl_crosshair_g.GetInt()*2)/3,
											(cl_crosshair_b.GetInt()*2)/3, cl_crosshair_a.GetInt()/3*circlealpha ) );
			for( int dx = -1; dx <= 1; dx++ )
				for( int dy = -1; dy <= 1; dy++ )
				{
					if( dx == 0 && dy == 0 )
						continue;

					int cx2 = cx + dx,
						cy2 = cy + dy;

					int lastx = (int)(cx2 + r*cosf((float)-step * M_PI / 180.f)),
						lasty =	(int)(cy2 + r*sinf((float)-step * M_PI / 180.f));

					for( int i = 0, j = 0; i < 360; i += step, j++ )
					{
						float	a = (float)i * M_PI / 180.f;

						int x = (int)(cx2 + r*cosf(a)),
							y = (int)(cy2 + r*sinf(a));

						//surface()->DrawSetColor( Color( (j&1)*255, (j&1)*255, (j&1)*255, 255 ) );

						surface()->DrawLine( lastx, lasty, x, y );

						lastx = x;
						lasty = y;
					}
				}

			int lastx = (int)(cx + r*cosf((float)-step * M_PI / 180.f)),
				lasty =	(int)(cy + r*sinf((float)-step * M_PI / 180.f));

			surface()->DrawSetColor( Color( cl_crosshair_r.GetInt(), cl_crosshair_g.GetInt(),
											cl_crosshair_b.GetInt(), cl_crosshair_a.GetInt()*circlealpha ) );

			for( int i = 0, j = 0; i < 360; i += step, j++ )
			{
				float	a = (float)i * M_PI / 180.f;

				int x = (int)(cx + r*cosf(a)),
					y = (int)(cy + r*sinf(a));

				surface()->DrawLine( lastx, lasty, x, y );

				lastx = x;
				lasty = y;
			}
		}

		if( cl_crosshair.GetInt() & 8 )
		{
			Color iconColor( 255, 80, 0, 255 );

			r *= cl_crosshair_scale.GetFloat();

			int x = ScreenWidth()/2 - r,
				y = ScreenHeight()/2 - r;

			m_pCrosshair->DrawSelf( x, y, 2*r, 2*r, iconColor );
		}
	}

	/*m_pCrosshair->DrawSelf( 
			x - 0.5f * m_pCrosshair->Width(), 
			y - 0.5f * m_pCrosshair->Height(),
			m_clrCrosshair );*/
	//
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshairAngle( const QAngle& angle )
{
	VectorCopy( angle, m_vecCrossHairOffsetAngle );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshair( CHudTexture *texture, Color& clr )
{
	//m_pCrosshair = texture; //BG2 - Commented -HairyPotter
	m_clrCrosshair = clr;
}

//-----------------------------------------------------------------------------
// Purpose: Resets the crosshair back to the default
//-----------------------------------------------------------------------------
void CHudCrosshair::ResetCrosshair()
{
	SetCrosshair( m_pDefaultCrosshair, Color(255, 255, 255, 255) ); //BG2 - Commented -HairyPotter
}
