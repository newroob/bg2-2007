//====== Copyright © 1996-2007, Valve Corporation, All rights reserved. =======
//
// Purpose: VGUI panel which will display a preview of the map during load time.
//
//=============================================================================

#include "cbase.h"
#include <vgui/IVGui.h>
#include "vgui/IInput.h"
#include <vgui/ISurface.h>
#include "ienginevgui.h"
#include "iclientmode.h"
#include "engine/ienginesound.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/EditablePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class BG2MapDisplay : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( BG2MapDisplay, vgui::EditablePanel );
public:

	BG2MapDisplay( unsigned int nXPos, unsigned int nYPos, unsigned int nHeight, unsigned int nWidth  );

	~BG2MapDisplay( void );

	virtual void Activate( void );
	virtual void Paint( void );
	virtual void DoModal( void );
	virtual void OnClose( void );
	virtual void GetPanelPos( int &xpos, int &ypos );

	void SetExitCommand( const char *pExitCommand )
	{
		if ( pExitCommand && pExitCommand[0] )
		{
			Q_strncpy( m_szExitCommand, pExitCommand, MAX_PATH );
		}
	}

	bool BeginPlayback( const char *pFilename );

	//void SetBlackBackground( bool bBlack ){ m_bBlackBackground = bBlack; }

protected:

	virtual void OnTick( void ) { BaseClass::OnTick(); }
	virtual void OnCommand( const char *pcCommand ) { BaseClass::OnCommand( pcCommand ); }
	virtual void OnVideoOver(){}

protected:
	IMaterial		*m_pMaterial;
	int				m_nPlaybackHeight;			// Calculated to address ratio changes
	int				m_nPlaybackWidth;
	char			m_szExitCommand[MAX_PATH];	// This call is fired at the engine when the video finishes or is interrupted

	float			m_flU;	// U,V ranges for video on its sheet
	float			m_flV;

	//bool			m_bBlackBackground;
};
// Creates a VGUI panel which plays a video and executes a client command at its finish (if specified)
extern bool BG2MapDisplay_Create( unsigned int nXPos, unsigned int nYPos, 
							   unsigned int nWidth, unsigned int nHeight, const char *pMapName );


BG2MapDisplay::BG2MapDisplay( unsigned int nXPos, unsigned int nYPos, unsigned int nHeight, unsigned int nWidth ) : 
	BaseClass( NULL, "BG2MapDisplay" ),
	m_nPlaybackWidth( 0 ),
	m_nPlaybackHeight( 0 )
{
	vgui::VPANEL pParent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	SetParent( pParent );
	SetVisible( false );
	
	// Must be passed in, off by default
	m_szExitCommand[0] = '\0';

	//m_bBlackBackground = true;

	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );

	SetProportional( false );
	SetVisible( true );
	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );
	
	// Set us up
	SetTall( nHeight );
	SetWide( nWidth );
	SetPos( nXPos, nYPos );

	//SetScheme(vgui::scheme()->LoadSchemeFromFile( "resource/VideoPanelScheme.res", "VideoPanelScheme"));
	//LoadControlSettings("resource/UI/VideoPanel.res");
}

//-----------------------------------------------------------------------------
// Properly shutdown out materials
//-----------------------------------------------------------------------------
BG2MapDisplay::~BG2MapDisplay( void )
{
	SetParent( (vgui::Panel *) NULL );

	// Shut down this video
	/*if ( m_BIKHandle != BIKHANDLE_INVALID )
	{
		bik->DestroyMaterial( m_BIKHandle );
		m_BIKHandle = BIKHANDLE_INVALID;
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: Begins playback of a movie
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool BG2MapDisplay::BeginPlayback( const char *pFilename )
{
	// Destroy any previously allocated video
	/*if ( m_BIKHandle != BIKHANDLE_INVALID )
	{
		bik->DestroyMaterial( m_BIKHandle );
		m_BIKHandle = BIKHANDLE_INVALID;
	}*/

	// Load and create our BINK video
	/*m_BIKHandle = bik->CreateMaterial( "VideoBIKMaterial", pFilename, "GAME" );
	if ( m_BIKHandle == BIKHANDLE_INVALID )
		return false;*/

	// We want to be the sole audio source
	// FIXME: This may not always be true!
	/*enginesound->NotifyBeginMoviePlayback();

	int nWidth, nHeight;
	//bik->GetFrameSize( m_BIKHandle, &nWidth, &nHeight );
	//bik->GetTexCoordRange( m_BIKHandle, &m_flU, &m_flV );
	//m_pMaterial = bik->GetMaterial( m_BIKHandle );

	float flFrameRatio = ( (float) GetWide() / (float) GetTall() );
	float flVideoRatio = ( (float) nWidth / (float) nHeight );

	if ( flVideoRatio > flFrameRatio )
	{
		m_nPlaybackWidth = GetWide();
		m_nPlaybackHeight = ( GetWide() / flVideoRatio );
	}
	else if ( flVideoRatio < flFrameRatio )
	{
		m_nPlaybackWidth = ( GetTall() * flVideoRatio );
		m_nPlaybackHeight = GetTall();
	}
	else
	{
		m_nPlaybackWidth = GetWide();
		m_nPlaybackHeight = GetTall();
	}*/

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void BG2MapDisplay::Activate( void )
{
	MoveToFront();
	RequestFocus();
	SetVisible( true );
	SetEnabled( true );

	vgui::surface()->SetMinimized( GetVPanel(), false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void BG2MapDisplay::DoModal( void )
{
	/*char szMapName[64];
	Q_FileBase( const_cast<char *>(m_pClientDllInterface->GetLevelName()), szMapName );

	if ( szMapName == NULL )
		return;*/

	MakePopup();
	Activate();

	vgui::input()->SetAppModalSurface( GetVPanel() );
	vgui::surface()->RestrictPaintToSinglePanel( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void BG2MapDisplay::OnClose( void )
{
	BaseClass::OnClose();

	if ( vgui::input()->GetAppModalSurface() == GetVPanel() )
	{
		vgui::input()->ReleaseAppModalSurface();
	}

	vgui::surface()->RestrictPaintToSinglePanel( NULL );

	SetVisible( false );
	MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void BG2MapDisplay::GetPanelPos( int &xpos, int &ypos )
{
	xpos = ( (float) ( GetWide() - m_nPlaybackWidth ) / 2 );
	ypos = ( (float) ( GetTall() - m_nPlaybackHeight ) / 2 );
}

//-----------------------------------------------------------------------------
// Purpose: Update and draw the frame
//-----------------------------------------------------------------------------
void BG2MapDisplay::Paint( void )
{
	BaseClass::Paint();

	// No video to play, so do nothing
	//if ( m_BIKHandle == BIKHANDLE_INVALID )
	//	return;

	// Update our frame
	/*if ( bik->Update( m_BIKHandle ) == false )
	{
		// Issue a close command
		OnVideoOver();
		OnClose();
	}*/

	// Sit in the "center"
	int xpos, ypos;
	GetPanelPos( xpos, ypos );

	// Black out the background (we could omit drawing under the video surface, but this is straight-forward)
	/*if ( m_bBlackBackground )
	{
		vgui::surface()->DrawSetColor(  0, 0, 0, 255 );
		vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: Create and playback a video in a panel
// Input  : nWidth - Width of panel (in pixels) 
//			nHeight - Height of panel
//			*pVideoFilename - Name of the file to play
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool BG2MapDisplay_Create( unsigned int nXPos, unsigned int nYPos, 
						unsigned int nWidth, unsigned int nHeight, const char *pMapName  )
{
	// Create the base video panel
	BG2MapDisplay *pMapPanel = new BG2MapDisplay( nXPos, nYPos, nHeight, nWidth  );
	if ( pMapPanel == NULL )
		return false;

	// Start it going
	/*if ( pMapPanel->BeginPlayback( pVideoFilename ) == false )
	{
		delete pMapPanel;
		return false;
	}*/

	// Take control
	//pMapPanel->DoModal();
	Msg("Tried to create a map preview panel. \n");

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Used to launch a video playback (Debug)
//-----------------------------------------------------------------------------

CON_COMMAND( map_preview, "Tests a map background. <mapname>" )
{
	if ( args.ArgC() < 1 )
		return;

	unsigned int nScreenWidth = Q_atoi( args[2] );
	unsigned int nScreenHeight = Q_atoi( args[3] );
	Msg("Did the map_preview command. \n");
	
	char strMapName[MAX_PATH];
	Q_strncpy( strMapName, "maps/previews/", MAX_PATH );	// Assume we must play out of the media/previews directory
	//char strFilename[MAX_PATH];
	//Q_StripExtension( args[1], strFilename, MAX_PATH );
	Q_strncat( strMapName, args[1], MAX_PATH );
	Q_strncat( strMapName, ".bmp", MAX_PATH );		// Assume we're a .bmp extension type

	if ( nScreenWidth == 0 )
	{
		nScreenWidth = ScreenWidth();
	}
	
	if ( nScreenHeight == 0 )
	{
		nScreenHeight = ScreenHeight();
	}

	// Create the panel and go!
	if ( BG2MapDisplay_Create( 0, 0, nScreenWidth, nScreenHeight, strMapName ) == false )
	{
		Warning( "Unable to display map preview for: %s\n", strMapName );
	}
}