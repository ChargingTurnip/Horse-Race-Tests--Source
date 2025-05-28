// TURNIP CODE 
// 5/5/2025

#ifndef HORSE_GOAL_H
#define HORSE_GOAL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

class CHorseGoal : public CBaseAnimating
{
public:
	DECLARE_CLASS(CHorseGoal, CBaseAnimating);
	DECLARE_DATADESC();

	CHorseGoal()
	{
		m_bFinished = false;
	}

	void			Precache(void);
	void			Spawn(void);
	void			TouchThink(CBaseEntity* pOther);
	void			Think(void);

private:

	bool	m_bFinished;
};

#endif // HORSE_GOAL_H