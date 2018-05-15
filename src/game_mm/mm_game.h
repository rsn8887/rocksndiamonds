// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_game.h
// ============================================================================

#ifndef MM_GAME_H
#define MM_GAME_H

#include "main_mm.h"


void InitAmoebaNr(int, int);
void GameWon_MM(void);
int NewHiScore_MM(void);

void Blurb(int, int);
void Impact(int, int);
void TurnRound(int, int);
int AmoebeNachbarNr(int, int);
void AmoebeUmwandeln(int, int);
void AmoebeUmwandelnBD(int, int, int);
void AmoebeWaechst(int, int);
void AmoebeAbleger(int, int);
void Life(int, int);
void Ablenk(int, int);
void Blubber(int, int);
void NussKnacken(int, int);
void SiebAktivieren(int, int, int);
void AusgangstuerPruefen(int, int);
void AusgangstuerOeffnen(int, int);
void AusgangstuerBlinken(int, int);
void EdelsteinFunkeln(int, int);
void MauerWaechst(int, int);
void MauerAbleger(int, int);

boolean MoveFigureOneStep(struct PlayerInfo *, int, int, int, int);
boolean MoveFigure(struct PlayerInfo *, int, int);
void ScrollFigure(struct PlayerInfo *, int);
void ScrollScreen(struct PlayerInfo *, int);

void TestIfGoodThingHitsBadThing(int, int);
void TestIfBadThingHitsGoodThing(int, int);
void TestIfHeroHitsBadThing(int, int);
void TestIfBadThingHitsHero(int, int);
void TestIfFriendHitsBadThing(int, int);
void TestIfBadThingHitsFriend(int, int);
void TestIfBadThingHitsOtherBadThing(int, int);
void KillHero(struct PlayerInfo *);
void BuryHero(struct PlayerInfo *);
void RemoveHero(struct PlayerInfo *);
int DigField(struct PlayerInfo *, int, int, int, int, int);
boolean SnapField(struct PlayerInfo *, int, int);
boolean PlaceBomb(struct PlayerInfo *);
void PlaySoundLevel(int, int, int);

void CreateGameButtons();
void UnmapGameButtons();

void AddLaserEdge(int, int);
void AddDamagedField(int, int);
void ScanLaser(void);
void DrawLaser(int, int);
boolean HitElement(int, int);
boolean HitOnlyAnEdge(int, int);
boolean HitPolarizer(int, int);
boolean HitBlock(int, int);
boolean HitLaserSource(int, int);
boolean HitLaserDestination(int, int);
boolean HitReflectingWalls(int, int);
boolean HitAbsorbingWalls(int, int);
void RotateMirror(int, int, int);
boolean ObjHit(int, int, int);
void DeletePacMan(int, int);

void ColorCycling(void);
void MovePacMen(void);

#endif
