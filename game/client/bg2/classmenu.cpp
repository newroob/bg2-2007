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

#define CVAR_FLAGS	( FCVAR_ARCHIVE )

#define LIMIT_DEFINES( iclass, name )\
	ConVar cl_kit_a_##iclass( "cl_kit_a_"#iclass, "1 0", CVAR_FLAGS,\
									"Default kit for American " name " class." );\
	ConVar cl_kit_b_##iclass( "cl_kit_b_"#iclass, "1 0", CVAR_FLAGS,\
									"Default kit for British " name " class." );

LIMIT_DEFINES( inf, "infantry" )
LIMIT_DEFINES( ski, "skirmisher" )

ConVar cl_kit_b_light("cl_kit_b_light", "1 0", CVAR_FLAGS, "Default kit for British light infantry class." );

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
		pThisMenu->SetScreen( 1, false );
		return;
	}


	pThisMenu->m_iClassSelection = m_iCommand; //This is used in the weapon/ammo display.

	cl_quickjoin.SetValue( pThisMenu->m_pQuickJoinCheckButton->IsSelected() ); //Set quickjoin var.

	if ( pThisMenu->m_pQuickJoinCheckButton->IsSelected() || !pThisMenu->SetScreen( 3, true, false ) ) //Just join game with the default kit.
	{
		GetParent()->SetVisible( false );
		SetSelected( false );
		PerformCommand();
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
				else
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Off_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		case CLASS_INFANTRY:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Inf_Path", pANSIPath, sizeof pANSIPath );
				else
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Inf_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		case CLASS_SNIPER:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Rif_Path", pANSIPath, sizeof pANSIPath );
				else
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Rif_Path", pANSIPath, sizeof pANSIPath );
				//else
				//	return;
			}
			break;
		case CLASS_SKIRMISHER:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Ski_Path", pANSIPath, sizeof pANSIPath );
				else
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

	PlaySound("Classmenu.Join");

	//Update the DefaultKit struct. This should be called so previously selected kits are loaded for people who quick join.
	pThisMenu->UpdateDefaultWeaponKit( pThisMenu->m_iTeamSelection, m_iCommand );

	char cmd[16];
	Q_snprintf( cmd, sizeof( cmd ), "kit %i %i \n", pThisMenu->DefaultKit.m_iGun, pThisMenu->DefaultKit.m_iAmmo );
	engine->ServerCmd( cmd );

	char sClass[16];
	Q_snprintf( sClass, sizeof( sClass ), "class %i %i \n", pThisMenu->m_iTeamSelection, m_iCommand );
	engine->ServerCmd( sClass );
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

	switch( m_iCommand )
	{
		case CANCEL:
			pThisMenu->SetScreen( 1, false );
			return;

		case TEAM_UNASSIGNED:
			engine->ServerCmd( "spectate", true );
			pThisMenu->SetScreen( 1, false );
			return;

		case -1: //autoassign
			int americans = g_Teams[TEAM_AMERICANS]->Get_Number_Players(),
				british = g_Teams[TEAM_BRITISH]->Get_Number_Players();

			//pick team with least players. or if equal, pick random
			if( americans > british )
				pThisMenu->m_iTeamSelection = TEAM_BRITISH;
			else if( americans < british )
				pThisMenu->m_iTeamSelection = TEAM_AMERICANS;
			else
				pThisMenu->m_iTeamSelection = random->RandomInt( TEAM_AMERICANS, TEAM_BRITISH );
			break;
	}

	PerformCommand();

	pThisMenu->SetScreen( 2, true, false ); //Don't check teams or class.
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
	vgui::IImage *m_pImage = scheme()->GetImage( "menu/classmenu", false );

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

	if ( !pPlayer || !m_pViewPort )
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
		SetScreen( 1, false );
	}
	/*else if( code == m_iOkayKey )
	{
		if( m_pOK->IsVisible() ) //And fourth.
			m_pOK->OnMousePressed( code2 );
	}*/
	else if( code == teammenu )
	{
		m_pViewPort->ShowPanel( PANEL_COMM, false );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );

		SetScreen( 1, !IsInTeamMenu() );
		
		return;
	}
	else if( code == classmenu )
	{
		
		m_pViewPort->ShowPanel( PANEL_COMM, false );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );
		
		SetScreen( 2, !IsInClassMenu() );
		
		return;
	}
	else if( code == weaponmenu )
	{
		m_pViewPort->ShowPanel( PANEL_COMM, false );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );
		
		SetScreen( 3, !IsInWeaponMenu() );

		return;
	}
	else if( code == commmenu )
	{
		ShowPanel( false ); //This panel.
		m_pViewPort->ShowPanel( PANEL_COMM, true );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );
		
		return;
	}
	else if( code == commmenu2 )
	{
		ShowPanel( false ); //This panel.
		m_pViewPort->ShowPanel( PANEL_COMM, false );
		m_pViewPort->ShowPanel( PANEL_COMM2, true );
		
		return;
	}
	else
		BaseClass::OnKeyCodePressed( code );
}

void CClassMenu::ShowPanel(bool bShow)
{
	if ( engine->IsPlayingDemo() ) //Don't show up in demos -HairyPotter
		return;

	if ( BaseClass::IsVisible() == bShow )
		return;

	//clear html screen
	char pANSIPath[512];

	if ( bShow )
	{
		Activate();
		
		ResolveLocalizedPath( "#BG2_InfoHTML_Blank_Path", pANSIPath, sizeof pANSIPath );

		ShowFile( pANSIPath );

		//BG2 - Tjoppen - show autoassign button if we're not in the class part and we're unassigned or spectator
		m_pAutoassignButton->SetVisible( IsInWeaponMenu() ? false : IsInClassMenu() ? false : C_BasePlayer::GetLocalPlayer() ? C_BasePlayer::GetLocalPlayer()->GetTeamNumber() <= TEAM_SPECTATOR : false );
	}
	else
		SetVisible( false );

	SetMouseInputEnabled( bShow );
	SetKeyBoardInputEnabled( bShow );
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
		SetScreen( 1, false );
		return;
	}

	BaseClass::OnThink();

	//show number of players in teams..
	wchar_t tmp[32];
	if( g_Teams[TEAM_BRITISH] )		//check just in case..
	{
		_snwprintf(tmp, sizeof( tmp ), L"%s (%i)", g_pVGuiLocalize->Find( "#BG2_Spec_British_Score" ), g_Teams[TEAM_BRITISH]->Get_Number_Players() );
		m_pBritishLabel->SetText( tmp );
	}

	if( g_Teams[TEAM_AMERICANS] )	//check just in case..
	{
		_snwprintf(tmp, sizeof( tmp ), L"%s (%i)", g_pVGuiLocalize->Find( "#BG2_Spec_American_Score" ), g_Teams[TEAM_AMERICANS]->Get_Number_Players() );
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
	char temp[32];
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

//Default weapon kits for menus. -HairyPotter
void CClassMenu::SetDefaultWeaponKit( int m_iTeam, int m_iClass, int weapon, int ammo )
{
	char temp[16];
	Q_snprintf( temp, sizeof temp, "%i %i", weapon, ammo );

	switch ( m_iTeam )
	{
		case TEAM_AMERICANS:
			switch( m_iClass )
			{
				case CLASS_INFANTRY:
					cl_kit_a_inf.SetValue( temp );
					break;
				case CLASS_SKIRMISHER:
					cl_kit_a_ski.SetValue( temp );
					break;
			}
			break;
		case TEAM_BRITISH:
			switch( m_iClass )
			{
				case CLASS_INFANTRY:
					cl_kit_b_inf.SetValue( temp );
					break;
				case CLASS_SKIRMISHER:
					cl_kit_b_ski.SetValue( temp );
					break;
				case CLASS_LIGHT_INFANTRY:
					cl_kit_b_light.SetValue( temp );
					break;
			}
		break;
	}
}

void CClassMenu::UpdateDefaultWeaponKit( int m_iTeam, int m_iClass )
{
	int args[2];

	//Just to make sure.
	args[0] = 1;
	args[1] = AMMO_KIT_BALL;
	//

	switch ( m_iTeam )
	{
		case TEAM_AMERICANS:
			switch( m_iClass )
			{
				case CLASS_INFANTRY:
					sscanf( cl_kit_a_inf.GetString(), "%i %i", &(args[0]), &(args[1]) );
					break;
				case CLASS_SKIRMISHER:
					sscanf( cl_kit_a_ski.GetString(), "%i %i", &(args[0]), &(args[1]) );
					break;
			}
			break;

		case TEAM_BRITISH:
			switch( m_iClass )
			{
				case CLASS_INFANTRY:
					sscanf( cl_kit_b_inf.GetString(), "%i %i", &(args[0]), &(args[1]) );
					break;
				case CLASS_SKIRMISHER:
					sscanf( cl_kit_b_ski.GetString(), "%i %i", &(args[0]), &(args[1]) );
					break;
				case CLASS_LIGHT_INFANTRY:
					sscanf( cl_kit_b_light.GetString(), "%i %i", &(args[0]), &(args[1]) );
					break;
			}
			break;
	}

	DefaultKit.m_iGun = args[0];
	DefaultKit.m_iAmmo = args[1];
}
//

bool CClassMenu::SetScreen( int m_iScreen, bool m_bVisible, bool m_bUpdate )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( pPlayer );

	if ( !pPlayer || !pHL2Player )
		return false;

	wchar_t msg[128];

	int iTeam = pPlayer->GetTeamNumber(),
		iClass = pHL2Player->GetClass();

	if ( m_bVisible ) //No sense updating information we won't be able to see anyway.
	{
		switch( m_iScreen )
		{
			case 2:
				if ( m_bUpdate )
				{
					if ( iTeam <= TEAM_SPECTATOR )
					{
						_snwprintf(msg, sizeof( msg ), L"%s", g_pVGuiLocalize->Find( "#BG2_Deny_Class_Selection" ) );
						internalCenterPrint->Print( msg );
						return false;
					}

					m_iTeamSelection = iTeam;
				}
				break;
			case 3:
				if ( m_bUpdate )
				{
					if( iTeam <= TEAM_SPECTATOR || iClass == -1 )
					{
						_snwprintf(msg, sizeof( msg ), L"%s", g_pVGuiLocalize->Find( "#BG2_Deny_Weapon_Selection" ) );
						internalCenterPrint->Print( msg );
						return false;	
					}

					m_iTeamSelection = iTeam;
					m_iClassSelection = iClass;
				}

				if ( m_iClassSelection == CLASS_OFFICER || m_iClassSelection == CLASS_SNIPER )
						return false;

				UpdateDefaultWeaponKit( m_iTeamSelection, m_iClassSelection ); //Make sure we're updated.
				break;
		}
	}

	ShowPanel( m_bVisible );
	ToggleButtons( m_iScreen );
	return true;
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
			//Default Weapon Kits. -HairyPotter
			m_pWeaponButton1->SetSelected( DefaultKit.m_iGun == 1 );
			m_pWeaponButton2->SetSelected( DefaultKit.m_iGun == 2 );
			m_pWeaponButton3->SetSelected( DefaultKit.m_iGun == 3 );
			m_pAmmoButton1->SetSelected( DefaultKit.m_iAmmo == AMMO_KIT_BALL );
			m_pAmmoButton2->SetSelected( DefaultKit.m_iAmmo == AMMO_KIT_BUCKSHOT );
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

void CWeaponButton::GetWeaponImages( char *m_szActiveImage, char *m_szInactiveImage )
{
	int wide, tall;
	vgui::IImage *m_pImage = NULL;

	GetSize( wide, tall );

	if ( m_bMouseOver || IsSelected() )
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
					pButton1->GetWeaponImages( AInf1M, AInf1 );
					pButton2->GetWeaponImages( AInf2M, AInf2 );
					pButton3->GetWeaponImages( AInf3M, AInf3 );
					break;
				case CLASS_SKIRMISHER: 
					pButton1->GetWeaponImages( ASki1M, ASki1 );
					pButton2->GetWeaponImages( ASki2M, ASki2 );
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
					pButton1->GetWeaponImages( BInf1M, BInf1 );
					pButton2->GetWeaponImages( BInf2M, BInf2 );
					pButton3->SetVisible( false );
					break;
				case CLASS_SKIRMISHER:
					pButton1->GetWeaponImages( BSki1M, BSki1 );
					pButton2->GetWeaponImages( BSki2M, BSki2 );
					pButton3->SetVisible( false );
					break;
				case CLASS_LIGHT_INFANTRY:
					pButton1->GetWeaponImages( BLight1M, BLight1 );
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

void COkayButton::OnMousePressed(MouseCode code)
{
	SetSelected( false );
	PerformCommand();
}

void COkayButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBorderEnabled(false); //No borders.
}

void COkayButton::PerformCommand( void )
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	PlaySound("Classmenu.Join");

	int m_iGunType = 1,
		m_iAmmoType = AMMO_KIT_BALL,
		iTeam = pThisMenu->m_iTeamSelection,
		iClass = pThisMenu->m_iClassSelection;

	pThisMenu->SetScreen( 1, false ); //Reset to first screen and make invisible.

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

	pThisMenu->SetDefaultWeaponKit( iTeam, iClass, m_iGunType, m_iAmmoType ); //Since we chose a kit ourselves, save it off.

	char cmd[32];
	Q_snprintf( cmd, sizeof( cmd ), "kit %i %i \n", m_iGunType, m_iAmmoType );
	engine->ServerCmd( cmd );

	char sClass[32];
	Q_snprintf( sClass, sizeof( sClass ), "class %i %i \n", iTeam, iClass );
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