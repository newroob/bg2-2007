#include "KeyValues.h"
#include "FileSystem.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/CheckButton.h>

extern ConVar	cl_crosshair,

				cl_crosshair_r,
				cl_crosshair_g,
				cl_crosshair_b,
				cl_crosshair_a,

				cl_simple_smoke,

				cl_flagstatusdetail,

				cl_hitverif,
				cl_winmusic;


class CBG2OptionsPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CBG2OptionsPanel, vgui::Frame );

	/*MESSAGE_FUNC_PARAMS( RedSliderChanged, "ValueChanged", data );
#define MESSAGE_FUNC_PARAMS( name, scriptname, p1 )*/

	//BG2 - Tjoppen - don't want any virtual crap.. everything in the class. thus the reason for not using
	//				the MESSAGE_FUNC_PARAMS macro
	_MessageFuncCommon( SliderMoved, "SliderMoved", 1, vgui::DATATYPE_KEYVALUES, NULL, 0, 0 );
	_MessageFuncCommon( CheckButtonChecked, "CheckButtonChecked", 1, vgui::DATATYPE_KEYVALUES, NULL, 0, 0 );
	_MessageFuncCommon( Command, "Command", 1, vgui::DATATYPE_KEYVALUES, NULL, 0, 0 );

	/* Write_Cvar
		writes a ConVar* to the specified FileHandle_t on the form "name\tvalue\r\n"
	*/
	void Write_Cvar( ConVar *cvar, FileHandle_t fh )
	{
		const char	*name = cvar->GetName(),
					*value = cvar->GetString();

		filesystem->Write( name, strlen(name), fh );
		filesystem->Write( "\t", 1, fh );
		filesystem->Write( value, strlen(value), fh );
		filesystem->Write( "\r\n", 2, fh );
	}

	/* Command
		a command was sent from a button. since the only button we currently have is the "OK" button, write config
		 and pass command to the baseclass (only "close" seems to be sent, but it doesn't hurt to write config if any
		 other are sent)
	*/
	void Command( KeyValues *data )
	{
		const char *command = data->GetString( "command" );

		FileHandle_t fh = filesystem->Open( "cfg/bg2_options.cfg", "w" );
		if(fh)
		{
			const char *blah = "//Auto-generated config for BG2 client-side cvars. Don\'t touch (it\'ll get rewritten).\r\n";

			filesystem->Write( blah, strlen(blah), fh );

			Write_Cvar( &cl_crosshair, fh );
			Write_Cvar( &cl_crosshair_r, fh );
			Write_Cvar( &cl_crosshair_g, fh );
			Write_Cvar( &cl_crosshair_b, fh );
			Write_Cvar( &cl_crosshair_a, fh );

			Write_Cvar( &cl_simple_smoke, fh );
			Write_Cvar( &cl_flagstatusdetail, fh );
			Write_Cvar( &cl_hitverif, fh );
			Write_Cvar( &cl_winmusic, fh );

			filesystem->Close(fh);
		}

		BaseClass::OnCommand( command );
	}

	/* SliderMoved
		some slider was moved. see which it was and update related cvar
	*/
	void SliderMoved( KeyValues *data )
	{
		vgui::Slider *pSender = (vgui::Slider*)data->GetPtr( "panel" );

		if( !pSender )
			return;

		//determine which
		if( pSender == m_pRedCrosshairSlider )
			cl_crosshair_r.SetValue( data->GetInt("position") );
		else if( pSender == m_pGreenCrosshairSlider )
			cl_crosshair_g.SetValue( data->GetInt("position") );
		else if( pSender == m_pBlueCrosshairSlider )
			cl_crosshair_b.SetValue( data->GetInt("position") );
		else if( pSender == m_pAlphaCrosshairSlider )
			cl_crosshair_a.SetValue( data->GetInt("position") );
	}

	/* CheckButtonChecked
		some checkbutton was checked. see which it was and update related cvar
	*/
	void CheckButtonChecked( KeyValues *data )
	{
		vgui::CheckButton *pSender = (vgui::CheckButton*)data->GetPtr( "panel" );

		if( !pSender )
			return;

		if( pSender == m_pSimpleSmokeCheckButton )
			cl_simple_smoke.SetValue( data->GetInt("state") );
		if( pSender == m_pSimpleFlagHUDCheckButton )
			cl_flagstatusdetail.SetValue( data->GetInt("state") ? 1 : 2 );
		else if( pSender == m_pFlag0CrosshairCheckButton )
			cl_crosshair.SetValue( (cl_crosshair.GetInt() & ~1) | (data->GetInt("state") ? 1 : 0) );
		else if( pSender == m_pFlag1CrosshairCheckButton )
			cl_crosshair.SetValue( (cl_crosshair.GetInt() & ~2) | (data->GetInt("state") ? 2 : 0) );
		else if( pSender == m_pFlag2CrosshairCheckButton )
			cl_crosshair.SetValue( (cl_crosshair.GetInt() & ~4) | (data->GetInt("state") ? 4 : 0) );
		else if( pSender == m_pFlag3CrosshairCheckButton )
			cl_crosshair.SetValue( (cl_crosshair.GetInt() & ~8) | (data->GetInt("state") ? 8 : 0) );
		else if( pSender == m_pHitverifCheckButton )
			cl_hitverif.SetValue( data->GetInt("state") );
		else if( pSender == m_pWinmusicCheckButton )
			cl_winmusic.SetValue( data->GetInt("state") );
	}

	/* SetAdjustors
		sets the various sliders and checkboxes to whatever value they should be
		this function is needed because this panel gets inited before bg2_options.cfg is loaded
	*/
	void SetAdjustors( void )
	{
		m_pRedCrosshairSlider->SetRange( 0, 255 );
		m_pRedCrosshairSlider->SetValue( cl_crosshair_r.GetInt() );

		m_pGreenCrosshairSlider->SetRange( 0, 255 );
		m_pGreenCrosshairSlider->SetValue( cl_crosshair_g.GetInt() );
		
		m_pBlueCrosshairSlider->SetRange( 0, 255 );
		m_pBlueCrosshairSlider->SetValue( cl_crosshair_b.GetInt() );
		
		m_pAlphaCrosshairSlider->SetRange( 0, 255 );
		m_pAlphaCrosshairSlider->SetValue( cl_crosshair_a.GetInt() );
		
		m_pSimpleSmokeCheckButton->SetSelected( cl_simple_smoke.GetBool() );
		m_pSimpleFlagHUDCheckButton->SetSelected( cl_flagstatusdetail.GetInt() == 1 ? true : false );
		
		m_pFlag0CrosshairCheckButton->SetSelected( (cl_crosshair.GetInt() & 1) ? true : false );
		m_pFlag1CrosshairCheckButton->SetSelected( (cl_crosshair.GetInt() & 2) ? true : false );
		m_pFlag2CrosshairCheckButton->SetSelected( (cl_crosshair.GetInt() & 4) ? true : false );
		m_pFlag3CrosshairCheckButton->SetSelected( (cl_crosshair.GetInt() & 8) ? true : false );

		m_pHitverifCheckButton->SetSelected( cl_hitverif.GetBool() );
		m_pWinmusicCheckButton->SetSelected( cl_winmusic.GetBool() );
		
	}

	/* CBG2OptionsPanel
		constructor..
	*/
	CBG2OptionsPanel( vgui::VPANEL parent ) : BaseClass( NULL, "BG2OptionsPanel" )
	{
		SetParent( parent );

		vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SourceScheme.res", "SourceScheme" );
		SetScheme( scheme );
		
		SetVisible( false );

		m_pRedCrosshairSlider = new vgui::Slider( this, "RedCrosshairSlider" );
		m_pRedCrosshairSlider->AddActionSignalTarget( this );

		m_pGreenCrosshairSlider = new vgui::Slider( this, "GreenCrosshairSlider" );
		m_pGreenCrosshairSlider->AddActionSignalTarget( this );

		m_pBlueCrosshairSlider = new vgui::Slider( this, "BlueCrosshairSlider" );
		m_pBlueCrosshairSlider->AddActionSignalTarget( this );

		m_pAlphaCrosshairSlider = new vgui::Slider( this, "AlphaCrosshairSlider" );
		m_pAlphaCrosshairSlider->AddActionSignalTarget( this );

		m_pSimpleSmokeCheckButton = new vgui::CheckButton( this, "SimpleSmokeCheckButton", "" );
		m_pSimpleSmokeCheckButton->AddActionSignalTarget( this );

		m_pSimpleFlagHUDCheckButton = new vgui::CheckButton( this, "SimpleFlagHUDCheckButton", "" );
		m_pSimpleFlagHUDCheckButton->AddActionSignalTarget( this );

		m_pFlag0CrosshairCheckButton = new vgui::CheckButton( this, "Flag0CrosshairCheckButton", "" );
		m_pFlag0CrosshairCheckButton->AddActionSignalTarget( this );

		m_pFlag1CrosshairCheckButton = new vgui::CheckButton( this, "Flag1CrosshairCheckButton", "" );
		m_pFlag1CrosshairCheckButton->AddActionSignalTarget( this );

		m_pFlag2CrosshairCheckButton = new vgui::CheckButton( this, "Flag2CrosshairCheckButton", "" );
		m_pFlag2CrosshairCheckButton->AddActionSignalTarget( this );

		m_pFlag3CrosshairCheckButton = new vgui::CheckButton( this, "Flag3CrosshairCheckButton", "" );
		m_pFlag3CrosshairCheckButton->AddActionSignalTarget( this );

		m_pHitverifCheckButton = new vgui::CheckButton( this, "HitverifCheckButton", "" );
		m_pHitverifCheckButton->AddActionSignalTarget( this );

		m_pWinmusicCheckButton = new vgui::CheckButton( this, "WinmusicCheckButton", "" );
		m_pWinmusicCheckButton->AddActionSignalTarget( this );

		//set sliders and checkboxes correctly
		SetAdjustors();

		LoadControlSettings( "resource/ui/BG2OptionsPanel.res" );
	}

	/* OnSetFocus
		called whenever this menu is brough up. makes sure all sliders and checkboxes are valid
	*/
	void OnSetFocus( void )
	{
		//when we're visible, make sure sliders and checkboxes are set correctly
		SetAdjustors();
	}

	/* ~CBG2OptionsPanel
		destructor
	*/
	~CBG2OptionsPanel()
	{
	}

	vgui::Slider	*m_pRedCrosshairSlider,
					*m_pGreenCrosshairSlider,
					*m_pBlueCrosshairSlider,
					*m_pAlphaCrosshairSlider;

	vgui::CheckButton	*m_pSimpleSmokeCheckButton,
						*m_pSimpleFlagHUDCheckButton,

						*m_pFlag0CrosshairCheckButton,
						*m_pFlag1CrosshairCheckButton,
						*m_pFlag2CrosshairCheckButton,
						*m_pFlag3CrosshairCheckButton,

						*m_pHitverifCheckButton,
						*m_pWinmusicCheckButton;

protected:
	//virtual void OnCommand(const char *pCommand);
	/*void OnThink( void )
	{
	}*/
};

extern CBG2OptionsPanel *bg2options;