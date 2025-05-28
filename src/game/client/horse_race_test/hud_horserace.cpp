//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"
#include "c_basehlplayer.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HUD_MATERIAL_NUMBERS		"vgui/icons/icon_numbers"
#define HUD_MATERIAL_WINBG			"vgui/icons/icon_winbg"

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHorseRace : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudHorseRace, vgui::Panel);

public:
	CHudHorseRace(const char* pElementName);
	virtual ~CHudHorseRace(void);
	void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void OnThink();
	void MsgFunc_HRActivateElement(bf_read& msg);
	void MsgFunc_HRSetWinHorse(bf_read& msg);
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

	void	DrawTextOutline(int x, int y, wchar_t* string, int thickness, int iterations, int modifyLen = 0);

protected:
	virtual void Paint();
	virtual void PaintBackground();

private:

	int		m_nCurrentNumber;

	float	m_flNumberStartInTime;
	float	m_flNumberEndInTime;
	float	m_flNumberStartOutTime;
	float	m_flNumberEndOutTime;
	float	m_flNumberStartOutTime2;

	float	m_flTimerStartTime;
	float	m_flTimerEndFadeTime;

	float	m_flTimerStartOutTime;
	float	m_flTimerEndOutTime;

	float	m_flBGStartFadeTime;
	float	m_flBGEndFadeTime;
	float	m_flBGStartFadeTime2;
	float	m_flBGEndFadeTime2;

	bool	m_bFinished;
	bool	m_bVisible;
	bool	m_bSetHorseGraphic;

	int		m_textureID_IconNumbers;
	int		m_textureID_IconWinBg;
	int		m_textureID_IconWinHorse;

	char	m_szWinningHorseGraphic[MAX_PATH];
	bool	m_bWinHorseReady;

	float	m_flCurrentTime;

	Color	m_cWinHorseColor;

	char	m_szWinningHorseName[MAX_PATH];

	HFont	winFont;
	HFont	statFont;

	wchar_t m_szCurrentTime[100];

	bool	m_bNumbersOver;

	//char pleaseStopCrashing[100];
};

DECLARE_HUDELEMENT(CHudHorseRace);
DECLARE_HUD_MESSAGE(CHudHorseRace, HRActivateElement );
DECLARE_HUD_MESSAGE(CHudHorseRace, HRSetWinHorse);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHorseRace::CHudHorseRace(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudHorseRace")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetProportional(true);

	m_nCurrentNumber = 0;

	m_flNumberStartInTime = -1.0f;
	m_flNumberEndInTime = -1.0f;
	m_flNumberStartOutTime = -1.0f;
	m_flNumberEndOutTime = -1.0f;
	m_flNumberStartOutTime2 = -1.0f;

	m_flTimerStartTime = -1.0f;
	m_flTimerEndFadeTime = -1.0f;

	m_flTimerStartOutTime = -1.0f;
	m_flTimerEndOutTime = -1.0f;

	m_flBGStartFadeTime = -1.0f;
	m_flBGEndFadeTime = -1.0f;

	m_flBGStartFadeTime2 = -1.0f;
	m_flBGEndFadeTime2 = -1.0f;

	m_bFinished = false;
	m_bVisible = false;
	m_bWinHorseReady = false;
	m_bSetHorseGraphic = false;

	m_flCurrentTime = 0.0f;

	m_textureID_IconNumbers = -1;
	m_textureID_IconWinBg = -1;
	m_textureID_IconWinHorse = -1;

	m_cWinHorseColor = Color(255, 255, 255, 255);

	IScheme* pScheme = scheme()->GetIScheme(GetScheme());
	winFont = pScheme->GetFont("HorseName");
	statFont = pScheme->GetFont("HorseStats");

	m_bNumbersOver = false;

	SetHiddenBits(HIDEHUD_PLAYERDEAD);
}

CHudHorseRace::~CHudHorseRace(void)
{
	if (vgui::surface())
	{
		if (m_textureID_IconNumbers != -1)
		{
			vgui::surface()->DestroyTextureID(m_textureID_IconNumbers);
			m_textureID_IconNumbers = -1;
		}
		if (m_textureID_IconWinBg != -1)
		{
			vgui::surface()->DestroyTextureID(m_textureID_IconWinBg);
			m_textureID_IconWinBg = -1;
		}
		if (m_textureID_IconWinHorse != -1)
		{
			vgui::surface()->DestroyTextureID(m_textureID_IconWinHorse);
			m_textureID_IconWinHorse = -1;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHorseRace::Init(void)
{
	HOOK_HUD_MESSAGE(CHudHorseRace, HRActivateElement);
	HOOK_HUD_MESSAGE(CHudHorseRace, HRSetWinHorse);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pScheme - 
//-----------------------------------------------------------------------------
void CHudHorseRace::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	int iWide, iTall;
	surface()->GetScreenSize(iWide, iTall);
	SetBounds(0, 0, iWide, iTall);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHorseRace::Reset()
{
	m_nCurrentNumber = 0;

	m_flNumberStartInTime = -1.0f;
	m_flNumberEndInTime = -1.0f;
	m_flNumberStartOutTime = -1.0f;
	m_flNumberEndOutTime = -1.0f;
	m_flNumberStartOutTime2 = -1.0f;

	m_flTimerStartTime = -1.0f;
	m_flTimerEndFadeTime = -1.0f;

	m_flTimerStartOutTime = -1.0f;
	m_flTimerEndOutTime = -1.0f;

	m_flBGStartFadeTime = -1.0f;
	m_flBGEndFadeTime = -1.0f;

	m_flBGStartFadeTime2 = -1.0f;
	m_flBGEndFadeTime2 = -1.0f;

	m_bFinished = false;
	m_bVisible = false;
	m_bWinHorseReady = false;
	m_bSetHorseGraphic = false;

	m_flCurrentTime = 0.0f;

	m_textureID_IconNumbers = -1;
	m_textureID_IconWinBg = -1;
	m_textureID_IconWinHorse = -1;

	m_cWinHorseColor = Color(255, 255, 255, 255);

	IScheme* pScheme = scheme()->GetIScheme(GetScheme());
	winFont = pScheme->GetFont("HorseName");
	statFont = pScheme->GetFont("HorseStats");

	m_bNumbersOver = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHorseRace::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHorseRace::OnThink()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHorseRace::MsgFunc_HRActivateElement(bf_read& msg)
{
	int element = msg.ReadShort();

	m_bVisible = true;

	if (element < 12)
	{
		m_nCurrentNumber = element;

		m_flNumberStartInTime = gpGlobals->curtime;
		m_flNumberEndInTime = gpGlobals->curtime + 0.2f;

		float dur = msg.ReadFloat();
		m_flNumberStartOutTime = m_flNumberEndInTime + dur;
		m_flNumberEndOutTime = m_flNumberStartOutTime + 0.2f;

		//m_flNumberStartOutTime2 = m_flNumberEndInTime + (dur * 0.8f);
		m_flNumberStartOutTime2 = m_flNumberStartOutTime;
	}

	if (element == 12)
	{
		m_bNumbersOver = true;

		m_flTimerStartTime = gpGlobals->curtime;
		m_flTimerEndFadeTime = gpGlobals->curtime + 1.0f;
	}

	if (element == 13)
	{
		m_bFinished = true;

		m_flTimerStartOutTime = gpGlobals->curtime;
		m_flTimerEndOutTime = gpGlobals->curtime + 1.0f;
	}

	if (element == 14)
	{
		m_flBGStartFadeTime = gpGlobals->curtime;
		m_flBGEndFadeTime = gpGlobals->curtime + 1.0f;

		m_flBGStartFadeTime2 = gpGlobals->curtime + 2.5f;
		m_flBGEndFadeTime2 = gpGlobals->curtime + 3.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHorseRace::MsgFunc_HRSetWinHorse(bf_read& msg)
{
	msg.ReadString(m_szWinningHorseGraphic, sizeof(m_szWinningHorseGraphic));
	msg.ReadString(m_szWinningHorseName, sizeof(m_szWinningHorseName));

	int r = msg.ReadShort();
	int g = msg.ReadShort();
	int b = msg.ReadShort();
	m_cWinHorseColor.SetColor(r, g, b);

	m_bWinHorseReady = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHorseRace::PaintBackground()
{
	// dont
}

void CHudHorseRace::Paint()
{
	if (!m_bVisible)
		return;

	// this stops it from crashing when you unpause for somereason
	if (engine->IsPaused())
		return;

	if (m_textureID_IconNumbers == -1)
	{
		m_textureID_IconNumbers = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_textureID_IconNumbers, HUD_MATERIAL_NUMBERS, true, false);
	}
	if (m_textureID_IconWinBg == -1)
	{
		m_textureID_IconWinBg = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_textureID_IconWinBg, HUD_MATERIAL_WINBG, true, false);
	}
	if (m_textureID_IconWinHorse == -1)
	{
		m_textureID_IconWinHorse = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_textureID_IconWinHorse, "vgui/icons/horses/icon_win_basic", true, false);
	}

	if (m_bWinHorseReady && !m_bSetHorseGraphic)
	{
		//Msg("%s\n", m_szWinningHorseGraphic);
		vgui::surface()->DrawSetTextureFile(m_textureID_IconWinHorse, m_szWinningHorseGraphic, true, true);
		m_bSetHorseGraphic = true;
	}

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	int wide, tall;
	int x, y;

	GetBounds(x, y, wide, tall);

	int fixedWide = clamp(wide, 0, 1920);

	float scale = 1.0f;
	float scale2 = 1.0f;
	int alpha = 0;
	int alpha2 = 0;

	int shakeX = random->RandomInt(-5, 5);
	int shakeY = random->RandomInt(-5, 5);
	float shakeMult = 0.0f;

	if (m_flNumberStartInTime > -1.0f && gpGlobals->curtime >= m_flNumberStartInTime)
	{
		float deltaTime = MIN(gpGlobals->curtime, m_flNumberEndInTime);
		scale = FLerp(10, 1, m_flNumberStartInTime, m_flNumberEndInTime, deltaTime);
		alpha = FLerp(0, 255, m_flNumberStartInTime, m_flNumberEndInTime, deltaTime);
	}
	if (m_flNumberEndInTime > -1.0f && gpGlobals->curtime >= m_flNumberEndInTime)
	{
		float deltaTime = MIN(gpGlobals->curtime, m_flNumberStartOutTime);
		shakeMult = FLerp(1, 0, m_flNumberEndInTime, m_flNumberStartOutTime, deltaTime);

		float deltaTime2 = MIN(gpGlobals->curtime, m_flNumberEndOutTime);
		scale = FLerp(1, 1.2, m_flNumberEndInTime, m_flNumberEndOutTime, deltaTime2);

		float deltaTime3 = MIN(gpGlobals->curtime, m_flNumberStartOutTime2);
		scale2 = FLerp(1, 2.5, m_flNumberEndInTime, m_flNumberStartOutTime2, deltaTime3);
		alpha2 = FLerp(60, 0, m_flNumberEndInTime, m_flNumberStartOutTime2, deltaTime3);
	}
	if (m_flNumberStartOutTime > -1.0f && gpGlobals->curtime >= m_flNumberStartOutTime)
	{
		float deltaTime = MIN(gpGlobals->curtime, m_flNumberEndOutTime);
		alpha = FLerp(255, 0, m_flNumberStartOutTime, m_flNumberEndOutTime, deltaTime);
	}

	unsigned int bruh[5];

	shakeX *= shakeMult;
	shakeY *= shakeMult;

	int pos3 = ((fixedWide / 3) * scale2);
	int pos2 = ((fixedWide / 3) * scale);


	int xPos = x + (wide / 2);
	int yPos = y + (tall / 2);

	//Msg("%d\n", alpha);

	//if (alpha2 == 0.0f && alpha == 0.0f && m_bNumbersOver)
	//{
	//}
	//else
	//{
		vgui::surface()->DrawSetTexture(m_textureID_IconNumbers);
		vgui::surface()->DrawSetTextureFrame(m_textureID_IconNumbers, m_nCurrentNumber, bruh);
		vgui::surface()->DrawSetColor(Color(255, 255, 255, alpha2));
		vgui::surface()->DrawTexturedRect(xPos - pos3, yPos - pos3, xPos + pos3, yPos + pos3);

		vgui::surface()->DrawSetColor(Color(255, 255, 255, alpha));
		vgui::surface()->DrawTexturedRect(xPos - pos2 + shakeX, yPos - pos2 + shakeY, xPos + pos2 + shakeX, yPos + pos2 + shakeY);
	//}

	int timerAlpha = 0;

	if (m_flTimerStartTime != -1.0f)
	{
		float deltaTime = MIN(gpGlobals->curtime, m_flTimerEndFadeTime);

		timerAlpha = FLerp(0, 255, m_flTimerStartTime, m_flTimerEndFadeTime, deltaTime);
	}

	if (m_flTimerStartOutTime != -1.0f)
	{
		float deltaTime = MIN(gpGlobals->curtime, m_flTimerEndOutTime);

		timerAlpha = FLerp(255, 0, m_flTimerStartOutTime, m_flTimerEndOutTime, deltaTime);
	}

	if (!m_bFinished)
		m_flCurrentTime = gpGlobals->curtime;

	int shadowOffset = 8;

	// shadow
	vgui::surface()->DrawSetTextColor(Color(255, 0, 0, timerAlpha));
	vgui::surface()->DrawSetTextFont(statFont);

	char buf[100];
	wchar_t tempString[100];

	Q_snprintf(buf, sizeof(buf), "%s", VarArgs("%02d:%02d:%02d", (int)floor((m_flCurrentTime - m_flTimerStartTime) / 60), (int)floor((m_flCurrentTime - m_flTimerStartTime)) % 60), (int)((m_flCurrentTime - m_flTimerStartTime) * 60) % 60);
	g_pVGuiLocalize->ConvertANSIToUnicode(buf, tempString, sizeof(tempString));

	V_wcsncpy(m_szCurrentTime, tempString, sizeof(m_szCurrentTime));

	DrawTextOutline(x + 25 + shadowOffset, y + tall - (100 - shadowOffset), tempString, 5, 24);
	vgui::surface()->DrawSetTextPos(x + 25 + shadowOffset, y + tall - (100 - shadowOffset));

	if (tempString)
		vgui::surface()->DrawPrintText(tempString, wcslen(tempString));

	// not shadow
	DrawTextOutline(x + 25, y + tall - 100, tempString, 5, 24);
	vgui::surface()->DrawSetTextPos(x + 25, y + tall - 100);
	vgui::surface()->DrawSetTextColor(Color(240, 240, 240, timerAlpha));

	if (tempString)
		vgui::surface()->DrawPrintText(tempString, wcslen(tempString));

	Q_snprintf(buf, sizeof(buf), "%s", StringAfterPrefix(engine->GetLevelName(), "maps/"));
	g_pVGuiLocalize->ConvertANSIToUnicode(buf, tempString, sizeof(tempString));

	//shadow
	vgui::surface()->DrawSetTextColor(Color(255, 0, 0, timerAlpha));
	DrawTextOutline(x + 25 + shadowOffset, y + 25 + shadowOffset, tempString, 5, 24, 4);
	vgui::surface()->DrawSetTextPos(x + 25 + shadowOffset, y + 25 + shadowOffset);
	if (tempString)
		vgui::surface()->DrawPrintText(tempString, wcslen(tempString) - 4);

	//not shadow
	DrawTextOutline(x + 25, y + 25, tempString, 5, 24, 4);
	vgui::surface()->DrawSetTextPos(x + 25, y + 25);
	vgui::surface()->DrawSetTextColor(Color(240, 240, 240, timerAlpha));
	if (tempString)
		vgui::surface()->DrawPrintText(tempString, wcslen(tempString) - 4);


	if (m_flBGStartFadeTime != -1.0f)
	{
		float deltaTime = MIN(gpGlobals->curtime, m_flBGEndFadeTime);

		int flashAlpha = 0;

		flashAlpha = FLerp(255, 0, m_flBGStartFadeTime, m_flBGEndFadeTime, deltaTime);

		vgui::surface()->DrawSetTexture(m_textureID_IconWinBg);
		vgui::surface()->DrawSetColor(Color(255, 255, 255, 255));
		vgui::surface()->DrawTexturedRect(x, y, wide, tall);

		vgui::surface()->DrawSetTexture(m_textureID_IconWinHorse);
		Color bruh;
		bruh.SetColor(m_cWinHorseColor.r(), m_cWinHorseColor.g(), m_cWinHorseColor.b(), 255);
		vgui::surface()->DrawSetColor(bruh);
		vgui::surface()->DrawTexturedRect(x, y, wide, tall);

		vgui::surface()->DrawSetColor(Color(255, 255, 255, flashAlpha));
		vgui::surface()->DrawFilledRect(x, y, wide, tall);
	}

	if (m_flBGStartFadeTime2 != -1.0f && gpGlobals->curtime >= m_flBGStartFadeTime2)
	{
		float deltaTime = MIN(gpGlobals->curtime, m_flBGEndFadeTime2);

		int nameAlpha = 0;

		nameAlpha = FLerp(0, 255, m_flBGStartFadeTime2, m_flBGEndFadeTime2, deltaTime);

		int shakeX = random->RandomInt(-3, 3);
		int shakeY = random->RandomInt(-3, 3);

		Q_snprintf(buf, sizeof(buf), "%s", m_szWinningHorseName);

		wchar_t tempString[100];
		g_pVGuiLocalize->ConvertANSIToUnicode(buf, tempString, sizeof(tempString));

		Vector horseColor = Vector(m_cWinHorseColor.r(), m_cWinHorseColor.g(), m_cWinHorseColor.b());
		Vector textColor;
		Vector shadowColor;
		float lerp = FLerp(0.0f, 1.0f, m_flBGStartFadeTime2, m_flBGEndFadeTime2, deltaTime);
		textColor = VectorLerp(Vector(255, 255, 255), horseColor, lerp);
		shadowColor = VectorLerp(Vector(255, 255, 255), Vector(0, 0, 0), lerp);

		Color finalColor;
		finalColor.SetColor(textColor.x, textColor.y, textColor.z, nameAlpha);

		vgui::surface()->DrawSetTextFont(winFont);

		vgui::surface()->DrawSetTextColor(finalColor);
		DrawTextOutline(x + (wide / 8) + shakeX, y + (tall / 2) + (tall / 4) + shakeY, tempString, 20, 48);

		vgui::surface()->DrawSetTextColor(Color(shadowColor.x, shadowColor.y, shadowColor.z, 255));
		DrawTextOutline(x + (wide / 8) + shakeX, y + (tall / 2) + (tall / 4) + shakeY, tempString, 10, 24);

		vgui::surface()->DrawSetTextPos(x + (wide / 8) + shakeX, y + (tall / 2) + (tall / 4) + shakeY);
		vgui::surface()->DrawSetTextColor(finalColor);
		//vgui::surface()->DrawSetTextScale(10, 10);

		if (tempString)
			vgui::surface()->DrawPrintText(tempString, wcslen(tempString));


		int iHeight = vgui::surface()->GetFontTall(winFont);

		vgui::surface()->DrawSetTextColor(Color(0, 0, 0, nameAlpha));
		vgui::surface()->DrawSetTextFont(statFont);

		DrawTextOutline(x + (wide / 8), y + (tall / 2) + (tall / 4) + iHeight + 30, m_szCurrentTime, 5, 16);

		vgui::surface()->DrawSetTextColor(finalColor);
		vgui::surface()->DrawSetTextPos(x + (wide / 8), y + (tall / 2) + (tall / 4) + iHeight + 30);

		if (m_szCurrentTime)
			vgui::surface()->DrawPrintText(m_szCurrentTime, wcslen(m_szCurrentTime));

	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------- 
void CHudHorseRace::DrawTextOutline(int x, int y, wchar_t* string, int thickness, int iterations, int modifyLen)
{
	int precision = 1000;
	for (float i = 0; i < (360 * precision); i += ((360 * precision) / iterations))
	{
		int shadowX = round(cos(i / precision) * thickness);
		int shadowY = round(sin(i / precision) * thickness);

		vgui::surface()->DrawSetTextPos(x + shadowX, y + shadowY);

		if (string)
			vgui::surface()->DrawPrintText(string, wcslen(string) - modifyLen);
	}
	/*
	for (int i = 0; i < 8; i++)
	{
		int shadowX = 0;
		int shadowY = 0;
		switch (i)
		{
		case 0:
			shadowY = -thickness;
			break;
		case 1:
			shadowX = thickness;
			shadowY = -thickness;
			break;
		case 2:
			shadowX = thickness;
			break;
		case 3:
			shadowX = thickness;
			shadowY = thickness;
			break;
		case 4:
			shadowY = thickness;
			break;
		case 5:
			shadowX = -thickness;
			shadowY = thickness;
			break;
		case 6:
			shadowX = -thickness;
			break;
		case 7:
			shadowX = -thickness;
			shadowY = -thickness;
			break;
		}
		vgui::surface()->DrawSetTextPos(x + shadowX, y + shadowY);

		if (string)
			vgui::surface()->DrawPrintText(string, wcslen(string));
	}
	*/
}