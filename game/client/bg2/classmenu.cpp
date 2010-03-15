//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cdll_client_int.h>
#include <globalvars_base.h>
#include <cdll_util.h>
#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IPanel.h>
/*#include <vgui_controls/ImageList.h>
#include <vgui_controls/MenuItem.h>*/

#include <stdio.h> // _snprintf define

//#include "spectatorgui.h"
#include "c_team.h"
#include "vguicenterprint.h"

#include <game/client/iviewport.h>
#include "commandmenu.h"
#include "hltvcamera.h"

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Menu.h>
#include "IGameUIFuncs.h" // for key bindings
#include <imapoverview.h>
#include <shareddefs.h>
#include <igameresources.h>
#include "engine/IEngineSound.h"

#include "classmenu.h"
#include "hl2mp_gamerules.h"

// Included for the port. -HairyPotter
#include "c_hl2mp_player.h"
//

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IGameUIFuncs *gameuifuncs; // for key binding details

ConVar cl_quickjoin( "cl_quickjoin", "0", FCVAR_ARCHIVE, "Automatically join the game after choosing a class, spawing with the default weapon kit.");
ConVar cl_classmenu_sounds( "cl_classmenu_sounds", "1", FCVAR_ARCHIVE, "Enable sounds in the team/class/weapon selection menu.");

#define CANCEL -2

using namespace vgui;

void ResolveLocalizedPath( const char *localString, char *pANSIPath, int iANSIPathSize )
{
	//this takes a "local" string and converts it to ANSI. used for paths, therefore the name
	const wchar_t *pPath = g_pVGuiLocalize->Find(localString);
	g_pVGuiLocalize->ConvertUnicodeToANSI( pPath, pANSIPath, iANSIPathSize );
}

//
//	CClassButton
//
void CClassButton::SetCommand( int command )
{
	m_iCommand = command;
}

void CClassButton::OnMousePressed(MouseCode code)
{
	CClassMenu * pThisMenu = (CClassMenu *)GetParent();

	if ( !pThisMenu )
		return;

	if ( m_iCommand == CANCEL )
	{
		pThisMenu->ToggleButtons( 1 );
		pThisMenu->ShowPanel( false );
		return;
	}


	pThisMenu->m_iClassSelection = m_iCommand; //This is used in the weapon/ammo display.

	cl_quickjoin.SetValue( pThisMenu->m_pQuickJoinCheckButton->IsSelected() ); //Set quickjoin var.

	if ( pThisMenu->m_pQuickJoinCheckButton->IsSelected() || m_iCommand == CLASS_OFFICER || m_iCommand == CLASS_SNIPER ) //Just join game with the default kit.
	{
		GetParent()->SetVisible( false );
		SetSelected( false );
		PerformCommand();
	}
	else
	{
		pThisMenu->ToggleButtons( 3 ); //Go to weapon selection.
	}	
}

void CClassButton::OnCursorEntered( void )
{
	m_bMouseOver = true;

	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	char pANSIPath[512] = "readme.txt";	//a default value that doesn't risk crashes in case of missing resource files

	//BG2 - Make it a switch for great justice. -HairyPotter
	switch ( m_iCommand )
	{
		case CLASS_OFFICER:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Off_Path", pANSIPath, sizeof pANSIPath );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Off_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		case CLASS_INFANTRY:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Inf_Path", pANSIPath, sizeof pANSIPath );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Inf_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		case CLASS_SNIPER:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Rif_Path", pANSIPath, sizeof pANSIPath );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Rif_Path", pANSIPath, sizeof pANSIPath );
				//else
				//	return;
			}
			break;
		case CLASS_SKIRMISHER:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Ski_Path", pANSIPath, sizeof pANSIPath );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Ski_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		case CLASS_LIGHT_INFANTRY:
			{
				//only british have light infantry
				if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Linf_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		default:
			return;
	}

	pThisMenu->ShowFile( pANSIPath );

	PlaySound("Classmenu.Class");
}

void CClassButton::PerformCommand( void )
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	char cmd[64];
	Q_snprintf( cmd, sizeof( cmd ), "class %i %i \n", pThisMenu->m_iTeamSelection, m_iCommand );
	engine->ServerCmd( cmd );
}

void CClassButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBorderEnabled(false); //No borders.
}
void CClassButton::Paint( void )
{
	BaseClass::Paint();

	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	int wide, tall;
	GetSize( wide, tall );
	vgui::IImage *m_pImage = NULL;

	switch( pThisMenu->m_iTeamSelection )
	{
		case TEAM_AMERICANS:
			if ( m_bMouseOver )
				m_pImage = scheme()->GetImage( AmericanMouseoverImage, false );
			else
				m_pImage = scheme()->GetImage( AmericanImage, false );
			break;
		case TEAM_BRITISH:
			if ( m_bMouseOver )
				m_pImage = scheme()->GetImage( BritishMouseoverImage, false );
			else
				m_pImage = scheme()->GetImage( BritishImage, false );
			break;
		default:
			m_pImage = NULL;
			break;
	}

	if ( m_pImage )
	{
		m_pImage->SetSize( wide, tall );
		m_pImage->Paint();
	}
}

//
//	CTeamButton
//
void CTeamButton::SetCommand( int command )
{
	m_iCommand = command;
}

void CTeamButton::OnMousePressed(MouseCode code)
{
	CClassMenu *pThisMenu = (CClassMenu *)GetParent();

	if ( !pThisMenu )
		return;

	SetSelected( false );

	if ( m_iCommand == CANCEL ) //Cancel Button
	{
		pThisMenu->ToggleButtons( 1 );
		pThisMenu->ShowPanel( false );
		return;
	}

	if( m_iCommand == TEAM_UNASSIGNED ) //Spectate
	{
		//join spectators
		engine->ServerCmd( "spectate", true );
		pThisMenu->ToggleButtons(1);
		pThisMenu->ShowPanel( false );
		return;
	}

	if( m_iCommand == -1 )
	{
		//autoassign
		int americans = g_Teams[TEAM_AMERICANS]->Get_Number_Players(),
			british = g_Teams[TEAM_BRITISH]->Get_Number_Players();

		//pick team with least players. or if equal, pick random
		if( americans > british )
			pThisMenu->m_iTeamSelection = TEAM_BRITISH;
		else if( americans < british )
			pThisMenu->m_iTeamSelection = TEAM_AMERICANS;
		else
			pThisMenu->m_iTeamSelection = random->RandomInt( TEAM_AMERICANS, TEAM_BRITISH );
	}

	PerformCommand();

	pThisMenu->ToggleButtons(2);
}

void CTeamButton::OnCursorEntered( void )
{
	//BaseClass::OnCursorEntered();
	m_bMouseOver = true;
	
	CClassMenu *pThisMenu = (CClassMenu *)GetParent();
	
	if ( !pThisMenu )
		return;

	char pANSIPath[512];

	//BG2 - Make it a switch for great justice. -HairyPotter
	switch ( m_iCommand )
	{
		case TEAM_AMERICANS:
			ResolveLocalizedPath( "#BG2_InfoHTML_Americans_Path", pANSIPath, sizeof pANSIPath );
			break;
		case TEAM_BRITISH:
			ResolveLocalizedPath( "#BG2_InfoHTML_British_Path", pANSIPath, sizeof pANSIPath );
			break;
		case TEAM_UNASSIGNED:	//Spectate
			ResolveLocalizedPath( "#BG2_InfoHTML_Spectate_Path", pANSIPath, sizeof pANSIPath );
			break;
		case -1:				//Autoassign
			ResolveLocalizedPath( "#BG2_InfoHTML_Autoassign_Path", pANSIPath, sizeof pANSIPath );
			break;
		default:
			return;
	}
	
	pThisMenu->ShowFile( pANSIPath );

	PlaySound("Classmenu.Team");
}

void CTeamButton::PerformCommand( void )
{
	CClassMenu * pThisMenu = (CClassMenu *)GetParent();

	if ( !pThisMenu )
		return;

	if ( m_iCommand > 0 ) //In case we autoassigned or something.
		pThisMenu->m_iTeamSelection = m_iCommand;
}

void CTeamButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBorderEnabled(false); //No borders.
}
void CTeamButton::Paint( void )
{
	BaseClass::Paint();

	int wide, tall;
	GetSize( wide, tall );
	vgui::IImage *m_pImage = NULL;

	if ( m_bMouseOver )
		m_pImage = scheme()->GetImage( TeamMouseoverImage, false );
	else
		m_pImage = scheme()->GetImage( TeamImage, false );

	if ( m_pImage )
	{
		m_pImage->SetSize( wide, tall );
		m_pImage->Paint();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClassMenu::CClassMenu( IViewPort *pViewPort ) : Frame( NULL, PANEL_CLASSES )
{
	m_iInfantryKey = m_iOfficerKey = m_iSniperKey = m_iSkirmisherKey = m_iCancelKey = m_iSlot6Key = 
	teammenu = classmenu = commmenu = commmenu2 = weaponmenu = -1; //Default all of these to -1
		
	m_pViewPort = pViewPort;

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetTitleBarVisible( false ); // don't draw a title bar
	SetMoveable( false );
	SetSizeable( false );
	SetProportional(false);

	SetScheme("ClientScheme");

	//Team Selection...
	m_pBritishButton = new CTeamButton( this, "BritishButton", "" );
	m_pAmericanButton = new CTeamButton( this, "AmericanButton", "" );
	m_pAutoassignButton = new CTeamButton( this, "AutoassignButton", "" );
	m_pSpectateButton = new CTeamButton( this, "SpectateButton", "" );
	m_pBritishLabel = new vgui::Label( this, "BritishLabel", "");
	m_pAmericanLabel = new vgui::Label( this, "AmericanLabel", "" );
	//
	
	//Class Selection.
	m_pInfantryButton = new CClassButton( this, "InfantryButton", "" );
	m_pOfficerButton = new CClassButton( this, "OfficerButton", "" );
	m_pSniperButton = new CClassButton( this, "RiflemanButton", "" );
	m_pSkirmisherButton = new CClassButton( this, "SkirmisherButton", "" );
	m_pLightInfantryButton = new CClassButton( this, "LightInfantryButton", "" );
	m_pInfantryLabel = new vgui::Label( this, "InfantryLabel", "" );
	m_pOfficerLabel = new vgui::Label( this, "OfficerLabel", "" );
	m_pRiflemanLabel = new vgui::Label( this, "RiflemanLabel", "" );
	m_pSkirmisherLabel = new vgui::Label( this, "SkirmisherLabel", "" );
	m_pLightInfantryLabel = new vgui::Label( this, "LightInfantryLabel", "" );
	//

	m_pQuickJoinCheckButton = new vgui::CheckButton( this, "QuickJoinCheckButton", "" );

	m_pWeaponSelection = new ImagePanel( this, "WeaponImage" );
	m_pAmmoSelection = new ImagePanel( this, "AmmoImage" );

	m_pWeaponButton1 = new CWeaponButton( this, "WeaponButton1", "" );
	m_pWeaponButton2 = new CWeaponButton( this, "WeaponButton2", "" );
	m_pWeaponButton3 = new CWeaponButton( this, "WeaponButton3", "" );
	m_pAmmoButton1 = new CAmmoButton( this, "AmmoButton1", "" );
	m_pAmmoButton2 = new CAmmoButton( this, "AmmoButton2", "" );
	m_pOK = new COkayButton( this, "Okay", "" );

	m_pCancelButton = new CClassButton( this, "CancelButton", "" );

	m_pInfoHTML = new HTML( this, "InfoHTML" );

	LoadControlSettings( "Resource/BG2Classmenu.res" );

	m_pBritishButton->SetCommand(TEAM_BRITISH);
	m_pAmericanButton->SetCommand(TEAM_AMERICANS);
	m_pAutoassignButton->SetCommand(-1);
	m_pSpectateButton->SetCommand(TEAM_UNASSIGNED);

	m_pOK->SetCommand( 1 );
	m_pCancelButton->SetCommand( CANCEL );

	m_pInfantryButton->SetCommand( CLASS_INFANTRY );
	m_pOfficerButton->SetCommand( CLASS_OFFICER );
	m_pSniperButton->SetCommand( CLASS_SNIPER );
	m_pSkirmisherButton->SetCommand( CLASS_SKIRMISHER );
	m_pLightInfantryButton->SetCommand( CLASS_LIGHT_INFANTRY );

	char pANSIPath[512];
	ResolveLocalizedPath( "#BG2_InfoHTML_Blank_Path", pANSIPath, sizeof pANSIPath );
	ShowFile( pANSIPath );

	ToggleButtons( 1 );
}

CClassMenu::~CClassMenu()
{
}

void CClassMenu::ShowFile( const char *filename )
{
	char localURL[_MAX_PATH + 7];
	Q_strncpy( localURL, "file://", sizeof( localURL ) );
		
	char pPathData[ _MAX_PATH ];
	g_pFullFileSystem->GetLocalPath( filename, pPathData, sizeof(pPathData) );
	Q_strncat( localURL, pPathData, sizeof( localURL ), COPY_ALL_CHARACTERS );

	m_pInfoHTML->OpenURL( localURL );
}

void CClassMenu::Paint( void )
{
	BaseClass::Paint();

	int wide, tall;
	GetSize( wide, tall );
	vgui::IImage *m_pImage = scheme()->GetImage( BGImage, false );

	if ( m_pImage )
	{
		m_pImage->SetSize( wide, tall );
		m_pImage->Paint();
	}
}

void CClassMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	int width, height;
	GetSize(width, height);
	SetPos( (ScreenWidth() - width) / 2, (ScreenHeight() - height) / 2 );

	m_pAmericanButton->MakeReadyForUse();
	m_pBritishButton->MakeReadyForUse();
	m_pSpectateButton->MakeReadyForUse();
	m_pAutoassignButton->MakeReadyForUse();

	/*m_pAmericanButton->SetBgColor( BLACK_BAR_COLOR );
	m_pBritishButton->SetBgColor( BLACK_BAR_COLOR );*/

	m_pInfantryButton->MakeReadyForUse();
	m_pOfficerButton->MakeReadyForUse();
	m_pSniperButton->MakeReadyForUse();
	m_pSkirmisherButton->MakeReadyForUse();
	m_pLightInfantryButton->MakeReadyForUse();

	// For weapons and ammo selection menu.. -HairyPotter
	m_pAmmoButton1->MakeReadyForUse();
	m_pAmmoButton2->MakeReadyForUse();
	m_pWeaponButton1->MakeReadyForUse();
	m_pWeaponButton2->MakeReadyForUse();
	m_pWeaponButton3->MakeReadyForUse();
	m_pOK->MakeReadyForUse();

	m_pOK->SetBgColor( BLACK_BAR_COLOR );
	//

	m_pCancelButton->MakeReadyForUse();
}

//-----------------------------------------------------------------------------
// Purpose: makes the GUI fill the screen
//-----------------------------------------------------------------------------
void CClassMenu::PerformLayout()
{
	BaseClass::PerformLayout();
}


//-----------------------------------------------------------------------------
// Purpose: when duck is pressed it hides the active part of the GUI
//-----------------------------------------------------------------------------
void CClassMenu::OnKeyCodePressed(KeyCode code)
{
	// we can't compare the keycode to a known code, because translation from bound keys
	// to vgui key codes is not 1:1. Get the engine version of the key for the binding
	// and the actual pressed key, and compare those..
//	int iLastTrappedKey = engine->GetLastPressedEngineKey();	// the enginekey version of the code param

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	MouseCode code2 = MOUSE_LEFT;	//for faking mouse presses
	if( code == m_iInfantryKey )
	{
		if( m_pBritishButton->IsVisible() ) //First slot, were in the Team Selection menu.
			m_pBritishButton->OnMousePressed( code2 );

		else if ( m_pInfantryButton->IsVisible() )
		{
			m_pInfantryButton->OnMousePressed( code2 );
		}
		else if ( m_pWeaponButton1->IsVisible() )
		{
			m_pWeaponButton1->OnMousePressed( code2 );
		}
	}
	else if( code == m_iOfficerKey )
	{
		if( m_pAmericanButton->IsVisible() ) //Second slot, were in the Team Selection menu.
			m_pAmericanButton->OnMousePressed( code2 );

		else if ( m_pOfficerButton->IsVisible() )
		{
			m_pOfficerButton->OnMousePressed( code2 );
		}
		else if ( m_pWeaponButton2->IsVisible() )
		{
			m_pWeaponButton2->OnMousePressed( code2 );
		}
	}
	else if( code == m_iSniperKey ) 
	{
		if( m_pAutoassignButton->IsVisible() ) //Third Slot...
			m_pAutoassignButton->OnMousePressed( code2 );

		else if( m_pSniperButton->IsVisible() )
		{
			m_pSniperButton->OnMousePressed( code2 );
		}
		else if ( m_pWeaponButton3->IsVisible() )
		{
			m_pWeaponButton3->OnMousePressed( code2 );
		}
	}
	else if( code == m_iSkirmisherKey )
	{
		if( m_pSpectateButton->IsVisible() ) //And fourth.
			m_pSpectateButton->OnMousePressed( code2 );

		else if( m_pSkirmisherButton->IsVisible() )
		{
			m_pSkirmisherButton->OnMousePressed( code2 );
		}
		else if ( m_pAmmoButton1->IsVisible() )
		{
			m_pAmmoButton1->OnMousePressed( code2 );
		}
	}
	else if( code == m_iLightInfantryKey )
	{
		if( m_pSpectateButton->IsVisible() ) //fifth
			m_pSpectateButton->OnMousePressed( code2 );

		else if( m_pLightInfantryButton->IsVisible() )
		{
			m_pLightInfantryButton->OnMousePressed( code2 );
		}
		else if ( m_pAmmoButton2->IsVisible() )
		{
			m_pAmmoButton2->OnMousePressed( code2 );
		}
	}
	/*else if( code == m_iSlot6Key ) //This is used only for
	{
	}*/
	else if( code == m_iCancelKey )
	{
		ToggleButtons(1);
		m_pViewPort->ShowPanel( this, false );
	}
	/*else if( code == m_iOkayKey )
	{
		if( m_pOK->IsVisible() ) //And fourth.
			m_pOK->OnMousePressed( code2 );
	}*/
	else if( code == teammenu )
	{
		if( m_pBritishButton->IsVisible() )
		{
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else
			ToggleButtons( 1 );
		
		return;
	}
	else if( code == classmenu )
	{
		if( IsInClassMenu() )
		{
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else if( !IsInClassMenu() && pPlayer->GetTeamNumber() <= TEAM_SPECTATOR )
		{
			internalCenterPrint->Print( "You can\'t select class before selecting team" );
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else
			ToggleButtons( 2 );
		
		return;
	}
	else if( code == weaponmenu )
	{
		if( IsInClassMenu() )
		{
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else if( !IsInClassMenu() && pPlayer->GetTeamNumber() <= TEAM_SPECTATOR )
		{
			internalCenterPrint->Print( "You can\'t select a weapon before choosing a team and class." );
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else
			ToggleButtons( 3 );
		
		return;
	}
	else if( code == commmenu )
	{
		if ( !pPlayer->IsAlive() ) //Make sure the player is on a team and alive to use voicecomms. -HairyPotter
			return;

		m_pViewPort->ShowPanel( PANEL_CLASSES, false );
		m_pViewPort->ShowPanel( PANEL_COMM, true );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );
		
		return;
	}
	else if( code == commmenu2 )
	{
		if ( !pPlayer->IsAlive() ) //Make sure the player is on a team and alive to use voicecomms. -HairyPotter
			return;

		m_pViewPort->ShowPanel( PANEL_CLASSES, false );
		m_pViewPort->ShowPanel( PANEL_COMM, false );
		m_pViewPort->ShowPanel( PANEL_COMM2, true );
		
		return;
	}
	else
		BaseClass::OnKeyCodePressed( code );
}

void CClassMenu::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	//clear html screen
	char pANSIPath[512];
	ResolveLocalizedPath( "#BG2_InfoHTML_Blank_Path", pANSIPath, sizeof pANSIPath );
	ShowFile( pANSIPath );

	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}

	m_pCancelButton->SetVisible( bShow );
}





//-----------------------------------------------------------------------------
// Purpose: Resets the list of players
//-----------------------------------------------------------------------------
void CClassMenu::Update( void )
{
	
	if( m_iInfantryKey < 0 ) m_iInfantryKey = gameuifuncs->GetButtonCodeForBind( "slot1" );
	if( m_iOfficerKey < 0 ) m_iOfficerKey = gameuifuncs->GetButtonCodeForBind( "slot2" );
	if( m_iSniperKey < 0 ) m_iSniperKey = gameuifuncs->GetButtonCodeForBind( "slot3" );
	if( m_iSkirmisherKey < 0 ) m_iSkirmisherKey = gameuifuncs->GetButtonCodeForBind( "slot4" );
	if( m_iLightInfantryKey < 0 ) m_iLightInfantryKey = gameuifuncs->GetButtonCodeForBind( "slot5" );
	//Below are for ammo selection only.
	if( m_iSlot6Key < 0 ) m_iSlot6Key = gameuifuncs->GetButtonCodeForBind( "slot6" );
	//

	if( m_iCancelKey < 0 ) m_iCancelKey = gameuifuncs->GetButtonCodeForBind( "slot10" );
	//if( m_iOkayKey < 0 ) m_iOkayKey = gameuifuncs->GetButtonCodeForBind( "enter" );

	if( teammenu < 0 ) teammenu = gameuifuncs->GetButtonCodeForBind( "teammenu" );
	if( classmenu < 0 ) classmenu = gameuifuncs->GetButtonCodeForBind( "classmenu" );
	if( weaponmenu < 0 ) weaponmenu = gameuifuncs->GetButtonCodeForBind( "weaponmenu" );
	if( commmenu < 0 ) commmenu = gameuifuncs->GetButtonCodeForBind( "commmenu" );
	if( commmenu2 < 0 ) commmenu2 = gameuifuncs->GetButtonCodeForBind( "commmenu2" );
}

void CClassMenu::OnThink()
{
	if ( !engine->IsInGame() ) //This prevents a crash when disconnecting from a game with the classmenu open, then rejoining. - HairyPotter
	{
		ToggleButtons( 1 ); //Reset the menu.
		return;
	}

	BaseClass::OnThink();
	//BG2 - Tjoppen - show autoassign button if we're not in the class part and we're unassigned or spectator
	m_pAutoassignButton->SetVisible( IsInClassMenu() ? false : C_BasePlayer::GetLocalPlayer() ? C_BasePlayer::GetLocalPlayer()->GetTeamNumber() <= TEAM_SPECTATOR : false );

	//show number of players in teams..
	char tmp[256];
	if( g_Teams[TEAM_BRITISH] )		//check just in case..
	{
		sprintf( tmp, "British : (%i)", /*g_pVGuiLocalize->Find( "#BG2_Spec_British_Score" ),*/ g_Teams[TEAM_BRITISH]->Get_Number_Players() );
		m_pBritishLabel->SetText( tmp );
	}

	if( g_Teams[TEAM_AMERICANS] )	//check just in case..
	{
		sprintf( tmp, "Americans : (%i)", /*g_pVGuiLocalize->Find( "#BG2_Spec_American_Score" ),*/ g_Teams[TEAM_AMERICANS]->Get_Number_Players() );
		m_pAmericanLabel->SetText( tmp );
	}

	//BG2 - Draco - set the proper names for the classes
	//BG2 - Tjoppen - updated to use UpdateClassButtonText()
	UpdateClassLabelText( m_pInfantryLabel, CLASS_INFANTRY );
	UpdateClassLabelText( m_pOfficerLabel, CLASS_OFFICER );
	UpdateClassLabelText( m_pRiflemanLabel, CLASS_SNIPER );
	UpdateClassLabelText( m_pSkirmisherLabel, CLASS_SKIRMISHER );
	UpdateClassLabelText( m_pLightInfantryLabel, CLASS_LIGHT_INFANTRY );

	UpdateAmmoButtons();
}

void CClassMenu::UpdateClassLabelText( vgui::Label *pLabel, int iClass )
{
	//Rehashed to change the text for the labels above each class button.
	//BG2 - Tjoppen - this function figures out what to print on the specified button's text field
	//					based on class limits and such
	char temp[512];
	int limit = HL2MPRules()->GetLimitTeamClass(m_iTeamSelection, iClass);

	if( m_iTeamSelection != TEAM_AMERICANS && m_iTeamSelection != TEAM_BRITISH )
		return;

	C_Team *pCurrentTeam = g_Teams[m_iTeamSelection];

	if( !pCurrentTeam )
		return;

	if( limit >= 0 )
		Q_snprintf( temp, sizeof temp, "(%i/%i)", pCurrentTeam->GetNumOfClass(iClass), limit );
	else
		Q_snprintf( temp, sizeof temp, "(%i)", pCurrentTeam->GetNumOfClass(iClass) );

	pLabel->SetText( temp );
}

void CClassMenu::UpdateAmmoButtons( void )
{
	//This handles the logic 
	m_pAmmoButton2->m_bRestricted = true;

	switch ( m_iTeamSelection )
	{
		case TEAM_AMERICANS:
			if ( m_iClassSelection == CLASS_SKIRMISHER )
			{
				if ( m_pWeaponButton1->IsSelected() ) //FOWLER
						m_pAmmoButton2->m_bRestricted = false; //Buckshot allowed
			}
			break;
		case TEAM_BRITISH:
			if ( m_iClassSelection == CLASS_LIGHT_INFANTRY )
			{
				if ( m_pWeaponButton1->IsSelected() ) //CARBINE
						m_pAmmoButton2->m_bRestricted = false; //Buckshot allowed
			}
			break;
	}
	//
}

void CClassMenu::SetData(KeyValues *data)
{
	//HACKHACK
	if( (int)data == 1 )
		ToggleButtons(2);
	else if( (int)data == 0 )
		ToggleButtons(1);
}

void CClassMenu::ToggleButtons(int iShowScreen)
{
	switch (iShowScreen)
	{
		case 1:
			m_pBritishButton->SetVisible(true);
			m_pAmericanButton->SetVisible(true);
			//BG2 - Tjoppen - show autoassign button if we're not in the class part and we're unassigned or spectator
			m_pAutoassignButton->SetVisible( IsInClassMenu() ? false : C_BasePlayer::GetLocalPlayer() ? C_BasePlayer::GetLocalPlayer()->GetTeamNumber() <= TEAM_SPECTATOR : false );
			//
			m_pSpectateButton->SetVisible(true);
			m_pBritishLabel->SetVisible(true);
			m_pAmericanLabel->SetVisible(true);

			//Invis the class selection screen...
			m_pOfficerButton->SetVisible(false);
			m_pInfantryButton->SetVisible(false);
			m_pSniperButton->SetVisible(false);
			m_pSkirmisherButton->SetVisible(false);
			m_pLightInfantryButton->SetVisible(false);
			m_pQuickJoinCheckButton->SetVisible( false );
			m_pInfantryLabel->SetVisible( false );
			m_pOfficerLabel->SetVisible( false );
			m_pRiflemanLabel->SetVisible( false );
			m_pSkirmisherLabel->SetVisible( false );
			m_pLightInfantryLabel->SetVisible( false );
			//
			//Invis the weapon seleection screen...
			m_pWeaponButton1->SetVisible(false);
			m_pWeaponButton2->SetVisible(false);
			m_pWeaponButton3->SetVisible(false);
			m_pAmmoButton1->SetVisible(false);
			m_pAmmoButton2->SetVisible(false);
			m_pOK->SetVisible(false);
			//
			//Misc
			m_pInfoHTML->SetVisible(true);
			m_pCancelButton->SetVisible(true);
			//
			break;
		case 2:
			//Invis the team selection screen...
			m_pBritishButton->SetVisible(false);
			m_pAmericanButton->SetVisible(false);
			m_pBritishLabel->SetVisible(false);
			m_pAmericanLabel->SetVisible(false);
			m_pAutoassignButton->SetVisible(false);
			m_pSpectateButton->SetVisible(false);
			// 
			//Make class selection visible
			m_pOfficerButton->SetVisible(true);
			m_pInfantryButton->SetVisible(true);
			m_pSniperButton->SetVisible(true);
			m_pSkirmisherButton->SetVisible(true);
			m_pLightInfantryButton->SetVisible(m_iTeamSelection == TEAM_BRITISH);
			m_pQuickJoinCheckButton->SetVisible( true );
			m_pQuickJoinCheckButton->SetSelected( cl_quickjoin.GetBool() );
			m_pInfantryLabel->SetVisible( true );
			m_pOfficerLabel->SetVisible( true );
			m_pRiflemanLabel->SetVisible( true );
			m_pSkirmisherLabel->SetVisible( true );
			m_pLightInfantryLabel->SetVisible(m_iTeamSelection == TEAM_BRITISH);
			//
			//Invis the weapon selection screen...
			m_pWeaponButton1->SetVisible(false);
			m_pWeaponButton2->SetVisible(false);
			m_pWeaponButton3->SetVisible(false);
			m_pAmmoButton1->SetVisible(false);
			m_pAmmoButton2->SetVisible(false);
			m_pOK->SetVisible(false);
			//
			//Misc
			m_pCancelButton->SetVisible(true);
			m_pInfoHTML->SetVisible(true);
			//
			break;
		case 3:
			// -- invis the team selection screen...
			m_pBritishButton->SetVisible(false);
			m_pAmericanButton->SetVisible(false);
			m_pBritishLabel->SetVisible(false);
			m_pAmericanLabel->SetVisible(false);
			m_pAutoassignButton->SetVisible(false);
			m_pSpectateButton->SetVisible(false);
			// 
			//Invis the class selection screen...
			m_pOfficerButton->SetVisible(false);
			m_pInfantryButton->SetVisible(false);
			m_pSniperButton->SetVisible(false);
			m_pSkirmisherButton->SetVisible(false);
			m_pLightInfantryButton->SetVisible(false);
			m_pQuickJoinCheckButton->SetVisible( false );
			m_pInfantryLabel->SetVisible( false );
			m_pOfficerLabel->SetVisible( false );
			m_pRiflemanLabel->SetVisible( false );
			m_pSkirmisherLabel->SetVisible( false );
			m_pLightInfantryLabel->SetVisible( false );
			//
			//Make weapon selection visible
			m_pWeaponButton1->SetVisible(true);
			m_pWeaponButton2->SetVisible(true);
			m_pWeaponButton3->SetVisible(true);
			m_pAmmoButton1->SetVisible(true);
			m_pAmmoButton2->SetVisible(true);
			m_pOK->SetVisible(true);
			//
			//Misc
			m_pCancelButton->SetVisible(true);
			m_pInfoHTML->SetVisible(false);
			//
			break;
	}
}

void CAmmoButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBorderEnabled(false); //No borders.
}

void CAmmoButton::OnCursorEntered( void )
{
	m_bMouseOver = true;
	PlaySound("Classmenu.Ammo");
}

void CAmmoButton::OnMousePressed(MouseCode code)
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	//Force the buttons to "reset" so we don't get mixed signals sent to the server.
	pThisMenu->m_pAmmoButton1->SetSelected ( false );
	pThisMenu->m_pAmmoButton2->SetSelected ( false );

	SetSelected( true );
}
void CAmmoButton::Paint ( void )
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	int wide, tall;
	vgui::IImage *m_pImage = NULL;
	GetSize( wide, tall );

	if ( m_bRestricted )
	{
		m_pImage = scheme()->GetImage( RestrictedImage, false );
	}
	else if ( IsSelected() || m_bMouseOver )
	{
		m_pImage = scheme()->GetImage( MouseoverImage, false );
	}
	else
	{
		m_pImage = scheme()->GetImage( Image, false );
	}

	if ( m_pImage )
	{
		m_pImage->SetSize( wide, tall );
		m_pImage->Paint();
	}

}

void CWeaponButton::OnCursorEntered( void )
{
	m_bMouseOver = true; 
	PlaySound("Classmenu.Weapon");
}

void CWeaponButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBorderEnabled(false); //No borders.
}

void CWeaponButton::OnMousePressed(MouseCode code)
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	//Force the buttons to "reset" so we don't get mixed signals sent to the server.
	pThisMenu->m_pWeaponButton1->SetSelected ( false );
	pThisMenu->m_pWeaponButton2->SetSelected ( false );
	pThisMenu->m_pWeaponButton3->SetSelected ( false );
	//

	SetSelected( true );
}

void CWeaponButton::GetWeaponImages( CWeaponButton *pButton, char *m_szActiveImage, char *m_szInactiveImage )
{
	if ( !pButton )
		return;

	int wide, tall;
	vgui::IImage *m_pImage = NULL;

	pButton->GetSize( wide, tall );

	if ( pButton->m_bMouseOver || pButton->IsSelected() )
		m_pImage = scheme()->GetImage( m_szActiveImage, false );
	else 
		m_pImage = scheme()->GetImage( m_szInactiveImage, false );

	if ( m_pImage )
	{
		m_pImage->SetSize( wide, tall );
		m_pImage->Paint();
	}
}

void CWeaponButton::Paint ( void )
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	CWeaponButton *pButton1 = pThisMenu->m_pWeaponButton1,
				  *pButton2 = pThisMenu->m_pWeaponButton2,
				  *pButton3 = pThisMenu->m_pWeaponButton3;

	if ( !pButton1 || !pButton2 || !pButton3 )
		return;

	switch ( pThisMenu->m_iTeamSelection )
	{
		case TEAM_AMERICANS:
			switch ( pThisMenu->m_iClassSelection )
			{
				case CLASS_INFANTRY:
					GetWeaponImages( pButton1, AInf1M, AInf1 );
					GetWeaponImages( pButton2, AInf2M, AInf2 );
					GetWeaponImages( pButton3, AInf3M, AInf3 );
					break;
				case CLASS_SKIRMISHER: 
					GetWeaponImages( pButton1, ASki1M, ASki1 );
					GetWeaponImages( pButton2, ASki2M, ASki2 );
					pButton3->SetVisible( false );
					break;
				default:
					pButton1->SetVisible( false );
					pButton2->SetVisible( false );
					pButton3->SetVisible( false );
					//Warning("There was an error determining which American class you chose to play as. \n");
					break;
			}
			break;
		case TEAM_BRITISH:
			switch ( pThisMenu->m_iClassSelection )
			{
				case CLASS_INFANTRY:
					GetWeaponImages( pButton1, BInf1M, BInf1 );
					GetWeaponImages( pButton2, BInf2M, BInf2 );
					pButton3->SetVisible( false );
					break;
				case CLASS_SKIRMISHER:
					GetWeaponImages( pButton1, BSki1M, BSki1 );
					GetWeaponImages( pButton2, BSki2M, BSki2 );
					pButton3->SetVisible( false );
					break;
				case CLASS_LIGHT_INFANTRY:
					GetWeaponImages( pButton1, BLight1M, BLight1 );
					pButton2->SetVisible( false );
					pButton3->SetVisible( false );
					break;
				default:
					pButton1->SetVisible( false );
					pButton2->SetVisible( false );
					pButton3->SetVisible( false );
					//Warning("There was an error determining which British class you chose to play as. \n");
					break;
			}
			break;
		default:
			//Msg("There was an error determining which class you chose in the menu. \n");
			break;
	}
}

//The button that gathers the weapons and ammo kit settings and sends the server command.
void COkayButton::SetCommand( int command, bool ammo )
{
}

void COkayButton::OnMousePressed(MouseCode code)
{
	SetSelected( false );
	PerformCommand();
}

void COkayButton::OnCursorEntered( void )
{
	//BaseClass::OnCursorEntered();
}

void COkayButton::PerformCommand( void )
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	int m_iGunType = 1,
		m_iAmmoType = AMMO_KIT_BALL;

	pThisMenu->ToggleButtons( 1 ); 
	GetParent()->SetVisible( false );

	if ( pThisMenu->m_pWeaponButton1->IsSelected() )
		m_iGunType = 1;

	if ( pThisMenu->m_pWeaponButton2->IsSelected() )
		m_iGunType = 2;

	if ( pThisMenu->m_pWeaponButton3->IsSelected() )
		m_iGunType = 3;

	//Note: use AMMO_KIT_* instead of magic numbers
	if ( pThisMenu->m_pAmmoButton1->IsSelected() )
		m_iAmmoType = AMMO_KIT_BALL;

	if ( pThisMenu->m_pAmmoButton2->IsSelected() )
		m_iAmmoType = AMMO_KIT_BUCKSHOT;

	char cmd[64];
	Q_snprintf( cmd, sizeof( cmd ), "kit %i %i \n", m_iGunType, m_iAmmoType );
	engine->ServerCmd( cmd );

	char sClass[64];
	Q_snprintf( sClass, sizeof( sClass ), "class %i %i \n", pThisMenu->m_iTeamSelection, pThisMenu->m_iClassSelection );
	engine->ServerCmd( sClass ); //Send the class AFTER we've selected a gun, otherwise you won't spawn with the right kit.
}

void PlaySound( const char *m_szSound )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	if ( !cl_classmenu_sounds.GetBool() )
		return;

	//This wonderful hack prevents the sounds from overlapping if you move to a new button before the old sound ends
	pPlayer->StopSound( m_szSound ); 

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, m_szSound );
}