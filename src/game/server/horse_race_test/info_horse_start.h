// TURNIP CODE 
// 5/4/2025

#ifndef INFO_HORSE_START_H
#define INFO_HORSE_START_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

class CInfoHorseStart : public CBaseAnimating
{
public:
	DECLARE_CLASS(CInfoHorseStart, CBaseAnimating);
	DECLARE_DATADESC();

	CInfoHorseStart()
	{
		m_bSet = false;
		m_bDisappearing = false;
		m_flDisappearStartTime = 0.0f;
		m_flDisappearEndTime = 0.0f;

		m_bActive = false;

		m_bObscured = false;
	}

	void			Precache(void);
	void			Spawn(void);
	void			Think(void);
	void			SetHorse(const char* szHorse);
	const char* GetHorse(void) { return m_szHorse; }
	void			RemoveHorse(void) 
	{ 
		UTIL_Remove(m_hHorse); 
		m_hHorse = NULL; 
		m_bSet = false; 
	}
	bool			IsSet(void) { return m_bSet; }
	void			Disappear(void) 
	{ 
		m_bDisappearing = true; 
		m_flDisappearStartTime = gpGlobals->curtime; 
		m_flDisappearEndTime = gpGlobals->curtime + 1.0f; 
		m_nStartDisappearAlpha = GetRenderColor().a; 
		RemoveGlowEffect();
	}
	void			SetActive(bool bActive) {m_bActive = bActive;}

private:

	bool		m_bSet;
	bool		m_bDisappearing = true;
	float		m_flDisappearStartTime;
	float		m_flDisappearEndTime;
	int			m_nStartDisappearAlpha;

	EHANDLE		m_hHorse;

	bool		m_bActive;

	bool		m_bObscured;

	char	m_szHorse[MAX_PATH];
};

#endif // INFO_HORSE_START_H