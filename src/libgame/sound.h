// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// sound.h
// ============================================================================

#ifndef SOUND_H
#define SOUND_H

#include "system.h"


/* values for platform specific sound initialization */
#define AUDIO_SAMPLE_RATE_22050		22050

#define AUDIO_FRAGMENT_SIZE_512		512
#define AUDIO_FRAGMENT_SIZE_1024	1024
#define AUDIO_FRAGMENT_SIZE_2048	2048
#define AUDIO_FRAGMENT_SIZE_4096	4096
#define AUDIO_FRAGMENT_SIZE_32768	32768

#define AUDIO_NUM_CHANNELS_MONO		1
#define AUDIO_NUM_CHANNELS_STEREO	2

#define AUDIO_FORMAT_UNKNOWN		(0)
#define AUDIO_FORMAT_U8			(1 << 0)
#define AUDIO_FORMAT_S16		(1 << 1)
#define AUDIO_FORMAT_LE			(1 << 2)
#define AUDIO_FORMAT_BE			(1 << 3)

#define DEFAULT_AUDIO_SAMPLE_RATE	AUDIO_SAMPLE_RATE_22050

#if defined(PLATFORM_WIN32)
#define DEFAULT_AUDIO_FRAGMENT_SIZE	AUDIO_FRAGMENT_SIZE_1024
#else
#define DEFAULT_AUDIO_FRAGMENT_SIZE	AUDIO_FRAGMENT_SIZE_512
#endif

#define NUM_MIXER_CHANNELS		MIX_CHANNELS

#define MUSIC_CHANNEL			0
#define FIRST_SOUND_CHANNEL		1


/* values for PlaySound(), StopSound() and friends */
#define SND_CTRL_NONE			(0)
#define SND_CTRL_MUSIC			(1 << 0)
#define SND_CTRL_LOOP			(1 << 1)
#define SND_CTRL_FADE			(1 << 2)
#define SND_CTRL_STOP			(1 << 3)
#define SND_CTRL_ALL_SOUNDS		(1 << 4)
#define SND_CTRL_RELOAD_SOUNDS		(1 << 5)
#define SND_CTRL_RELOAD_MUSIC		(1 << 6)
#define SND_CTRL_EXPIRE_LOOPS		(1 << 7)

#define SND_CTRL_PLAY_SOUND		(SND_CTRL_NONE)
#define SND_CTRL_PLAY_LOOP		(SND_CTRL_LOOP)
#define SND_CTRL_PLAY_MUSIC		(SND_CTRL_LOOP | SND_CTRL_MUSIC)

#define SND_CTRL_FADE_SOUND		(SND_CTRL_FADE)
#define SND_CTRL_FADE_MUSIC		(SND_CTRL_FADE | SND_CTRL_MUSIC)
#define SND_CTRL_FADE_ALL		(SND_CTRL_FADE | SND_CTRL_ALL_SOUNDS)

#define SND_CTRL_STOP_SOUND		(SND_CTRL_STOP)
#define SND_CTRL_STOP_MUSIC		(SND_CTRL_STOP | SND_CTRL_MUSIC)
#define SND_CTRL_STOP_ALL		(SND_CTRL_STOP | SND_CTRL_ALL_SOUNDS)

#define IS_MUSIC(x)			((x).state & SND_CTRL_MUSIC)
#define IS_LOOP(x)			((x).state & SND_CTRL_LOOP)
#define IS_FADING(x)			((x).state & SND_CTRL_FADE)
#define IS_STOPPING(x)			((x).state & SND_CTRL_STOP)
#define IS_RELOADING(x)			((x).state & (SND_CTRL_RELOAD_SOUNDS |\
						      SND_CTRL_RELOAD_MUSIC))
#define ALL_SOUNDS(x)			((x).state & SND_CTRL_ALL_SOUNDS)
#define SET_EXPIRE_LOOPS(x)		((x).state & SND_CTRL_EXPIRE_LOOPS)

#define MAP_NOCONF_MUSIC(x)		(-((x) + 1))
#define UNMAP_NOCONF_MUSIC(x)		MAP_NOCONF_MUSIC(x)


#define SOUND_MIN_VOLUME		0
#define SOUND_MAX_VOLUME		SDL_MIX_MAXVOLUME

#define SOUND_MAX_LEFT			0
#define SOUND_MAX_RIGHT			255
#define SOUND_MAX_LEFT2RIGHT		255
#define SOUND_MIDDLE			(SOUND_MAX_LEFT2RIGHT / 2)


/* general sound functions */
void UnixOpenAudio(void);
void UnixCloseAudio(void);

/* mixer functions */ 
void Mixer_InitChannels(void);
void StartMixer(void);

/* sound client functions */
void PlayMusic(int);
void PlaySound(int);
void PlaySoundStereo(int, int);
void PlaySoundLoop(int);
void PlaySoundMusic(int);
void PlaySoundExt(int, int, int, int);
void FadeMusic(void);
void FadeSound(int);
void FadeSounds(void);
void FadeSoundsAndMusic(void);
void StopMusic(void);
void StopSound(int);
void StopSounds(void);
void StopSoundExt(int, int);
void ExpireSoundLoops(boolean);

int getSoundListSize();
int getMusicListSize();
struct FileInfo *getSoundListEntry(int);
struct FileInfo *getMusicListEntry(int);
char *getMusicInfoEntryFilename(int);
char *getCurrentlyPlayingMusicFilename();
int getSoundListPropertyMappingSize();
int getMusicListPropertyMappingSize();
struct PropertyMapping *getSoundListPropertyMapping();
struct PropertyMapping *getMusicListPropertyMapping();
void InitSoundList(struct ConfigInfo *, int, struct ConfigTypeInfo *,
		   char **, char **, char **, char **, char **);
void InitMusicList(struct ConfigInfo *, int, struct ConfigTypeInfo *,
		   char **, char **, char **, char **, char **);
void InitReloadCustomSounds(char *);
void InitReloadCustomMusic(char *);
void FreeAllSounds(void);
void FreeAllMusic(void);

#endif
