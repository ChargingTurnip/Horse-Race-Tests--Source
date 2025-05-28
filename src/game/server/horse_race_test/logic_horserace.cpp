// TURNIP CODE 
// 5/4/2025

#include "cbase.h"
#include "info_horse_start.h"
#include "recipientfilter.h"
#include "usermessages.h"
#include "logic_horserace.h"
#include "horse.h"

LINK_ENTITY_TO_CLASS(logic_horserace, CLogicHorseRace);

BEGIN_DATADESC(CLogicHorseRace)

	DEFINE_THINKFUNC(Think),

	DEFINE_KEYFIELD(m_bBattleMode, FIELD_BOOLEAN, "battle"),
	DEFINE_OUTPUT(m_OnRaceStart, "OnRaceStart"),
	DEFINE_OUTPUT(m_OnRaceFinish, "OnRaceFinish"),
	//DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "startenabled"),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Precache function for the entity
//-----------------------------------------------------------------------------
void CLogicHorseRace::Activate(void)
{
	PrecacheScriptSound("Announcer.Ready");
	PrecacheScriptSound("Announcer.10");
	PrecacheScriptSound("Announcer.9");
	PrecacheScriptSound("Announcer.8");
	PrecacheScriptSound("Announcer.7");
	PrecacheScriptSound("Announcer.6");
	PrecacheScriptSound("Announcer.5");
	PrecacheScriptSound("Announcer.4");
	PrecacheScriptSound("Announcer.3");
	PrecacheScriptSound("Announcer.2");
	PrecacheScriptSound("Announcer.1");
	PrecacheScriptSound("Announcer.0");

	PrecacheScriptSound("Announcer.Gun");

	PrecacheScriptSound("UI.Camera");

	CHandle< CInfoHorseStart > pFirst = NULL;
	pFirst = (CInfoHorseStart*)gEntList.FindEntityByClassname(pFirst, "info_horse_start");
	if (pFirst)
		pFirst->SetActive(true);

	SetContextThink(&CLogicHorseRace::Think, gpGlobals->curtime, "logichorseracethink");
}

void CLogicHorseRace::Think( void )
{
	if (!m_bActivated)
	{
		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		UserMessageBegin(user, "OpenHorseSelect");
		WRITE_BYTE(0),
		MessageEnd();
		m_bActivated = true;
	}

	if (gpGlobals->curtime >= m_flCountdownStartTime && m_nCurrentCountdownNum == -1 && m_flCountdownStartTime != -1.0f)
		m_flCountdown = 10.0f;

	if (m_flCountdown > -1.0f)
	{
		m_flCountdown = 10.0f - (gpGlobals->curtime - m_flCountdownStartTime);
		if (m_nCurrentCountdownNum != ceil(m_flCountdown) && ceil(m_flCountdown) != -1)
		{
			m_nCurrentCountdownNum = ceil(m_flCountdown);

			CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
			UserMessageBegin(user, "HRActivateElement");
			WRITE_SHORT(m_nCurrentCountdownNum),
			WRITE_FLOAT(0.6f),
			MessageEnd();

			CPASAttenuationFilter sndFilter(this, UTIL_VarArgs("Announcer.%d", m_nCurrentCountdownNum));
			EmitSound(sndFilter, entindex(), UTIL_VarArgs("Announcer.%d", m_nCurrentCountdownNum));
		}
	}

	if (m_flCountdown <= -1.0f && m_nCurrentCountdownNum == 0 && !m_bRacing)
	{
		m_bRacing = true;

		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		UserMessageBegin(user, "HRActivateElement");
		WRITE_SHORT(12),
		WRITE_FLOAT(0.0f),
		MessageEnd();

		CPASAttenuationFilter sndFilter(this, UTIL_VarArgs("Announcer.Gun", m_nCurrentCountdownNum));
		EmitSound(sndFilter, entindex(), UTIL_VarArgs("Announcer.Gun", m_nCurrentCountdownNum));

		m_OnRaceStart.FireOutput(this, this);

		CHandle< CHorse > pSearch = NULL;
		do
		{
			pSearch = (CHorse*)gEntList.FindEntityByClassname(pSearch, "horse");
			if (pSearch)
			{
				pSearch->Start();
				if (m_bBattleMode)
					pSearch->SetBattleMode(true);
			}

		} while (pSearch);
	}

	if (!m_bRacing)
	{
		bool bFoundActive = false;
		CHandle< CInfoHorseStart > pStart = NULL;
		do
		{
			pStart = (CInfoHorseStart*)gEntList.FindEntityByClassname(pStart, "info_horse_start");
			if (pStart)
			{
				if (!pStart->IsSet() && !bFoundActive)
				{
					pStart->SetActive(true);
					bFoundActive = true;
				}
				else
					pStart->SetActive(false);
			}

		} while (pStart);
	}

	if (m_flFinishStartTime != -1.0f && gpGlobals->curtime >= m_flFinishStartTime)
	{
		float deltaTime = MIN(gpGlobals->curtime, m_flFinishEndTime);
		float lerp = FLerp(0.0f, 1.0f, m_flFinishStartTime, m_flFinishEndTime, deltaTime);

		if (UTIL_GetLocalPlayer())
		{
			if (UTIL_GetLocalPlayer()->GetViewEntity())
			{
				Vector goalPos = m_hGoal->GetAbsOrigin();
				goalPos.z += 48;
				Vector pos = VectorLerp(m_vecCameraPos, goalPos, lerp);

				float fov = Lerp(lerp, m_flCameraFOV, 40.0f);

				UTIL_GetLocalPlayer()->GetViewEntity()->SetAbsOrigin(pos);
				UTIL_GetLocalPlayer()->SetFOV(this, fov, 0.2f);
			}
		}
	}
	if (m_flFinishBGTime != -1.0f && gpGlobals->curtime >= m_flFinishBGTime && !m_bShownWinScreen)
	{
		m_bShownWinScreen = true;

		CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
		UserMessageBegin(user, "HRActivateElement");
		WRITE_SHORT(14),
		WRITE_FLOAT(0.0f),
		MessageEnd();

		CPASAttenuationFilter sndFilter(this, "UI.Camera");
		EmitSound(sndFilter, entindex(), "UI.Camera");
	}

	SetNextThink(gpGlobals->curtime, "logichorseracethink");
}

void CLogicHorseRace::SetHorse(const char* szHorse)
{
	bool bRemoved = false;

	CHandle< CInfoHorseStart > pSearch = NULL;
	do
	{
		pSearch = (CInfoHorseStart*)gEntList.FindEntityByClassname(pSearch, "info_horse_start");
		if (pSearch)
		{
			if (pSearch->IsSet() && !V_stricmp(pSearch->GetHorse(), szHorse))
			{
				pSearch->RemoveHorse();
				bRemoved = true;
			}
		}

	} while (pSearch);

	// dont add a horse we just removed
	if (bRemoved)
		return;

	pSearch = NULL;
	do
	{
		pSearch = (CInfoHorseStart*)gEntList.FindEntityByClassname(pSearch, "info_horse_start");
		if (pSearch)
		{
			if (!pSearch->IsSet())
			{
				pSearch->SetHorse(szHorse);
				break;
			}
		}

	} while (pSearch);
}

void CLogicHorseRace::ClearHorses(void)
{
	CHandle< CInfoHorseStart > pSearch = NULL;
	do
	{
		pSearch = (CInfoHorseStart*)gEntList.FindEntityByClassname(pSearch, "info_horse_start");
		if (pSearch)
		{
			if (pSearch->IsSet())
				pSearch->RemoveHorse();

			pSearch->SetActive(false);
		}

	} while (pSearch);
}

void CLogicHorseRace::Start(void)
{
	CHandle< CInfoHorseStart > pSearch = NULL;
	do
	{
		pSearch = (CInfoHorseStart*)gEntList.FindEntityByClassname(pSearch, "info_horse_start");
		if (pSearch)
		{
			pSearch->Disappear();
			pSearch->SetActive(false);
		}

	} while (pSearch);

	CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
	UserMessageBegin(user, "HRActivateElement");
	WRITE_SHORT(11),
	WRITE_FLOAT(1.28f),
	MessageEnd();

	CPASAttenuationFilter sndFilter(this, UTIL_VarArgs("Announcer.Ready", m_nCurrentCountdownNum));
	EmitSound(sndFilter, entindex(), UTIL_VarArgs("Announcer.Ready", m_nCurrentCountdownNum));

	m_flCountdownStartTime = gpGlobals->curtime + 2.3f;
}

void CLogicHorseRace::Finish(CHorse* hHorse)
{
	m_OnRaceFinish.FireOutput(this, this);

	hHorse->Win();

	CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
	UserMessageBegin(user, "HRActivateElement");
	WRITE_SHORT(13),
	WRITE_FLOAT(0.0f),
	MessageEnd();

	UserMessageBegin(user, "HRSetWinHorse");
	WRITE_STRING(hHorse->GetGraphic()),
	WRITE_STRING(hHorse->GetName()),
	WRITE_SHORT(hHorse->GetColor().r()),
	WRITE_SHORT(hHorse->GetColor().g()),
	WRITE_SHORT(hHorse->GetColor().b()),
	MessageEnd();

	CHandle< CHorse > pSearch = NULL;
	do
	{
		pSearch = (CHorse*)gEntList.FindEntityByClassname(pSearch, "horse");
		if (pSearch)
		{
			if (pSearch != hHorse)
				pSearch->Stop();
		}

	} while (pSearch);

	if (UTIL_GetLocalPlayer())
	{
		if (UTIL_GetLocalPlayer()->GetViewEntity())
		{
			m_vecCameraPos = UTIL_GetLocalPlayer()->GetViewEntity()->GetAbsOrigin();
			m_flCameraFOV = UTIL_GetLocalPlayer()->GetFOV();
		}
	}

	m_flFinishStartTime = gpGlobals->curtime + 1.5f;
	m_flFinishEndTime = gpGlobals->curtime + 5.0f;

	m_flFinishBGTime = m_flFinishEndTime;
}