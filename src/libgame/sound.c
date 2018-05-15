// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// sound.c
// ============================================================================

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <math.h>
#include <errno.h>

#include "platform.h"
#include "system.h"
#include "sound.h"
#include "misc.h"
#include "setup.h"
#include "text.h"


/* expiration time (in milliseconds) for sound loops */
#define SOUND_LOOP_EXPIRATION_TIME	200

/* one second fading interval == 1000 ticks (milliseconds) */
#define SOUND_FADING_INTERVAL		1000

#define SND_TYPE_NONE			0
#define SND_TYPE_WAV			1

#define MUS_TYPE_NONE			0
#define MUS_TYPE_WAV			1
#define MUS_TYPE_MOD			2

#define DEVICENAME_DSP			"/dev/dsp"
#define DEVICENAME_SOUND_DSP		"/dev/sound/dsp"
#define DEVICENAME_AUDIO		"/dev/audio"
#define DEVICENAME_AUDIOCTL		"/dev/audioCtl"

#define SOUND_VOLUME_LEFT(x)		(stereo_volume[x])
#define SOUND_VOLUME_RIGHT(x)		(stereo_volume[SOUND_MAX_LEFT2RIGHT-x])

#define SAME_SOUND_NR(x,y)		((x).nr == (y).nr)
#define SAME_SOUND_DATA(x,y)		((x).data_ptr == (y).data_ptr)

#define SOUND_VOLUME_FROM_PERCENT(v,p)	((p) < 0   ? SOUND_MIN_VOLUME :	\
					 (p) > 100 ? (v) :		\
					 (p) * (v) / 100)

#define SOUND_VOLUME_SIMPLE(v) SOUND_VOLUME_FROM_PERCENT(v, setup.volume_simple)
#define SOUND_VOLUME_LOOPS(v)  SOUND_VOLUME_FROM_PERCENT(v, setup.volume_loops)
#define SOUND_VOLUME_MUSIC(v)  SOUND_VOLUME_FROM_PERCENT(v, setup.volume_music)

#define SETUP_SOUND_VOLUME(v,s)		((s) == SND_CTRL_PLAY_MUSIC ?	\
					 SOUND_VOLUME_MUSIC(v) :	\
					 (s) == SND_CTRL_PLAY_LOOP ?	\
					 SOUND_VOLUME_LOOPS(v) :	\
					 SOUND_VOLUME_SIMPLE(v))

struct AudioFormatInfo
{
  boolean stereo;		/* availability of stereo sound */
  int format;			/* size and endianess of sample data */
  int sample_rate;		/* sample frequency */
  int fragment_size;		/* audio device fragment size in bytes */
};

struct SampleInfo
{
  char *source_filename;
  int num_references;

  int type;
  int format;
  void *data_ptr;		/* pointer to first sample (8 or 16 bit) */
  int data_len;			/* number of samples, NOT number of bytes */
  int num_channels;		/* mono: 1 channel, stereo: 2 channels */
};
typedef struct SampleInfo SoundInfo;
typedef struct SampleInfo MusicInfo;

struct SoundControl
{
  boolean active;

  int nr;
  int volume;
  int stereo_position;

  int state;

  unsigned int playing_starttime;
  unsigned int playing_pos;

  int type;
  int format;
  void *data_ptr;		/* pointer to first sample (8 or 16 bit) */
  int data_len;		/* number of samples, NOT number of bytes */
  int num_channels;		/* mono: 1 channel, stereo: 2 channels */
};
typedef struct SoundControl SoundControl;

static struct ArtworkListInfo *sound_info = NULL;
static struct ArtworkListInfo *music_info = NULL;

static MusicInfo **Music_NoConf = NULL;

static int num_music_noconf = 0;
static int stereo_volume[SOUND_MAX_LEFT2RIGHT + 1];

static char *currently_playing_music_filename = NULL;


/* ========================================================================= */
/* THE STUFF BELOW IS ONLY USED BY THE SOUND SERVER CHILD PROCESS            */

static struct SoundControl mixer[NUM_MIXER_CHANNELS];
static int mixer_active_channels = 0;
static boolean expire_loop_sounds = FALSE;

static void ReloadCustomSounds();
static void ReloadCustomMusic();
static void FreeSound(void *);
static void FreeMusic(void *);
static void FreeAllMusic_NoConf();

static SoundInfo *getSoundInfoEntryFromSoundID(int);
static MusicInfo *getMusicInfoEntryFromMusicID(int);


/* ------------------------------------------------------------------------- */
/* mixer functions                                                           */
/* ------------------------------------------------------------------------- */

void Mixer_InitChannels()
{
  int i;

  for (i = 0; i < audio.num_channels; i++)
    mixer[i].active = FALSE;
  mixer_active_channels = 0;
}

static void Mixer_ResetChannelExpiration(int channel)
{
  mixer[channel].playing_starttime = Counter();

  if (expire_loop_sounds &&
      IS_LOOP(mixer[channel]) && !IS_MUSIC(mixer[channel]))
    Mix_ExpireChannel(channel, SOUND_LOOP_EXPIRATION_TIME);
}

static boolean Mixer_ChannelExpired(int channel)
{
  if (!mixer[channel].active)
    return TRUE;

  if (expire_loop_sounds &&
      IS_LOOP(mixer[channel]) && !IS_MUSIC(mixer[channel]) &&
      DelayReached(&mixer[channel].playing_starttime,
		   SOUND_LOOP_EXPIRATION_TIME))
    return TRUE;

  if (!Mix_Playing(channel))
    return TRUE;

  return FALSE;
}

static boolean Mixer_AllocateChannel(int channel)
{
  return TRUE;
}

static void Mixer_SetChannelProperties(int channel)
{
  Mix_Volume(channel, mixer[channel].volume);
  Mix_SetPanning(channel,
		 SOUND_VOLUME_LEFT(mixer[channel].stereo_position),
		 SOUND_VOLUME_RIGHT(mixer[channel].stereo_position));
}

static void Mixer_StartChannel(int channel)
{
  Mix_PlayChannel(channel, mixer[channel].data_ptr,
		  IS_LOOP(mixer[channel]) ? -1 : 0);
}

static void Mixer_PlayChannel(int channel)
{
  /* start with inactive channel in case something goes wrong */
  mixer[channel].active = FALSE;

  if (mixer[channel].type != MUS_TYPE_WAV)
    return;

  if (!Mixer_AllocateChannel(channel))
    return;

  Mixer_SetChannelProperties(channel);
  Mixer_StartChannel(channel);

  Mixer_ResetChannelExpiration(channel);

  mixer[channel].playing_pos = 0;
  mixer[channel].active = TRUE;
  mixer_active_channels++;
}

static void Mixer_PlayMusicChannel()
{
  Mixer_PlayChannel(audio.music_channel);

  if (mixer[audio.music_channel].type != MUS_TYPE_WAV)
  {
    // use short fade-in to prevent "plop" sound for certain music files
    // (this may happen when switching on music while playing the game)
    Mix_VolumeMusic(mixer[audio.music_channel].volume);
    Mix_FadeInMusic(mixer[audio.music_channel].data_ptr, -1, 100);

#if defined(PLATFORM_WIN32)
    // playing MIDI music is broken since Windows Vista, as it sets the volume
    // for MIDI music also for all other sounds and music, which cannot be set
    // back to normal unless playing MIDI music again with that desired volume
    // (more details: https://www.artsoft.org/forum/viewtopic.php?f=7&t=2253)
    // => workaround: always play MIDI music with maximum volume
    if (Mix_GetMusicType(NULL) == MUS_MID)
      Mix_VolumeMusic(SOUND_MAX_VOLUME);
#endif
  }
}

static void Mixer_StopChannel(int channel)
{
  if (!mixer[channel].active)
    return;

  Mix_HaltChannel(channel);

  mixer[channel].active = FALSE;
  mixer_active_channels--;
}

static void Mixer_StopMusicChannel()
{
  Mixer_StopChannel(audio.music_channel);

  Mix_HaltMusic();

  setString(&currently_playing_music_filename, NULL);
}

static void Mixer_FadeChannel(int channel)
{
  if (!mixer[channel].active)
    return;

  mixer[channel].state |= SND_CTRL_FADE;

  Mix_FadeOutChannel(channel, SOUND_FADING_INTERVAL);
}

static void Mixer_FadeMusicChannel()
{
  Mixer_FadeChannel(audio.music_channel);

  Mix_FadeOutMusic(SOUND_FADING_INTERVAL);

#if defined(PLATFORM_WIN32)
  // playing MIDI music is broken since Windows Vista, as it sets the volume
  // for MIDI music also for all other sounds and music, which cannot be set
  // back to normal unless playing MIDI music again with that desired volume
  // (more details: https://www.artsoft.org/forum/viewtopic.php?f=7&t=2253)
  // => workaround: never fade MIDI music to lower volume, but just stop it
  if (Mix_GetMusicType(NULL) == MUS_MID)
    Mixer_StopMusicChannel();
#endif

  setString(&currently_playing_music_filename, NULL);
}

static void Mixer_UnFadeChannel(int channel)
{
  if (!mixer[channel].active || !IS_FADING(mixer[channel]))
    return;

  mixer[channel].state &= ~SND_CTRL_FADE;
  mixer[channel].volume = SOUND_MAX_VOLUME;

  Mix_ExpireChannel(channel, -1);
  Mix_Volume(channel, mixer[channel].volume);
}

static void Mixer_InsertSound(SoundControl snd_ctrl)
{
  SoundInfo *snd_info;
  int i, k;

  if (IS_MUSIC(snd_ctrl))
    snd_info = getMusicInfoEntryFromMusicID(snd_ctrl.nr);
  else
    snd_info = getSoundInfoEntryFromSoundID(snd_ctrl.nr);

  if (snd_info == NULL)
    return;

  /* copy sound sample and format information */
  snd_ctrl.type         = snd_info->type;
  snd_ctrl.format       = snd_info->format;
  snd_ctrl.data_ptr     = snd_info->data_ptr;
  snd_ctrl.data_len     = snd_info->data_len;
  snd_ctrl.num_channels = snd_info->num_channels;

  /* play music samples on a dedicated music channel */
  if (IS_MUSIC(snd_ctrl))
  {
    Mixer_StopMusicChannel();

    mixer[audio.music_channel] = snd_ctrl;
    Mixer_PlayMusicChannel();

    setString(&currently_playing_music_filename,
	      getBaseNamePtr(snd_info->source_filename));

    return;
  }

  /* check if (and how often) this sound sample is already playing */
  for (k = 0, i = audio.first_sound_channel; i < audio.num_channels; i++)
    if (mixer[i].active && SAME_SOUND_DATA(mixer[i], snd_ctrl))
      k++;

  /* reset expiration delay for already playing loop sounds */
  if (k > 0 && IS_LOOP(snd_ctrl))
  {
    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
    {
      if (mixer[i].active && SAME_SOUND_DATA(mixer[i], snd_ctrl))
      {
	if (IS_FADING(mixer[i]))
	  Mixer_UnFadeChannel(i);

	/* restore settings like volume and stereo position */
	mixer[i].volume = snd_ctrl.volume;
	mixer[i].stereo_position = snd_ctrl.stereo_position;

	Mixer_SetChannelProperties(i);
	Mixer_ResetChannelExpiration(i);
      }
    }

    return;
  }

  /* don't play sound more than n times simultaneously (with n == 2 for now) */
  if (k >= 2)
  {
    unsigned int playing_current = Counter();
    int longest = 0, longest_nr = audio.first_sound_channel;

    /* look for oldest equal sound */
    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
    {
      int playing_time = playing_current - mixer[i].playing_starttime;
      int actual;

      if (!mixer[i].active || !SAME_SOUND_NR(mixer[i], snd_ctrl))
	continue;

      actual = 1000 * playing_time / mixer[i].data_len;

      if (actual >= longest)
      {
	longest = actual;
	longest_nr = i;
      }
    }

    Mixer_StopChannel(longest_nr);
  }

  /* If all (non-music) channels are active, stop the channel that has
     played its sound sample most completely (in percent of the sample
     length). As we cannot currently get the actual playing position
     of the channel's sound sample when compiling with the SDL mixer
     library, we use the current playing time (in milliseconds) instead. */

#if DEBUG
  /* channel allocation sanity check -- should not be needed */
  if (mixer_active_channels ==
      audio.num_channels - (mixer[audio.music_channel].active ? 0 : 1))
  {
    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
    {
      if (!mixer[i].active)
      {
	Error(ERR_INFO, "Mixer_InsertSound: Channel %d inactive", i);
	Error(ERR_INFO, "Mixer_InsertSound: This should never happen!");

	mixer_active_channels--;
      }
    }
  }
#endif

  if (mixer_active_channels ==
      audio.num_channels - (mixer[audio.music_channel].active ? 0 : 1))
  {
    unsigned int playing_current = Counter();
    int longest = 0, longest_nr = audio.first_sound_channel;

    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
    {
      int playing_time = playing_current - mixer[i].playing_starttime;
      int actual = 1000 * playing_time / mixer[i].data_len;

      if (!IS_LOOP(mixer[i]) && actual > longest)
      {
	longest = actual;
	longest_nr = i;
      }
    }

    Mixer_StopChannel(longest_nr);
  }

  /* add the new sound to the mixer */
  for (i = audio.first_sound_channel; i < audio.num_channels; i++)
  {
    if (!mixer[i].active)
    {
      mixer[i] = snd_ctrl;
      Mixer_PlayChannel(i);

      break;
    }
  }
}

static void HandleSoundRequest(SoundControl snd_ctrl)
{
  int i;

  /* deactivate channels that have expired since the last request */
  for (i = 0; i < audio.num_channels; i++)
    if (mixer[i].active && Mixer_ChannelExpired(i))
      Mixer_StopChannel(i);

  if (IS_RELOADING(snd_ctrl))		/* load new sound or music files */
  {
    Mixer_StopMusicChannel();
    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
      Mixer_StopChannel(i);

    if (snd_ctrl.state & SND_CTRL_RELOAD_SOUNDS)
      ReloadCustomSounds();
    else
      ReloadCustomMusic();
  }
  else if (IS_FADING(snd_ctrl))		/* fade out existing sound or music */
  {
    if (IS_MUSIC(snd_ctrl))
    {
      Mixer_FadeMusicChannel();
      return;
    }

    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
      if (SAME_SOUND_NR(mixer[i], snd_ctrl) || ALL_SOUNDS(snd_ctrl))
	Mixer_FadeChannel(i);
  }
  else if (IS_STOPPING(snd_ctrl))	/* stop existing sound or music */
  {
    if (IS_MUSIC(snd_ctrl))
    {
      Mixer_StopMusicChannel();
      return;
    }

    for (i = audio.first_sound_channel; i < audio.num_channels; i++)
      if (SAME_SOUND_NR(mixer[i], snd_ctrl) || ALL_SOUNDS(snd_ctrl))
	Mixer_StopChannel(i);
  }
  else if (SET_EXPIRE_LOOPS(snd_ctrl))	/* set loop expiration on or off */
  {
    expire_loop_sounds = snd_ctrl.active;
  }
  else if (snd_ctrl.active)		/* add new sound to mixer */
  {
    Mixer_InsertSound(snd_ctrl);
  }
}

void StartMixer(void)
{
  int i;

  if (!audio.sound_available)
    return;

  /* initialize stereo position conversion information */
  for (i = 0; i <= SOUND_MAX_LEFT2RIGHT; i++)
    stereo_volume[i] =
      (int)sqrt((float)(SOUND_MAX_LEFT2RIGHT * SOUND_MAX_LEFT2RIGHT - i * i));
}


/* THE STUFF ABOVE IS ONLY USED BY THE SOUND SERVER CHILD PROCESS            */
/* ========================================================================= */
/* THE STUFF BELOW IS ONLY USED BY THE MAIN PROCESS                          */

#define CHUNK_ID_LEN            4       /* IFF style chunk id length */
#define WAV_HEADER_SIZE		16	/* size of WAV file header */

static void *Load_WAV(char *filename)
{
  SoundInfo *snd_info;

  if (!audio.sound_available)
    return NULL;

  snd_info = checked_calloc(sizeof(SoundInfo));

  if ((snd_info->data_ptr = Mix_LoadWAV(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot read sound file '%s': %s", filename, Mix_GetError());
    free(snd_info);
    return NULL;
  }

  snd_info->data_len = ((Mix_Chunk *)snd_info->data_ptr)->alen;

  snd_info->type = SND_TYPE_WAV;
  snd_info->source_filename = getStringCopy(filename);

  return snd_info;
}

static void *Load_MOD(char *filename)
{
  MusicInfo *mod_info;

  if (!audio.sound_available)
    return NULL;

  mod_info = checked_calloc(sizeof(MusicInfo));

  if ((mod_info->data_ptr = Mix_LoadMUS(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot read music file '%s': %s", filename, Mix_GetError());
    free(mod_info);
    return NULL;
  }

  mod_info->type = MUS_TYPE_MOD;
  mod_info->source_filename = getStringCopy(filename);

  return mod_info;
}

static void *Load_WAV_or_MOD(char *filename)
{
  if (FileIsMusic(filename))
    return Load_MOD(filename);
  else if (FileIsSound(filename))
    return Load_WAV(filename);
  else
    return NULL;
}

void LoadCustomMusic_NoConf(void)
{
  static boolean draw_init_text = TRUE;		/* only draw at startup */
  static char *last_music_directory = NULL;
  char *music_directory = getCustomMusicDirectory();
  Directory *dir;
  DirectoryEntry *dir_entry;
  int num_music = getMusicListSize();

  if (!audio.sound_available)
    return;

  if (last_music_directory != NULL &&
      strEqual(last_music_directory, music_directory))
    return;	/* old and new music directory are the same */

  if (last_music_directory != NULL)
    free(last_music_directory);
  last_music_directory = getStringCopy(music_directory);

  FreeAllMusic_NoConf();

  if ((dir = openDirectory(music_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read music directory '%s'", music_directory);

    audio.music_available = FALSE;

    return;
  }

  if (draw_init_text)
    DrawInitText("Loading music", 120, FC_GREEN);

  while ((dir_entry = readDirectory(dir)) != NULL)	/* loop all entries */
  {
    char *basename = dir_entry->basename;
    MusicInfo *mus_info = NULL;
    boolean music_already_used = FALSE;
    int i;

    /* skip all music files that are configured in music config file */
    for (i = 0; i < num_music; i++)
    {
      struct FileInfo *music = getMusicListEntry(i);

      if (strEqual(basename, music->filename))
      {
	music_already_used = TRUE;
	break;
      }
    }

    if (music_already_used)
      continue;

    if (draw_init_text)
      DrawInitText(basename, 150, FC_YELLOW);

    if (FileIsMusic(dir_entry->filename))
      mus_info = Load_WAV_or_MOD(dir_entry->filename);

    if (mus_info)
    {
      num_music_noconf++;
      Music_NoConf = checked_realloc(Music_NoConf,
				     num_music_noconf * sizeof(MusicInfo *));
      Music_NoConf[num_music_noconf - 1] = mus_info;
    }
  }

  closeDirectory(dir);

  draw_init_text = FALSE;
}

int getSoundListSize()
{
  return (sound_info->num_file_list_entries +
	  sound_info->num_dynamic_file_list_entries);
}

int getMusicListSize()
{
  return (music_info->num_file_list_entries +
	  music_info->num_dynamic_file_list_entries);
}

struct FileInfo *getSoundListEntry(int pos)
{
  int num_sounds = getSoundListSize();
  int num_list_entries = sound_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);

  if (pos < 0 || pos >= num_sounds)	/* invalid sound */
    return NULL;

  return (pos < num_list_entries ? &sound_info->file_list[list_pos] :
	  &sound_info->dynamic_file_list[list_pos]);
}

struct FileInfo *getMusicListEntry(int pos)
{
  int num_music = getMusicListSize();
  int num_list_entries = music_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);

  if (pos < 0 || pos >= num_music)	/* invalid music */
    return NULL;

  return (pos < num_list_entries ? &music_info->file_list[list_pos] :
	  &music_info->dynamic_file_list[list_pos]);
}

static SoundInfo *getSoundInfoEntryFromSoundID(int pos)
{
  int num_sounds = getSoundListSize();
  int num_list_entries = sound_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);
  SoundInfo **snd_info =
    (SoundInfo **)(pos < num_list_entries ? sound_info->artwork_list :
		   sound_info->dynamic_artwork_list);

  if (pos < 0 || pos >= num_sounds)	/* invalid sound */
    return NULL;

  return snd_info[list_pos];
}

static MusicInfo *getMusicInfoEntryFromMusicID(int pos)
{
  int num_music = getMusicListSize();
  int num_list_entries = music_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);
  MusicInfo **mus_info =
    (MusicInfo **)(pos < num_list_entries ? music_info->artwork_list :
		   music_info->dynamic_artwork_list);

  if (pos >= num_music)			/* invalid music */
    return NULL;

  if (pos < 0)				/* undefined music */
  {
    if (num_music_noconf == 0)		/* no fallback music available */
      return NULL;

    pos = UNMAP_NOCONF_MUSIC(pos) % num_music_noconf;

    return Music_NoConf[pos];
  }

  return mus_info[list_pos];
}

char *getMusicInfoEntryFilename(int pos)
{
  MusicInfo *mus_info = getMusicInfoEntryFromMusicID(pos);

  if (mus_info == NULL)
    return NULL;

  return getBaseNamePtr(mus_info->source_filename);
}

char *getCurrentlyPlayingMusicFilename()
{
  return currently_playing_music_filename;
}

int getSoundListPropertyMappingSize()
{
  return sound_info->num_property_mapping_entries;
}

int getMusicListPropertyMappingSize()
{
  return music_info->num_property_mapping_entries;
}

struct PropertyMapping *getSoundListPropertyMapping()
{
  return sound_info->property_mapping;
}

struct PropertyMapping *getMusicListPropertyMapping()
{
  return music_info->property_mapping;
}

void InitSoundList(struct ConfigInfo *config_list, int num_file_list_entries,
		   struct ConfigTypeInfo *config_suffix_list,
		   char **base_prefixes, char **ext1_suffixes,
		   char **ext2_suffixes, char **ext3_suffixes,
		   char **ignore_tokens)
{
  int i;

  sound_info = checked_calloc(sizeof(struct ArtworkListInfo));
  sound_info->type = ARTWORK_TYPE_SOUNDS;

  /* ---------- initialize file list and suffix lists ---------- */

  sound_info->num_file_list_entries = num_file_list_entries;
  sound_info->num_dynamic_file_list_entries = 0;

  sound_info->file_list =
    getFileListFromConfigList(config_list, config_suffix_list, ignore_tokens,
			      num_file_list_entries);
  sound_info->dynamic_file_list = NULL;

  sound_info->num_suffix_list_entries = 0;
  for (i = 0; config_suffix_list[i].token != NULL; i++)
    sound_info->num_suffix_list_entries++;

  sound_info->suffix_list = config_suffix_list;

  /* ---------- initialize base prefix and suffixes lists ---------- */

  sound_info->num_base_prefixes = 0;
  for (i = 0; base_prefixes[i] != NULL; i++)
    sound_info->num_base_prefixes++;

  sound_info->num_ext1_suffixes = 0;
  for (i = 0; ext1_suffixes[i] != NULL; i++)
    sound_info->num_ext1_suffixes++;

  sound_info->num_ext2_suffixes = 0;
  for (i = 0; ext2_suffixes[i] != NULL; i++)
    sound_info->num_ext2_suffixes++;

  sound_info->num_ext3_suffixes = 0;
  for (i = 0; ext3_suffixes[i] != NULL; i++)
    sound_info->num_ext3_suffixes++;

  sound_info->num_ignore_tokens = 0;
  for (i = 0; ignore_tokens[i] != NULL; i++)
    sound_info->num_ignore_tokens++;

  sound_info->base_prefixes = base_prefixes;
  sound_info->ext1_suffixes = ext1_suffixes;
  sound_info->ext2_suffixes = ext2_suffixes;
  sound_info->ext3_suffixes = ext3_suffixes;
  sound_info->ignore_tokens = ignore_tokens;

  sound_info->num_property_mapping_entries = 0;

  sound_info->property_mapping = NULL;

  /* ---------- initialize artwork reference and content lists ---------- */

  sound_info->sizeof_artwork_list_entry = sizeof(SoundInfo *);

  sound_info->artwork_list =
    checked_calloc(num_file_list_entries * sizeof(SoundInfo *));
  sound_info->dynamic_artwork_list = NULL;

  sound_info->content_list = NULL;

  /* ---------- initialize artwork loading/freeing functions ---------- */

  sound_info->load_artwork = Load_WAV;
  sound_info->free_artwork = FreeSound;
}

void InitMusicList(struct ConfigInfo *config_list, int num_file_list_entries,
		   struct ConfigTypeInfo *config_suffix_list,
		   char **base_prefixes, char **ext1_suffixes,
		   char **ext2_suffixes, char **ext3_suffixes,
		   char **ignore_tokens)
{
  int i;

  music_info = checked_calloc(sizeof(struct ArtworkListInfo));
  music_info->type = ARTWORK_TYPE_MUSIC;

  /* ---------- initialize file list and suffix lists ---------- */

  music_info->num_file_list_entries = num_file_list_entries;
  music_info->num_dynamic_file_list_entries = 0;

  music_info->file_list =
    getFileListFromConfigList(config_list, config_suffix_list, ignore_tokens,
			      num_file_list_entries);
  music_info->dynamic_file_list = NULL;

  music_info->num_suffix_list_entries = 0;
  for (i = 0; config_suffix_list[i].token != NULL; i++)
    music_info->num_suffix_list_entries++;

  music_info->suffix_list = config_suffix_list;

  /* ---------- initialize base prefix and suffixes lists ---------- */

  music_info->num_base_prefixes = 0;
  for (i = 0; base_prefixes[i] != NULL; i++)
    music_info->num_base_prefixes++;

  music_info->num_ext1_suffixes = 0;
  for (i = 0; ext1_suffixes[i] != NULL; i++)
    music_info->num_ext1_suffixes++;

  music_info->num_ext2_suffixes = 0;
  for (i = 0; ext2_suffixes[i] != NULL; i++)
    music_info->num_ext2_suffixes++;

  music_info->num_ext3_suffixes = 0;
  for (i = 0; ext3_suffixes[i] != NULL; i++)
    music_info->num_ext3_suffixes++;

  music_info->num_ignore_tokens = 0;
  for (i = 0; ignore_tokens[i] != NULL; i++)
    music_info->num_ignore_tokens++;

  music_info->base_prefixes = base_prefixes;
  music_info->ext1_suffixes = ext1_suffixes;
  music_info->ext2_suffixes = ext2_suffixes;
  music_info->ext3_suffixes = ext3_suffixes;
  music_info->ignore_tokens = ignore_tokens;

  music_info->num_property_mapping_entries = 0;

  music_info->property_mapping = NULL;

  /* ---------- initialize artwork reference and content lists ---------- */

  music_info->sizeof_artwork_list_entry = sizeof(MusicInfo *);

  music_info->artwork_list =
    checked_calloc(num_file_list_entries * sizeof(MusicInfo *));
  music_info->dynamic_artwork_list = NULL;

  music_info->content_list = NULL;

  /* ---------- initialize artwork loading/freeing functions ---------- */

  music_info->load_artwork = Load_WAV_or_MOD;
  music_info->free_artwork = FreeMusic;
}

void PlayMusic(int nr)
{
  if (!audio.music_available)
    return;

  PlaySoundMusic(nr);
}

void PlaySound(int nr)
{
  if (!setup.sound_simple)
    return;

  PlaySoundExt(nr, SOUND_MAX_VOLUME, SOUND_MIDDLE, SND_CTRL_PLAY_SOUND);
}

void PlaySoundStereo(int nr, int stereo_position)
{
  if (!setup.sound_simple)
    return;

  PlaySoundExt(nr, SOUND_MAX_VOLUME, stereo_position, SND_CTRL_PLAY_SOUND);
}

void PlaySoundLoop(int nr)
{
  if (!setup.sound_loops)
    return;

  PlaySoundExt(nr, SOUND_MAX_VOLUME, SOUND_MIDDLE, SND_CTRL_PLAY_LOOP);
}

void PlaySoundMusic(int nr)
{
  if (!setup.sound_music)
    return;

  PlaySoundExt(nr, SOUND_MAX_VOLUME, SOUND_MIDDLE, SND_CTRL_PLAY_MUSIC);
}

void PlaySoundExt(int nr, int volume, int stereo_position, int state)
{
  SoundControl snd_ctrl;

  if (!audio.sound_available ||
      !audio.sound_enabled ||
      audio.sound_deactivated)
    return;

  volume = SETUP_SOUND_VOLUME(volume, state);

  if (volume < SOUND_MIN_VOLUME)
    volume = SOUND_MIN_VOLUME;
  else if (volume > SOUND_MAX_VOLUME)
    volume = SOUND_MAX_VOLUME;

  if (stereo_position < SOUND_MAX_LEFT)
    stereo_position = SOUND_MAX_LEFT;
  else if (stereo_position > SOUND_MAX_RIGHT)
    stereo_position = SOUND_MAX_RIGHT;

  clear_mem(&snd_ctrl, sizeof(SoundControl));	/* to make valgrind happy */

  snd_ctrl.active = TRUE;
  snd_ctrl.nr = nr;
  snd_ctrl.volume = volume;
  snd_ctrl.stereo_position = stereo_position;
  snd_ctrl.state = state;

  HandleSoundRequest(snd_ctrl);
}

void FadeMusic(void)
{
  if (!audio.music_available)
    return;

  StopSoundExt(-1, SND_CTRL_FADE_MUSIC);
}

void FadeSound(int nr)
{
  StopSoundExt(nr, SND_CTRL_FADE_SOUND);
}

void FadeSounds()
{
  StopSoundExt(-1, SND_CTRL_FADE_ALL);
}

void FadeSoundsAndMusic()
{
  FadeSounds();
  FadeMusic();
}

void StopMusic(void)
{
  if (!audio.music_available)
    return;

  StopSoundExt(-1, SND_CTRL_STOP_MUSIC);
}

void StopSound(int nr)
{
  StopSoundExt(nr, SND_CTRL_STOP_SOUND);
}

void StopSounds()
{
  StopMusic();
  StopSoundExt(-1, SND_CTRL_STOP_ALL);
}

void StopSoundExt(int nr, int state)
{
  SoundControl snd_ctrl;

  if (!audio.sound_available)
    return;

  clear_mem(&snd_ctrl, sizeof(SoundControl));	/* to make valgrind happy */

  snd_ctrl.active = FALSE;
  snd_ctrl.nr = nr;
  snd_ctrl.state = state;

  HandleSoundRequest(snd_ctrl);
}

void ExpireSoundLoops(boolean active)
{
  SoundControl snd_ctrl;

  if (!audio.sound_available)
    return;

  clear_mem(&snd_ctrl, sizeof(SoundControl));	/* to make valgrind happy */

  snd_ctrl.active = active;
  snd_ctrl.state = SND_CTRL_EXPIRE_LOOPS;

  HandleSoundRequest(snd_ctrl);
}

static void ReloadCustomSounds()
{
  LoadArtworkConfig(sound_info);
  ReloadCustomArtworkList(sound_info);
}

static void ReloadCustomMusic()
{
  LoadArtworkConfig(music_info);
  ReloadCustomArtworkList(music_info);

  /* load all music files from directory not defined in "musicinfo.conf" */
  LoadCustomMusic_NoConf();
}

void InitReloadCustomSounds(char *set_identifier)
{
  if (!audio.sound_available)
    return;

  ReloadCustomSounds();
}

void InitReloadCustomMusic(char *set_identifier)
{
  if (!audio.music_available)
    return;

  ReloadCustomMusic();
}

void FreeSound(void *ptr)
{
  SoundInfo *sound = (SoundInfo *)ptr;

  if (sound == NULL)
    return;

  if (sound->data_ptr)
  {
    Mix_FreeChunk(sound->data_ptr);
  }

  checked_free(sound->source_filename);

  free(sound);
}

void FreeMusic(void *ptr)
{
  MusicInfo *music = (MusicInfo *)ptr;

  if (music == NULL)
    return;

  if (music->data_ptr)
  {
    if (music->type == MUS_TYPE_MOD)
      Mix_FreeMusic(music->data_ptr);
    else
      Mix_FreeChunk(music->data_ptr);
  }

  free(music);
}

static void FreeAllMusic_NoConf()
{
  int i;

  if (Music_NoConf == NULL)
    return;

  for (i = 0; i < num_music_noconf; i++)
    FreeMusic(Music_NoConf[i]);

  free(Music_NoConf);

  Music_NoConf = NULL;
  num_music_noconf = 0;
}

void FreeAllSounds()
{
  FreeCustomArtworkLists(sound_info);
}

void FreeAllMusic()
{
  FreeCustomArtworkLists(music_info);
  FreeAllMusic_NoConf();
}

/* THE STUFF ABOVE IS ONLY USED BY THE MAIN PROCESS                          */
/* ========================================================================= */
