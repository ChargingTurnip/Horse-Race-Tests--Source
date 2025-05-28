//==========================================================================//
//
// Purpose: VGUI Keypad made by Maestro Fénix 2015
//
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include "vgui_horseselect.h"

#include "KeyValues.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CVgui_HorseSelect);
DECLARE_HUD_MESSAGE(CVgui_HorseSelect, OpenHorseSelect)
DECLARE_HUD_MESSAGE(CVgui_HorseSelect, ToggleHorseSelect)

CVgui_HorseSelect::CVgui_HorseSelect(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "Vgui_HorseSelect")
{	
	m_showSelect = false;

	vgui::Panel *parent = g_pClientMode->GetViewport();
	SetParent(parent);

	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetSize(300, 600);

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("resource/ui/keypad.res");

	//m_pScreenTitle->SetText(szFrameLabel);

	// Select background
	m_pBackgroundPanel = FindControl<ImagePanel>("SelectBackground", true);
	m_pBackgroundPanel->SetImage(scheme()->GetImage("keypad/keypad_base", false));
	m_pBackgroundPanel->SetSize(512, 1024);

	// Ready Button
	m_pReadyButton = FindControl<Button>("ButtonReady", true);
	m_pReadyButton->SetDepressedSound("buttons/blip1.wav");
	m_pReadyButton->SetSize(64, 64);
	m_pReadyButton->SetText("Ready");
	m_pReadyButton->SetBgColor(Color(128, 128, 128, 255));
	m_pReadyButton->SetAlpha(255);

	// Clear Button
	m_pClearButton = FindControl<Button>("ButtonClear", true);
	m_pClearButton->SetDepressedSound("buttons/blip1.wav");
	m_pClearButton->SetSize(64, 64);
	m_pClearButton->SetText("Clear");
	m_pClearButton->SetBgColor(Color(128, 128, 128, 255));
	m_pClearButton->SetAlpha(255);

	m_pScreenTitle = FindControl<Label>("SelectLabelTitle", true);
	m_pScreenTitle->SetVisible(true);
	IScheme* pScheme = scheme()->GetIScheme(GetScheme());
	HFont font = pScheme->GetFont("CreditsText");
	font = pScheme->GetFont("CreditsText");
	m_pScreenTitle->SetFont(font);
	m_pScreenTitle->SetWide(512);
	m_pScreenTitle->SetText("Select Your Horses");

	m_nHorses = 0;
	KeyValues* pPresetDataFile = new KeyValues("HorsePresetData");
	if (pPresetDataFile)
	{
		if (pPresetDataFile->LoadFromFile(g_pFullFileSystem, "scripts/horsepresets.txt"))
		{
			for (int i = 0; i < 100; i++)
			{
				//Msg("%s\n", pPresetDataFile->GetString(VarArgs("horseindex.%d", i), "null1234567890"));
				if (Q_stricmp(pPresetDataFile->GetString(VarArgs("horseindex.%d", i), "null1234567890"), "null1234567890") != 0)
				{
					m_nHorses++;
					m_szHorseNames[i] = pPresetDataFile->GetString(VarArgs("horse.%s.name", pPresetDataFile->GetString(VarArgs("horseindex.%d", i), "")), "No Name");
					m_szHorseInternalNames[i] = pPresetDataFile->GetString(VarArgs("horseindex.%d", i), "");
				}
				else
					break;
			}
		}
	}

	// Horse Buttons
	for (int i = 0; i < m_nHorses; i++)
	{
		vgui::Button *pButton = new Button(this, NULL, m_szHorseNames[i], this, VarArgs("%d", i));
		pButton->SetZPos(i + 1);
		pButton->SetPos(105, 170 + (70 * i));
		pButton->SetSize(150, 64);
		pButton->SetAlpha(255);
		pButton->SetDepressedSound("buttons/blip1.wav");
		pButton->SetTabPosition(i + 1);
		m_pHorseButtons[i] = pButton;
	}
}

void CVgui_HorseSelect::Init()
{
	HOOK_HUD_MESSAGE(CVgui_HorseSelect, OpenHorseSelect);
	HOOK_HUD_MESSAGE(CVgui_HorseSelect, ToggleHorseSelect);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVgui_HorseSelect::Reset()
{
	for (int i = 0; i < 100; i++)
	{
		m_bSelectedHorses[i] = false;
	}
}

void CVgui_HorseSelect::PaintBackground()
{
	//vgui::surface()->DrawSetColor(100, 100, 100, 255);
}

void CVgui_HorseSelect::Paint()
{
	for (int i = 0; i < m_nHorses; i++)
	{
		if (m_pHorseButtons[i]->IsArmed())
		{
			m_pHorseButtons[i]->SetAlpha(255);
		}
		else
		{
			m_pHorseButtons[i]->SetAlpha(150);
		}

		if (m_bSelectedHorses[i])
			m_pHorseButtons[i]->SetBgColor(Color(128, 128, 0, 255));
		else
			m_pHorseButtons[i]->SetBgColor(Color(128, 128, 128, 255));
	}

	if (m_pReadyButton->IsArmed())
		m_pReadyButton->SetAlpha(255);
	else
		m_pReadyButton->SetAlpha(150);

	if (m_pClearButton->IsArmed())
		m_pClearButton->SetAlpha(255);
	else
		m_pClearButton->SetAlpha(150);
}

void CVgui_HorseSelect::OnTick()
{
	BaseClass::OnTick();
}

void CVgui_HorseSelect::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);

	if (!Q_stricmp(pcCommand, "clear"))
	{
		for (int i = 0; i < m_nHorses; i++)
		{
			m_bSelectedHorses[i] = false;

			engine->ClientCmd("clearhorses");
		}
	}
		
	if (!Q_stricmp(pcCommand, "ready"))
	{ 
		m_showSelect = false;

		engine->ClientCmd("readyhorses");
	}

	for (int i = 0; i < m_nHorses; i++)
	{
		if (!Q_stricmp(pcCommand, VarArgs("%d", i)))
		{
			m_bSelectedHorses[i] = !m_bSelectedHorses[i];

			engine->ClientCmd(VarArgs("selecthorse %s", m_szHorseInternalNames[i]));
		}
	}
}

void CVgui_HorseSelect::CheckPass(int number)
{
	char numberN[8];
	sprintf(numberN, "%i", number);

	m_pLabel->SetVisible(true);

	m_pLabel->SetText(szEnteredNumbers);
}

void CVgui_HorseSelect::MsgFunc_OpenHorseSelect(bf_read &msg)
{
	m_showSelect = true;
}

void CVgui_HorseSelect::OnThink(void)
{
	if (m_showSelect)
	{
		SetVisible(true);
		SetMouseInputEnabled(true);

		SetKeyBoardInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
		
		SetKeyBoardInputEnabled(false);
	}

	BaseClass::OnThink();
}

void CVgui_HorseSelect::MsgFunc_ToggleHorseSelect(bf_read& msg)
{
	m_showSelect = !m_showSelect;
}