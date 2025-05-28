//==========================================================================//
//
// Purpose: VGUI Keypad made by Maestro Fénix 2015
//
// $NoKeywords: $
//===========================================================================//

using namespace vgui;

#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls\Panel.h>
#include <vgui\ISurface.h> 
#include "usermessages.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "engine/IEngineSound.h"

class CVgui_HorseSelect : public vgui::Frame, public CHudElement
{
	DECLARE_CLASS(CVgui_HorseSelect, vgui::Frame);

public:

	CVgui_HorseSelect(const char *pElementName);
	~CVgui_HorseSelect(){};

	void CheckPass(int number);
	void OnTick();
	void Init();
	void Reset();
	void MsgFunc_OpenHorseSelect(bf_read &msg);
	void MsgFunc_ToggleHorseSelect(bf_read& msg);

protected:

	void PaintBackground();
	void Paint();
	virtual void OnCommand(const char* pcCommand);
	virtual void OnThink(void);

private:

	Button		*m_pReadyButton;
	Button		*m_pClearButton;
	Button		*m_pHorseButtons[100];

	Label		*m_pScreenTitle;
	Label		*m_pLabel;

	ImagePanel	*m_pBackgroundPanel;

	char		szEnteredNumbers[32];
	char		szFrameLabel[32];
	char		szCode[32];

	bool		m_pHidePass;
	bool		m_showSelect;
	bool		m_bSelectedHorses[100];

	int			m_nHorses;

	const char*	m_szHorseNames[100];
	const char*	m_szHorseInternalNames[100];
};