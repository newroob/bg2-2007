//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CLASSMENU_H
#define CLASSMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui/keycode.h>
#include <vgui/MouseCode.h>
#include <vgui_controls/HTML.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/CheckButton.h>
#include "vgui_BitmapImage.h"
#include "vgui_bitmapbutton.h"
#include <vgui_controls/ImagePanel.h>
#include <imagemouseoverbutton.h>

#include <game/client/iviewport.h>

class KeyValues;

namespace vgui
{
	class TextEntry;
	class Button;
	class Panel;
	class ImagePanel;
	class ComboBox;
}

#define BLACK_BAR_COLOR	Color(0, 0, 0, 196)

class IBaseFileSystem;

//-----------------------------------------------------------------------------
// Purpose: the bottom bar panel, this is a seperate panel because it
// wants mouse input and the main window doesn't
//----------------------------------------------------------------------------
class CClassButton : public vgui::Button
{
public:
	DECLARE_CLASS_SIMPLE( CClassButton, vgui::Button);

	CClassButton(Panel *parent, const char *panelName, const char *text) : Button( parent, panelName, text ) { m_iCommand = 0; }

	void SetCommand( int command );
	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void );
	void PerformCommand( void );
	//Added functionality for images as buttons. -HairyPotter
	virtual void Paint();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void ApplySettings( KeyValues *inResourceData )
	{
		// Active Image
		delete [] BritishImage,
				  AmericanImage,
				  temp;

		BritishImage = AmericanImage = temp = NULL;
		int len;

		temp = inResourceData->GetString( "BritishImage", "" );
		len = Q_strlen( temp ) + 1;
		BritishImage = new char[ len ];
		Q_strncpy( (char *)BritishImage, temp, len );

		temp = inResourceData->GetString( "AmericanImage", "" );
		len = Q_strlen( temp ) + 1;
		AmericanImage = new char[ len ];
		Q_strncpy( (char *)AmericanImage, temp, len );
		
		BaseClass::ApplySettings( inResourceData );
	}
	//

private:
	int m_iCommand;

	const char *BritishImage,
			*AmericanImage,
			*temp;
};

class CTeamButton : public vgui::Button
{
public:
	DECLARE_CLASS_SIMPLE( CTeamButton, vgui::Button);

	CTeamButton(Panel *parent, const char *panelName, const char *text) : Button( parent, panelName, text ) { m_iCommand = 0; }

	void SetCommand( int command );
	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void );
	void PerformCommand( void );
	//Added functionality for images as buttons. -HairyPotter
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Paint( void );
	virtual void ApplySettings( KeyValues *inResourceData )
	{
		// Active Image
		delete [] ImageName;

		ImageName = NULL;

		temp = inResourceData->GetString( "Image", "" );
		int len = Q_strlen( temp ) + 1;
		ImageName = new char[ len ];
		Q_strncpy( (char *)ImageName, temp, len );

		BaseClass::ApplySettings( inResourceData );
	}
	//

private:
	int m_iCommand;

	const char *ImageName,
			   *temp;

};

class COkayButton : public vgui::Button
{
public:
	DECLARE_CLASS_SIMPLE( COkayButton, vgui::Button );

	COkayButton(Panel *parent, const char *panelName, const char *text) : Button( parent, panelName, text ) { }

	void SetCommand( int command, bool ammo = false );
	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void );
	void PerformCommand( void );
};

class CWeaponButton : public vgui::ToggleButton
{
public:
	DECLARE_CLASS_SIMPLE( CWeaponButton, vgui::ToggleButton );

	CWeaponButton(Panel *parent, const char *panelName, const char *text) : ToggleButton( parent, panelName, text ) {  }

	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void );
};

class CAmmoButton : public vgui::ToggleButton
{
public:
	DECLARE_CLASS_SIMPLE( CAmmoButton, vgui::ToggleButton );

	CAmmoButton(Panel *parent, const char *panelName, const char *text) : ToggleButton( parent, panelName, text ) {  }

	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void );
};

class CClassMenu : public vgui::Frame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE( CClassMenu, vgui::Frame );

public:
	CClassMenu( IViewPort *pViewPort );
	~CClassMenu();

	virtual const char *GetName( void ) { return PANEL_CLASSES; }
	virtual void SetData(KeyValues *data);
	virtual void Reset( void ) { }//m_pPlayerList->DeleteAllItems(); }
	virtual void Update( void );
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );


	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

	CClassButton	*m_pInfantryButton,
					*m_pOfficerButton,
					*m_pSkirmisherButton,
					*m_pSniperButton,
					*m_pLightInfantryButton;

	CTeamButton		*m_pBritishButton,
					*m_pAmericanButton,
					*m_pSpectateButton,
					*m_pAutoassignButton;

	CWeaponButton	*m_pWeaponButton1,
					*m_pWeaponButton2,
					*m_pWeaponButton3;

	CAmmoButton		*m_pAmmoButton1,
					*m_pAmmoButton2,
					*m_pAmmoButton3;

	vgui::ImagePanel *m_pWeaponSelection,
					 *m_pAmmoSelection,
					 *m_pBackground;

	vgui::Label *m_pBritishLabel,
				*m_pAmericanLabel,
				*m_pInfantryLabel,
				*m_pOfficerLabel,
				*m_pRiflemanLabel,
				*m_pSkirmisherLabel,
				*m_pLightInfantryLabel;

	vgui::CheckButton		*m_pQuickJoinCheckButton;

	COkayButton		*m_pOK;

	vgui::HTML		*m_pInfoHTML;

	int m_iTeamSelection,
		m_iClassSelection;

	void OnThink();
	void UpdateClassLabelText( vgui::Label *pLabel, int iClass);
	void UpdateAmmoButtonText( void );
	void UpdateWeaponButtonText( void );
	void ToggleButtons(int iShowScreen);

	//being in classmenu mode means we must be visible aswell
	//Kind of a hack here with the weapon kit selection. I mean.. there's always going to be at least 1 ammo selection.. so yeah..
	bool IsInClassMenu( void ) { return m_pInfantryButton->IsVisible() && IsVisible() || m_pAmmoButton1->IsVisible(); }

	void ShowFile( const char *filename );

private:
	// VGUI2 overrides
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();

	void SetViewModeText( const char *text ) { }//m_pViewOptions->SetText( text ); }

	void SetPlayerFgColor( Color c1 ) { }//m_pPlayerList->SetFgColor(c1); }

	IViewPort *m_pViewPort;

	int				m_iInfantryKey,
					m_iOfficerKey,
					m_iSniperKey,
					m_iSkirmisherKey,
					m_iLightInfantryKey,
					m_iSlot6Key,
					m_iOkayKey;

	CClassButton	*m_pCancelButton;
	int				m_iCancelKey,

					teammenu,
					classmenu,
					weaponmenu,
					commmenu,
					commmenu2;
public:
};

#endif // CLASSMENU_H
