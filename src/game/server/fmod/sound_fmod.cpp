#include "cbase.h"
#include "sound_fmod.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar volume;
extern ConVar snd_musicvolume;

#define SF_AMBIENT_SOUND_EVERYWHERE			1
#define SF_AMBIENT_SOUND_START_SILENT		16
#define SF_AMBIENT_SOUND_NOT_LOOPING		32
#define SF_AMBIENT_SOUND_MUSIC				64

LINK_ENTITY_TO_CLASS( ambient_fmod, CAmbientFMOD );

BEGIN_DATADESC( CAmbientFMOD )

	DEFINE_THINKFUNC(Think),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "PlaySound", InputPlaySound ),
	DEFINE_INPUTFUNC( FIELD_VOID, "StopSound", InputStopSound ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetVolume", InputSetVolume ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "IncreaseVolume", InputIncreaseVolume ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "DecreaseVolume", InputDecreaseVolume ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetMusicSpeed", InputSetMusicSpeed ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "Pitch", InputPitch ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "IncreasePitch", InputIncreasePitch ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "DecreasePitch", InputDecreasePitch ),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetFadeTime", InputSetFadeTime),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "Fade", InputFade),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "Fade2", InputFade2),

	// Fields
	DEFINE_KEYFIELD( m_iszSound, FIELD_SOUNDNAME, "message" ),
	DEFINE_KEYFIELD( m_flMaxDistance,	FIELD_FLOAT, "maxdistance" ),
	DEFINE_KEYFIELD( m_flVolume, FIELD_FLOAT, "volume" ),
	DEFINE_KEYFIELD( m_flPitch, FIELD_FLOAT, "pitch" ),
	DEFINE_KEYFIELD( m_sSourceEntName,	FIELD_STRING, "SourceEntityName" ),

	DEFINE_FIELD( m_bLooping, FIELD_BOOLEAN ),
	DEFINE_FIELD(m_bActive, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bGlobal, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bMusic, FIELD_BOOLEAN),
	DEFINE_FIELD(m_hSoundSource, FIELD_EHANDLE),
	DEFINE_FIELD(m_iszSound, FIELD_SOUNDNAME),
	DEFINE_FIELD(m_flMaxDistance, FIELD_FLOAT),
	DEFINE_FIELD(m_flVolume, FIELD_FLOAT),
	DEFINE_FIELD(m_flPitch, FIELD_FLOAT),
	DEFINE_FIELD(m_flMusicSpeed, FIELD_FLOAT),

	DEFINE_FIELD(m_flFadeTime, FIELD_FLOAT),
	DEFINE_FIELD(m_flFadeEndTime, FIELD_FLOAT),
	DEFINE_FIELD(m_flFadeVolume, FIELD_FLOAT),
	DEFINE_FIELD(m_flFadeStartVolume, FIELD_FLOAT),
	DEFINE_FIELD(m_bFading, FIELD_BOOLEAN),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CAmbientFMOD, DT_AmbientFMOD )
	SendPropStringT ( SENDINFO( m_iszSound ) ),
	SendPropFloat( SENDINFO( m_flMaxDistance ), 8, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flVolume ), 8, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flPitch ), 8, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flMusicSpeed ), 8, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bGlobal ) ),
	SendPropBool( SENDINFO( m_bActive ) ),
	SendPropBool( SENDINFO( m_bLooping ) ),
	SendPropBool( SENDINFO( m_bMusic ) ),
	SendPropEHandle( SENDINFO( m_hSoundSource ) ),
END_SEND_TABLE()

CAmbientFMOD::CAmbientFMOD()
{
	m_iszSound = NULL_STRING;

	m_flVolume = 1.0f;
	m_flPitch = 100.0f;
	m_flMusicSpeed = 1.0f;

	m_bGlobal = false;
	m_bActive = false;
	m_bLooping = false;
	m_bMusic = false;

	m_flFadeTime = 0.0f;
	m_flFadeEndTime = 0.0f;
	m_flFadeVolume = 0.0f;
	m_flFadeStartVolume = 0.0f;
	m_bFading = false;
}

void CAmbientFMOD::Spawn( void )
{
	BaseClass::Spawn();

	if ( FBitSet ( m_spawnflags, SF_AMBIENT_SOUND_EVERYWHERE ) )
		m_bGlobal = true;
	else
		m_bGlobal = false;

	if ( FBitSet ( m_spawnflags, SF_AMBIENT_SOUND_NOT_LOOPING ) )
		m_bLooping = false;
	else
		m_bLooping = true;

	if ( FBitSet ( m_spawnflags, SF_AMBIENT_SOUND_START_SILENT ) )
		m_bActive = false;
	else
		m_bActive = true;

	if ( FBitSet ( m_spawnflags, SF_AMBIENT_SOUND_MUSIC ) )
		m_bMusic = true;
	else
		m_bMusic = false;

	if ( m_sSourceEntName != NULL_STRING )
		m_hSoundSource = gEntList.FindEntityByName( NULL, m_sSourceEntName );

	if ( !m_hSoundSource )
		m_hSoundSource = this;

	if ( m_flVolume.Get() < 0.0f )
		m_flVolume = 0.0f;
	else
	{
		if ( m_flVolume.Get() / 10.0f > 1.0f )
			m_flVolume = 1.0f;
		else
			m_flVolume = ( m_flVolume.Get() /10.0f );
	}

	// Talk to the client class when data changes
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	SetContextThink(&CAmbientFMOD::Think, gpGlobals->curtime, "ambientfmodthink");
}

void CAmbientFMOD::Think(void)
{
	if (m_bFading)
	{
		if (gpGlobals->curtime < m_flFadeEndTime)
		{
			float modify = ((m_flFadeStartVolume - m_flFadeVolume) / (66 * m_flFadeTime)) * -1;
			m_flVolume += modify;
		}
		else
		{
			m_bFading = false;
			m_flVolume = m_flFadeVolume;
		}
	}

	SetNextThink(gpGlobals->curtime, "ambientfmodthink");
}

void CAmbientFMOD::InputPlaySound( inputdata_t &inputData )
{
	m_bActive = true;
}

void CAmbientFMOD::InputStopSound( inputdata_t &inputData )
{
	m_bActive = false;
}

void CAmbientFMOD::InputSetVolume( inputdata_t &inputData )
{
	if ( inputData.value.Float() < 0.0f )
		m_flVolume = 0.0f;
	else if ( inputData.value.Float() > 10.0f )
		m_flVolume = 1.0f;
	else if ( inputData.value.Float() >= 0.0f &&  inputData.value.Float() <= 10.0f )
		m_flVolume = ( inputData.value.Float() / 10.0f );
}

void CAmbientFMOD::InputIncreaseVolume( inputdata_t &inputData )
{
	float flIncrease = ( inputData.value.Float() / 10.0f );

	if ( flIncrease + m_flVolume.Get() > 1.0f )
		m_flVolume = 1.0f;
	else
		m_flVolume = m_flVolume.Get() + flIncrease;
}

void CAmbientFMOD::InputDecreaseVolume( inputdata_t &inputData )
{
	if ( ( m_flVolume.Get() - ( inputData.value.Float() / 10.0f ) ) < 0.0f )
		m_flVolume = 0.0f;
	else
		m_flVolume = ( m_flVolume.Get() - ( inputData.value.Float() / 10.0f ) );
}

void CAmbientFMOD::InputSetMusicSpeed( inputdata_t &inputData )
{
	m_flMusicSpeed = inputData.value.Float();
}

void CAmbientFMOD::InputPitch( inputdata_t &inputData )
{
	m_flPitch = inputData.value.Float();
}

void CAmbientFMOD::InputIncreasePitch( inputdata_t &inputData )
{
	if ( m_flPitch + inputData.value.Float() > 255.0f )
		m_flPitch = 255.0f;
	else
		m_flPitch += inputData.value.Float();
}

void CAmbientFMOD::InputDecreasePitch( inputdata_t &inputData )
{
	if ( m_flPitch - inputData.value.Float() < 0.0f )
		m_flPitch = 0.0f;
	else
		m_flPitch -= inputData.value.Float();
}

void CAmbientFMOD::InputSetFadeTime(inputdata_t& inputData)
{
	m_flFadeTime = inputData.value.Float();
}

void CAmbientFMOD::InputFade(inputdata_t& inputData)
{
	m_flFadeStartVolume = m_flVolume;
	m_flFadeEndTime = gpGlobals->curtime + m_flFadeTime;
	m_flFadeVolume = inputData.value.Float() / 10.0f;
	m_bFading = true;
}

void CAmbientFMOD::InputFade2(inputdata_t& inputData)
{
	m_flVolume = 0.0f;
	m_flFadeStartVolume = m_flVolume;
	m_flFadeEndTime = gpGlobals->curtime + m_flFadeTime;
	m_flFadeVolume = inputData.value.Float() / 10.0f;
	m_bFading = true;
}