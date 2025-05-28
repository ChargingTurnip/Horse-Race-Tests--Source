// TURNIP CODE 
// 5/3/2025

#include "cbase.h"
#include "player.h"
#include "props.h"
#include "effect_dispatch_data.h"

#include "KeyValues.h"
#include "filesystem.h"

#include "horse.h"
#include "movevars_shared.h"

extern ConVar sv_gravity;

LINK_ENTITY_TO_CLASS(horse, CHorse);


BEGIN_DATADESC(CHorse)

	DEFINE_KEYFIELD(m_iszHorsePreset, FIELD_STRING, "horse_preset"),

	DEFINE_THINKFUNC(Think),
	DEFINE_ENTITYFUNC(TouchThink),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void CHorse::Precache(void)
{
	if (m_szModelName[0])
		PrecacheModel(m_szModelName);
	else
		PrecacheModel("models/horses/horse_basic.mdl");
	if (m_szBounceParticleName[0])
		PrecacheParticleSystem(m_szBounceParticleName);
	if (m_szBounceSoundName[0])
		PrecacheScriptSound(m_szBounceSoundName);
	if (m_szWinSoundName[0])
		PrecacheScriptSound(m_szWinSoundName);
	if (m_szWinMusicName[0])
		PrecacheScriptSound(m_szWinMusicName);

	if (m_szHorseHurtParticle[0])
		PrecacheParticleSystem(m_szHorseHurtParticle);
	if (m_szHorseHurtSound[0])
		PrecacheScriptSound(m_szHorseHurtSound);

	if (m_szHorseDeathParticle[0])
		PrecacheParticleSystem(m_szHorseDeathParticle);
	if (m_szHorseDeathSound[0])
		PrecacheScriptSound(m_szHorseDeathSound);

	PrecacheParticleSystem("water_splash_01");
	PrecacheParticleSystem("water_splash_01_surface3");
	PrecacheScriptSound("BaseEntity.ExitWater");
	PrecacheScriptSound("Physics.WaterSplash");
	PrecacheScriptSound("Airboat.FireGunHeavy");

	PrecacheParticleSystem("aurora_shockwave_ring");
	BaseClass::Precache();
}

void CHorse::Start(void)
{
	m_bStarted = true;

	ResetSequence(LookupSequence("move"));
	m_flAnimTime = gpGlobals->curtime;
	SetCycle(RandomFloat(0.0f, 1.0f));

	QAngle randomAngle = GetAbsAngles();
	randomAngle.y += RandomFloat(-70, 70);
	SetAbsAngles(randomAngle);
}


//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CHorse::Spawn(void)
{
	if (m_iszHorsePreset != NULL_STRING)
	{
		KeyValues* pPresetDataFile = new KeyValues("HorsePresetData");
		if (pPresetDataFile)
		{
			if (pPresetDataFile->LoadFromFile(g_pFullFileSystem, "scripts/horsepresets.txt"))
			{
				m_szModelName = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.model", m_iszHorsePreset), "models/horses/horse_basic.mdl");
				m_szHorseName = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.name", m_iszHorsePreset), "Horse Basic");
				m_flHorseSpeed = pPresetDataFile->GetFloat(UTIL_VarArgs("horse.%s.speed", m_iszHorsePreset), 600.0f);
				m_flHorseSwimSpeed = pPresetDataFile->GetFloat(UTIL_VarArgs("horse.%s.swimspeed", m_iszHorsePreset), m_flHorseSpeed);
				m_szBounceParticleName = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.bounceparticle", m_iszHorsePreset), "hunter_projectile_explosion_2f");
				m_szBounceSoundName = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.bouncesound", m_iszHorsePreset), "Boulder.ImpactSoft");
				m_cHorseColor = pPresetDataFile->GetColor(UTIL_VarArgs("horse.%s.color", m_iszHorsePreset));

				m_vecHorseMin.x = pPresetDataFile->GetInt(UTIL_VarArgs("horse.%s.hullmin.x", m_iszHorsePreset), -32);
				m_vecHorseMin.y = pPresetDataFile->GetInt(UTIL_VarArgs("horse.%s.hullmin.y", m_iszHorsePreset), -32);
				m_vecHorseMin.z = pPresetDataFile->GetInt(UTIL_VarArgs("horse.%s.hullmin.z", m_iszHorsePreset), 0);

				m_vecHorseMax.x = pPresetDataFile->GetInt(UTIL_VarArgs("horse.%s.hullmax.x", m_iszHorsePreset), 32);
				m_vecHorseMax.y = pPresetDataFile->GetInt(UTIL_VarArgs("horse.%s.hullmax.y", m_iszHorsePreset), 32);
				m_vecHorseMax.z = pPresetDataFile->GetInt(UTIL_VarArgs("horse.%s.hullmax.z", m_iszHorsePreset), 72);

				m_szWinSoundName = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.winsound", m_iszHorsePreset), "Horse.WinBasic");
				m_szWinMusicName = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.winmusic", m_iszHorsePreset), "Horse.WinMusicBasic");
					
				int alpha = pPresetDataFile->GetInt(UTIL_VarArgs("horse.%s.outlinealpha", m_iszHorsePreset), 255);
				m_cHorseColor.SetColor(m_cHorseColor.r(), m_cHorseColor.g(), m_cHorseColor.b(), alpha);

				m_cHorseOutlineColor = pPresetDataFile->GetColor(UTIL_VarArgs("horse.%s.outlinecolor", m_iszHorsePreset));
				m_cHorseOutlineColor.SetColor(m_cHorseOutlineColor.r(), m_cHorseOutlineColor.g(), m_cHorseOutlineColor.b(), alpha);

				int bruh = pPresetDataFile->GetInt(UTIL_VarArgs("horse.%s.useoutlinecolor", m_iszHorsePreset), 0);

				if (bruh > 0)
					m_bUseOutlineColor = true;

				m_szHorseGraphic = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.wingraphic", m_iszHorsePreset), "vgui/icons/horses/icon_win_basic");

				m_szHorseHurtParticle = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.hurtparticle", m_iszHorsePreset), "blood_impact_red_01_goop");
				m_szHorseHurtSound = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.hurtsound", m_iszHorsePreset), "Horse.HurtBasic");

				m_szHorseDeathParticle = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.deathparticle", m_iszHorsePreset), "dust2_splinter_stalactite");
				m_szHorseDeathSound = pPresetDataFile->GetString(UTIL_VarArgs("horse.%s.deathsound", m_iszHorsePreset), "Horse.DeathBasic");
			}
		}
	}

	Precache();
	if (m_szModelName[0])
		SetModel(m_szModelName);
	else
		SetModel("models/horses/horse_basic.mdl");

	BaseClass::Spawn();

	SetSolid(SOLID_BBOX);
	SetSize(m_vecHorseMin, m_vecHorseMax);
	SetMoveType(MOVETYPE_STEP);

	SetRenderMode(kRenderGlow);
	SetRenderColor(m_cHorseColor.r(), m_cHorseColor.g(), m_cHorseColor.b());

	AddFlag(FL_NPC);

	//UseClientSideAnimation();

	SetNumAnimOverlays(16);

	SetGravity(sv_gravity.GetFloat() / 3);

	SetTouch(&CHorse::TouchThink);
	SetContextThink(&CHorse::Think, gpGlobals->curtime, "horsethink");
}

void CHorse::TouchThink(CBaseEntity* pOther)
{
	SetNextThink(gpGlobals->curtime);

	if (!m_bStarted)
		return;

	if (m_bDead)
		return;

	if (pOther)
	{
		if (pOther->IsWorld()) // we handle this elsewhere
			return;

		//Msg("%s\n", pOther->GetClassname());

		if (pOther == GetGroundEntity())
			return;

		if (pOther->GetSolidFlags() & FSOLID_TRIGGER)
			return;

		if (pOther->GetSolidFlags() & FSOLID_NOT_SOLID)
			return;

		if (pOther == m_hLastHorse && gpGlobals->curtime < m_flAllowHorseAgainTime)
			return;

		Vector normal = pOther->GetAbsOrigin() - GetAbsOrigin();
		normal.z = 0;
		normal.x = -normal.x;
		normal.y = -normal.y;
		normal.NormalizeInPlace();

		if (m_bInBattleMode)
		{
			if (m_hHorseToDamage == pOther)
			{
				CHorse* pHorse = dynamic_cast<CHorse*>(pOther);

				// dont know why we would need this check because m_hHorseToDamage only gets set if the original entity is a horse in the first place 
				// but i guess its here because it might as well be
				if (pHorse)
					pHorse->HorseDamage();

				m_hHorseToDamage = NULL;
			}
			else
				m_hHorseToDamage = NULL;	
		}

		Bounce(normal);

		m_hLastHorse = pOther;
		m_flAllowHorseAgainTime = gpGlobals->curtime + 0.1f;
	}
}

void CHorse::Think()
{
	SetNextThink(gpGlobals->curtime, "horsethink");

	if (m_bDead)
	{
		if (!m_bObscured)
			AddGlowEffect();
		else
			m_bObscured = false;

		SetGlowEffectColor(255, 0, 0, 50);

		SetRenderColorA(50);
		SetSolid(SOLID_NONE);
		SetMoveType(MOVETYPE_NONE);
		return;
	}

	if (m_flWinMusicStartTime != -1.0f && gpGlobals->curtime >= m_flWinMusicStartTime && !m_bPlayedWinMusic)
	{
		m_bPlayedWinMusic = true;

		variant_t mt;
		m_hWinMusic->AcceptInput("PlaySound", this, this, mt, 0);

		//CPASAttenuationFilter sndFilter(this, m_szWinMusicName);
		//EmitSound(sndFilter, entindex(), m_szWinMusicName);
	}

	if (UTIL_GetLocalPlayer())
	{
		if (UTIL_GetLocalPlayer()->GetViewEntity())
		{
			trace_t tr;
			Vector vecCameraPos = UTIL_GetLocalPlayer()->GetViewEntity()->GetAbsOrigin();
			Vector vecCenterPos = GetAbsOrigin();
			vecCenterPos.z += 48;
			UTIL_TraceLine(vecCameraPos, vecCenterPos, MASK_BLOCKLOS, this, COLLISION_GROUP_NONE, &tr);

			if (tr.DidHit() || !m_bStarted)
			{
				if (!m_bObscured)
				{
					AddGlowEffect();

					float r, g, b, a;

					if (!m_bUseOutlineColor)
					{
						r = m_cHorseColor.r();
						r /= 255;
						b = m_cHorseColor.b();
						b /= 255;
						g = m_cHorseColor.g();
						g /= 255;
						a = m_cHorseColor.a();
						a /= 255;
					}
					else
					{
						r = m_cHorseOutlineColor.r();
						r /= 255;
						b = m_cHorseOutlineColor.b();
						b /= 255;
						g = m_cHorseOutlineColor.g();
						g /= 255;
						a = m_cHorseOutlineColor.a();
						a /= 255;
					}


					SetGlowEffectColor(r, g, b, a);
				}
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

	if (!m_bStarted)
		return;

	if (m_bInBattleMode)
		BattleModeThink();

	StudioFrameAdvance();
	DispatchAnimEvents(this);

	Vector vecForward;
	AngleVectors(GetAbsAngles(), &vecForward);

	if (GetWaterLevel() >= WL_Waist)
	{
		Vector pos = GetAbsOrigin();
		pos.z += 48;

		if (gpGlobals->curtime >= m_flNextSplashTime)
		{
			SpawnParticlesInCircle("water_splash_01_surface3", pos, 8, 24);
			m_flNextSplashTime = gpGlobals->curtime + 0.1f;
		}

		if (gpGlobals->curtime >= m_flNextWadeTime)
		{
			CPASAttenuationFilter sndFilter(this, "BaseEntity.ExitWater");
			EmitSound(sndFilter, entindex(), "BaseEntity.ExitWater");
			m_flNextWadeTime = gpGlobals->curtime + 0.5f + RandomFloat(0.0f, 0.01f);
		}

		SetAbsVelocity(vecForward * (m_flHorseSwimSpeed * 0.6f));

		if (!m_bInWater)
		{
			m_bInWater = true;

			CPASAttenuationFilter sndFilter(this, "Physics.WaterSplash");
			EmitSound(sndFilter, entindex(), "Physics.WaterSplash");

			SpawnParticlesInCircle("water_splash_01", pos, 32, 32);
		}
	}
	else
	{
		if (GetGroundEntity() != NULL)
			SetAbsVelocity(vecForward * m_flHorseSpeed);
		else
			SetAbsVelocity(vecForward * (m_flHorseSpeed * 0.7f));

		m_bInWater = false;
	}

	m_flPlaybackRate = GetAbsVelocity().Length() / 450.0f;

	trace_t trace;

	Vector vecSrc;

	vecSrc = GetLocalOrigin() + Vector(0, 0, 16);

	Vector vecEnd;
	Vector forward;

	QAngle angle;
	angle = GetLocalAngles();
	AngleVectors(angle, &forward);

	vecEnd = vecSrc + forward * 10;

	if (m_bInBattleMode)
	{
		trace_t trace2;

		UTIL_TraceHull(vecSrc, vecEnd, m_vecHorseMin, m_vecHorseMax, MASK_SOLID, this, COLLISION_GROUP_NONE, &trace2);

		if (trace2.fraction != 1.0 && trace2.DidHitNonWorldEntity())
		{
			CHorse* pHorse = dynamic_cast<CHorse*>(UTIL_EntityByIndex(trace2.GetEntityIndex()));
			if (pHorse)
			{
				Vector ourForward;
				angle.x = 0; // align pitch to horizon
				AngleVectors(angle, &ourForward);

				Vector theirForward;
				QAngle theirAngle = pHorse->GetLocalAngles();
				theirAngle.x = 0; // align pitch to horizon
				AngleVectors(theirAngle, &theirForward);

				bool bHit = false;

				if (DotProduct(ourForward, theirForward) > -0.1f)
					bHit = true;

				if (!bHit)
				{
					if (random->RandomInt(0, 1) == 1)
						bHit = true;
				}

				if (bHit)
				{
					if ((m_hLastDamageHorse == pHorse && gpGlobals->curtime >= m_flAllowDamageTime) || m_hLastDamageHorse == NULL)
					{
						m_hHorseToDamage = pHorse; // save this for when the touchfunc finally hits the horse
						pHorse->SetLastAttacker(this);
						m_hLastDamageHorse = NULL;
					}
				}
				else
				{
					Vector pos = GetAbsOrigin();
					pos.z += 48;
					DispatchParticleEffect("aurora_shockwave_ring", pos, GetAbsAngles());

					CPASAttenuationFilter sndFilter(this, "Airboat.FireGunHeavy");
					EmitSound(sndFilter, entindex(), "Airboat.FireGunHeavy");
				}
			}
		}
	}

	float angledelta = 15.0;
	int maxtries = (int)360.0 / angledelta;

	//vecEnd = vecSrc - (forward * 9);

	while (--maxtries >= 0)
	{
		vecSrc = GetLocalOrigin() + Vector(0, 0, 16);
		vecEnd = vecSrc + forward * 10;

		UTIL_TraceHull(vecSrc, vecEnd, m_vecHorseMin, m_vecHorseMax,
			MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace);

		if (trace.fraction != 1.0)
		{
			if (UTIL_EntityByIndex(trace.GetEntityIndex()))
			{
				EHANDLE pEnt = UTIL_EntityByIndex(trace.GetEntityIndex());

				if (pEnt->GetSolidFlags() & FSOLID_TRIGGER)
					return;

				if (pEnt->GetSolidFlags() & FSOLID_NOT_SOLID)
					return;
			}

			Bounce(trace.plane.normal);

			m_hHorseToDamage = NULL; // we must have not ended up hitting the horse by some very small margin, reset our internal tracker to null

			break;
		}
	}
}

void CHorse::Bounce(Vector vecNormal, bool bRandom)
{
	if (!m_bStarted)
		return;

	QAngle wallAngle;
	VectorAngles(vecNormal, wallAngle);
	
	QAngle reflectionAngle = 2 * wallAngle - GetAbsAngles();
	reflectionAngle.y += 180;

	int randomEffect = random->RandomInt(0, 11);
	if (randomEffect == 10 && bRandom)
		reflectionAngle.y += RandomFloat(-20, 20);
	if (randomEffect == 11 && bRandom)
		reflectionAngle.y += RandomFloat(-80, 80);
	
	QAngle newAngles = GetAbsAngles();
	newAngles.y = reflectionAngle.y;
	SetAbsAngles(newAngles);

	if (gpGlobals->curtime >= m_flBounceAnimAgainTime)
	{
		AddGestureSequence(LookupSequence("bounce_gesture"));
		m_flBounceAnimAgainTime = gpGlobals->curtime /*+ 0.05f*/;
	}

	if (m_szBounceSoundName[0])
	{
		CPASAttenuationFilter sndFilter(this, m_szBounceSoundName);
		EmitSound(sndFilter, entindex(), m_szBounceSoundName);
	}

	if (m_szBounceParticleName[0])
	{
		Vector pos = GetAbsOrigin();
		pos.z += 48;
		DispatchParticleEffect(m_szBounceParticleName, pos, GetAbsAngles());
	}
}

void CHorse::Win()
{
	if (!m_bStarted)
		return;

	SetAbsVelocity(Vector(0, 0, 0));

	if (m_hWinMusic == NULL)
	{
		m_hWinMusic = (CAmbientFMOD*)CreateEntityByName("ambient_fmod");
		if (m_hWinMusic != NULL)
		{
			// Setup our basic parameters
			m_hWinMusic->KeyValue("spawnflags", "113");
			m_hWinMusic->KeyValue("message", m_szWinMusicName);
			m_hWinMusic->KeyValue("volume", "1");
			m_hWinMusic->SetAbsOrigin(GetAbsOrigin());
			m_hWinMusic->SetAbsAngles(GetAbsAngles());
			DispatchSpawn(m_hWinMusic);
			if (gpGlobals->curtime > 0.5f)
				m_hWinMusic->Activate();
		}
	}

	m_bStarted = false;

	m_flWinMusicStartTime = gpGlobals->curtime + 1.5f;

	CPASAttenuationFilter sndFilter(this, m_szWinSoundName);
	EmitSound(sndFilter, entindex(), m_szWinSoundName);
}

void CHorse::SpawnParticlesInCircle(const char* szName, Vector vecPos, int nNum, float flDist)
{
	// center
	DispatchParticleEffect(szName, vecPos, GetAbsAngles());

	int precision = 1000;
	for (float i = 0; i < (360 * precision); i += ((360 * precision) / nNum))
	{
		Vector splashPos = vecPos;
		splashPos.x += cos(i / precision) * flDist;
		splashPos.y += sin(i / precision) * flDist;

		DispatchParticleEffect(szName, splashPos, GetAbsAngles());
	}
}

void CHorse::HorseDamage()
{
	m_nHorseHealth--;

	m_flAllowDamageTime = gpGlobals->curtime + 0.05f;

	if (m_nHorseHealth > 0)
	{
		m_flDamageTime = gpGlobals->curtime + 0.3f;

		CPASAttenuationFilter sndFilter(this, m_szHorseHurtSound);
		EmitSound(sndFilter, entindex(), m_szHorseHurtSound);

		Vector pos = GetAbsOrigin();

		for (int i = 0; i < 4; i++)
		{
			SpawnParticlesInCircle(m_szHorseHurtParticle, pos, 12, 48);

			pos.z += 16;
		}
	}
}

void CHorse::BattleModeThink()
{
	if (!m_bStarted)
		return;

	if (!m_bInBattleMode)
		return;

	if (m_nHorseHealth <= 0)
	{
		if (!m_bDead)
		{
			m_bDead = true;

			CPASAttenuationFilter sndFilter(this, m_szHorseDeathSound);
			EmitSound(sndFilter, entindex(), m_szHorseDeathSound);

			Vector pos = GetAbsOrigin();

			for (int i = 0; i < 4; i++)
			{
				SpawnParticlesInCircle(m_szHorseDeathParticle, pos, 12, 48);

				pos.z += 16;
			}
		}
	}
	else
	{
		if (gpGlobals->curtime < m_flDamageTime)
			SetRenderColorA(100);
		else
			SetRenderColorA(255);
	}
}

