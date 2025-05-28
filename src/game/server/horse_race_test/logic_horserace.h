// TURNIP CODE 
// 5/4/2025

#ifndef LOGIC_HORSERACE_H
#define LOGIC_HORSERACE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "horse.h"

class CLogicHorseRace : public CLogicalEntity
{
public:
	DECLARE_CLASS(CLogicHorseRace, CLogicalEntity);
	DECLARE_DATADESC();

	// Constructor
	CLogicHorseRace()
	{
		m_bActivated = false;
		m_bRacing = false;
		m_flCountdown = -1.0f;
		m_nCurrentCountdownNum = -1;
		m_flCountdownStartTime = -1.0f;

		m_flFinishStartTime = -1.0f;
		m_flFinishEndTime = -1.0f;
		m_flFinishBGTime = -1.0f;

		m_flCameraFOV = 90.0f;

		m_bShownWinScreen = false;

		m_vecCameraPos = Vector(0, 0, 0);

		m_bBattleMode = false;
	}

	void Think(void);
	void Activate(void);
	void SetHorse(const char* szHorse);
	void ClearHorses(void);
	void Start(void);
	void Finish(CHorse* hHorse);
	void SetGoal(EHANDLE hGoal) { m_hGoal = hGoal; }

private:
	bool	m_bActivated;
	bool	m_bRacing;

	int		m_nCurrentCountdownNum;
	float	m_flCountdown;
	float	m_flCountdownStartTime;

	float	m_flFinishStartTime;
	float	m_flFinishEndTime;

	float	m_flFinishBGTime;

	Vector	m_vecCameraPos;

	float	m_flCameraFOV;

	bool	m_bShownWinScreen;

	bool	m_bBattleMode;

	EHANDLE m_hGoal;

	COutputEvent	m_OnRaceStart;
	COutputEvent	m_OnRaceFinish;
};

#endif // LOGIC_HORSERACE_H