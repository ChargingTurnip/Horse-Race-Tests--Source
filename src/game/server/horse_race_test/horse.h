// TURNIP CODE 
// 5/4/2025

#ifndef HORSE_H
#define HORSE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "fmod/sound_fmod.h"

class CHorse : public CBaseFlex
{
public:
	DECLARE_CLASS(CHorse, CBaseFlex);
	DECLARE_DATADESC();

	CHorse()
	{
		m_flHorseSpeed = 600.0f;
		m_flHorseSwimSpeed = 600.0f;
		m_flBounceAnimAgainTime = 0.0f;
		m_bStarted = false;

		m_flWinMusicStartTime = -1.0f;
		m_bPlayedWinMusic = false;

		m_bObscured = false;

		m_cHorseColor = Color(255, 255, 255);
		m_cHorseOutlineColor = Color(255, 255, 255);

		m_bUseOutlineColor = false;

		m_vecHorseMin = Vector(-32, -32, 0);
		m_vecHorseMax = Vector(32, 32, 72);

		m_flNextWadeTime = 0.0f;
		m_flNextSplashTime = 0.0f;

		m_bInWater = false;
		m_bInBattleMode = false;

		m_nHorseHealth = 5;

		m_flDamageTime = 0.0f;

		m_bDead = false;

		m_hHorseToDamage = NULL;

		m_flAllowDamageTime = 0.0f;

		m_hLastDamageHorse = NULL;
	}

	void			Precache(void);
	void			Spawn(void);
	void			TouchThink(CBaseEntity* pOther);
	void			Think(void);
	void			Bounce(Vector vecNormal, bool bRandom = true);
	void			Start(void);
	void			Win(void);
	void			Stop(void) { m_bStarted = false; SetAbsVelocity(Vector(0, 0, 0));}
	bool			HasStarted(void) { return m_bStarted; }
	const char*		GetGraphic(void) { return m_szHorseGraphic; }
	const char*		GetName(void) { return m_szHorseName; }
	Color			GetColor(void) { return m_cHorseColor; }
	void			SpawnParticlesInCircle(const char* szName, Vector vecPos, int nNum, float flDist);
	void			SetBattleMode(bool bEnable) { m_bInBattleMode = bEnable; }
	void			SetHorseHealth(int nHealth) { m_nHorseHealth = nHealth; }
	void			HorseDamage(void);
	void			BattleModeThink(void);
	void			SetLastAttacker(EHANDLE hAttacker) { m_hLastDamageHorse = hAttacker; }

private:

	string_t	m_iszHorsePreset;
	const char* m_szModelName;
	const char* m_szBounceParticleName;
	const char* m_szBounceSoundName;
	const char* m_szHorseName;
	const char* m_szHorseGraphic;

	const char* m_szHorseHurtParticle;
	const char* m_szHorseHurtSound;

	const char* m_szHorseDeathParticle;
	const char* m_szHorseDeathSound;

	Color		m_cHorseColor;
	Color		m_cHorseOutlineColor;

	bool		m_bUseOutlineColor;

	Vector		m_vecHorseMin;
	Vector		m_vecHorseMax;

	EHANDLE		m_hLastHorse;
	CHandle<CAmbientFMOD>	m_hWinMusic;
	float		m_flAllowHorseAgainTime;

	float		m_flBounceAnimAgainTime;

	float		m_flHorseSpeed;
	float		m_flHorseSwimSpeed;

	float		m_flWinMusicStartTime;
	bool		m_bPlayedWinMusic;

	bool		m_bStarted;

	bool		m_bObscured;

	float		m_flNextWadeTime;
	float		m_flNextSplashTime;

	bool		m_bInWater;
	bool		m_bInBattleMode;

	int			m_nHorseHealth;

	float		m_flDamageTime;

	bool		m_bDead;

	CHorse*		m_hHorseToDamage;

	float		m_flAllowDamageTime;
	EHANDLE		m_hLastDamageHorse;

	const char* m_szWinSoundName;
	const char* m_szWinMusicName;
};

#endif // HORSE_H