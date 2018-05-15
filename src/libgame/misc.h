// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// misc.h
// ============================================================================

#ifndef MISC_H
#define MISC_H

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include "system.h"


/* values for InitCounter() and Counter() */
#define INIT_COUNTER			0
#define READ_COUNTER			1

/* values for InitRND() */
#define NEW_RANDOMIZE			0

#define RANDOM_ENGINE			0
#define RANDOM_SIMPLE			1

#define InitEngineRandom(seed)		init_random_number(RANDOM_ENGINE, seed)
#define InitSimpleRandom(seed)		init_random_number(RANDOM_SIMPLE, seed)
#define GetEngineRandom(max)		get_random_number(RANDOM_ENGINE, max)
#define GetSimpleRandom(max)		get_random_number(RANDOM_SIMPLE, max)

/* values for Error() */
#define ERR_UNKNOWN			0
#define ERR_DEBUG			(1 << 0)
#define ERR_INFO			(1 << 1)
#define ERR_INFO_LINE			(1 << 2)
#define ERR_WARN			(1 << 3)
#define ERR_EXIT			(1 << 4)
#define ERR_HELP			(1 << 5)
#define ERR_SOUND_SERVER		(1 << 6)
#define ERR_NETWORK_SERVER		(1 << 7)
#define ERR_NETWORK_CLIENT		(1 << 8)
#define ERR_FROM_SERVER			(ERR_SOUND_SERVER | ERR_NETWORK_SERVER)
#define ERR_EXIT_HELP			(ERR_EXIT | ERR_HELP)
#define ERR_EXIT_SOUND_SERVER		(ERR_EXIT | ERR_SOUND_SERVER)
#define ERR_EXIT_NETWORK_SERVER		(ERR_EXIT | ERR_NETWORK_SERVER)
#define ERR_EXIT_NETWORK_CLIENT		(ERR_EXIT | ERR_NETWORK_CLIENT)

/* values for getFile...() and putFile...() */
#define BYTE_ORDER_BIG_ENDIAN		0
#define BYTE_ORDER_LITTLE_ENDIAN	1

/* values for cursor bitmap creation */
#define BIT_ORDER_MSB			0
#define BIT_ORDER_LSB			1

/* values for createDirectory() */
#define PERMS_PRIVATE			0
#define PERMS_PUBLIC			1

/* values for general file handling stuff */
#define MAX_FILENAME_LEN		256
#define MAX_LINE_LEN			1024

/* values for general username handling stuff */
#define MAX_USERNAME_LEN		1024

#if defined(PLATFORM_ANDROID)
/* values for Android asset handling */
#define ASSET_TOC_BASENAME		".toc"
#endif


/* structure definitions */

typedef struct
{
  char *filename;
  FILE *file;
  boolean end_of_file;

#if defined(PLATFORM_ANDROID)
  boolean file_is_asset;
  SDL_RWops *asset_file;
#endif
} File;

typedef struct
{
  boolean is_directory;
  char *basename;
  char *filename;
} DirectoryEntry;

typedef struct
{
  char *filename;
  DIR *dir;
  DirectoryEntry *dir_entry;

#if defined(PLATFORM_ANDROID)
  boolean directory_is_asset;
  SDL_RWops *asset_toc_file;
  char *current_entry;
#endif
} Directory;


/* function definitions */

void fprintf_line(FILE *, char *, int);
void fprintf_line_with_prefix(FILE *, char *, char *, int);
void printf_line(char *, int);
void printf_line_with_prefix(char *, char *, int);

void Print(char *, ...);
void PrintNoLog(char *, ...);
void PrintLine(char *, int);
void PrintLineWithPrefix(char *, char *, int);

char *int2str(int, int);
char *i_to_a(unsigned int);
int log_2(unsigned int);

boolean getTokenValueFromString(char *, char **, char **);

void InitCounter(void);
unsigned int Counter(void);
void Delay(unsigned int);
boolean DelayReachedExt(unsigned int *, unsigned int, unsigned int);
boolean FrameReached(unsigned int *, unsigned int);
boolean DelayReached(unsigned int *, unsigned int);
void ResetDelayCounterExt(unsigned int *, unsigned int);
void ResetFrameCounter(unsigned int *);
void ResetDelayCounter(unsigned int *);
int WaitUntilDelayReached(unsigned int *, unsigned int);
void SkipUntilDelayReached(unsigned int *, unsigned int, int *, int);

unsigned int init_random_number(int, int);
unsigned int get_random_number(int, int);

char *getLoginName(void);
char *getRealName(void);

time_t getFileTimestampEpochSeconds(char *);

char *getBasePath(char *);
char *getBaseName(char *);
char *getBaseNamePtr(char *);
char *getBaseNameNoSuffix(char *);

char *getStringCat2WithSeparator(char *, char *, char *);
char *getStringCat3WithSeparator(char *, char *, char *, char *);
char *getStringCat2(char *, char *);
char *getStringCat3(char *, char *, char *);
char *getPath2(char *, char *);
char *getPath3(char *, char *, char*);
char *getImg2(char *, char *);
char *getImg3(char *, char *, char*);
char *getStringCopy(const char *);
char *getStringCopyN(const char *, int);
char *getStringCopyNStatic(const char *, int);
char *getStringToLower(const char *);
void setString(char **, char *);
boolean strEqual(char *, char *);
boolean strEqualN(char *, char *, int);
boolean strPrefix(char *, char *);
boolean strSuffix(char *, char *);
boolean strPrefixLower(char *, char *);
boolean strSuffixLower(char *, char *);

void GetOptions(int, char **,
		void (*print_usage_function)(void),
		void (*print_version_function)(void));

void SetError(char *, ...);
char *GetError(void);
void Error(int, char *, ...);

void *checked_malloc(unsigned int);
void *checked_calloc(unsigned int);
void *checked_realloc(void *, unsigned int);
void checked_free(void *);
void clear_mem(void *, unsigned int);

void swap_numbers(int *, int *);
void swap_number_pairs(int *, int *, int *, int *);

int getFile8BitInteger(File *);
int putFile8BitInteger(FILE *, int);
int getFile16BitInteger(File *, int);
int putFile16BitInteger(FILE *, int, int);
int getFile32BitInteger(File *, int);
int putFile32BitInteger(FILE *, int, int);

boolean getFileChunk(File *, char *, int *, int);
int putFileChunk(FILE *, char *, int, int);
int getFileVersion(File *);
int putFileVersion(FILE *, int);

void ReadBytesFromFile(File *, byte *, unsigned int);
void WriteBytesToFile(FILE *, byte *, unsigned int);

void ReadUnusedBytesFromFile(File *, unsigned int);
void WriteUnusedBytesToFile(FILE *, unsigned int);

#define getFile8Bit(f)        getFile8BitInteger(f)
#define putFile8Bit(f,x)      putFile8BitInteger(f,x)
#define getFile16BitBE(f)     getFile16BitInteger(f,BYTE_ORDER_BIG_ENDIAN)
#define getFile16BitLE(f)     getFile16BitInteger(f,BYTE_ORDER_LITTLE_ENDIAN)
#define putFile16BitBE(f,x)   putFile16BitInteger(f,x,BYTE_ORDER_BIG_ENDIAN)
#define putFile16BitLE(f,x)   putFile16BitInteger(f,x,BYTE_ORDER_LITTLE_ENDIAN)
#define getFile32BitBE(f)     getFile32BitInteger(f,BYTE_ORDER_BIG_ENDIAN)
#define getFile32BitLE(f)     getFile32BitInteger(f,BYTE_ORDER_LITTLE_ENDIAN)
#define putFile32BitBE(f,x)   putFile32BitInteger(f,x,BYTE_ORDER_BIG_ENDIAN)
#define putFile32BitLE(f,x)   putFile32BitInteger(f,x,BYTE_ORDER_LITTLE_ENDIAN)

#define getFileChunkBE(f,s,x) getFileChunk(f,s,x,BYTE_ORDER_BIG_ENDIAN)
#define getFileChunkLE(f,s,x) getFileChunk(f,s,x,BYTE_ORDER_LITTLE_ENDIAN)
#define putFileChunkBE(f,s,x) putFileChunk(f,s,x,BYTE_ORDER_BIG_ENDIAN)
#define putFileChunkLE(f,s,x) putFileChunk(f,s,x,BYTE_ORDER_LITTLE_ENDIAN)

char *getKeyNameFromKey(Key);
char *getX11KeyNameFromKey(Key);
Key getKeyFromKeyName(char *);
Key getKeyFromX11KeyName(char *);
char getCharFromKey(Key);
char getValidConfigValueChar(char);

int get_integer_from_string(char *);
boolean get_boolean_from_string(char *);
int get_switch3_from_string(char *);

ListNode *newListNode(void);
void addNodeToList(ListNode **, char *, void *);
void deleteNodeFromList(ListNode **, char *, void (*function)(void *));
ListNode *getNodeFromKey(ListNode *, char *);
int getNumNodes(ListNode *);

File *openFile(char *, char *);
int closeFile(File *);
int checkEndOfFile(File *);
size_t readFile(File *, void *, size_t, size_t);
size_t writeFile(File *, void *, size_t, size_t);
int seekFile(File *, long, int);
int getByteFromFile(File *);
char *getStringFromFile(File *, char *, int);
int copyFile(char *, char *);

Directory *openDirectory(char *);
int closeDirectory(Directory *);
DirectoryEntry *readDirectory(Directory *);
void freeDirectoryEntry(DirectoryEntry *);

boolean directoryExists(char *);
boolean fileExists(char *);
boolean FileIsGraphic(char *);
boolean FileIsSound(char *);
boolean FileIsMusic(char *);
boolean FileIsArtworkType(char *, int);

char *get_mapped_token(char *);

int get_parameter_value(char *, char *, int);

struct FileInfo *getFileListFromConfigList(struct ConfigInfo *,
					   struct ConfigTypeInfo *,
					   char **, int);
void LoadArtworkConfig(struct ArtworkListInfo *);
void ReloadCustomArtworkList(struct ArtworkListInfo *);
void FreeCustomArtworkLists(struct ArtworkListInfo *);

char *getLogFilename(char *);
void OpenLogFiles();
void CloseLogFiles();
void DumpLogFile(int);

void NotifyUserAboutErrorFile();

#if DEBUG
void debug_print_timestamp(int, char *);
#endif
void print_timestamp_init(char *);
void print_timestamp_time(char *);
void print_timestamp_done(char *);


#endif /* MISC_H */
