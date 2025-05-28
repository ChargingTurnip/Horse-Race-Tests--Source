// TURNIP CODE 
// 5/4/2025

#include "cbase.h"
#include "player.h"
#include "props.h"
#include "effect_dispatch_data.h"
#include "info_horse_start.h"

LINK_ENTITY_TO_CLASS(info_horse_start, CInfoHorseStart);


BEGIN_DATADESC(CInfoHorseStart)

DEFINE_THINKFUNC(Think),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void CInfoHorseStart::Precache(void)
{
	PrecacheModel("models/info_horse_start.mdl");
	PrecacheParticleSystem("aurora_shockwave");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CInfoHorseStart::Spawn(void)
{
	Precache();
	SetModel("models/info_horse_start.mdl");

	ResetSequence(LookupSequence("idle"));
	m_flAnimTime = gpGlobals->curtime;
	m_flPlaybackRate = 1.0f;
	//SetCycle(RandomFloat(0.0f, 1.0f));
	SetCycle(0.0f);

	BaseClass::Spawn();

	AddEffects(EF_NOSHADOW);

	SetRenderMode(kRenderGlow);

	SetContextThink(&CInfoHorseStart::Think, gpGlobals->curtime, "infohorsestartthink");
}

void CInfoHorseStart::SetHorse(const char* szHorse)
{
	if (m_bSet)
		return;

	if (m_hHorse == NULL)
	{
		m_hHorse = CreateEntityByName("horse");
		if (m_hHorse != NULL)
		{
			// Setup our basic parameters
			m_hHorse->KeyValue("horse_preset", szHorse);
			m_hHorse->SetAbsOrigin(GetAbsOrigin());
			m_hHorse->SetAbsAngles(GetAbsAngles());
			DispatchSpawn(m_hHorse);
			if (gpGlobals->curtime > 0.5f)
				m_hHorse->Activate();

			V_strcpy(m_szHorse, szHorse);
		}
	}

	DispatchParticleEffect("aurora_shockwave", GetAbsOrigin(), GetAbsAngles());
	m_bSet = true;
}

void CInfoHorseStart::Think()
{
	SetNextThink(gpGlobals->curtime, "infohorsestartthink");

	StudioFrameAdvance();
	DispatchAnimEvents(this);

	if (m_bActive)
		SetBodygroup(1, 1);
	else
		SetBodygroup(1, 0);

	if (!m_bDisappearing)
	{
		if (UTIL_GetLocalPlayer())
		{
			if (UTIL_GetLocalPlayer()->GetViewEntity())
			{
				trace_t tr;
				Vector vecCameraPos = UTIL_GetLocalPlayer()->GetViewEntity()->GetAbsOrigin();
				Vector vecCenterPos = GetAbsOrigin();
				vecCenterPos.z += 24;
				UTIL_TraceLine(vecCameraPos, vecCenterPos, MASK_BLOCKLOS, this, COLLISION_GROUP_NONE, &tr);

				if (tr.DidHit())
				{
					if (!m_bObscured)
						AddGlowEffect();

					m_bObscured = true;
				}
				else
				{
					if (m_bObscured)
						RemoveGlowEffect();

					m_bObscured = false;
				}
			}
		}

		if (m_bSet)
		{
			SetRenderColorA(32);
			SetGlowEffectColor(0.3f, 0.3f, 0.3f, 0.3f);
		}
		else
		{
			if (m_bActive)
				SetRenderColorA(230);
			else
				SetRenderColorA(128);

			SetGlowEffectColor(1.0f, 1.0f, 1.0f, 0.5f);
		}
	}

	if (m_bDisappearing)
	{
		float deltaTime MIN(gpGlobals->curtime, m_flDisappearEndTime);
		SetRenderColorA(FLerp(m_nStartDisappearAlpha, 0, m_flDisappearStartTime, m_flDisappearEndTime, deltaTime));

		if (GetRenderColor().a == 0)
		{
			m_bDisappearing = false;
			UTIL_Remove(this);
		}
	}
}