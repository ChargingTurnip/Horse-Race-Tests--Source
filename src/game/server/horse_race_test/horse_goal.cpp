// TURNIP CODE 
// 5/5/2025

#include "cbase.h"
#include "player.h"
#include "props.h"
#include "effect_dispatch_data.h"

#include "KeyValues.h"
#include "filesystem.h"

#include "horse.h"
#include "horse_goal.h"
#include "logic_horserace.h"

LINK_ENTITY_TO_CLASS(horse_goal, CHorseGoal);


BEGIN_DATADESC(CHorseGoal)

	DEFINE_THINKFUNC(Think),
	DEFINE_ENTITYFUNC(TouchThink),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void CHorseGoal::Precache(void)
{
	if (GetModelName() != NULL_STRING)
		PrecacheModel(STRING(GetModelName()));

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CHorseGoal::Spawn(void)
{
	Precache();
	if (GetModelName() != NULL_STRING)
		SetModel(STRING(GetModelName()));

	BaseClass::Spawn();

	SetSolid(SOLID_BBOX);
	SetSize(Vector(-8, -8, 0), Vector(8, 8, 8));

	AddSolidFlags(FSOLID_TRIGGER);
	CollisionProp()->UseTriggerBounds(true, 24);

	ResetSequence(LookupSequence("idle"));
	m_flAnimTime = gpGlobals->curtime;
	m_flPlaybackRate = 1.0f;
	SetCycle(RandomFloat(0.0f, 1.0f));

	AddGlowEffect();
	SetGlowEffectColor(0.89f, 0.6f, 0.058f);

	SetTouch(&CHorseGoal::TouchThink);
	SetContextThink(&CHorseGoal::Think, gpGlobals->curtime, "horsegoalthink");
}

void CHorseGoal::TouchThink(CBaseEntity* pOther)
{
	SetNextThink(gpGlobals->curtime);

	if (m_bFinished)
		return;

	if (pOther)
	{
		CHorse* pHorse = dynamic_cast<CHorse*>(pOther);
		if (pHorse)
		{
			if (pHorse->HasStarted())
			{
				CHandle< CLogicHorseRace > pLogic = NULL;
				pLogic = (CLogicHorseRace*)gEntList.FindEntityByClassname(pLogic, "logic_horserace");
				if (pLogic)
				{
					pLogic->SetGoal(this);
					pLogic->Finish(pHorse);
				}
				m_bFinished = true;
			}
		}
	}
}

void CHorseGoal::Think()
{
	SetNextThink(gpGlobals->curtime, "horsegoalthink");

	if (m_bFinished)
		return;

	StudioFrameAdvance();
	DispatchAnimEvents(this);
}
