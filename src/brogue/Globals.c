/*
 *  Globals.c
 *  Brogue
 *
 *  Created by Brian Walker on 1/10/09.
 *  Copyright 2012. All rights reserved.
 *
 *  This file is part of Brogue.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Rogue.h"

tcell tmap[DCOLS][DROWS];                       // grids with info about the map
pcell pmap[DCOLS][DROWS];
short **scentMap;
cellDisplayBuffer displayBuffer[COLS][ROWS];    // used to optimize plotCharWithColor
short terrainRandomValues[DCOLS][DROWS][8];
short **safetyMap;                              // used to help monsters flee
short **allySafetyMap;                          // used to help allies flee
short **chokeMap;                               // used to assess the importance of the map's various chokepoints
const short nbDirs[8][2] = {{0,-1}, {0,1}, {-1,0}, {1,0}, {-1,-1}, {-1,1}, {1,-1}, {1,1}};
const short cDirs[8][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};
short numberOfWaypoints;
levelData *levels;
creature player;
playerCharacter rogue;
creature *monsters;
creature *dormantMonsters;
creature *graveyard;
creature *purgatory;
item *floorItems;
item *packItems;
item *monsterItemsHopper;

char displayedMessage[MESSAGE_LINES][COLS*2];
boolean messageConfirmed[MESSAGE_LINES];
char combatText[COLS * 2];
short messageArchivePosition;
char messageArchive[MESSAGE_ARCHIVE_LINES][COLS*2];

char currentFilePath[BROGUE_FILENAME_MAX];

char displayDetail[DCOLS][DROWS];       // used to make certain per-cell data accessible to external code (e.g. terminal adaptations)

#ifdef AUDIT_RNG
FILE *RNGLogFile;
#endif

unsigned char inputRecordBuffer[INPUT_RECORD_BUFFER + 100];
unsigned short locationInRecordingBuffer;
unsigned long randomNumbersGenerated;
unsigned long positionInPlaybackFile;
unsigned long lengthOfPlaybackFile;
unsigned long recordingLocation;
unsigned long maxLevelChanges;
char annotationPathname[BROGUE_FILENAME_MAX];   // pathname of annotation file
uint64_t previousGameSeed;

//                                  Red     Green   Blue    RedRand GreenRand   BlueRand    Rand    Dances?
// basic colors
const color white =                 {100,   100,    100,    0,      0,          0,          0,      false};
const color gray =                  {50,    50,     50,     0,      0,          0,          0,      false};
const color darkGray =              {30,    30,     30,     0,      0,          0,          0,      false};
const color veryDarkGray =          {15,    15,     15,     0,      0,          0,          0,      false};
const color black =                 {0,     0,      0,      0,      0,          0,          0,      false};
const color yellow =                {100,   100,    0,      0,      0,          0,          0,      false};
const color darkYellow =            {50,    50,     0,      0,      0,          0,          0,      false};
const color teal =                  {30,    100,    100,    0,      0,          0,          0,      false};
const color purple =                {100,   0,      100,    0,      0,          0,          0,      false};
const color darkPurple =            {50,    0,      50,     0,      0,          0,          0,      false};
const color brown =                 {60,    40,     0,      0,      0,          0,          0,      false};
const color green =                 {0,     100,    0,      0,      0,          0,          0,      false};
const color darkGreen =             {0,     50,     0,      0,      0,          0,          0,      false};
const color orange =                {100,   50,     0,      0,      0,          0,          0,      false};
const color darkOrange =            {50,    25,     0,      0,      0,          0,          0,      false};
const color blue =                  {0,     0,      100,    0,      0,          0,          0,      false};
const color darkBlue =              {0,     0,      50,     0,      0,          0,          0,      false};
const color darkTurquoise =         {0,     40,     65,     0,      0,          0,          0,      false};
const color lightBlue =             {40,    40,     100,    0,      0,          0,          0,      false};
const color pink =                  {100,   60,     66,     0,      0,          0,          0,      false};
const color red  =                  {100,   0,      0,      0,      0,          0,          0,      false};
const color darkRed =               {50,    0,      0,      0,      0,          0,          0,      false};
const color tanColor =              {80,    67,     15,     0,      0,          0,          0,      false};

// bolt colors
const color rainbow =               {-70,   -70,    -70,    170,    170,        170,        0,      true};
const color descentBoltColor =      {-40,   -40,    -40,    0,      0,          80,         80,     true};
const color discordColor =          {25,    0,      25,     66,     0,          0,          0,      true};
const color poisonColor =           {0,     0,      0,      10,     50,         10,         0,      true};
const color beckonColor =           {10,    10,     10,     5,      5,          5,          50,     true};
const color invulnerabilityColor =  {25,    0,      25,     0,      0,          66,         0,      true};
const color dominationColor =       {0,     0,      100,    80,     25,         0,          0,      true};
const color empowermentColor =      {30,    100,    40,     25,     80,         25,         0,      true};
const color fireBoltColor =         {500,   150,    0,      45,     30,         0,          0,      true};
const color yendorLightColor =      {50,    -100,    30,     0,      0,          0,          0,      true};
const color dragonFireColor =       {500,   150,    0,      45,     30,         45,         0,      true};
const color flamedancerCoronaColor ={500,   150,    100,    45,     30,         0,          0,      true};
//const color shieldingColor =      {100,   50,     0,      0,      50,         100,        0,      true};
const color shieldingColor =        {150,   75,     0,      0,      50,         175,        0,      true};

// tile colors
const color undiscoveredColor =     {0,     0,      0,      0,      0,          0,          0,      false};

const color wallForeColor =         {7,     7,      7,      3,      3,          3,          0,      false};

color wallBackColor;
const color wallBackColorStart =    {45,    40,     40,     15,     0,          5,          20,     false};
const color wallBackColorEnd =      {40,    30,     35,     0,      20,         30,         20,     false};

const color mudWallForeColor =      {55,    45,     0,      5,      5,          5,          1,      false};
//const color mudWallForeColor =      {40,  34,     7,      0,      3,          0,          3,      false};
const color mudWallBackColor =      {20,    12,     3,      8,      4,          3,          0,      false};

const color graniteBackColor =      {10,    10,     10,     0,      0,          0,          0,      false};

const color floorForeColor =        {30,    30,     30,     0,      0,          0,          35,     false};

color floorBackColor;
const color floorBackColorStart =   {2,     2,      10,     2,      2,          0,          0,      false};
const color floorBackColorEnd =     {5,     5,      5,      2,      2,          0,          0,      false};

const color stairsBackColor =       {15,    15,     5,      0,      0,          0,          0,      false};
const color firstStairsBackColor =  {10,    10,     25,     0,      0,          0,          0,      false};

const color refuseBackColor =       {6,     5,      3,      2,      2,          0,          0,      false};
const color rubbleBackColor =       {7,     7,      8,      2,      2,          1,          0,      false};
const color bloodflowerForeColor =  {30,    5,      40,     5,      1,          3,          0,      false};
const color bloodflowerPodForeColor = {50,  5,      25,     5,      1,          3,          0,      false};
const color bloodflowerBackColor =  {15,    3,      10,     3,      1,          3,          0,      false};
const color bedrollBackColor =      {10,    8,      5,      1,      1,          0,          0,      false};

const color obsidianBackColor =     {6,     0,      8,      2,      0,          3,          0,      false};
const color carpetForeColor =       {23,    30,     38,     0,      0,          0,          0,      false};
const color carpetBackColor =       {15,    8,      5,      0,      0,          0,          0,      false};
const color marbleForeColor =       {30,    23,     38,     0,      0,          0,          0,      false};
const color marbleBackColor =       {6,     5,      13,     1,      0,          1,          0,      false};
const color doorForeColor =         {70,    35,     15,     0,      0,          0,          0,      false};
const color doorBackColor =         {30,    10,     5,      0,      0,          0,          0,      false};
//const color ironDoorForeColor =       {40,    40,     40,     0,      0,          0,          0,      false};
const color ironDoorForeColor =     {500,   500,    500,    0,      0,          0,          0,      false};
const color ironDoorBackColor =     {15,    15,     30,     0,      0,          0,          0,      false};
const color bridgeFrontColor =      {33,    12,     12,     12,     7,          2,          0,      false};
const color bridgeBackColor =       {12,    3,      2,      3,      2,          1,          0,      false};
const color statueBackColor =       {20,    20,     20,     0,      0,          0,          0,      false};
const color glyphColor =            {20,    5,      5,      50,     0,          0,          0,      true};
const color glyphLightColor =       {150,   0,      0,      150,    0,          0,          0,      true};
const color sacredGlyphColor =      {5,     20,     5,      0,      50,         0,          0,      true};
const color sacredGlyphLightColor = {45,    150,    60,     25,     80,         25,         0,      true};

//const color deepWaterForeColor =  {5,     5,      40,     0,      0,          10,         10,     true};
//color deepWaterBackColor;
//const color deepWaterBackColorStart = {5, 5,      55,     5,      5,          10,         10,     true};
//const color deepWaterBackColorEnd =   {5,     5,      45,     2,      2,          5,          5,      true};
//const color shallowWaterForeColor =   {40,    40,     90,     0,      0,          10,         10,     true};
//color shallowWaterBackColor;
//const color shallowWaterBackColorStart ={30,30,       80,     0,      0,          10,         10,     true};
//const color shallowWaterBackColorEnd ={20,    20,     60,     0,      0,          5,          5,      true};

const color deepWaterForeColor =    {5,     8,      20,     0,      4,          15,         10,     true};
color deepWaterBackColor;
const color deepWaterBackColorStart = {5,   10,     31,     5,      5,          5,          6,      true};
const color deepWaterBackColorEnd = {5,     8,      20,     2,      3,          5,          5,      true};
const color shallowWaterForeColor = {28,    28,     60,     0,      0,          10,         10,     true};
color shallowWaterBackColor;
const color shallowWaterBackColorStart ={20,20,     60,     0,      0,          10,         10,     true};
const color shallowWaterBackColorEnd ={12,  15,     40,     0,      0,          5,          5,      true};

const color mudForeColor =          {18,    14,     5,      5,      5,          0,          0,      false};
const color mudBackColor =          {23,    17,     7,      5,      5,          0,          0,      false};
const color chasmForeColor =        {7,     7,      15,     4,      4,          8,          0,      false};
color chasmEdgeBackColor;
const color chasmEdgeBackColorStart ={5,    5,      25,     2,      2,          2,          0,      false};
const color chasmEdgeBackColorEnd = {8,     8,      20,     2,      2,          2,          0,      false};
const color fireForeColor =         {70,    20,     0,      15,     10,         0,          0,      true};
const color lavaForeColor =         {20,    20,     20,     100,    10,         0,          0,      true};
const color brimstoneForeColor =    {100,   50,     10,     0,      50,         40,         0,      true};
const color brimstoneBackColor =    {18,    12,     9,      0,      0,          5,          0,      false};

const color lavaBackColor =         {70,    20,     0,      15,     10,         0,          0,      true};
const color acidBackColor =         {15,    80,     25,     5,      15,         10,         0,      true};

const color lightningColor =        {100,   150,    500,    50,     50,         0,          50,     true};
const color fungusLightColor =      {2,     11,     11,     4,      3,          3,          0,      true};
const color lavaLightColor =        {47,    13,     0,      10,     7,          0,          0,      true};
const color deepWaterLightColor =   {10,    30,     100,    0,      30,         100,        0,      true};

const color grassColor =            {15,    40,     15,     15,     50,         15,         10,     false};
const color deadGrassColor =        {20,    13,     0,      20,     10,         5,          10,     false};
const color fungusColor =           {15,    50,     50,     0,      25,         0,          30,     true};
const color grayFungusColor =       {30,    30,     30,     5,      5,          5,          10,     false};
const color foliageColor =          {25,    100,    25,     15,     0,          15,         0,      false};
const color deadFoliageColor =      {20,    13,     0,      30,     15,         0,          20,     false};
const color lichenColor =           {50,    5,      25,     10,     0,          5,          0,      true};
const color hayColor =              {70,    55,     5,      0,      20,         20,         0,      false};
const color ashForeColor =          {20,    20,     20,     0,      0,          0,          20,     false};
const color bonesForeColor =        {80,    80,     30,     5,      5,          35,         5,      false};
const color ectoplasmColor =        {45,    20,     55,     25,     0,          25,         5,      false};
const color forceFieldColor =       {0,     25,     25,     0,      25,         25,         0,      true};
const color wallCrystalColor =      {40,    40,     60,     20,     20,         40,         0,      true};
const color altarForeColor =        {5,     7,      9,      0,      0,          0,          0,      false};
const color altarBackColor =        {35,    18,     18,     0,      0,          0,          0,      false};
const color greenAltarBackColor =   {18,    25,     18,     0,      0,          0,          0,      false};
const color goldAltarBackColor =    {25,    24,     12,     0,      0,          0,          0,      false};
const color pedestalBackColor =     {10,    5,      20,     0,      0,          0,          0,      false};

// monster colors
const color goblinColor =           {40,    30,     20,     0,      0,          0,          0,      false};
const color jackalColor =           {60,    42,     27,     0,      0,          0,          0,      false};
const color ogreColor =             {60,    25,     25,     0,      0,          0,          0,      false};
const color eelColor =              {30,    12,     12,     0,      0,          0,          0,      false};
const color goblinConjurerColor =   {67,    10,     100,    0,      0,          0,          0,      false};
const color spectralBladeColor =    {15,    15,     60,     0,      0,          70,         50,     true};
const color spectralImageColor =    {13,    0,      0,      25,     0,          0,          0,      true};
const color toadColor =             {40,    65,     30,     0,      0,          0,          0,      false};
const color trollColor =            {40,    60,     15,     0,      0,          0,          0,      false};
const color centipedeColor =        {75,    25,     85,     0,      0,          0,          0,      false};
const color dragonColor =           {20,    80,     15,     0,      0,          0,          0,      false};
const color krakenColor =           {100,   55,     55,     0,      0,          0,          0,      false};
const color salamanderColor =       {40,    10,     0,      8,      5,          0,          0,      true};
const color pixieColor =            {60,    60,     60,     40,     40,         40,         0,      true};
const color darPriestessColor =     {0,     50,     50,     0,      0,          0,          0,      false};
const color darMageColor =          {50,    50,     0,      0,      0,          0,          0,      false};
const color wraithColor =           {66,    66,     25,     0,      0,          0,          0,      false};
const color pinkJellyColor =        {100,   40,     40,     5,      5,          5,          20,     true};
const color wormColor =             {80,    60,     40,     0,      0,          0,          0,      false};
const color sentinelColor =         {3,     3,      30,     0,      0,          10,         0,      true};
const color goblinMysticColor =     {10,    67,     100,    0,      0,          0,          0,      false};
const color ifritColor =            {50,    10,     100,    75,     0,          20,         0,      true};
const color phoenixColor =          {100,   0,      0,      0,      100,        0,          0,      true};

// light colors
color minersLightColor;
const color minersLightStartColor = {180,   180,    180,    0,      0,          0,          0,      false};
const color minersLightEndColor =   {90,    90,     120,    0,      0,          0,          0,      false};
const color torchColor =            {150,   75,     30,     0,      30,         20,         0,      true};
const color torchLightColor =       {75,    38,     15,     0,      15,         7,          0,      true};
//const color hauntedTorchColor =     {75,  30,     150,    30,     20,         0,          0,      true};
const color hauntedTorchColor =     {75,    20,     40,     30,     10,         0,          0,      true};
//const color hauntedTorchLightColor ={19,     7,       37,     8,      4,          0,          0,      true};
const color hauntedTorchLightColor ={67,    10,     10,     20,     4,          0,          0,      true};
const color ifritLightColor =       {0,     10,     150,    100,    0,          100,        0,      true};
//const color unicornLightColor =       {-50,   -50,    -50,    200,    200,        200,        0,      true};
const color unicornLightColor =     {-50,   -50,    -50,    250,    250,        250,        0,      true};
const color wispLightColor =        {75,    100,    250,    33,     10,         0,          0,      true};
const color summonedImageLightColor ={200,  0,      75,     0,      0,          0,          0,      true};
const color spectralBladeLightColor ={40,   0,      230,    0,      0,          0,          0,      true};
const color ectoplasmLightColor =   {23,    10,     28,     13,     0,          13,         3,      false};
const color explosionColor =        {10,    8,      2,      0,      2,          2,          0,      true};
const color explosiveAuraColor =    {2000,  0,      -1000,  200,    200,        0,          0,      true};
const color sacrificeTargetColor =  {100,   -100,   -300,   0,      100,        100,        0,      true};
const color dartFlashColor =        {500,   500,    500,    0,      2,          2,          0,      true};
const color lichLightColor =        {-50,   80,     30,     0,      0,          20,         0,      true};
const color forceFieldLightColor =  {10,    10,     10,     0,      50,         50,         0,      true};
const color crystalWallLightColor = {10,    10,     10,     0,      0,          50,         0,      true};
const color sunLightColor =         {100,   100,    75,     0,      0,          0,          0,      false};
const color fungusForestLightColor ={30,    40,     60,     0,      0,          0,          40,     true};
const color fungusTrampledLightColor ={10,  10,     10,     0,      50,         50,         0,      true};
const color redFlashColor =         {100,   10,     10,     0,      0,          0,          0,      false};
const color darknessPatchColor =    {-10,   -10,    -10,    0,      0,          0,          0,      false};
const color darknessCloudColor =    {-20,   -20,    -20,    0,      0,          0,          0,      false};
const color magicMapFlashColor =    {60,    20,     60,     0,      0,          0,          0,      false};
const color sentinelLightColor =    {20,    20,     120,    10,     10,         60,         0,      true};
const color telepathyColor =        {30,    30,     130,    0,      0,          0,          0,      false};
const color confusionLightColor =   {10,    10,     10,     10,     10,         10,         0,      true};
const color portalActivateLightColor ={300, 400,    500,    0,      0,          0,          0,      true};
const color descentLightColor =     {20,    20,     70,     0,      0,          0,          0,      false};
const color algaeBlueLightColor =   {20,    15,     50,     0,      0,          0,          0,      false};
const color algaeGreenLightColor =  {15,    50,     20,     0,      0,          0,          0,      false};

// flare colors
const color scrollProtectionColor = {375,   750,    0,      0,      0,          0,          0,      true};
const color scrollEnchantmentColor ={250,   225,    300,    0,      0,          450,        0,      true};
const color potionStrengthColor =   {1000,  0,      400,    600,    0,          0,          0,      true};
const color empowermentFlashColor = {500,   1000,   600,    0,      500,        0,          0,      true};
const color genericFlashColor =     {800,   800,    800,    0,      0,          0,          0,      false};
const color summoningFlashColor =   {0,     0,      0,      600,    0,          1200,       0,      true};
const color fireFlashColor =        {750,   225,    0,      100,    50,         0,          0,      true};
const color explosionFlareColor =   {10000, 6000,   1000,   0,      0,          0,          0,      false};
const color quietusFlashColor =     {0,     -1000,  -200,   0,      0,          0,          0,      true};
const color slayingFlashColor =     {-1000, -200,   0,      0,      0,          0,          0,      true};

// color multipliers
const color colorDim25 =            {25,    25,     25,     25,     25,         25,         25,     false};
const color colorMultiplier100 =    {100,   100,    100,    100,    100,        100,        100,    false};
const color memoryColor =           {25,    25,     50,     20,     20,         20,         0,      false};
const color memoryOverlay =         {25,    25,     50,     0,      0,          0,          0,      false};
const color magicMapColor =         {60,    20,     60,     60,     20,         60,         0,      false};
const color clairvoyanceColor =     {50,    90,     50,     50,     90,         50,         66,     false};
const color telepathyMultiplier =   {30,    30,     130,    30,     30,         130,        66,     false};
const color omniscienceColor =      {140,   100,    60,     140,    100,        60,         90,     false};
const color basicLightColor =       {180,   180,    180,    180,    180,        180,        180,    false};

// blood colors
const color humanBloodColor =       {60,    20,     10,     15,     0,          0,          15,     false};
const color insectBloodColor =      {10,    60,     20,     0,      15,         0,          15,     false};
const color vomitColor =            {60,    50,     5,      0,      15,         15,         0,      false};
const color urineColor =            {70,    70,     40,     0,      0,          0,          10,     false};
const color methaneColor =          {45,    60,     15,     0,      0,          0,          0,      false};

// gas colors
const color poisonGasColor =        {75,    25,     85,     0,      0,          0,          0,      false};
const color confusionGasColor =     {60,    60,     60,     40,     40,         40,         0,      true};

// interface colors
const color itemColor =             {100,   95,     -30,    0,      0,          0,          0,      false};
const color blueBar =               {15,    10,     50,     0,      0,          0,          0,      false};
const color redBar =                {45,    10,     15,     0,      0,          0,          0,      false};
const color hiliteColor =           {100,   100,    0,      0,      0,          0,          0,      false};
const color interfaceBoxColor =     {7,     6,      15,     0,      0,          0,          0,      false};
const color interfaceButtonColor =  {18,    15,     38,     0,      0,          0,          0,      false};
const color buttonHoverColor =      {100,   70,     40,     0,      0,          0,          0,      false};
const color titleButtonColor =      {23,    15,     30,     0,      0,          0,          0,      false};

const color playerInvisibleColor =  {20,    20,     30,     0,      0,          80,         0,      true};
const color playerInLightColor =    {100,   90,     30,     0,      0,          0,          0,      false};
const color playerInShadowColor =   {60,    60,     100,    0,      0,          0,          0,      false};
const color playerInDarknessColor = {30,    30,     65,     0,      0,          0,          0,      false};

const color inLightMultiplierColor ={150,   150,    75,     150,    150,        75,         100,    true};
const color inDarknessMultiplierColor={66,  66,     120,    66,     66,         120,        66,     true};

const color goodMessageColor =      {60,    50,     100,    0,      0,          0,          0,      false};
const color badMessageColor =       {100,   50,     60,     0,      0,          0,          0,      false};
const color advancementMessageColor ={50,   100,    60,     0,      0,          0,          0,      false};
const color itemMessageColor =      {100,   100,    50,     0,      0,          0,          0,      false};
const color flavorTextColor =       {50,    40,     90,     0,      0,          0,          0,      false};
const color backgroundMessageColor ={60,    20,     70,     0,      0,          0,          0,      false};

const color superVictoryColor =     {150,   100,    300,    0,      0,          0,          0,      false};

//const color flameSourceColor = {0, 0, 0, 65, 40, 100, 0, true}; // 1
//const color flameSourceColor = {0, 0, 0, 80, 50, 100, 0, true}; // 2
//const color flameSourceColor = {25, 13, 25, 50, 25, 50, 0, true}; // 3
//const color flameSourceColor = {20, 20, 20, 60, 20, 40, 0, true}; // 4
//const color flameSourceColor = {30, 18, 18, 70, 36, 36, 0, true}; // 7**
const color flameSourceColor = {20, 7, 7, 60, 40, 40, 0, true}; // 8
const color flameSourceColorSecondary = {7, 2, 0, 10, 0, 0, 0, true};

//const color flameTitleColor = {0, 0, 0, 17, 10, 6, 0, true}; // pale orange
//const color flameTitleColor = {0, 0, 0, 7, 7, 10, 0, true}; // *pale blue*
const color flameTitleColor = {0, 0, 0, 9, 9, 15, 0, true}; // *pale blue**
//const color flameTitleColor = {0, 0, 0, 11, 11, 18, 0, true}; // *pale blue*
//const color flameTitleColor = {0, 0, 0, 15, 15, 9, 0, true}; // pale yellow
//const color flameTitleColor = {0, 0, 0, 15, 9, 15, 0, true}; // pale purple

const color *dynamicColors[NUMBER_DYNAMIC_COLORS][3] = {
    // used color           shallow color               deep color
    {&minersLightColor,     &minersLightStartColor,     &minersLightEndColor},
    {&wallBackColor,        &wallBackColorStart,        &wallBackColorEnd},
    {&deepWaterBackColor,   &deepWaterBackColorStart,   &deepWaterBackColorEnd},
    {&shallowWaterBackColor,&shallowWaterBackColorStart,&shallowWaterBackColorEnd},
    {&floorBackColor,       &floorBackColorStart,       &floorBackColorEnd},
    {&chasmEdgeBackColor,   &chasmEdgeBackColorStart,   &chasmEdgeBackColorEnd},
};

const autoGenerator autoGeneratorCatalog[NUMBER_AUTOGENERATORS] = {
    // Ordinary features of the dungeon
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_GRANITE_COLUMN,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 1,
        .maxDepth = DEEPEST_LEVEL,
        .frequency = 60,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 4
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_CRYSTAL_WALL,
        .machine = 0,
        .requiredDungeonFoundationType = WALL,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 14,
        .maxDepth = DEEPEST_LEVEL,
        .frequency = 15,
        .minNumberIntercept = -325,
        .minNumberSlope = 25,
        .maxNumber = 5
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_LUMINESCENT_FUNGUS,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 7,
        .maxDepth = DEEPEST_LEVEL,
        .frequency = 15,
        .minNumberIntercept = -300,
        .minNumberSlope = 70,
        .maxNumber = 14
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_GRASS,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 0,
        .maxDepth = 10,
        .frequency = 0,
        .minNumberIntercept = 1000,
        .minNumberSlope = -80,
        .maxNumber = 10
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_DEAD_GRASS,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 4,
        .maxDepth = 9,
        .frequency = 0,
        .minNumberIntercept = -200,
        .minNumberSlope = 80,
        .maxNumber = 10
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_DEAD_GRASS,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 9,
        .maxDepth = 14,
        .frequency = 0,
        .minNumberIntercept = 1200,
        .minNumberSlope = -80,
        .maxNumber = 10
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_BONES,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 12,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 30,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 4
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_RUBBLE,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 0,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 30,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 4
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_FOLIAGE,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 0,
        .maxDepth = 8,
        .frequency = 15,
        .minNumberIntercept = 1000,
        .minNumberSlope = -333,
        .maxNumber = 10
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_FUNGUS_FOREST,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 13,
        .maxDepth = DEEPEST_LEVEL,
        .frequency = 30,
        .minNumberIntercept = -600,
        .minNumberSlope = 50,
        .maxNumber = 12
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_BUILD_ALGAE_WELL,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = DEEP_WATER,
        .minDepth = 10,
        .maxDepth = DEEPEST_LEVEL,
        .frequency = 50,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = STATUE_INERT,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = WALL,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 6,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 5,
        .minNumberIntercept = -100,
        .minNumberSlope = 35,
        .maxNumber = 3
    },
    {
        .terrain = STATUE_INERT,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 10,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 50,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
    {
        .terrain = TORCH_WALL,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = WALL,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 6,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 5,
        .minNumberIntercept = -200,
        .minNumberSlope = 70,
        .maxNumber = 12
    },

    // Pre-revealed traps
    {
        .terrain = GAS_TRAP_POISON,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 2,
        .maxDepth = 4,
        .frequency = 20,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 1
    },
    {
        .terrain = NET_TRAP,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 2,
        .maxDepth = 5,
        .frequency = 20,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 1
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_PARALYSIS_TRAP_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 2,
        .maxDepth = 6,
        .frequency = 20,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 1
    },
    {
        .terrain = ALARM_TRAP,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 4,
        .maxDepth = 7,
        .frequency = 20,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 1
    },
    {
        .terrain = GAS_TRAP_CONFUSION,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 2,
        .maxDepth = 10,
        .frequency = 20,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 1
    },
    {
        .terrain = FLAMETHROWER,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 4,
        .maxDepth = 12,
        .frequency = 20,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 1
    },
    {
        .terrain = FLOOD_TRAP,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 10,
        .maxDepth = 14,
        .frequency = 20,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 1
    },

    // Hidden traps
    {
        .terrain = GAS_TRAP_POISON_HIDDEN,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 5,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 20,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
    {
        .terrain = NET_TRAP_HIDDEN,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 6,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 20,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_PARALYSIS_TRAP_HIDDEN_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 7,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 20,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
    {
        .terrain = ALARM_TRAP_HIDDEN,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 8,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 20,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = TRAP_DOOR_HIDDEN,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 9,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 20,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = GAS_TRAP_CONFUSION_HIDDEN,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 11,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 20,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
    {
        .terrain = FLAMETHROWER_HIDDEN,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 13,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 20,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
    {
        .terrain = FLOOD_TRAP_HIDDEN,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 15,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 20,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_SWAMP_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 1,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 30,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_SUNLIGHT,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 0,
        .maxDepth = 5,
        .frequency = 15,
        .minNumberIntercept = 500,
        .minNumberSlope = -150,
        .maxNumber = 10
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_DARKNESS,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 1,
        .maxDepth = 15,
        .frequency = 15,
        .minNumberIntercept = 500,
        .minNumberSlope = -50,
        .maxNumber = 10
    },
    {
        .terrain = STEAM_VENT,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 16,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 30,
        .minNumberIntercept = 100,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
    {
        .terrain = CRYSTAL_WALL,
        .layer = DUNGEON,
        .DFType = 0,
        .machine = 0,
        .requiredDungeonFoundationType = WALL,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = DEEPEST_LEVEL,
        .maxDepth = DEEPEST_LEVEL,
        .frequency = 100,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 600
    },

    // Dewars
    {
        .terrain = DEWAR_CAUSTIC_GAS,
        .layer = DUNGEON,
        .DFType = DF_CARPET_AREA,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 8,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 2,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = DEWAR_CONFUSION_GAS,
        .layer = DUNGEON,
        .DFType = DF_CARPET_AREA,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 8,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 2,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = DEWAR_PARALYSIS_GAS,
        .layer = DUNGEON,
        .DFType = DF_CARPET_AREA,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 8,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 2,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = DEWAR_METHANE_GAS,
        .layer = DUNGEON,
        .DFType = DF_CARPET_AREA,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 8,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 2,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },

    // Flavor machines
    {
        .terrain = 0,
        .layer = 0,
        .DFType = DF_LUMINESCENT_FUNGUS,
        .machine = 0,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = DEEPEST_LEVEL,
        .maxDepth = DEEPEST_LEVEL,
        .frequency = 100,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 200
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_BLOODFLOWER_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 1,
        .maxDepth = 30,
        .frequency = 25,
        .minNumberIntercept = 140,
        .minNumberSlope = -10,
        .maxNumber = 3
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_SHRINE_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 5,
        .maxDepth = AMULET_LEVEL,
        .frequency = 7,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 1
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_IDYLL_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 1,
        .maxDepth = 5,
        .frequency = 15,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 1
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_REMNANT_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 10,
        .maxDepth = DEEPEST_LEVEL,
        .frequency = 15,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_DISMAL_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 7,
        .maxDepth = DEEPEST_LEVEL,
        .frequency = 12,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 5
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_BRIDGE_TURRET_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 5,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 6,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_LAKE_PATH_TURRET_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 5,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 6,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_TRICK_STATUE_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 6,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 15,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_SENTINEL_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 12,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 10,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 2
    },
    {
        .terrain = 0,
        .layer = 0,
        .DFType = 0,
        .machine = MT_WORM_AREA,
        .requiredDungeonFoundationType = FLOOR,
        .requiredLiquidFoundationType = NOTHING,
        .minDepth = 12,
        .maxDepth = DEEPEST_LEVEL-1,
        .frequency = 12,
        .minNumberIntercept = 0,
        .minNumberSlope = 0,
        .maxNumber = 3
    },
};

// promoteChance is in hundredths of a percent per turn
const floorTileType tileCatalog[NUMBER_TILETYPES] = {
    // dungeon layer (this layer must have all of fore color, back color and char)
    {
        .displayChar = ' ',
        .foreColor = &black,
        .backColor = &black,
        .drawPriority = 100,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "a chilly void",
        .flavorText = ""
    },
    {
        .displayChar = G_GRANITE,
        .foreColor = &wallBackColor,
        .backColor = &graniteBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a rough granite wall",
        .flavorText = "The granite is split open with splinters of rock jutting out at odd angles."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_CARPET,
        .foreColor = &carpetForeColor,
        .backColor = &carpetBackColor,
        .drawPriority = 85,
        .chanceToIgnite = 0,
        .fireType = DF_EMBERS,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "the carpet",
        .flavorText = "Ornate carpeting fills this room, a relic of ages past."
    },
    {
        .displayChar = G_CARPET,
        .foreColor = &marbleForeColor,
        .backColor = &marbleBackColor,
        .drawPriority = 85,
        .chanceToIgnite = 0,
        .fireType = DF_EMBERS,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "the marble ground",
        .flavorText = "Light from the nearby crystals catches the grain of the lavish marble floor."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a stone wall",
        .flavorText = "The rough stone wall is firm and unyielding."
    },
    {
        .displayChar = G_CLOSED_DOOR,
        .foreColor = &doorForeColor,
        .backColor = &doorBackColor,
        .drawPriority = 25,
        .chanceToIgnite = 50,
        .fireType = DF_EMBERS,
        .discoverType = 0,
        .promoteType = DF_OPEN_DOOR,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_VISION | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP | TM_VISUALLY_DISTINCT),
        .description = "a wooden door",
        .flavorText = "you pass through the doorway."
    },
    {
        .displayChar = G_OPEN_DOOR,
        .foreColor = &doorForeColor,
        .backColor = &doorBackColor,
        .drawPriority = 25,
        .chanceToIgnite = 50,
        .fireType = DF_EMBERS,
        .discoverType = 0,
        .promoteType = DF_CLOSED_DOOR,
        .promoteChance = 10000,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "an open door",
        .flavorText = "you pass through the doorway."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 50,
        .fireType = DF_EMBERS,
        .discoverType = DF_SHOW_DOOR,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET),
        .description = "a stone wall",
        .flavorText = "The rough stone wall is firm and unyielding."
    },
    {
        .displayChar = G_CLOSED_IRON_DOOR,
        .foreColor = &ironDoorForeColor,
        .backColor = &ironDoorBackColor,
        .drawPriority = 15,
        .chanceToIgnite = 50,
        .fireType = DF_EMBERS,
        .discoverType = 0,
        .promoteType = DF_OPEN_IRON_DOOR_INERT,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_WITH_KEY | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a locked iron door",
        .flavorText = "you search your pack but do not have a matching key."
    },
    {
        .displayChar = G_OPEN_IRON_DOOR,
        .foreColor = &white,
        .backColor = &ironDoorBackColor,
        .drawPriority = 90,
        .chanceToIgnite = 50,
        .fireType = DF_EMBERS,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VISUALLY_DISTINCT),
        .description = "an open iron door",
        .flavorText = "you pass through the doorway."
    },
    {
        .displayChar = G_DOWN_STAIRS,
        .foreColor = &itemColor,
        .backColor = &stairsBackColor,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_REPEL_CREATURES,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_PROMOTES_ON_STEP | TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a downward staircase",
        .flavorText = "stairs spiral downward into the depths."
    },
    {
        .displayChar = G_UP_STAIRS,
        .foreColor = &itemColor,
        .backColor = &stairsBackColor,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_REPEL_CREATURES,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_PROMOTES_ON_STEP | TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "an upward staircase",
        .flavorText = "stairs spiral upward."
    },
    {
        .displayChar = G_DOORWAY,
        .foreColor = &lightBlue,
        .backColor = &firstStairsBackColor,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_REPEL_CREATURES,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_PROMOTES_ON_STEP | TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "the dungeon exit",
        .flavorText = "the gilded doors leading out of the dungeon are sealed by an invisible force."
    },
    {
        .displayChar = G_DOORWAY,
        .foreColor = &wallCrystalColor,
        .backColor = &firstStairsBackColor,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_REPEL_CREATURES,
        .promoteChance = 0,
        .glowLight = INCENDIARY_DART_LIGHT,
        .flags = (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_PROMOTES_ON_STEP | TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a crystal portal",
        .flavorText = "dancing lights play across the plane of this sparkling crystal portal."
    },
    {
        .displayChar = G_TORCH,
        .foreColor = &torchColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = TORCH_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a wall-mounted torch",
        .flavorText = "The torch is anchored firmly to the wall and sputters quietly in the gloom."
    },
    {
        .displayChar = G_CRYSTAL,
        .foreColor = &wallCrystalColor,
        .backColor = &wallCrystalColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = CRYSTAL_WALL_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_DIAGONAL_MOVEMENT),
        .mechFlags = (TM_STAND_IN_TILE | TM_REFLECTS_BOLTS),
        .description = "a crystal formation",
        .flavorText = "You feel the crystal's glossy surface and admire the dancing lights beneath."
    },
    {
        .displayChar = G_PORTCULLIS,
        .foreColor = &gray,
        .backColor = &floorBackColor,
        .drawPriority = 10,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_OPEN_PORTCULLIS,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_CONNECTS_LEVEL),
        .description = "a heavy portcullis",
        .flavorText = "The iron bars rattle but will not budge; they are firmly locked in place."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_ACTIVATE_PORTCULLIS,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_BARRICADE,
        .foreColor = &doorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 10,
        .chanceToIgnite = 100,
        .fireType = DF_WOODEN_BARRICADE_BURN,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_CONNECTS_LEVEL),
        .description = "a dry wooden barricade",
        .flavorText = "The wooden barricade is firmly set but has dried over the years. Might it burn?"
    },
    {
        .displayChar = G_TORCH,
        .foreColor = &torchLightColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_PILOT_LIGHT,
        .promoteChance = 0,
        .glowLight = TORCH_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "a wall-mounted torch",
        .flavorText = "The torch is anchored firmly to the wall, and sputters quietly in the gloom."
    },
    {
        .displayChar = G_FIRE,
        .foreColor = &fireForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = TORCH_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING | T_IS_FIRE),
        .mechFlags = (TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR),
        .description = "a fallen torch",
        .flavorText = "The torch lies at the foot of the wall, spouting gouts of flame haphazardly."
    },
    {
        .displayChar = G_TORCH,
        .foreColor = &torchColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_HAUNTED_TORCH_TRANSITION,
        .promoteChance = 0,
        .glowLight = TORCH_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "a wall-mounted torch",
        .flavorText = "The torch is anchored firmly to the wall and sputters quietly in the gloom."
    },
    {
        .displayChar = G_TORCH,
        .foreColor = &torchColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_HAUNTED_TORCH,
        .promoteChance = 2000,
        .glowLight = TORCH_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "a wall-mounted torch",
        .flavorText = "The torch is anchored firmly to the wall and sputters quietly in the gloom."
    },
    {
        .displayChar = G_TORCH,
        .foreColor = &hauntedTorchColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = HAUNTED_TORCH_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a sputtering torch",
        .flavorText = "A dim purple flame sputters and spits atop this wall-mounted torch."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = DF_REVEAL_LEVER,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET),
        .description = "a stone wall",
        .flavorText = "The rough stone wall is firm and unyielding."
    },
    {
        .displayChar = G_LEVER,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_PULL_LEVER,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_PLAYER_ENTRY | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a lever",
        .flavorText = "The lever moves."
    },
    {
        .displayChar = G_LEVER_PULLED,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "an inactive lever",
        .flavorText = "The lever won't budge."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_CREATE_LEVER,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_IS_WIRED),
        .description = "a stone wall",
        .flavorText = "The rough stone wall is firm and unyielding."
    },
    {
        .displayChar = G_STATUE,
        .foreColor = &wallBackColor,
        .backColor = &statueBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a marble statue",
        .flavorText = "The cold marble statue has weathered the years with grace."
    },
    {
        .displayChar = G_STATUE,
        .foreColor = &wallBackColor,
        .backColor = &statueBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_CRACKING_STATUE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "a marble statue",
        .flavorText = "The cold marble statue has weathered the years with grace."
    },
    {
        .displayChar = G_CRACKED_STATUE,
        .foreColor = &wallBackColor,
        .backColor = &statueBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_STATUE_SHATTER,
        .promoteChance = 3500,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_LIST_IN_SIDEBAR),
        .description = "a cracking statue",
        .flavorText = "Deep cracks ramble down the side of the statue even as you watch."
    },
    {
        .displayChar = G_STATUE,
        .foreColor = &wallBackColor,
        .backColor = &statueBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_STATUE_SHATTER,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "a marble statue",
        .flavorText = "The cold marble statue has weathered the years with grace."
    },
    {
        .displayChar = G_DOORWAY,
        .foreColor = &wallBackColor,
        .backColor = &floorBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_PORTAL_ACTIVATE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_ITEMS),
        .mechFlags = (TM_STAND_IN_TILE | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a stone archway",
        .flavorText = "This ancient moss-covered stone archway radiates a strange, alien energy."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_TURRET_EMERGE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "a stone wall",
        .flavorText = "The rough stone wall is firm and unyielding."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_WALL_SHATTER,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "a stone wall",
        .flavorText = "The rough stone wall is firm and unyielding."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_DARKENING_FLOOR,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_DARK_FLOOR,
        .promoteChance = 1500,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = DARKNESS_CLOUD_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_PLAYER_ENTRY),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_ALTAR,
        .foreColor = &altarForeColor,
        .backColor = &altarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a candle-lit altar",
        .flavorText = "a gilded altar is adorned with candles that flicker in the breeze."
    },
    {
        .displayChar = G_ORB_ALTAR,
        .foreColor = &altarForeColor,
        .backColor = &altarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_PROMOTES_WITH_KEY | TM_IS_WIRED | TM_LIST_IN_SIDEBAR),
        .description = "a candle-lit altar",
        .flavorText = "ornate gilding spirals around a spherical depression in the top of the altar."
    },
    {
        .displayChar = G_ALTAR,
        .foreColor = &altarForeColor,
        .backColor = &altarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_ITEM_CAGE_CLOSE,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_WITHOUT_KEY | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a candle-lit altar",
        .flavorText = "a cage, open on the bottom, hangs over this altar on a retractable chain."
    },
    {
        .displayChar = G_CLOSED_CAGE,
        .foreColor = &altarBackColor,
        .backColor = &veryDarkGray,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_ITEM_CAGE_OPEN,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_WITH_KEY | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "an iron cage",
        .flavorText = "the missing item must be replaced before you can access the remaining items."
    },
    {
        .displayChar = G_SAC_ALTAR,
        .foreColor = &altarForeColor,
        .backColor = &altarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_ALTAR_INERT,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_ITEM_PICKUP | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a candle-lit altar",
        .flavorText = "a weathered stone altar is adorned with candles that flicker in the breeze."
    },
    {
        .displayChar = G_SAC_ALTAR,
        .foreColor = &altarForeColor,
        .backColor = &altarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_ALTAR_RETRACT,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_ITEM_PICKUP | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a candle-lit altar",
        .flavorText = "a weathered stone altar is adorned with candles that flicker in the breeze."
    },
    {
        .displayChar = G_CLOSED_CAGE,
        .foreColor = &altarBackColor,
        .backColor = &veryDarkGray,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_CAGE_DISAPPEARS,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "an iron cage",
        .flavorText = "the cage won't budge. Perhaps there is a way to raise it nearby..."
    },
    {
        .displayChar = G_PEDESTAL,
        .foreColor = &altarForeColor,
        .backColor = &pedestalBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (0),
        .description = "a stone pedestal",
        .flavorText = "elaborate carvings wind around this ancient pedestal."
    },
    {
        .displayChar = G_OPEN_CAGE,
        .foreColor = &floorBackColor,
        .backColor = &veryDarkGray,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "an open cage",
        .flavorText = "the interior of the cage is filthy and reeks of decay."
    },
    {
        .displayChar = G_CLOSED_CAGE,
        .foreColor = &gray,
        .backColor = &darkGray,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_MONSTER_CAGE_OPENS,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_WITH_KEY | TM_LIST_IN_SIDEBAR | TM_INTERRUPT_EXPLORATION_WHEN_SEEN),
        .description = "a locked iron cage",
        .flavorText = "the bars of the cage are firmly set and will not budge."
    },
    {
        .displayChar = G_CLOSED_COFFIN,
        .foreColor = &bridgeFrontColor,
        .backColor = &bridgeBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 20,
        .fireType = DF_COFFIN_BURNS,
        .discoverType = 0,
        .promoteType = DF_COFFIN_BURSTS,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_IS_WIRED | TM_VANISHES_UPON_PROMOTION | TM_LIST_IN_SIDEBAR),
        .description = "a sealed coffin",
        .flavorText = "a coffin made from thick wooden planks rests in a bed of moss."
    },
    {
        .displayChar = G_OPEN_COFFIN,
        .foreColor = &black,
        .backColor = &bridgeBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 20,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_LIST_IN_SIDEBAR),
        .description = "an empty coffin",
        .flavorText = "an open wooden coffin rests in a bed of moss."
    },

    // traps (part of dungeon layer):
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_POISON_GAS_CLOUD,
        .discoverType = DF_SHOW_POISON_GAS_TRAP,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_IS_SECRET),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_TRAP,
        .foreColor = &poisonGasColor,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_POISON_GAS_CLOUD,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a caustic gas trap",
        .flavorText = "there is a hidden pressure plate in the floor above a reserve of caustic gas."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_POISON_GAS_CLOUD,
        .discoverType = DF_SHOW_TRAPDOOR,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_AUTO_DESCENT),
        .mechFlags = (TM_IS_SECRET),
        .description = "the ground",
        .flavorText = "you plunge through a hidden trap door!"
    },
    {
        .displayChar = G_CHASM,
        .foreColor = &chasmForeColor,
        .backColor = &black,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_POISON_GAS_CLOUD,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_AUTO_DESCENT),
        .mechFlags = (0),
        .description = "a hole",
        .flavorText = "you plunge through a hole in the ground!"
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = DF_SHOW_PARALYSIS_GAS_TRAP,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_IS_SECRET | TM_IS_WIRED),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_TRAP,
        .foreColor = &pink,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a paralysis trigger",
        .flavorText = "there is a hidden pressure plate in the floor."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = DF_DISCOVER_PARALYSIS_VENT,
        .promoteType = DF_PARALYSIS_VENT_SPEW,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET | TM_IS_WIRED),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_VENT,
        .foreColor = &pink,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_PARALYSIS_VENT_SPEW,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "an inactive gas vent",
        .flavorText = "A dormant gas vent is connected to a reserve of paralytic gas."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_CONFUSION_GAS_TRAP_CLOUD,
        .discoverType = DF_SHOW_CONFUSION_GAS_TRAP,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_IS_SECRET),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_TRAP,
        .foreColor = &confusionGasColor,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_CONFUSION_GAS_TRAP_CLOUD,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a confusion trap",
        .flavorText = "A hidden pressure plate accompanies a reserve of psychotropic gas."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_FLAMETHROWER,
        .discoverType = DF_SHOW_FLAMETHROWER_TRAP,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_IS_SECRET),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_TRAP,
        .foreColor = &red,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_FLAMETHROWER,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a fire trap",
        .flavorText = "A hidden pressure plate is connected to a crude flamethrower mechanism."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_FLOOD,
        .discoverType = DF_SHOW_FLOOD_TRAP,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_IS_SECRET),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_TRAP,
        .foreColor = &blue,
        .backColor = 0,
        .drawPriority = 58,
        .chanceToIgnite = 0,
        .fireType = DF_FLOOD,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a flood trap",
        .flavorText = "A hidden pressure plate is connected to floodgates in the walls and ceiling."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_NET,
        .discoverType = DF_SHOW_NET_TRAP,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_IS_SECRET),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_TRAP,
        .foreColor = &tanColor,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_NET,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a net trap",
        .flavorText = "you see netting subtly concealed in the ceiling over a hidden pressure plate."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_AGGRAVATE_TRAP,
        .discoverType = DF_SHOW_ALARM_TRAP,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_IS_SECRET),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_TRAP,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_AGGRAVATE_TRAP,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "an alarm trap",
        .flavorText = "a hidden pressure plate is connected to a loud alarm mechanism."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = DF_SHOW_POISON_GAS_VENT,
        .promoteType = DF_POISON_GAS_VENT_OPEN,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET | TM_IS_WIRED),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_VENT,
        .foreColor = &floorForeColor,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_POISON_GAS_VENT_OPEN,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "an inactive gas vent",
        .flavorText = "An inactive gas vent is hidden in a crevice in the ground."
    },
    {
        .displayChar = G_VENT,
        .foreColor = &floorForeColor,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_VENT_SPEW_POISON_GAS,
        .promoteChance = 10000,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a gas vent",
        .flavorText = "Clouds of caustic gas are wafting out of a hidden vent in the floor."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = DF_SHOW_METHANE_VENT,
        .promoteType = DF_METHANE_VENT_OPEN,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET | TM_IS_WIRED),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_VENT,
        .foreColor = &floorForeColor,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_METHANE_VENT_OPEN,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "an inactive gas vent",
        .flavorText = "An inactive gas vent is hidden in a crevice in the ground."
    },
    {
        .displayChar = G_VENT,
        .foreColor = &floorForeColor,
        .backColor = 0,
        .drawPriority = 30,
        .chanceToIgnite = 15,
        .fireType = DF_EMBERS,
        .discoverType = 0,
        .promoteType = DF_VENT_SPEW_METHANE,
        .promoteChance = 5000,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a gas vent",
        .flavorText = "Clouds of explosive gas are wafting out of a hidden vent in the floor."
    },
    {
        .displayChar = G_VENT,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 15,
        .chanceToIgnite = 15,
        .fireType = DF_EMBERS,
        .discoverType = 0,
        .promoteType = DF_STEAM_PUFF,
        .promoteChance = 250,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_ITEMS),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a steam vent",
        .flavorText = "A natural crevice in the floor periodically vents scalding gouts of steam."
    },
    {
        .displayChar = G_TRAP,
        .foreColor = &white,
        .backColor = &chasmEdgeBackColor,
        .drawPriority = 15,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_MACHINE_PRESSURE_PLATE_USED,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_DF_TRAP),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a pressure plate",
        .flavorText = "There is an exposed pressure plate here. A thrown item might trigger it."
    },
    {
        .displayChar = G_TRAP,
        .foreColor = &darkGray,
        .backColor = &chasmEdgeBackColor,
        .drawPriority = 15,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_LIST_IN_SIDEBAR),
        .description = "an inactive pressure plate",
        .flavorText = "This pressure plate has already been depressed."
    },
    {
        .displayChar = G_MAGIC_GLYPH,
        .foreColor = &glyphColor,
        .backColor = 0,
        .drawPriority = 42,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_INACTIVE_GLYPH,
        .promoteChance = 0,
        .glowLight = GLYPH_LIGHT_DIM,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_PLAYER_ENTRY | TM_VISUALLY_DISTINCT),
        .description = "a magical glyph",
        .flavorText = "A strange glyph, engraved into the floor, flickers with magical light."
    },
    {
        .displayChar = G_MAGIC_GLYPH,
        .foreColor = &glyphColor,
        .backColor = 0,
        .drawPriority = 42,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_ACTIVE_GLYPH,
        .promoteChance = 10000,
        .glowLight = GLYPH_LIGHT_BRIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "a glowing glyph",
        .flavorText = "A strange glyph, engraved into the floor, radiates magical light."
    },
    {
        .displayChar = G_DEWAR,
        .foreColor = &poisonGasColor,
        .backColor = &darkGray,
        .drawPriority = 10,
        .chanceToIgnite = 20,
        .fireType = DF_DEWAR_CAUSTIC,
        .discoverType = 0,
        .promoteType = DF_DEWAR_CAUSTIC,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_LIST_IN_SIDEBAR | TM_PROMOTES_ON_PLAYER_ENTRY | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a glass dewar of caustic gas",
        .flavorText = ""
    },
    {
        .displayChar = G_DEWAR,
        .foreColor = &confusionGasColor,
        .backColor = &darkGray,
        .drawPriority = 10,
        .chanceToIgnite = 20,
        .fireType = DF_DEWAR_CONFUSION,
        .discoverType = 0,
        .promoteType = DF_DEWAR_CONFUSION,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_LIST_IN_SIDEBAR | TM_PROMOTES_ON_PLAYER_ENTRY | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a glass dewar of confusion gas",
        .flavorText = ""
    },
    {
        .displayChar = G_DEWAR,
        .foreColor = &pink,
        .backColor = &darkGray,
        .drawPriority = 10,
        .chanceToIgnite = 20,
        .fireType = DF_DEWAR_PARALYSIS,
        .discoverType = 0,
        .promoteType = DF_DEWAR_PARALYSIS,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_LIST_IN_SIDEBAR | TM_PROMOTES_ON_PLAYER_ENTRY | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a glass dewar of paralytic gas",
        .flavorText = ""
    },
    {
        .displayChar = G_DEWAR,
        .foreColor = &methaneColor,
        .backColor = &darkGray,
        .drawPriority = 10,
        .chanceToIgnite = 20,
        .fireType = DF_DEWAR_METHANE,
        .discoverType = 0,
        .promoteType = DF_DEWAR_METHANE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_LIST_IN_SIDEBAR | TM_PROMOTES_ON_PLAYER_ENTRY | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a glass dewar of methane gas",
        .flavorText = ""
    },

    // liquid layer
    {
        .displayChar = G_LIQUID,
        .foreColor = &deepWaterForeColor,
        .backColor = &deepWaterBackColor,
        .drawPriority = 40,
        .chanceToIgnite = 100,
        .fireType = DF_STEAM_ACCUMULATION,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE | T_IS_DEEP_WATER),
        .mechFlags = (TM_ALLOWS_SUBMERGING | TM_STAND_IN_TILE | TM_EXTINGUISHES_FIRE),
        .description = "the murky waters",
        .flavorText = "the current tugs you in all directions."
    },
    {
        .displayChar = 0,
        .foreColor = &shallowWaterForeColor,
        .backColor = &shallowWaterBackColor,
        .drawPriority = 55,
        .chanceToIgnite = 0,
        .fireType = DF_STEAM_ACCUMULATION,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING),
        .description = "shallow water",
        .flavorText = "the water is cold and reaches your knees."
    },
    {
        .displayChar = G_BOG,
        .foreColor = &mudForeColor,
        .backColor = &mudBackColor,
        .drawPriority = 55,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_METHANE_GAS_PUFF,
        .promoteChance = 100,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_ALLOWS_SUBMERGING),
        .description = "a bog",
        .flavorText = "you are knee-deep in thick, foul-smelling mud."
    },
    {
        .displayChar = G_CHASM,
        .foreColor = &chasmForeColor,
        .backColor = &black,
        .drawPriority = 40,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_AUTO_DESCENT),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a chasm",
        .flavorText = "you plunge downward into the chasm!"
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &white,
        .backColor = &chasmEdgeBackColor,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "the brink of a chasm",
        .flavorText = "chilly winds blow upward from the stygian depths."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_SPREADABLE_COLLAPSE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &white,
        .backColor = &chasmEdgeBackColor,
        .drawPriority = 45,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_COLLAPSE_SPREADS,
        .promoteChance = 2500,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "the crumbling ground",
        .flavorText = "cracks are appearing in the ground beneath your feet!"
    },
    {
        .displayChar = G_LIQUID,
        .foreColor = &fireForeColor,
        .backColor = &lavaBackColor,
        .drawPriority = 40,
        .chanceToIgnite = 0,
        .fireType = DF_OBSIDIAN,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = LAVA_LIGHT,
        .flags = (T_LAVA_INSTA_DEATH),
        .mechFlags = (TM_STAND_IN_TILE | TM_ALLOWS_SUBMERGING),
        .description = "lava",
        .flavorText = "searing heat rises from the lava."
    },
    {
        .displayChar = G_LIQUID,
        .foreColor = &fireForeColor,
        .backColor = &lavaBackColor,
        .drawPriority = 40,
        .chanceToIgnite = 0,
        .fireType = DF_OBSIDIAN,
        .discoverType = 0,
        .promoteType = DF_RETRACTING_LAVA,
        .promoteChance = 0,
        .glowLight = LAVA_LIGHT,
        .flags = (T_LAVA_INSTA_DEATH),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_ALLOWS_SUBMERGING),
        .description = "lava",
        .flavorText = "searing heat rises from the lava."
    },
    {
        .displayChar = G_LIQUID,
        .foreColor = &fireForeColor,
        .backColor = &lavaBackColor,
        .drawPriority = 40,
        .chanceToIgnite = 0,
        .fireType = DF_OBSIDIAN,
        .discoverType = 0,
        .promoteType = DF_OBSIDIAN_WITH_STEAM,
        .promoteChance = -1500,
        .glowLight = LAVA_LIGHT,
        .flags = (T_LAVA_INSTA_DEATH),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_ALLOWS_SUBMERGING),
        .description = "cooling lava",
        .flavorText = "searing heat rises from the lava."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 90,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = SUN_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a patch of sunlight",
        .flavorText = "sunlight streams through cracks in the ceiling."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 90,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = DARKNESS_PATCH_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "a patch of shadows",
        .flavorText = "this area happens to be cloaked in shadows -- perhaps a safe place to hide."
    },
    {
        .displayChar = G_ASHES,
        .foreColor = &brimstoneForeColor,
        .backColor = &brimstoneBackColor,
        .drawPriority = 40,
        .chanceToIgnite = 100,
        .fireType = DF_INERT_BRIMSTONE,
        .discoverType = 0,
        .promoteType = DF_INERT_BRIMSTONE,
        .promoteChance = 10,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE | T_SPONTANEOUSLY_IGNITES),
        .mechFlags = (0),
        .description = "hissing brimstone",
        .flavorText = "the jagged brimstone hisses and spits ominously as it crunches under your feet."
    },
    {
        .displayChar = G_ASHES,
        .foreColor = &brimstoneForeColor,
        .backColor = &brimstoneBackColor,
        .drawPriority = 40,
        .chanceToIgnite = 0,
        .fireType = DF_INERT_BRIMSTONE,
        .discoverType = 0,
        .promoteType = DF_ACTIVE_BRIMSTONE,
        .promoteChance = 800,
        .glowLight = NO_LIGHT,
        .flags = (T_SPONTANEOUSLY_IGNITES),
        .mechFlags = (0),
        .description = "hissing brimstone",
        .flavorText = "the jagged brimstone hisses and spits ominously as it crunches under your feet."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &darkGray,
        .backColor = &obsidianBackColor,
        .drawPriority = 50,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "the obsidian ground",
        .flavorText = "the ground has fused into obsidian."
    },
    {
        .displayChar = G_BRIDGE,
        .foreColor = &bridgeFrontColor,
        .backColor = &bridgeBackColor,
        .drawPriority = 45,
        .chanceToIgnite = 50,
        .fireType = DF_BRIDGE_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "a rickety rope bridge",
        .flavorText = "the rickety rope bridge creaks underfoot."
    },
    {
        .displayChar = G_BRIDGE,
        .foreColor = &bridgeFrontColor,
        .backColor = &bridgeBackColor,
        .drawPriority = 45,
        .chanceToIgnite = 50,
        .fireType = DF_BRIDGE_FALL,
        .discoverType = 0,
        .promoteType = DF_BRIDGE_FALL,
        .promoteChance = 10000,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "a plummeting bridge",
        .flavorText = "the bridge is plunging into the chasm before your eyes!"
    },
    {
        .displayChar = G_BRIDGE,
        .foreColor = &bridgeFrontColor,
        .backColor = &bridgeBackColor,
        .drawPriority = 45,
        .chanceToIgnite = 50,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "a rickety rope bridge",
        .flavorText = "the rickety rope bridge is staked to the edge of the chasm."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &white,
        .backColor = &chasmEdgeBackColor,
        .drawPriority = 20,
        .chanceToIgnite = 50,
        .fireType = DF_BRIDGE_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "a stone bridge",
        .flavorText = "the narrow stone bridge winds precariously across the chasm."
    },
    {
        .displayChar = 0,
        .foreColor = &shallowWaterForeColor,
        .backColor = &shallowWaterBackColor,
        .drawPriority = 60,
        .chanceToIgnite = 0,
        .fireType = DF_STEAM_ACCUMULATION,
        .discoverType = 0,
        .promoteType = DF_SPREADABLE_WATER,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING),
        .description = "shallow water",
        .flavorText = "the water is cold and reaches your knees."
    },
    {
        .displayChar = 0,
        .foreColor = &shallowWaterForeColor,
        .backColor = &shallowWaterBackColor,
        .drawPriority = 60,
        .chanceToIgnite = 0,
        .fireType = DF_STEAM_ACCUMULATION,
        .discoverType = 0,
        .promoteType = DF_WATER_SPREADS,
        .promoteChance = 2500,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING),
        .description = "shallow water",
        .flavorText = "the water is cold and reaches your knees."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &mudForeColor,
        .backColor = &mudBackColor,
        .drawPriority = 55,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_MUD_ACTIVATE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_ALLOWS_SUBMERGING),
        .description = "a bog",
        .flavorText = "you are knee-deep in thick, foul-smelling mud."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &white,
        .backColor = &lightBlue,
        .drawPriority = 35,
        .chanceToIgnite = 100,
        .fireType = DF_DEEP_WATER_THAW,
        .discoverType = 0,
        .promoteType = DF_DEEP_WATER_MELTING,
        .promoteChance = -100,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "ice",
        .flavorText = "a sheet of ice extends into the water, ripples frozen into its glossy surface."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &black,
        .backColor = &lightBlue,
        .drawPriority = 35,
        .chanceToIgnite = 100,
        .fireType = DF_DEEP_WATER_THAW,
        .discoverType = 0,
        .promoteType = DF_DEEP_WATER_THAW,
        .promoteChance = 10000,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "melting ice",
        .flavorText = "cracks extend across the surface of the ice as it melts before your eyes."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &white,
        .backColor = &lightBlue,
        .drawPriority = 35,
        .chanceToIgnite = 100,
        .fireType = DF_SHALLOW_WATER_THAW,
        .discoverType = 0,
        .promoteType = DF_SHALLOW_WATER_MELTING,
        .promoteChance = -100,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "ice",
        .flavorText = "a sheet of ice extends into the water, ripples frozen into its glossy surface."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &black,
        .backColor = &lightBlue,
        .drawPriority = 35,
        .chanceToIgnite = 100,
        .fireType = DF_DEEP_WATER_THAW,
        .discoverType = 0,
        .promoteType = DF_SHALLOW_WATER_THAW,
        .promoteChance = 10000,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "melting ice",
        .flavorText = "cracks extend across the surface of the ice as it melts before your eyes."
    },

    // surface layer
    {
        .displayChar = G_CHASM,
        .foreColor = &chasmForeColor,
        .backColor = &black,
        .drawPriority = 9,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_HOLE_DRAIN,
        .promoteChance = -1000,
        .glowLight = NO_LIGHT,
        .flags = (T_AUTO_DESCENT),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "a hole",
        .flavorText = "you plunge downward into the hole!"
    },
    {
        .displayChar = G_CHASM,
        .foreColor = &chasmForeColor,
        .backColor = &black,
        .drawPriority = 9,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_HOLE_DRAIN,
        .promoteChance = -1000,
        .glowLight = DESCENT_LIGHT,
        .flags = (T_AUTO_DESCENT),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "a hole",
        .flavorText = "you plunge downward into the hole!"
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &white,
        .backColor = &chasmEdgeBackColor,
        .drawPriority = 50,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = -500,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "translucent ground",
        .flavorText = "chilly gusts of air blow upward through the translucent floor."
    },
    {
        .displayChar = G_LIQUID,
        .foreColor = &deepWaterForeColor,
        .backColor = &deepWaterBackColor,
        .drawPriority = 41,
        .chanceToIgnite = 100,
        .fireType = DF_STEAM_ACCUMULATION,
        .discoverType = 0,
        .promoteType = DF_FLOOD_DRAIN,
        .promoteChance = -200,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE | T_IS_DEEP_WATER),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING),
        .description = "sloshing water",
        .flavorText = "roiling water floods the room."
    },
    {
        .displayChar = 0,
        .foreColor = &shallowWaterForeColor,
        .backColor = &shallowWaterBackColor,
        .drawPriority = 50,
        .chanceToIgnite = 0,
        .fireType = DF_STEAM_ACCUMULATION,
        .discoverType = 0,
        .promoteType = DF_PUDDLE,
        .promoteChance = -100,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING),
        .description = "shallow water",
        .flavorText = "knee-deep water drains slowly into holes in the floor."
    },
    {
        .displayChar = G_GRASS,
        .foreColor = &grassColor,
        .backColor = 0,
        .drawPriority = 60,
        .chanceToIgnite = 15,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "grass-like fungus",
        .flavorText = "grass-like fungus crunches underfoot."
    },
    {
        .displayChar = G_GRASS,
        .foreColor = &deadGrassColor,
        .backColor = 0,
        .drawPriority = 60,
        .chanceToIgnite = 40,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "withered fungus",
        .flavorText = "dead fungus covers the ground."
    },
    {
        .displayChar = G_GRASS,
        .foreColor = &grayFungusColor,
        .backColor = 0,
        .drawPriority = 51,
        .chanceToIgnite = 10,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "withered fungus",
        .flavorText = "groping tendrils of pale fungus rise from the muck."
    },
    {
        .displayChar = G_GRASS,
        .foreColor = &fungusColor,
        .backColor = 0,
        .drawPriority = 60,
        .chanceToIgnite = 10,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = FUNGUS_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "luminescent fungus",
        .flavorText = "luminescent fungus casts a pale, eerie glow."
    },
    {
        .displayChar = G_LICHEN,
        .foreColor = &lichenColor,
        .backColor = 0,
        .drawPriority = 60,
        .chanceToIgnite = 50,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_LICHEN_GROW,
        .promoteChance = 10000,
        .glowLight = NO_LIGHT,
        .flags = (T_CAUSES_POISON | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "deadly lichen",
        .flavorText = "venomous barbs cover the quivering tendrils of this fast-growing lichen."
    },
    {
        .displayChar = G_GRASS,
        .foreColor = &hayColor,
        .backColor = &refuseBackColor,
        .drawPriority = 57,
        .chanceToIgnite = 50,
        .fireType = DF_STENCH_BURN,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "filthy hay",
        .flavorText = "a pile of hay, matted with filth, has been arranged here as a makeshift bed."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &humanBloodColor,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a pool of blood",
        .flavorText = "the floor is splattered with blood."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &insectBloodColor,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a pool of green blood",
        .flavorText = "the floor is splattered with green blood."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &poisonGasColor,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a pool of purple blood",
        .flavorText = "the floor is splattered with purple blood."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &acidBackColor,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "a puddle of acid",
        .flavorText = "the floor is splattered with acid."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &vomitColor,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a puddle of vomit",
        .flavorText = "the floor is caked with vomit."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &urineColor,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 100,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "a puddle of urine",
        .flavorText = "a puddle of urine covers the ground."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &white,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = UNICORN_POOP_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "unicorn poop",
        .flavorText = "a pile of lavender-scented unicorn poop sparkles with rainbow light."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &wormColor,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a pool of worm entrails",
        .flavorText = "worm viscera cover the ground."
    },
    {
        .displayChar = G_ASHES,
        .foreColor = &ashForeColor,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a pile of ashes",
        .flavorText = "charcoal and ash crunch underfoot."
    },
    {
        .displayChar = G_ASHES,
        .foreColor = &ashForeColor,
        .backColor = 0,
        .drawPriority = 87,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "burned carpet",
        .flavorText = "the carpet has been scorched by an ancient fire."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &shallowWaterBackColor,
        .backColor = 0,
        .drawPriority = 80,
        .chanceToIgnite = 20,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 100,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "a puddle of water",
        .flavorText = "a puddle of water covers the ground."
    },
    {
        .displayChar = G_BONES,
        .foreColor = &bonesForeColor,
        .backColor = 0,
        .drawPriority = 70,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a pile of bones",
        .flavorText = "unidentifiable bones, yellowed with age, litter the ground."
    },
    {
        .displayChar = G_RUBBLE,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 70,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a pile of rubble",
        .flavorText = "rocky rubble covers the ground."
    },
    {
        .displayChar = G_BONES,
        .foreColor = &mudBackColor,
        .backColor = &refuseBackColor,
        .drawPriority = 50,
        .chanceToIgnite = 20,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a pile of filthy effects",
        .flavorText = "primitive tools, carvings and trinkets are strewn about the area."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &white,
        .backColor = 0,
        .drawPriority = 70,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "shattered glass",
        .flavorText = "jagged chunks of glass from the broken dewar litter the ground."
    },
    {
        .displayChar = G_FLOOR_ALT,
        .foreColor = &ectoplasmColor,
        .backColor = 0,
        .drawPriority = 70,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = ECTOPLASM_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "ectoplasmic residue",
        .flavorText = "a thick, glowing substance has congealed on the ground."
    },
    {
        .displayChar = G_ASHES,
        .foreColor = &fireForeColor,
        .backColor = 0,
        .drawPriority = 70,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_ASH,
        .promoteChance = 300,
        .glowLight = EMBER_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "sputtering embers",
        .flavorText = "sputtering embers cover the ground."
    },
    {
        .displayChar = G_WEB,
        .foreColor = &white,
        .backColor = 0,
        .drawPriority = 19,
        .chanceToIgnite = 100,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_ENTANGLES | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "a spiderweb",
        .flavorText = "thick, sticky spiderwebs fill the area."
    },
    {
        .displayChar = G_NET,
        .foreColor = &brown,
        .backColor = 0,
        .drawPriority = 19,
        .chanceToIgnite = 40,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_ENTANGLES | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "a net",
        .flavorText = "a dense tangle of netting fills the area."
    },
    {
        .displayChar = G_FOLIAGE,
        .foreColor = &foliageColor,
        .backColor = 0,
        .drawPriority = 45,
        .chanceToIgnite = 15,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_TRAMPLED_FOLIAGE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_VISION | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP),
        .description = "dense foliage",
        .flavorText = "dense foliage fills the area, thriving on what sunlight trickles in."
    },
    {
        .displayChar = G_FOLIAGE,
        .foreColor = &deadFoliageColor,
        .backColor = 0,
        .drawPriority = 45,
        .chanceToIgnite = 80,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_SMALL_DEAD_GRASS,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_VISION | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP),
        .description = "dead foliage",
        .flavorText = "the decaying husk of a fungal growth fills the area."
    },
    {
        .displayChar = G_GRASS,
        .foreColor = &foliageColor,
        .backColor = 0,
        .drawPriority = 60,
        .chanceToIgnite = 15,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_FOLIAGE_REGROW,
        .promoteChance = 100,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "trampled foliage",
        .flavorText = "dense foliage fills the area, thriving on what sunlight trickles in."
    },
    {
        .displayChar = G_FOLIAGE,
        .foreColor = &fungusForestLightColor,
        .backColor = 0,
        .drawPriority = 45,
        .chanceToIgnite = 15,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_TRAMPLED_FUNGUS_FOREST,
        .promoteChance = 0,
        .glowLight = FUNGUS_FOREST_LIGHT,
        .flags = (T_OBSTRUCTS_VISION | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP),
        .description = "a luminescent fungal forest",
        .flavorText = "luminescent fungal growth fills the area, groping upward from the rich soil."
    },
    {
        .displayChar = G_GRASS,
        .foreColor = &fungusForestLightColor,
        .backColor = 0,
        .drawPriority = 60,
        .chanceToIgnite = 15,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_FUNGUS_FOREST_REGROW,
        .promoteChance = 100,
        .glowLight = FUNGUS_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "trampled fungal foliage",
        .flavorText = "luminescent fungal growth fills the area, groping upward from the rich soil."
    },
    {
        .displayChar = G_CRYSTAL,
        .foreColor = &forceFieldColor,
        .backColor = &forceFieldColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_FORCEFIELD_MELT,
        .promoteChance = -200,
        .glowLight = FORCEFIELD_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_GAS | T_OBSTRUCTS_DIAGONAL_MOVEMENT),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP),
        .description = "a green crystal",
        .flavorText = "The translucent green crystal is melting away in front of your eyes."
    },
    {
        .displayChar = G_CRYSTAL,
        .foreColor = &black,
        .backColor = &forceFieldColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = -10000,
        .glowLight = FORCEFIELD_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_GAS | T_OBSTRUCTS_DIAGONAL_MOVEMENT),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "a dissolving crystal",
        .flavorText = "The translucent green crystal is melting away in front of your eyes."
    },
    {
        .displayChar = G_MAGIC_GLYPH,
        .foreColor = &sacredGlyphColor,
        .backColor = 0,
        .drawPriority = 57,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = SACRED_GLYPH_LIGHT,
        .flags = (T_SACRED),
        .mechFlags = (0),
        .description = "a sacred glyph",
        .flavorText = "a sacred glyph adorns the floor, glowing with a powerful warding enchantment."
    },
    {
        .displayChar = G_CHAIN_TOP_LEFT,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 20,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "an iron manacle",
        .flavorText = "a thick iron manacle is anchored to the ceiling."
    },
    {
        .displayChar = G_CHAIN_BOTTOM_RIGHT,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 20,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "an iron manacle",
        .flavorText = "a thick iron manacle is anchored to the floor."
    },
    {
        .displayChar = G_CHAIN_TOP_RIGHT,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 20,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "an iron manacle",
        .flavorText = "a thick iron manacle is anchored to the ceiling."
    },
    {
        .displayChar = G_CHAIN_BOTTOM_LEFT,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 20,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "an iron manacle",
        .flavorText = "a thick iron manacle is anchored to the floor."
    },
    {
        .displayChar = G_CHAIN_TOP,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 20,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "an iron manacle",
        .flavorText = "a thick iron manacle is anchored to the wall."
    },
    {
        .displayChar = G_CHAIN_BOTTOM,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 20,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "an iron manacle",
        .flavorText = "a thick iron manacle is anchored to the wall."
    },
    {
        .displayChar = G_CHAIN_LEFT,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 20,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "an iron manacle",
        .flavorText = "a thick iron manacle is anchored to the wall."
    },
    {
        .displayChar = G_CHAIN_RIGHT,
        .foreColor = &gray,
        .backColor = 0,
        .drawPriority = 20,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "an iron manacle",
        .flavorText = "a thick iron manacle is anchored to the wall."
    },
    {
        .displayChar = 0,
        .foreColor = 0,
        .backColor = 0,
        .drawPriority = 1,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 10000,
        .glowLight = PORTAL_ACTIVATE_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "blinding light",
        .flavorText = "blinding light streams out of the archway."
    },
    {
        .displayChar = 0,
        .foreColor = 0,
        .backColor = 0,
        .drawPriority = 100,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 10000,
        .glowLight = GLYPH_LIGHT_BRIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),
        .description = "a red glow",
        .flavorText = "a red glow fills the area."
    },

    // fire tiles
    {
        .displayChar = G_FIRE,
        .foreColor = &fireForeColor,
        .backColor = 0,
        .drawPriority = 10,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_EMBERS,
        .promoteChance = 500,
        .glowLight = FIRE_LIGHT,
        .flags = (T_IS_FIRE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "billowing flames",
        .flavorText = "flames billow upward."
    },
    {
        .displayChar = G_FIRE,
        .foreColor = &fireForeColor,
        .backColor = 0,
        .drawPriority = 10,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 2500,
        .glowLight = BRIMSTONE_FIRE_LIGHT,
        .flags = (T_IS_FIRE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "sulfurous flames",
        .flavorText = "sulfurous flames leap from the unstable bed of brimstone."
    },
    {
        .displayChar = G_FIRE,
        .foreColor = &fireForeColor,
        .backColor = 0,
        .drawPriority = 10,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_OBSIDIAN,
        .promoteChance = 5000,
        .glowLight = FIRE_LIGHT,
        .flags = (T_IS_FIRE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "clouds of infernal flame",
        .flavorText = "billowing infernal flames eat at the floor."
    },
    {
        .displayChar = G_FIRE,
        .foreColor = &fireForeColor,
        .backColor = 0,
        .drawPriority = 10,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 8000,
        .glowLight = FIRE_LIGHT,
        .flags = (T_IS_FIRE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "a cloud of burning gas",
        .flavorText = "burning gas fills the air with flame."
    },
    {
        .displayChar = G_FIRE,
        .foreColor = &yellow,
        .backColor = 0,
        .drawPriority = 10,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 10000,
        .glowLight = EXPLOSION_LIGHT,
        .flags = (T_IS_FIRE | T_CAUSES_EXPLOSIVE_DAMAGE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "a violent explosion",
        .flavorText = "the force of the explosion slams into you."
    },
    {
        .displayChar = G_FIRE,
        .foreColor = &white,
        .backColor = 0,
        .drawPriority = 10,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 10000,
        .glowLight = INCENDIARY_DART_LIGHT,
        .flags = (T_IS_FIRE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "a flash of fire",
        .flavorText = "flames burst out of the incendiary dart."
    },
    {
        .displayChar = G_FIRE,
        .foreColor = &white,
        .backColor = 0,
        .drawPriority = 10,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_EMBERS,
        .promoteChance = 3000,
        .glowLight = FIRE_LIGHT,
        .flags = (T_IS_FIRE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "crackling flames",
        .flavorText = "crackling flames rise from the blackened item."
    },
    {
        .displayChar = G_FIRE,
        .foreColor = &white,
        .backColor = 0,
        .drawPriority = 10,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_EMBERS,
        .promoteChance = 3000,
        .glowLight = FIRE_LIGHT,
        .flags = (T_IS_FIRE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),
        .description = "greasy flames",
        .flavorText = "greasy flames rise from the corpse."
    },
                                                                                   
    // gas layer
    {
        .displayChar =    ' ',
        .foreColor = 0,
        .backColor = &poisonGasColor,
        .drawPriority = 35,
        .chanceToIgnite = 100,
        .fireType = DF_GAS_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE | T_CAUSES_DAMAGE),
        .mechFlags = (TM_STAND_IN_TILE | TM_GAS_DISSIPATES),
        .description = "a cloud of caustic gas",
        .flavorText = "you can feel the purple gas eating at your flesh."
    },
    {
        .displayChar =    ' ',
        .foreColor = 0,
        .backColor = &confusionGasColor,
        .drawPriority = 35,
        .chanceToIgnite = 100,
        .fireType = DF_GAS_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = CONFUSION_GAS_LIGHT,
        .flags = (T_IS_FLAMMABLE | T_CAUSES_CONFUSION),
        .mechFlags = (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),
        .description = "a cloud of confusion gas",
        .flavorText = "the rainbow-colored gas tickles your brain."
    },
    {
        .displayChar =    ' ',
        .foreColor = 0,
        .backColor = &vomitColor,
        .drawPriority = 35,
        .chanceToIgnite = 100,
        .fireType = DF_GAS_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE | T_CAUSES_NAUSEA),
        .mechFlags = (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),
        .description = "a cloud of putrescence",
        .flavorText = "the stench of rotting flesh is overpowering."
    },
    {
        .displayChar =    ' ',
        .foreColor = 0,
        .backColor = &vomitColor,
        .drawPriority = 35,
        .chanceToIgnite = 0,
        .fireType = DF_GAS_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_CAUSES_NAUSEA),
        .mechFlags = (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),
        .description = "a cloud of putrid smoke",
        .flavorText = "you retch violently at the smell of the greasy smoke."
    },
    {
        .displayChar =    ' ',
        .foreColor = 0,
        .backColor = &pink,
        .drawPriority = 35,
        .chanceToIgnite = 100,
        .fireType = DF_GAS_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE | T_CAUSES_PARALYSIS),
        .mechFlags = (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),
        .description = "a cloud of paralytic gas",
        .flavorText = "the pale gas causes your muscles to stiffen."
    },
    {
        .displayChar =    ' ',
        .foreColor = 0,
        .backColor = &methaneColor,
        .drawPriority = 35,
        .chanceToIgnite = 100,
        .fireType = DF_GAS_FIRE,
        .discoverType = 0,
        .promoteType = DF_EXPLOSION_FIRE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_EXPLOSIVE_PROMOTE),
        .description = "a cloud of explosive gas",
        .flavorText = "the smell of explosive swamp gas fills the air."
    },
    {
        .displayChar =    ' ',
        .foreColor = 0,
        .backColor = &white,
        .drawPriority = 35,
        .chanceToIgnite = 0,
        .fireType = DF_GAS_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_CAUSES_DAMAGE),
        .mechFlags = (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),
        .description = "a cloud of scalding steam",
        .flavorText = "scalding steam fills the air!"
    },
    {
        .displayChar =    ' ',
        .foreColor = 0,
        .backColor = 0,
        .drawPriority = 35,
        .chanceToIgnite = 0,
        .fireType = DF_GAS_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = DARKNESS_CLOUD_LIGHT,
        .flags = (0),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a cloud of supernatural darkness",
        .flavorText = "everything is obscured by an aura of supernatural darkness."
    },
    {
        .displayChar =    ' ',
        .foreColor = 0,
        .backColor = &darkRed,
        .drawPriority = 35,
        .chanceToIgnite = 0,
        .fireType = DF_GAS_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_CAUSES_HEALING),
        .mechFlags = (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),
        .description = "a cloud of healing spores",
        .flavorText = "bloodwort spores, renowned for their healing properties, fill the air."
    },

    // bloodwort pods
    {
        .displayChar = G_BLOODWORT_STALK,
        .foreColor = &bloodflowerForeColor,
        .backColor = &bloodflowerBackColor,
        .drawPriority = 10,
        .chanceToIgnite = 20,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_BLOODFLOWER_PODS_GROW,
        .promoteChance = 100,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_IS_FLAMMABLE),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a bloodwort stalk",
        .flavorText = "this spindly plant grows seed pods famous for their healing properties."
    },
    {
        .displayChar = G_BLOODWORT_POD,
        .foreColor = &bloodflowerPodForeColor,
        .backColor = 0,
        .drawPriority = 11,
        .chanceToIgnite = 20,
        .fireType = DF_BLOODFLOWER_POD_BURST,
        .discoverType = 0,
        .promoteType = DF_BLOODFLOWER_POD_BURST,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_PLAYER_ENTRY | TM_VISUALLY_DISTINCT | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a bloodwort pod",
        .flavorText = "the bloodwort seed pod bursts, releasing a cloud of healing spores."
    },

    // shrine accoutrements
    {
        .displayChar = G_BEDROLL,
        .foreColor = &black,
        .backColor = &bedrollBackColor,
        .drawPriority = 57,
        .chanceToIgnite = 50,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "an abandoned bedroll",
        .flavorText = "a bedroll lies in the corner, disintegrating with age."
    },

    // algae
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_ALGAE_1,
        .promoteChance = 100,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "the ground",
        .flavorText = ""
    },
    {
        .displayChar = G_LIQUID,
        .foreColor = &deepWaterForeColor,
        .backColor = &deepWaterBackColor,
        .drawPriority = 40,
        .chanceToIgnite = 100,
        .fireType = DF_STEAM_ACCUMULATION,
        .discoverType = 0,
        .promoteType = DF_ALGAE_1,
        .promoteChance = 500,
        .glowLight = LUMINESCENT_ALGAE_BLUE_LIGHT,
        .flags = (T_IS_FLAMMABLE | T_IS_DEEP_WATER),
        .mechFlags = (TM_STAND_IN_TILE | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING),
        .description = "luminescent waters",
        .flavorText = "blooming algae fills the waters with a swirling luminescence."
    },
    {
        .displayChar = G_LIQUID,
        .foreColor = &deepWaterForeColor,
        .backColor = &deepWaterBackColor,
        .drawPriority = 39,
        .chanceToIgnite = 100,
        .fireType = DF_STEAM_ACCUMULATION,
        .discoverType = 0,
        .promoteType = DF_ALGAE_REVERT,
        .promoteChance = 300,
        .glowLight = LUMINESCENT_ALGAE_GREEN_LIGHT,
        .flags = (T_IS_FLAMMABLE | T_IS_DEEP_WATER),
        .mechFlags = (TM_STAND_IN_TILE | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING),
        .description = "luminescent waters",
        .flavorText = "blooming algae fills the waters with a swirling luminescence."
    },

    // ancient spirit terrain
    {
        .displayChar = G_VINE,
        .foreColor = &lichenColor,
        .backColor = 0,
        .drawPriority = 19,
        .chanceToIgnite = 100,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_ANCIENT_SPIRIT_GRASS,
        .promoteChance = 1000,
        .glowLight = NO_LIGHT,
        .flags = (T_ENTANGLES | T_CAUSES_DAMAGE | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_PROMOTES_ON_PLAYER_ENTRY),
        .description = "thorned vines",
        .flavorText = "thorned vines make a rustling noise as they quiver restlessly."
    }, // +tile
    {
        .displayChar = G_GRASS,
        .foreColor = &grassColor,
        .backColor = 0,
        .drawPriority = 60,
        .chanceToIgnite = 15,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a tuft of grass",
        .flavorText = "tufts of lush grass have improbably pushed upward through the stone ground."
    },

    // Yendor amulet floor tile
    {
        .displayChar = G_FLOOR,
        .foreColor = &floorForeColor,
        .backColor = &floorBackColor,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_ITEM_PICKUP),
        .description = "the ground",
        .flavorText = ""
    },

    // commutation device
    {
        .displayChar = G_ORB_ALTAR,
        .foreColor = &altarForeColor,
        .backColor = &greenAltarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_ALTAR_COMMUTE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_SWAP_ENCHANTS_ACTIVATION | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a commutation altar",
        .flavorText = "crude diagrams on this altar and its twin invite you to place items upon them."
    },
    {
        .displayChar = G_ORB_ALTAR,
        .foreColor = &black,
        .backColor = &greenAltarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a scorched altar",
        .flavorText = "scorch marks cover the surface of the altar, but it is cold to the touch."
    },
    {
        .displayChar = G_PIPES,
        .foreColor = &veryDarkGray,
        .backColor = 0,
        .drawPriority = 45,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_INERT_PIPE,
        .promoteChance = 0,
        .glowLight = CONFUSION_GAS_LIGHT,
        .flags = (0),
        .mechFlags = (TM_IS_WIRED | TM_VANISHES_UPON_PROMOTION),
        .description = "glowing glass pipes",
        .flavorText = "glass pipes are set into the floor and emit a soft glow of shifting color."
    },
    {
        .displayChar = G_PIPES,
        .foreColor = &black,
        .backColor = 0,
        .drawPriority = 45,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (0),
        .description = "charred glass pipes",
        .flavorText = "the inside of the glass pipes are charred."
    },

    // resurrection altar
    {
        .displayChar = G_ALTAR,
        .foreColor = &altarForeColor,
        .backColor = &goldAltarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_ALTAR_RESURRECT,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a resurrection altar",
        .flavorText = "the souls of the dead surround you. A deceased ally might be called back."
    },
    {
        .displayChar = G_ALTAR,
        .foreColor = &black,
        .backColor = &goldAltarBackColor,
        .drawPriority = 16,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a scorched altar",
        .flavorText = "scorch marks cover the surface of the altar, but it is cold to the touch."
    },
    {
        .displayChar = 0,
        .foreColor = 0,
        .backColor = 0,
        .drawPriority = 95,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_IS_WIRED | TM_PROMOTES_ON_PLAYER_ENTRY),
        .description = "the ground",
        .flavorText = ""
    },

    // sacrifice altar
    {
        .displayChar = G_SAC_ALTAR,
        .foreColor = &altarForeColor,
        .backColor = &altarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_SACRIFICE_ALTAR,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "a sacrifice altar",
        .flavorText = "demonological symbols decorate this altar."
    },
    {
        .displayChar = G_SAC_ALTAR,
        .foreColor = &altarForeColor,
        .backColor = &altarBackColor,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_SACRIFICE_COMPLETE,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_PROMOTES_ON_SACRIFICE_ENTRY),
        .description = "a sacrifice altar",
        .flavorText = "demonological symbols decorate this altar."
    },
    {
        .displayChar = G_LIQUID,
        .foreColor = &fireForeColor,
        .backColor = &lavaBackColor,
        .drawPriority = 40,
        .chanceToIgnite = 0,
        .fireType = DF_OBSIDIAN,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = LAVA_LIGHT,
        .flags = (T_LAVA_INSTA_DEATH),
        .mechFlags = (TM_ALLOWS_SUBMERGING | TM_LIST_IN_SIDEBAR),
        .description = "a sacrificial pit",
        .flavorText = "the smell of burnt flesh lingers over this pit of lava."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &altarBackColor,
        .backColor = &veryDarkGray,
        .drawPriority = 17,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_SACRIFICE_CAGE_ACTIVE,
        .promoteChance = 0,
        .glowLight = CANDLE_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),
        .description = "an iron cage",
        .flavorText = "the cage won't budge. Perhaps there is a way to raise it nearby..."
    },
    {
        .displayChar = G_STATUE,
        .foreColor = &wallBackColor,
        .backColor = &statueBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = DEMONIC_STATUE_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a demonic statue",
        .flavorText = "An obsidian statue of a leering demon looms over the room."
    },

    // doorway statues
    {
        .displayChar = G_STATUE,
        .foreColor = &wallBackColor,
        .backColor = &statueBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_CONNECTS_LEVEL),
        .description = "a marble statue",
        .flavorText = "The cold marble statue has weathered the years with grace."
    },
    {
        .displayChar = G_STATUE,
        .foreColor = &wallBackColor,
        .backColor = &statueBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_CRACKING_STATUE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_CONNECTS_LEVEL),
        .description = "a marble statue",
        .flavorText = "The cold marble statue has weathered the years with grace."
    },

    // extensible stone bridge
    {
        .displayChar = G_CHASM,
        .foreColor = &chasmForeColor,
        .backColor = &black,
        .drawPriority = 40,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_AUTO_DESCENT),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a chasm",
        .flavorText = "you plunge downward into the chasm!"
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &white,
        .backColor = &chasmEdgeBackColor,
        .drawPriority = 40,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_BRIDGE_ACTIVATE,
        .promoteChance = 6000,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "a stone bridge",
        .flavorText = "the narrow stone bridge is extending across the chasm."
    },
    {
        .displayChar = G_FLOOR,
        .foreColor = &white,
        .backColor = &chasmEdgeBackColor,
        .drawPriority = 80,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_BRIDGE_ACTIVATE_ANNOUNCE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_IS_WIRED),
        .description = "the brink of a chasm",
        .flavorText = "chilly winds blow upward from the stygian depths."
    },

    // rat trap
    {
        .displayChar = G_WALL,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_WALL_CRACK,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "a stone wall",
        .flavorText = "The rough stone wall is firm and unyielding."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_WALL_SHATTER,
        .promoteChance = 500,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_LIST_IN_SIDEBAR),
        .description = "a cracking wall",
        .flavorText = "Cracks are running ominously across the base of this rough stone wall."
    },

    // electric crystals
    {
        .displayChar = G_ELECTRIC_CRYSTAL,
        .foreColor = &wallCrystalColor,
        .backColor = &graniteBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_ELECTRIC_CRYSTAL_ON,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_PROMOTES_ON_ELECTRICITY | TM_IS_CIRCUIT_BREAKER | TM_IS_WIRED | TM_LIST_IN_SIDEBAR),
        .description = "a darkened crystal globe",
        .flavorText = "A slight electric shock startles you when you touch the inert crystal globe."
    },
    {
        .displayChar = G_ELECTRIC_CRYSTAL,
        .foreColor = &white,
        .backColor = &wallCrystalColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = CRYSTAL_WALL_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS),
        .mechFlags = (TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR),
        .description = "a shining crystal globe",
        .flavorText = "Crackling light streams out of the crystal globe."
    },
    {
        .displayChar = G_LEVER,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_TURRET_LEVER,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_PLAYER_ENTRY | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_INVERT_WHEN_HIGHLIGHTED),
        .description = "a lever",
        .flavorText = "The lever moves."
    },

    // worm tunnels
    {
        .displayChar = 0,
        .foreColor = 0,
        .backColor = 0,
        .drawPriority = 100,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_WORM_TUNNEL_MARKER_ACTIVE,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),
        .description = "",
        .flavorText = ""
    },
    {
        .displayChar = 0,
        .foreColor = 0,
        .backColor = 0,
        .drawPriority = 100,
        .chanceToIgnite = 0,
        .fireType = 0,
        .discoverType = 0,
        .promoteType = DF_GRANITE_CRUMBLES,
        .promoteChance = -2000,
        .glowLight = NO_LIGHT,
        .flags = (0),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "a rough granite wall",
        .flavorText = "The granite is split open with splinters of rock jutting out at odd angles."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &wallForeColor,
        .backColor = &wallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = DF_WALL_SHATTER,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_CONNECTS_LEVEL),
        .description = "a stone wall",
        .flavorText = "The rough stone wall is firm and unyielding."
    },

    // zombie crypt
    {
        .displayChar = G_FIRE,
        .foreColor = &fireForeColor,
        .backColor = &statueBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = BURNING_CREATURE_LIGHT,
        .flags = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_IS_FIRE),
        .mechFlags = (TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR),
        .description = "a ceremonial brazier",
        .flavorText = "The ancient brazier smolders with a deep crimson flame."
    },

    // goblin warren
    {
        .displayChar = G_FLOOR,
        .foreColor = &mudBackColor,
        .backColor = &refuseBackColor,
        .drawPriority = 85,
        .chanceToIgnite = 0,
        .fireType = DF_STENCH_SMOLDER,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_IS_FLAMMABLE),
        .mechFlags = (TM_VANISHES_UPON_PROMOTION),
        .description = "the mud floor",
        .flavorText = "Rotting animal matter has been ground into the mud floor; the stench is awful."
    },
    {
        .displayChar = G_WALL,
        .foreColor = &mudWallForeColor,
        .backColor = &mudWallBackColor,
        .drawPriority = 0,
        .chanceToIgnite = 0,
        .fireType = DF_PLAIN_FIRE,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_EVERYTHING),
        .mechFlags = (TM_STAND_IN_TILE),
        .description = "a mud-covered wall",
        .flavorText = "A malodorous layer of clay and fecal matter coats the wall."
    },
    {
        .displayChar = G_DOORWAY,
        .foreColor = &mudWallForeColor,
        .backColor = &refuseBackColor,
        .drawPriority = 25,
        .chanceToIgnite = 50,
        .fireType = DF_EMBERS,
        .discoverType = 0,
        .promoteType = 0,
        .promoteChance = 0,
        .glowLight = NO_LIGHT,
        .flags = (T_OBSTRUCTS_VISION | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE),
        .mechFlags = (TM_STAND_IN_TILE | TM_VISUALLY_DISTINCT),
        .description = "hanging animal skins",
        .flavorText = "you push through the animal skins that hang across the threshold."
    },
};

// Features in the gas layer use the startprob as volume, ignore probdecr, and spawn in only a single point.
// Intercepts and slopes are in units of 0.01.
dungeonFeature dungeonFeatureCatalog[NUMBER_DUNGEON_FEATURES] = {
    {0}, // nothing
    {
        .tile = GRANITE,
        .layer = DUNGEON,
        .startProbability = 80,
        .probabilityDecrement = 70,
        .flags = (DFF_CLEAR_OTHER_TERRAIN),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = CRYSTAL_WALL,
        .layer = DUNGEON,
        .startProbability = 200,
        .probabilityDecrement = 50,
        .flags = (DFF_CLEAR_OTHER_TERRAIN),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = LUMINESCENT_FUNGUS,
        .layer = SURFACE,
        .startProbability = 60,
        .probabilityDecrement = 8,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GRASS,
        .layer = SURFACE,
        .startProbability = 75,
        .probabilityDecrement = 5,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DEAD_GRASS,
        .layer = SURFACE,
        .startProbability = 75,
        .probabilityDecrement = 5,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_DEAD_FOLIAGE
    },
    {
        .tile = BONES,
        .layer = SURFACE,
        .startProbability = 75,
        .probabilityDecrement = 23,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = RUBBLE,
        .layer = SURFACE,
        .startProbability = 45,
        .probabilityDecrement = 23,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FOLIAGE,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 33,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FUNGUS_FOREST,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 45,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DEAD_FOLIAGE,
        .layer = SURFACE,
        .startProbability = 50,
        .probabilityDecrement = 30,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // misc. liquids
    {
        .tile = SUNLIGHT_POOL,
        .layer = LIQUID,
        .startProbability = 65,
        .probabilityDecrement = 6,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DARKNESS_PATCH,
        .layer = LIQUID,
        .startProbability = 65,
        .probabilityDecrement = 11,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // Dungeon features spawned during gameplay:

    // revealed secrets
    {
        .tile = DOOR,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GAS_TRAP_POISON,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GAS_TRAP_PARALYSIS,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = CHASM_EDGE,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = TRAP_DOOR,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_CLEAR_OTHER_TERRAIN),
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_SHOW_TRAPDOOR_HALO
    },
    {
        .tile = GAS_TRAP_CONFUSION,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FLAMETHROWER,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FLOOD_TRAP,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = NET_TRAP,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ALARM_TRAP,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // bloods
    // Start probability is actually a percentage for bloods.
    // Base probability is 15 + (damage * 2/3), and then take the given percentage of that.
    // If it's a gas, we multiply the base by an additional 100.
    // Thus to get a starting gas volume of a poison potion (1000), with a hit for 10 damage, use a starting probability of 48.
    {
        .tile = RED_BLOOD,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GREEN_BLOOD,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PURPLE_BLOOD,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = WORM_BLOOD,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ACID_SPLATTER,
        .layer = SURFACE,
        .startProbability = 200,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ASH,
        .layer = SURFACE,
        .startProbability = 50,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = EMBERS,
        .layer = SURFACE,
        .startProbability = 125,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ECTOPLASM,
        .layer = SURFACE,
        .startProbability = 110,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = RUBBLE,
        .layer = SURFACE,
        .startProbability = 33,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ROT_GAS,
        .layer = GAS,
        .startProbability = 12,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // monster effects
    {
        .tile = VOMIT,
        .layer = SURFACE,
        .startProbability = 30,
        .probabilityDecrement = 10,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = POISON_GAS,
        .layer = GAS,
        .startProbability = 2000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GAS_EXPLOSION,
        .layer = SURFACE,
        .startProbability = 350,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = EXPLOSION_FLARE_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = RED_BLOOD,
        .layer = SURFACE,
        .startProbability = 150,
        .probabilityDecrement = 30,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FLAMEDANCER_FIRE,
        .layer = SURFACE,
        .startProbability = 200,
        .probabilityDecrement = 75,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // mutation effects
    {
        .tile = GAS_EXPLOSION,
        .layer = SURFACE,
        .startProbability = 350,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "The corpse detonates with terrifying force!",
        .lightFlare = EXPLOSION_FLARE_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = LICHEN,
        .layer = SURFACE,
        .startProbability = 70,
        .probabilityDecrement = 60,
        .flags = 0,
        .description = "Poisonous spores burst from the corpse!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // misc
    {
        .tile = NOTHING,
        .layer = GAS,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_EVACUATE_CREATURES_FIRST),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ROT_GAS,
        .layer = GAS,
        .startProbability = 15,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = STEAM,
        .layer = GAS,
        .startProbability = 325,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = STEAM,
        .layer = GAS,
        .startProbability = 15,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = METHANE_GAS,
        .layer = GAS,
        .startProbability = 2,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = EMBERS,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = URINE,
        .layer = SURFACE,
        .startProbability = 65,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = UNICORN_POOP,
        .layer = SURFACE,
        .startProbability = 65,
        .probabilityDecrement = 40,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PUDDLE,
        .layer = SURFACE,
        .startProbability = 13,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ASH,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ECTOPLASM,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FORCEFIELD,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 50,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FORCEFIELD_MELT,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = SACRED_GLYPH,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = EMPOWERMENT_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = LICHEN,
        .layer = SURFACE,
        .startProbability = 2,
        .probabilityDecrement = 100,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    }, // Lichen won't spread through lava.
    {
        .tile = RUBBLE,
        .layer = SURFACE,
        .startProbability = 45,
        .probabilityDecrement = 23,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = RUBBLE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    {
        .tile = SPIDERWEB,
        .layer = SURFACE,
        .startProbability = 15,
        .probabilityDecrement = 12,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = SPIDERWEB,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 39,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    {
        .tile = ANCIENT_SPIRIT_VINES,
        .layer = SURFACE,
        .startProbability = 75,
        .probabilityDecrement = 70,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ANCIENT_SPIRIT_GRASS,
        .layer = SURFACE,
        .startProbability = 50,
        .probabilityDecrement = 47,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // foliage
    {
        .tile = TRAMPLED_FOLIAGE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DEAD_GRASS,
        .layer = SURFACE,
        .startProbability = 75,
        .probabilityDecrement = 75,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FOLIAGE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = TRAMPLED_FUNGUS_FOREST,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FUNGUS_FOREST,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // brimstone
    {
        .tile = ACTIVE_BRIMSTONE,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = INERT_BRIMSTONE,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_BRIMSTONE_FIRE
    },

    // bloodwort
    {
        .tile = BLOODFLOWER_POD,
        .layer = SURFACE,
        .startProbability = 60,
        .probabilityDecrement = 60,
        .flags = (DFF_EVACUATE_CREATURES_FIRST),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = BLOODFLOWER_POD,
        .layer = SURFACE,
        .startProbability = 10,
        .probabilityDecrement = 10,
        .flags = (DFF_EVACUATE_CREATURES_FIRST),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = HEALING_CLOUD,
        .layer = GAS,
        .startProbability = 350,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // dewars
    {
        .tile = POISON_GAS,
        .layer = GAS,
        .startProbability = 20000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the dewar shatters and pressurized caustic gas explodes outward!",
        .lightFlare = 0,
        .flashColor = &poisonGasColor,
        .effectRadius = 4,
        .propagationTerrain = 0,
        .subsequentDF = DF_DEWAR_GLASS
    },
    {
        .tile = CONFUSION_GAS,
        .layer = GAS,
        .startProbability = 20000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the dewar shatters and pressurized confusion gas explodes outward!",
        .lightFlare = 0,
        .flashColor = &confusionGasColor,
        .effectRadius = 4,
        .propagationTerrain = 0,
        .subsequentDF = DF_DEWAR_GLASS
    },
    {
        .tile = PARALYSIS_GAS,
        .layer = GAS,
        .startProbability = 20000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the dewar shatters and pressurized paralytic gas explodes outward!",
        .lightFlare = 0,
        .flashColor = &pink,
        .effectRadius = 4,
        .propagationTerrain = 0,
        .subsequentDF = DF_DEWAR_GLASS
    },
    {
        .tile = METHANE_GAS,
        .layer = GAS,
        .startProbability = 20000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the dewar shatters and pressurized methane gas explodes outward!",
        .lightFlare = 0,
        .flashColor = &methaneColor,
        .effectRadius = 4,
        .propagationTerrain = 0,
        .subsequentDF = DF_DEWAR_GLASS
    },
    {
        .tile = BROKEN_GLASS,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 70,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = CARPET,
        .layer = DUNGEON,
        .startProbability = 120,
        .probabilityDecrement = 20,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // algae
    {
        .tile = DEEP_WATER_ALGAE_WELL,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_SUPERPRIORITY),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DEEP_WATER_ALGAE_1,
        .layer = LIQUID,
        .startProbability = 50,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = DEEP_WATER,
        .subsequentDF = DF_ALGAE_2
    },
    {
        .tile = DEEP_WATER_ALGAE_2,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DEEP_WATER,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_SUPERPRIORITY),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // doors, item cages, altars, glyphs, guardians -- reusable machine components
    {
        .tile = OPEN_DOOR,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DOOR,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = OPEN_IRON_DOOR_INERT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ALTAR_CAGE_OPEN,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the cages lift off of the altars as you approach.",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ALTAR_CAGE_CLOSED,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_EVACUATE_CREATURES_FIRST),
        .description = "the cages lower to cover the altars.",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ALTAR_INERT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FLOOR_FLOODABLE,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the altar retracts into the ground with a grinding sound.",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PORTAL_LIGHT,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_EVACUATE_CREATURES_FIRST | DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "the archway flashes, and you catch a glimpse of another world!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MACHINE_GLYPH_INACTIVE,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MACHINE_GLYPH,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GUARDIAN_GLOW,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GUARDIAN_GLOW,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the glyph beneath you glows, and the guardians take a step!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GUARDIAN_GLOW,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the mirrored totem flashes, reflecting the red glow of the glyph beneath you.",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MACHINE_GLYPH,
        .layer = DUNGEON,
        .startProbability = 200,
        .probabilityDecrement = 95,
        .flags = (DFF_BLOCKED_BY_OTHER_LAYERS),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = WALL_LEVER,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "you notice a lever hidden behind a loose stone in the wall.",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = WALL_LEVER_PULLED,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = WALL_LEVER_HIDDEN,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    {
        .tile = BRIDGE_FALLING,
        .layer = LIQUID,
        .startProbability = 200,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = BRIDGE,
        .subsequentDF = 0
    },
    {
        .tile = CHASM,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_BRIDGE_FALL_PREP
    },

    // fire
    {
        .tile = PLAIN_FIRE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GAS_FIRE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GAS_EXPLOSION,
        .layer = SURFACE,
        .startProbability = 60,
        .probabilityDecrement = 17,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DART_EXPLOSION,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = BRIMSTONE_FIRE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = 0,
        .layer = 0,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the rope bridge snaps from the heat and plunges into the chasm!",
        .lightFlare = FALLEN_TORCH_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_BRIDGE_FALL
    },
    {
        .tile = PLAIN_FIRE,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 37,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = EMBERS,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = EMBERS,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 94,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = OBSIDIAN,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ITEM_FIRE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = FALLEN_TORCH_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = CREATURE_FIRE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = FALLEN_TORCH_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    {
        .tile = FLOOD_WATER_SHALLOW,
        .layer = SURFACE,
        .startProbability = 225,
        .probabilityDecrement = 37,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_FLOOD_2
    },
    {
        .tile = FLOOD_WATER_DEEP,
        .layer = SURFACE,
        .startProbability = 175,
        .probabilityDecrement = 37,
        .flags = 0,
        .description = "the area is flooded as water rises through imperceptible holes in the ground.",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FLOOD_WATER_SHALLOW,
        .layer = SURFACE,
        .startProbability = 10,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = HOLE,
        .layer = SURFACE,
        .startProbability = 200,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = HOLE_EDGE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // ice effects
    {
        .tile = ICE_DEEP,
        .layer = LIQUID,
        .startProbability = 150,
        .probabilityDecrement = 50,
        .flags = (DFF_EVACUATE_CREATURES_FIRST),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = DEEP_WATER,
        .subsequentDF = DF_ALGAE_1_FREEZE
    },
    {
        .tile = ICE_DEEP,
        .layer = LIQUID,
        .startProbability = 150,
        .probabilityDecrement = 50,
        .flags = (DFF_EVACUATE_CREATURES_FIRST),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = DEEP_WATER_ALGAE_1,
        .subsequentDF = DF_ALGAE_2_FREEZE
    },
    {
        .tile = ICE_DEEP,
        .layer = LIQUID,
        .startProbability = 150,
        .probabilityDecrement = 50,
        .flags = (DFF_EVACUATE_CREATURES_FIRST),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = DEEP_WATER_ALGAE_2,
        .subsequentDF = DF_SHALLOW_WATER_FREEZE
    },
    {
        .tile = ICE_DEEP_MELT,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DEEP_WATER,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ICE_SHALLOW,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 50,
        .flags = (DFF_EVACUATE_CREATURES_FIRST),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = SHALLOW_WATER,
        .subsequentDF = 0
    },
    {
        .tile = ICE_SHALLOW_MELT,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = SHALLOW_WATER,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // gas trap effects
    {
        .tile = POISON_GAS,
        .layer = GAS,
        .startProbability = 1000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "a cloud of caustic gas sprays upward from the floor!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = CONFUSION_GAS,
        .layer = GAS,
        .startProbability = 300,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "a sparkling cloud of confusion gas sprays upward from the floor!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = NETTING,
        .layer = SURFACE,
        .startProbability = 300,
        .probabilityDecrement = 90,
        .flags = 0,
        .description = "a net falls from the ceiling!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = 0,
        .layer = 0,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_AGGRAVATES_MONSTERS),
        .description = "a piercing shriek echoes through the nearby rooms!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = DCOLS/2,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = METHANE_GAS,
        .layer = GAS,
        .startProbability = 10000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    }, // debugging toy

    // potions
    {
        .tile = POISON_GAS,
        .layer = GAS,
        .startProbability = 1000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = &poisonGasColor,
        .effectRadius = 4,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PARALYSIS_GAS,
        .layer = GAS,
        .startProbability = 1000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = &pink,
        .effectRadius = 4,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = CONFUSION_GAS,
        .layer = GAS,
        .startProbability = 1000,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = &confusionGasColor,
        .effectRadius = 4,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PLAIN_FIRE,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 37,
        .flags = 0,
        .description = "",
        .lightFlare = EXPLOSION_FLARE_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DARKNESS_CLOUD,
        .layer = GAS,
        .startProbability = 200,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = HOLE_EDGE,
        .layer = SURFACE,
        .startProbability = 300,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = &darkBlue,
        .effectRadius = 3,
        .propagationTerrain = 0,
        .subsequentDF = DF_HOLE_2
    },
    {
        .tile = LICHEN,
        .layer = SURFACE,
        .startProbability = 70,
        .probabilityDecrement = 60,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // other items
    {
        .tile = PLAIN_FIRE,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 45,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = &yellow,
        .effectRadius = 3,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = HOLE_GLOW,
        .layer = SURFACE,
        .startProbability = 200,
        .probabilityDecrement = 100,
        .flags = (DFF_SUBSEQ_EVERYWHERE),
        .description = "",
        .lightFlare = 0,
        .flashColor = &darkBlue,
        .effectRadius = 3,
        .propagationTerrain = 0,
        .subsequentDF = DF_STAFF_HOLE_EDGE
    },
    {
        .tile = HOLE_EDGE,
        .layer = SURFACE,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // machine components

    // commutation altars
    {
        .tile = COMMUTATION_ALTAR_INERT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the items on the two altars flash with a brilliant light!",
        .lightFlare = SCROLL_ENCHANTMENT_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PIPE_GLOWING,
        .layer = SURFACE,
        .startProbability = 90,
        .probabilityDecrement = 60,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PIPE_INERT,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = SCROLL_ENCHANTMENT_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // resurrection altars
    {
        .tile = RESURRECTION_ALTAR_INERT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_RESURRECT_ALLY),
        .description = "An old friend emerges from a bloom of sacred light!",
        .lightFlare = EMPOWERMENT_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MACHINE_TRIGGER_FLOOR_REPEATING,
        .layer = LIQUID,
        .startProbability = 300,
        .probabilityDecrement = 100,
        .flags = (DFF_SUPERPRIORITY),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // sacrifice altars
    {
        .tile = SACRIFICE_ALTAR,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "a demonic presence whispers its demand: \"bring to me the marked sacrifice!\"",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = SACRIFICE_LAVA,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "demonic cackling echoes through the room as the altar plunges downward!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = ALTAR_CAGE_RETRACTABLE,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // coffin bursts open to reveal vampire:
    {
        .tile = COFFIN_OPEN,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "the coffin opens and a dark figure rises!",
        .lightFlare = 0,
        .flashColor = &darkGray,
        .effectRadius = 3,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PLAIN_FIRE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "as flames begin to lick the coffin, its tenant bursts forth!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_EMBERS_PATCH
    },
    {
        .tile = MACHINE_TRIGGER_FLOOR,
        .layer = DUNGEON,
        .startProbability = 200,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // throwing tutorial:
    {
        .tile = ALTAR_INERT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the cage lifts off of the altar.",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = TRAP_DOOR,
        .layer = LIQUID,
        .startProbability = 225,
        .probabilityDecrement = 100,
        .flags = (DFF_CLEAR_OTHER_TERRAIN | DFF_SUBSEQ_EVERYWHERE),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_SHOW_TRAPDOOR_HALO
    },
    {
        .tile = LAVA,
        .layer = LIQUID,
        .startProbability = 225,
        .probabilityDecrement = 100,
        .flags = (DFF_CLEAR_OTHER_TERRAIN),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MACHINE_PRESSURE_PLATE_USED,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // rat trap:
    {
        .tile = RAT_TRAP_WALL_CRACKING,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "a scratching sound emanates from the nearby walls!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_RUBBLE
    },

    // wooden barricade at entrance:
    {
        .tile = PLAIN_FIRE,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "flames quickly consume the wooden barricade.",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // wooden barricade around altar:
    {
        .tile = WOODEN_BARRICADE,
        .layer = DUNGEON,
        .startProbability = 220,
        .probabilityDecrement = 100,
        .flags = (DFF_TREAT_AS_BLOCKING | DFF_SUBSEQ_EVERYWHERE),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_SMALL_DEAD_GRASS
    },

    // shallow water flood machine:
    {
        .tile = MACHINE_FLOOD_WATER_SPREADING,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "you hear a heavy click, and the nearby water begins flooding the area!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = SHALLOW_WATER,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MACHINE_FLOOD_WATER_SPREADING,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = FLOOR_FLOODABLE,
        .subsequentDF = DF_SHALLOW_WATER
    },
    {
        .tile = MACHINE_FLOOD_WATER_DORMANT,
        .layer = LIQUID,
        .startProbability = 250,
        .probabilityDecrement = 100,
        .flags = (DFF_TREAT_AS_BLOCKING),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_SPREADABLE_DEEP_WATER_POOL
    },
    {
        .tile = DEEP_WATER,
        .layer = LIQUID,
        .startProbability = 90,
        .probabilityDecrement = 100,
        .flags = (DFF_CLEAR_OTHER_TERRAIN | DFF_PERMIT_BLOCKING),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // unstable floor machine:
    {
        .tile = MACHINE_COLLAPSE_EDGE_SPREADING,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "you hear a deep rumbling noise as the floor begins to collapse!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = CHASM,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_CLEAR_OTHER_TERRAIN),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_SHOW_TRAPDOOR_HALO
    },
    {
        .tile = MACHINE_COLLAPSE_EDGE_SPREADING,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = FLOOR_FLOODABLE,
        .subsequentDF = DF_COLLAPSE
    },
    {
        .tile = MACHINE_COLLAPSE_EDGE_DORMANT,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // levitation bridge machine:
    {
        .tile = CHASM_WITH_HIDDEN_BRIDGE_ACTIVE,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = CHASM_WITH_HIDDEN_BRIDGE,
        .subsequentDF = DF_BRIDGE_APPEARS
    },
    {
        .tile = CHASM_WITH_HIDDEN_BRIDGE_ACTIVE,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "a stone bridge extends from the floor with a grinding sound.",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = CHASM_WITH_HIDDEN_BRIDGE,
        .subsequentDF = DF_BRIDGE_APPEARS
    },
    {
        .tile = STONE_BRIDGE,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MACHINE_CHASM_EDGE,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // retracting lava pool:
    {
        .tile = LAVA_RETRACTABLE,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = LAVA,
        .subsequentDF = 0
    },
    {
        .tile = LAVA_RETRACTING,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "hissing fills the air as the lava begins to cool.",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = OBSIDIAN,
        .layer = SURFACE,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_STEAM_ACCUMULATION
    },

    // hidden poison vent machine:
    {
        .tile = MACHINE_POISON_GAS_VENT_DORMANT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "you notice an inactive gas vent hidden in a crevice of the floor.",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MACHINE_POISON_GAS_VENT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "deadly purple gas starts wafting out of hidden vents in the floor!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PORTCULLIS_CLOSED,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_EVACUATE_CREATURES_FIRST),
        .description = "with a heavy mechanical sound, an iron portcullis falls from the ceiling!",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PORTCULLIS_DORMANT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the portcullis slowly rises from the ground into a slot in the ceiling.",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = POISON_GAS,
        .layer = GAS,
        .startProbability = 25,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // hidden methane vent machine:
    {
        .tile = MACHINE_METHANE_VENT_DORMANT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "you notice an inactive gas vent hidden in a crevice of the floor.",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MACHINE_METHANE_VENT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "explosive methane gas starts wafting out of hidden vents in the floor!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_VENT_SPEW_METHANE
    },
    {
        .tile = METHANE_GAS,
        .layer = GAS,
        .startProbability = 60,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PILOT_LIGHT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "a torch falls from its mount and lies sputtering on the floor.",
        .lightFlare = FALLEN_TORCH_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // paralysis trap:
    {
        .tile = MACHINE_PARALYSIS_VENT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "you notice an inactive gas vent hidden in a crevice of the floor.",
        .lightFlare = GENERIC_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = PARALYSIS_GAS,
        .layer = GAS,
        .startProbability = 350,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "paralytic gas sprays upward from hidden vents in the floor!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_REVEAL_PARALYSIS_VENT_SILENTLY
    },
    {
        .tile = MACHINE_PARALYSIS_VENT,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // thematic dungeon:
    {
        .tile = RED_BLOOD,
        .layer = SURFACE,
        .startProbability = 75,
        .probabilityDecrement = 25,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // statuary:
    {
        .tile = STATUE_CRACKING,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "cracks begin snaking across the marble surface of the statue!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_RUBBLE
    },
    {
        .tile = RUBBLE,
        .layer = SURFACE,
        .startProbability = 120,
        .probabilityDecrement = 100,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "the statue shatters!",
        .lightFlare = 0,
        .flashColor = &darkGray,
        .effectRadius = 3,
        .propagationTerrain = 0,
        .subsequentDF = DF_RUBBLE
    },

    // hidden turrets:
    {
        .tile = WALL,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "you hear a click, and the stones in the wall shift to reveal turrets!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_RUBBLE
    },

    // worm tunnels:
    {
        .tile = WORM_TUNNEL_MARKER_DORMANT,
        .layer = LIQUID,
        .startProbability = 5,
        .probabilityDecrement = 5,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = GRANITE,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = WORM_TUNNEL_MARKER_ACTIVE,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = FLOOR,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_SUPERPRIORITY | DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_TUNNELIZE
    },
    {
        .tile = FLOOR,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the nearby wall cracks and collapses in a cloud of dust!",
        .lightFlare = 0,
        .flashColor = &darkGray,
        .effectRadius = 5,
        .propagationTerrain = 0,
        .subsequentDF = DF_TUNNELIZE
    },

    // haunted room:
    {
        .tile = DARK_FLOOR_DARKENING,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the light in the room flickers and you feel a chill in the air.",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DARK_FLOOR,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_ECTOPLASM_DROPLET
    },
    {
        .tile = HAUNTED_TORCH_TRANSITIONING,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = HAUNTED_TORCH,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // mud pit:
    {
        .tile = MACHINE_MUD_DORMANT,
        .layer = LIQUID,
        .startProbability = 100,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = MUD,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "across the bog, bubbles rise ominously from the mud.",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // electric crystals:
    {
        .tile = ELECTRIC_CRYSTAL_ON,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "the crystal absorbs the electricity and begins to glow.",
        .lightFlare = CHARGE_FLASH_LIGHT,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = WALL,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "the wall above the lever shifts to reveal a spark turret!",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // idyll:
    {
        .tile = SHALLOW_WATER,
        .layer = LIQUID,
        .startProbability = 150,
        .probabilityDecrement = 100,
        .flags = (DFF_PERMIT_BLOCKING),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = DEEP_WATER,
        .layer = LIQUID,
        .startProbability = 90,
        .probabilityDecrement = 100,
        .flags = (DFF_TREAT_AS_BLOCKING | DFF_CLEAR_OTHER_TERRAIN | DFF_SUBSEQ_EVERYWHERE),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_SHALLOW_WATER_POOL
    },

    // swamp:
    {
        .tile = SHALLOW_WATER,
        .layer = LIQUID,
        .startProbability = 30,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = GRAY_FUNGUS,
        .layer = SURFACE,
        .startProbability = 80,
        .probabilityDecrement = 50,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_SWAMP_MUD
    },
    {
        .tile = MUD,
        .layer = LIQUID,
        .startProbability = 75,
        .probabilityDecrement = 5,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_SWAMP_WATER
    },

    // camp:
    {
        .tile = HAY,
        .layer = SURFACE,
        .startProbability = 90,
        .probabilityDecrement = 87,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },
    {
        .tile = JUNK,
        .layer = SURFACE,
        .startProbability = 20,
        .probabilityDecrement = 20,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // remnants:
    {
        .tile = CARPET,
        .layer = DUNGEON,
        .startProbability = 110,
        .probabilityDecrement = 20,
        .flags = (DFF_SUBSEQ_EVERYWHERE),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_REMNANT_ASH
    },
    {
        .tile = BURNED_CARPET,
        .layer = SURFACE,
        .startProbability = 120,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // chasm catwalk:
    {
        .tile = CHASM,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_CLEAR_OTHER_TERRAIN),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_SHOW_TRAPDOOR_HALO
    },
    {
        .tile = STONE_BRIDGE,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_CLEAR_OTHER_TERRAIN),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // lake catwalk:
    {
        .tile = DEEP_WATER,
        .layer = LIQUID,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = (DFF_CLEAR_OTHER_TERRAIN),
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_LAKE_HALO
    },
    {
        .tile = SHALLOW_WATER,
        .layer = LIQUID,
        .startProbability = 160,
        .probabilityDecrement = 100,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // worms pop out of walls:
    {
        .tile = RUBBLE,
        .layer = SURFACE,
        .startProbability = 120,
        .probabilityDecrement = 100,
        .flags = (DFF_ACTIVATE_DORMANT_MONSTER),
        .description = "the nearby wall explodes in a shower of stone fragments!",
        .lightFlare = 0,
        .flashColor = &darkGray,
        .effectRadius = 3,
        .propagationTerrain = 0,
        .subsequentDF = DF_RUBBLE
    },

    // monster cages open:
    {
        .tile = MONSTER_CAGE_OPEN,
        .layer = DUNGEON,
        .startProbability = 0,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = 0
    },

    // goblin warren:
    {
        .tile = STENCH_SMOKE_GAS,
        .layer = GAS,
        .startProbability = 50,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_PLAIN_FIRE
    },
    {
        .tile = STENCH_SMOKE_GAS,
        .layer = GAS,
        .startProbability = 50,
        .probabilityDecrement = 0,
        .flags = 0,
        .description = "",
        .lightFlare = 0,
        .flashColor = 0,
        .effectRadius = 0,
        .propagationTerrain = 0,
        .subsequentDF = DF_EMBERS
    },
};

dungeonProfile dungeonProfileCatalog[NUMBER_DUNGEON_PROFILES] = {
    // Room frequencies:
    //      0. Cross room
    //      1. Small symmetrical cross room
    //      2. Small room
    //      3. Circular room
    //      4. Chunky room
    //      5. Cave
    //      6. Cavern (the kind that fills a level)
    //      7. Entrance room (the big upside-down T room at the start of depth 1)

    // Room frequencies
    // 0    1   2   3   4   5   6   7   Corridor chance
    {{2,    1,  1,  1,  7,  1,  0,  0}, 10},    // Basic dungeon generation (further adjusted by depth)
    {{10,   0,  0,  3,  7,  10, 10, 0}, 0},     // First room for basic dungeon generation (further adjusted by depth)

    {{0,    0,  1,  0,  0,  0,  0,  0}, 0},     // Goblin warrens
    {{0,    5,  0,  1,  0,  0,  0,  0}, 0},     // Sentinel sanctuaries
};

// radius is in units of 0.01
const lightSource lightCatalog[NUMBER_LIGHT_KINDS] = {
    //color                 radius range            fade%   passThroughCreatures
    {0},                                                                // NO_LIGHT
    {&minersLightColor,     {0, 0, 1},              35,     true},      // miners light
    {&fireBoltColor,        {300, 400, 1},          0,      false},     // burning creature light
    {&wispLightColor,       {400, 800, 1},          0,      false},     // will-o'-the-wisp light
    {&fireBoltColor,        {300, 400, 1},          0,      false},     // salamander glow
    {&pink,                 {600, 600, 1},          0,      true},      // imp light
    {&pixieColor,           {400, 600, 1},          50,     false},     // pixie light
    {&lichLightColor,       {1500, 1500, 1},        0,      false},     // lich light
    {&flamedancerCoronaColor,{1000, 2000, 1},       0,      false},     // flamedancer light
    {&sentinelLightColor,   {300, 500, 1},          0,      false},     // sentinel light
    {&unicornLightColor,    {300, 400, 1},          0,      false},     // unicorn light
    {&ifritLightColor,      {300, 600, 1},          0,      false},     // ifrit light
    {&fireBoltColor,        {400, 600, 1},          0,      false},     // phoenix light
    {&fireBoltColor,        {150, 300, 1},          0,      false},     // phoenix egg light
    {&yendorLightColor,     {1500, 1500, 1},        0,      false},     // Yendorian light
    {&spectralBladeLightColor,{350, 350, 1},        0,      false},     // spectral blades
    {&summonedImageLightColor,{350, 350, 1},        0,      false},     // weapon images
    {&lightningColor,       {250, 250, 1},          35,     false},     // lightning turret light
    {&explosiveAuraColor,   {150, 200, 1},          0,      true},      // explosive bloat light
    {&lightningColor,       {300, 300, 1},          0,      false},     // bolt glow
    {&telepathyColor,       {200, 200, 1},          0,      true},      // telepathy light
    {&sacrificeTargetColor, {250, 250, 1},          0,      true},      // sacrifice doom light

    // flares:
    {&scrollProtectionColor,{600, 600, 1},          0,      true},      // scroll of protection flare
    {&scrollEnchantmentColor,{600, 600, 1},         0,      true},      // scroll of enchantment flare
    {&potionStrengthColor,  {600, 600, 1},          0,      true},      // potion of strength flare
    {&empowermentFlashColor,{600, 600, 1},          0,      true},      // empowerment flare
    {&genericFlashColor,    {300, 300, 1},          0,      true},      // generic flash flare
    {&fireFlashColor,       {800, 800, 1},          0,      false},     // fallen torch flare
    {&summoningFlashColor,  {600, 600, 1},          0,      true},      // summoning flare
    {&explosionFlareColor,  {5000, 5000, 1},        0,      true},      // explosion (explosive bloat or incineration potion)
    {&quietusFlashColor,    {300, 300, 1},          0,      true},      // quietus activation flare
    {&slayingFlashColor,    {300, 300, 1},          0,      true},      // slaying activation flare
    {&lightningColor,       {800, 800, 1},          0,      false},     // electric crystal activates

    // glowing terrain:
    {&torchLightColor,      {1000, 1000, 1},        50,     false},     // torch
    {&lavaLightColor,       {300, 300, 1},          50,     false},     // lava
    {&sunLightColor,        {200, 200, 1},          25,     true},      // sunlight
    {&darknessPatchColor,   {400, 400, 1},          0,      true},      // darkness patch
    {&fungusLightColor,     {300, 300, 1},          50,     false},     // luminescent fungus
    {&fungusForestLightColor,{500, 500, 1},         0,      false},     // luminescent forest
    {&algaeBlueLightColor,  {300, 300, 1},          0,      false},     // luminescent algae blue
    {&algaeGreenLightColor, {300, 300, 1},          0,      false},     // luminescent algae green
    {&ectoplasmColor,       {200, 200, 1},          50,     false},     // ectoplasm
    {&unicornLightColor,    {200, 200, 1},          0,      false},     // unicorn poop light
    {&lavaLightColor,       {200, 200, 1},          50,     false},     // embers
    {&lavaLightColor,       {500, 1000, 1},         0,      false},     // fire
    {&lavaLightColor,       {200, 300, 1},          0,      false},     // brimstone fire
    {&explosionColor,       {DCOLS*100,DCOLS*100,1},100,    false},     // explosions
    {&dartFlashColor,       {15*100,15*100,1},      0,      false},     // incendiary darts
    {&portalActivateLightColor, {DCOLS*100,DCOLS*100,1},0,  false},     // portal activation
    {&confusionLightColor,  {300, 300, 1},          100,    false},     // confusion gas
    {&darknessCloudColor,   {500, 500, 1},          0,      true},      // darkness cloud
    {&forceFieldLightColor, {200, 200, 1},          50,     false},     // forcefield
    {&crystalWallLightColor,{300, 500, 1},          50,     false},     // crystal wall
    {&torchLightColor,      {200, 400, 1},          0,      false},     // candle light
    {&hauntedTorchColor,    {400, 600, 1},          0,      false},     // haunted torch
    {&glyphLightColor,      {100, 100, 1},          0,      false},     // glyph dim light
    {&glyphLightColor,      {300, 300, 1},          0,      false},     // glyph bright light
    {&sacredGlyphColor,     {300, 300, 1},          0,      false},     // sacred glyph light
    {&descentLightColor,    {600, 600, 1},          0,      false},     // magical pit light
    {&sacrificeTargetColor, {800, 1200, 1},          0,      true},      // demonic statue light
};

const blueprint blueprintCatalog[NUMBER_BLUEPRINTS] = {
    {{0}}, // nothing
    //BLUEPRINTS:
    //depths            roomSize    freq    featureCt   dungeonProfileType  flags   (features on subsequent lines)

        //FEATURES:
        //DF        terrain     layer       instanceCtRange minInsts    itemCat     itemKind    monsterKind     reqSpace        hordeFl     itemFlags   featureFlags

    // -- REWARD ROOMS --

    // Mixed item library -- can check one item out at a time
    {{1, 12},           {30, 50},   30,     6,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {1,1},      1,          WAND,       -1,         0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_PLAYER_AVOIDS), (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {3,3},      3,          (WEAPON|ARMOR|WAND),-1, 0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_PLAYER_AVOIDS), (MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {2,3},      2,          (STAFF|RING|CHARM),-1,  0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN | ITEM_PLAYER_AVOIDS),    (MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         STATUE_INERT,DUNGEON,       {2,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
    // Single item category library -- can check one item out at a time
    {{1, 12},           {30, 50},   15,     5,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {3,4},      3,          (RING),     -1,         0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN | ITEM_PLAYER_AVOIDS),    (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {4,5},      4,          (STAFF),    -1,         0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN | ITEM_PLAYER_AVOIDS),    (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE | MF_IMPREGNABLE)},
        {0,         STATUE_INERT,DUNGEON,       {2,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
    // Treasure room -- apothecary or archive (potions or scrolls)
    {{8, AMULET_LEVEL}, {20, 40},   20,     6,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         0,          0,              {5,7},      2,          (POTION),   -1,         0,              2,              0,          0,          (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {4,6},      2,          (SCROLL),   -1,         0,              2,              0,          0,          (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         FUNGUS_FOREST,SURFACE,      {3,4},      0,          0,          -1,         0,              2,              0,          0,          0},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
        {0,         STATUE_INERT,DUNGEON,       {2,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
    // Guaranteed good permanent item on a glowing pedestal (runic weapon/armor or 2 staffs)
    {{5, 16},           {10, 30},   30,     6,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         STATUE_INERT,DUNGEON,       {2,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (WEAPON),   -1,         0,              2,              0,          ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (ARMOR),    -1,         0,              2,              0,          ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_TREAT_AS_BLOCKING)},
        {0,         PEDESTAL,   DUNGEON,        {2,2},      2,          (STAFF),    -1,         0,              2,              0,          (ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN),   (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    // Guaranteed good consumable item on glowing pedestals (scrolls of enchanting, potion of life)
    {{10, AMULET_LEVEL},{10, 30},   30,     5,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         STATUE_INERT,DUNGEON,       {1,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (SCROLL),   SCROLL_ENCHANTING, 0,       2,              0,          (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (POTION),   POTION_LIFE,0,              2,              0,          (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    // Commutation altars
    {{13, AMULET_LEVEL},{10, 30},   50,     4,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         STATUE_INERT,DUNGEON,       {1,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
        {DF_MAGIC_PIPING,COMMUTATION_ALTAR,DUNGEON,{2,2},   2,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    // Resurrection altar
    {{13, AMULET_LEVEL},{10, 30},   30,     4,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         STATUE_INERT,DUNGEON,       {1,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
        {DF_MACHINE_FLOOR_TRIGGER_REPEATING, RESURRECTION_ALTAR,DUNGEON, {1,1}, 1, 0, -1,       0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    // Outsourced item -- same item possibilities as in the good permanent item reward room (plus charms), but directly adopted by 1-2 key machines.
    {{5, 17},           {0, 0},     20,     4,          0,                  (BP_REWARD | BP_NO_INTERIOR_FLAG),  {
        {0,         0,          0,              {1,1},      1,          (WEAPON),   -1,         0,              0,              0,          ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_NO_THROWING_WEAPONS | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)},
        {0,         0,          0,              {1,1},      1,          (ARMOR),    -1,         0,              0,              0,          ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)},
        {0,         0,          0,              {2,2},      2,          (STAFF),    -1,         0,              0,              0,          ITEM_KIND_AUTO_ID, (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)},
        {0,         0,          0,              {1,2},      1,          (CHARM),    -1,         0,              0,              0,          ITEM_KIND_AUTO_ID, (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
    // Dungeon -- two allies chained up for the taking
    {{5, AMULET_LEVEL}, {30, 80},   12,     5,          0,                  (BP_ROOM | BP_REWARD),  {
        {0,         VOMIT,      SURFACE,        {2,2},      2,          0,          -1,         0,              2,              (HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING)},
        {DF_AMBIENT_BLOOD,MANACLE_T,SURFACE,    {1,2},      1,          0,          -1,         0,              1,              0,          0,          0},
        {DF_AMBIENT_BLOOD,MANACLE_L,SURFACE,    {1,2},      1,          0,          -1,         0,              1,              0,          0,          0},
        {DF_BONES,  0,          0,              {2,3},      1,          0,          -1,         0,              1,              0,          0,          0},
        {DF_VOMIT,  0,          0,              {2,3},      1,          0,          -1,         0,              1,              0,          0,          0},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    // Kennel -- allies locked in cages in an open room; choose one or two to unlock and take with you.
    {{5, AMULET_LEVEL}, {30, 80},   12,     4,          0,                  (BP_ROOM | BP_REWARD),  {
        {0,         MONSTER_CAGE_CLOSED,DUNGEON,{3,5},      3,          0,          -1,         0,              2,              (HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         0,          0,              {1,2},      1,          KEY,        KEY_CAGE,   0,              1,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_PERMIT_BLOCKING | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_SKELETON_KEY | MF_KEY_DISPOSABLE)},
        {DF_AMBIENT_BLOOD, 0,   0,              {3,5},      3,          0,          -1,         0,              1,              0,          0,          0},
        {DF_BONES,  0,          0,              {3,5},      3,          0,          -1,         0,              1,              0,          0,          0},
        {0,         TORCH_WALL, DUNGEON,        {2,3},      2,          0,          0,          0,              1,              0,          0,          (MF_BUILD_IN_WALLS)}}},
    // Vampire lair -- allies locked in cages and chained in a hidden room with a vampire in a coffin; vampire has one cage key.
    {{10, AMULET_LEVEL},{50, 80},   5,      4,          0,                  (BP_ROOM | BP_REWARD | BP_SURROUND_WITH_WALLS | BP_PURGE_INTERIOR), {
        {DF_AMBIENT_BLOOD,0,    0,              {1,2},      1,          0,          -1,         0,              2,              (HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {DF_AMBIENT_BLOOD,MONSTER_CAGE_CLOSED,DUNGEON,{2,4},2,          0,          -1,         0,              2,              (HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY)},
        {DF_TRIGGER_AREA,COFFIN_CLOSED,0,       {1,1},      1,          KEY,        KEY_CAGE,   MK_VAMPIRE,     1,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_SKELETON_KEY | MF_MONSTER_TAKE_ITEM | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN | MF_KEY_DISPOSABLE)},
        {DF_AMBIENT_BLOOD,SECRET_DOOR,DUNGEON,  {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)}}},
    // Legendary ally -- approach the altar with the crystal key to activate a portal and summon a legendary ally.
    {{8, AMULET_LEVEL}, {30, 50},   15,     2,          0,                  (BP_ROOM | BP_REWARD),  {
        {DF_LUMINESCENT_FUNGUS, ALTAR_KEYHOLE, DUNGEON, {1,1}, 1,       KEY,        KEY_PORTAL, 0,              2,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_NOT_IN_HALLWAY | MF_NEAR_ORIGIN | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_KEY_DISPOSABLE)},
        {DF_LUMINESCENT_FUNGUS, PORTAL, DUNGEON,{1,1},      1,          0,          -1,         0,              2,              HORDE_MACHINE_LEGENDARY_ALLY,0, (MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN)}}},
    // Goblin warren
    {{5, 15},           {100, 200}, 15,     9,          DP_GOBLIN_WARREN,   (BP_ROOM | BP_REWARD | BP_MAXIMIZE_INTERIOR | BP_REDESIGN_INTERIOR),    {
        {0,         MUD_FLOOR,  DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         MUD_DOORWAY,DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         MUD_WALL,   DUNGEON,        {1,1},      100,        0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_EVERYWHERE)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (SCROLL),   SCROLL_ENCHANTING, MK_GOBLIN_CHIEFTAN, 2,   0,          (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_MONSTER_SLEEPING | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (POTION),   POTION_LIFE, MK_GOBLIN_CHIEFTAN, 2,         0,          (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_MONSTER_SLEEPING | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         0,          0,              {5, 8},     5,          0,          -1,         0,              2,              HORDE_MACHINE_GOBLIN_WARREN,    0,  (MF_GENERATE_HORDE | MF_NOT_IN_HALLWAY | MF_MONSTER_SLEEPING)},
        {0,         0,          0,              {2,3},      2,          (WEAPON|ARMOR), -1,     0,              1,              0,          0,          (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {DF_HAY,    0,          0,              {10, 15},   1,          0,          -1,         0,              1,              0,          0,          (MF_NOT_IN_HALLWAY)},
        {DF_JUNK,   0,          0,              {7, 12},    1,          0,          -1,         0,              1,              0,          0,          (MF_NOT_IN_HALLWAY)}}},
    // Sentinel sanctuary
    {{10, 23},           {100, 200}, 15,  10,           DP_SENTINEL_SANCTUARY, (BP_ROOM | BP_REWARD | BP_MAXIMIZE_INTERIOR | BP_REDESIGN_INTERIOR), {
        {0,         MARBLE_FLOOR,DUNGEON,       {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         CRYSTAL_WALL,DUNGEON,       {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_BUILD_IN_WALLS | MF_EVERYWHERE)},
        {0,         PEDESTAL, DUNGEON, {1,1},   1,          (SCROLL),   SCROLL_ENCHANTING,0,    2,              0,              (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         PEDESTAL, DUNGEON, {1,1},   1,          (POTION),   POTION_LIFE,0,          2,              0,              (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         MACHINE_GLYPH,DUNGEON,      {30, 35},   20,         0,          -1,         0,              1,              0,          0,          (MF_PERMIT_BLOCKING)},
        {0,         STATUE_INERT,DUNGEON,       {3, 5},     3,          0,          -1,         MK_SENTINEL,    2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         STATUE_INERT,DUNGEON,       {10, 15},   8,          0,          -1,         MK_SENTINEL,    2,              0,          0,          MF_BUILD_IN_WALLS},
        {0,         0,          0,              {4, 6},     4,          0,          -1,         MK_GUARDIAN,    1,              0,          0,          MF_TREAT_AS_BLOCKING},
        {0,         0,          0,              {0, 2},     0,          0,          -1,         MK_WINGED_GUARDIAN, 1,          0,          0,          MF_TREAT_AS_BLOCKING},
        {0,         0,          0,              {2,3},      2,          (SCROLL | POTION), -1,  0,              1,              0,          0,          (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},

    // -- AMULET HOLDER --
    // Statuary -- key on an altar, area full of statues; take key to cause statues to burst and reveal monsters
    {{10, AMULET_LEVEL},{35, 40},   0,      4,          0,                  (BP_PURGE_INTERIOR | BP_OPEN_INTERIOR), {
        {DF_LUMINESCENT_FUNGUS, AMULET_SWITCH, DUNGEON, {1,1}, 1,       AMULET,     -1,         0,              2,              0,          0,          (MF_GENERATE_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         FUNGUS_FOREST,SURFACE,      {2,3},      0,          0,          -1,         0,              2,              0,          0,          MF_NOT_IN_HALLWAY},
        {0,         STATUE_INSTACRACK,DUNGEON,  {1,1},      1,          0,          -1,         MK_WARDEN_OF_YENDOR,1,          0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_IMPREGNABLE)},
        {0,         TORCH_WALL, DUNGEON,        {3,4},      0,          0,          0,          0,              1,              0,          0,          (MF_BUILD_IN_WALLS)}}},

    // -- VESTIBULES --

    // Plain locked door, key guarded by an adoptive room
    {{1, AMULET_LEVEL}, {1, 1},     100,        1,      0,                  (BP_VESTIBULE), {
        {0,         LOCKED_DOOR, DUNGEON,       {1,1},      1,          KEY,        KEY_DOOR,   0,              1,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS), (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_KEY_DISPOSABLE | MF_IMPREGNABLE)}}},
    // Plain secret door
    {{2, AMULET_LEVEL}, {1, 1},     1,      1,          0,                  (BP_VESTIBULE), {
        {0,         SECRET_DOOR, DUNGEON,       {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)}}},
    // Lever and either an exploding wall or a portcullis
    {{4, AMULET_LEVEL}, {1, 1},     8,      3,          0,                  (BP_VESTIBULE), {
        {0,         WORM_TUNNEL_OUTER_WALL,DUNGEON,{1,1},   1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_IMPREGNABLE | MF_ALTERNATIVE)},
        {0,         PORTCULLIS_CLOSED,DUNGEON,  {1,1},      1,          0,          0,          0,              3,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_IMPREGNABLE | MF_ALTERNATIVE)},
        {0,         WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
    // Flammable barricade in the doorway -- burn the wooden barricade to enter
    {{1, 6},            {1, 1},     10,     3,          0,                  (BP_VESTIBULE), {
        {0,         WOODEN_BARRICADE,DUNGEON,   {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          WEAPON,     INCENDIARY_DART, 0,         1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_INCINERATION, 0,     1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)}}},
    // Statue in the doorway -- use a scroll of shattering to enter
    {{1, AMULET_LEVEL}, {1, 1},     6,      2,          0,                  (BP_VESTIBULE), {
        {0,         STATUE_INERT_DOORWAY,DUNGEON,       {1,1},1,        0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          SCROLL,     SCROLL_SHATTERING, 0,       1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY)}}},
    // Statue in the doorway -- bursts to reveal monster
    {{5, AMULET_LEVEL}, {2, 2},     6,      2,          0,                  (BP_VESTIBULE), {
        {0,         STATUE_DORMANT_DOORWAY,DUNGEON,     {1, 1}, 1,      0,          -1,         0,              1,              HORDE_MACHINE_STATUE,0, (MF_PERMIT_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         MACHINE_TRIGGER_FLOOR,DUNGEON,{0,0},    1,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)}}},
    // Throwing tutorial -- toss an item onto the pressure plate to retract the portcullis
    {{1, 4},            {70, 70},   8,      3,          0,                  (BP_VESTIBULE), {
        {DF_MEDIUM_HOLE, MACHINE_PRESSURE_PLATE, LIQUID, {1,1}, 1,      0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         PORTCULLIS_CLOSED,DUNGEON,  {1,1},      1,          0,          0,          0,              3,              0,          0,          (MF_IMPREGNABLE | MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         WORM_TUNNEL_OUTER_WALL,DUNGEON,{1,1},   1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_IMPREGNABLE | MF_ALTERNATIVE)}}},
    // Pit traps -- area outside entrance is full of pit traps
    {{1, AMULET_LEVEL}, {30, 60},   8,      3,          0,                  (BP_VESTIBULE | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG),    {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         SECRET_DOOR,DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_IMPREGNABLE | MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         TRAP_DOOR_HIDDEN,DUNGEON,   {60, 60},   1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)}}},
    // Beckoning obstacle -- a mirrored totem guards the door, and glyph are around the doorway.
    {{5, AMULET_LEVEL}, {15, 30},   8,      3,          0,                  (BP_VESTIBULE | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR), {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      0,          0,          -1,         0,              1,              0,          0,          (MF_NEAR_ORIGIN | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          0,          -1,         MK_MIRRORED_TOTEM,3,            0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL)},
        {0,         MACHINE_GLYPH,DUNGEON,      {3,5},      2,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)}}},
    // Guardian obstacle -- a guardian is in the door on a glyph, with other glyphs scattered around.
    {{6, AMULET_LEVEL}, {25, 25},   8,      4,          0,                  (BP_VESTIBULE | BP_OPEN_INTERIOR),  {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          MK_GUARDIAN,    2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_ALTERNATIVE)},
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          MK_WINGED_GUARDIAN,2,           0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_ALTERNATIVE)},
        {0,         MACHINE_GLYPH,DUNGEON,      {10,10},    3,          0,          -1,         0,              1,              0,          0,          (MF_PERMIT_BLOCKING| MF_NEAR_ORIGIN)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      0,          0,          -1,         0,              2,              0,          0,          (MF_EVERYWHERE | MF_PERMIT_BLOCKING | MF_NOT_IN_HALLWAY)}}},

    // -- KEY HOLDERS --

    // Nested item library -- can check one item out at a time, and one is a disposable key to another reward room
    {{1, AMULET_LEVEL}, {30, 50},   35,     7,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_ADOPT_ITEM | BP_IMPREGNABLE), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         WALL,       DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {1,2},      1,          (WEAPON|ARMOR|WAND),-1, 0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_PLAYER_AVOIDS), (MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {1,2},      1,          (STAFF|RING|CHARM),-1,  0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN | ITEM_PLAYER_AVOIDS),    (MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {1,1},      1,          0,          -1,         0,              2,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_MAX_CHARGES_KNOWN),    (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         STATUE_INERT,DUNGEON,       {1,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
    // Secret room -- key on an altar in a secret room
    {{1, AMULET_LEVEL}, {15, 100},  1,      2,          0,                  (BP_ROOM | BP_ADOPT_ITEM), {
        {0,         ALTAR_INERT,DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          ITEM_PLAYER_AVOIDS, (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         SECRET_DOOR,DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)}}},
    // Throwing tutorial -- toss an item onto the pressure plate to retract the cage and reveal the key
    {{1, 4},            {70, 80},   8,      2,          0,                  (BP_ADOPT_ITEM), {
        {0,         ALTAR_CAGE_RETRACTABLE,DUNGEON,{1,1},   1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY)},
        {DF_MEDIUM_HOLE, MACHINE_PRESSURE_PLATE, LIQUID, {1,1}, 1,      0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    // Rat trap -- getting the key triggers paralysis vents nearby and also causes rats to burst out of the walls
    {{1,8},             {30, 70},   7,      3,          0,                  (BP_ADOPT_ITEM | BP_ROOM),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_PARALYSIS_VENT_HIDDEN,DUNGEON,{1,1},1,      0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)},
        {0,         RAT_TRAP_WALL_DORMANT,DUNGEON,{10,20},  5,          0,          -1,         MK_RAT,         1,              0,          0,          (MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_NOT_ON_LEVEL_PERIMETER)}}},
    // Fun with fire -- trigger the fire trap and coax the fire over to the wooden barricade surrounding the altar and key
    {{3, 10},           {80, 100},  10,     6,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR), {
        {DF_SURROUND_WOODEN_BARRICADE,ALTAR_INERT,DUNGEON,{1,1},1,      0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         GRASS,      SURFACE,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE | MF_ALTERNATIVE)},
        {DF_SWAMP,  0,          0,              {4,4},      2,          0,          -1,         0,              2,              0,          0,          (MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN)},
        {0,         FLAMETHROWER_HIDDEN,DUNGEON,{1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NEAR_ORIGIN)},
        {0,         GAS_TRAP_POISON_HIDDEN,DUNGEON,{3, 3},  1,          0,          -1,         0,              5,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE)},
        {0,         0,          0,              {2,2},      1,          POTION,     POTION_LICHEN, 0,           3,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)}}},
    // Flood room -- key on an altar in a room with pools of eel-infested waters; take key to flood room with shallow water
    {{3, AMULET_LEVEL}, {80, 180},  10,     4,          0,                  (BP_ROOM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS | BP_PURGE_PATHING_BLOCKERS | BP_ADOPT_ITEM),  {
        {0,         FLOOR_FLOODABLE,LIQUID,     {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              5,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {DF_SPREADABLE_WATER_POOL,0,0,          {2, 4},     1,          0,          -1,         0,              5,              HORDE_MACHINE_WATER_MONSTER,0,MF_GENERATE_HORDE},
        {DF_GRASS,  FOLIAGE,    SURFACE,        {3, 4},     3,          0,          -1,         0,              1,              0,          0,          0}}},
    // Fire trap room -- key on an altar, pools of water, fire traps all over the place.
    {{4, AMULET_LEVEL}, {80, 180},  6,      5,          0,                  (BP_ROOM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS | BP_PURGE_PATHING_BLOCKERS | BP_ADOPT_ITEM),  {
        {0,         ALTAR_INERT,DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         0,          0,              {1, 1},     1,          0,          -1,         0,              4,              0,          0,          MF_BUILD_AT_ORIGIN},
        {0,         FLAMETHROWER_HIDDEN,DUNGEON,{40, 60},   20,         0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING)},
        {DF_DEEP_WATER_POOL,0,  0,              {4, 4},     1,          0,          -1,         0,              4,              HORDE_MACHINE_WATER_MONSTER,0,MF_GENERATE_HORDE},
        {DF_GRASS,  FOLIAGE,    SURFACE,        {3, 4},     3,          0,          -1,         0,              1,              0,          0,          0}}},
    // Thief area -- empty altar, monster with item, permanently fleeing.
    {{3, AMULET_LEVEL}, {15, 20},   10,     2,          0,                  (BP_ADOPT_ITEM),    {
        {DF_LUMINESCENT_FUNGUS, ALTAR_INERT,DUNGEON,{1,1},  1,          0,          -1,         0,              2,              HORDE_MACHINE_THIEF,0,          (MF_ADOPT_ITEM | MF_BUILD_AT_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_GENERATE_HORDE | MF_MONSTER_TAKE_ITEM | MF_MONSTER_FLEEING)},
        {0,         STATUE_INERT,0,             {3, 5},     2,          0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    // Collapsing floor area -- key on an altar in an area; take key to cause the floor of the area to collapse
    {{1, AMULET_LEVEL}, {45, 65},   13,     3,          0,                  (BP_ADOPT_ITEM | BP_TREAT_AS_BLOCKING), {
        {0,         FLOOR_FLOODABLE,DUNGEON,    {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         ALTAR_SWITCH_RETRACTING,DUNGEON,{1,1},  1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {DF_ADD_MACHINE_COLLAPSE_EDGE_DORMANT,0,0,{3, 3},   2,          0,          -1,         0,              3,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)}}},
    // Pit traps -- key on an altar, room full of pit traps
    {{1, AMULET_LEVEL}, {30, 100},  10,     3,          0,                  (BP_ROOM | BP_ADOPT_ITEM),  {
        {0,         ALTAR_INERT,DUNGEON,        {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         TRAP_DOOR_HIDDEN,DUNGEON,   {30, 40},   1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {0,         SECRET_DOOR,DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)}}},
    // Levitation challenge -- key on an altar, room filled with pit, levitation or lever elsewhere on level, bridge appears when you grab the key/lever.
    {{1, 13},           {75, 120},  10,     9,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         TORCH_WALL, DUNGEON,        {1,4},      0,          0,          0,          0,              1,              0,          0,          (MF_BUILD_IN_WALLS)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              3,              0,          0,          MF_BUILD_AT_ORIGIN},
        {DF_ADD_DORMANT_CHASM_HALO, CHASM,LIQUID,{120, 120},1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_ADD_DORMANT_CHASM_HALO, CHASM_WITH_HIDDEN_BRIDGE,LIQUID,{1,1},1,0,      0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_LEVITATION, 0,       1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE)}}},
    // Web climbing -- key on an altar, room filled with pit, spider at altar to shoot webs, bridge appears when you grab the key
    {{7, AMULET_LEVEL}, {55, 90},   10,     7,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         MK_SPIDER,      3,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_IN_VIEW_OF_ORIGIN)},
        {0,         TORCH_WALL, DUNGEON,        {1,4},      0,          0,          0,          0,              1,              0,          0,          (MF_BUILD_IN_WALLS)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              3,              0,          0,          MF_BUILD_AT_ORIGIN},
        {DF_ADD_DORMANT_CHASM_HALO, CHASM,LIQUID,   {120, 120}, 1,      0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_ADD_DORMANT_CHASM_HALO, CHASM_WITH_HIDDEN_BRIDGE,LIQUID,{1,1},1,0,      0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)}}},
    // Lava moat room -- key on an altar, room filled with lava, levitation/fire immunity/lever elsewhere on level, lava retracts when you grab the key/lever
    {{3, 13},           {75, 120},  7,      7,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         LAVA,       LIQUID,         {60,60},    1,          0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_LAVA_RETRACTABLE, LAVA_RETRACTABLE, LIQUID, {1,1}, 1,       0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_LEVITATION, 0,       1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_FIRE_IMMUNITY, 0,    1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE)}}},
    // Lava moat area -- key on an altar, surrounded with lava, levitation/fire immunity elsewhere on level, lava retracts when you grab the key
    {{3, 13},           {40, 60},   3,      5,          0,                  (BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_TREAT_AS_BLOCKING),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_BUILD_AT_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         LAVA,       LIQUID,         {60,60},    1,          0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_LAVA_RETRACTABLE, LAVA_RETRACTABLE, LIQUID, {1,1}, 1,       0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_LEVITATION, 0,       1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_FIRE_IMMUNITY, 0,    1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)}}},
    // Poison gas -- key on an altar; take key to cause a caustic gas vent to appear and the door to be blocked; there is a hidden trapdoor or an escape item somewhere inside
    {{4, AMULET_LEVEL}, {35, 60},   7,      7,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM), {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING)},
        {0,         MACHINE_POISON_GAS_VENT_HIDDEN,DUNGEON,{1,2}, 1,    0,          -1,         0,              2,              0,          0,          0},
        {0,         TRAP_DOOR_HIDDEN,DUNGEON,   {1,1},      1,          0,          -1,         0,              2,              0,          0,          MF_ALTERNATIVE},
        {0,         0,          0,              {1,1},      1,          SCROLL,     SCROLL_TELEPORT, 0,         2,              0,          0,          (MF_GENERATE_ITEM | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_DESCENT, 0,          2,              0,          0,          (MF_GENERATE_ITEM | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         WALL_LEVER_HIDDEN_DORMANT,DUNGEON,{1,1},1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS)},
        {0,         PORTCULLIS_DORMANT,DUNGEON,{1,1},       1,          0,          0,          0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)}}},
    // Explosive situation -- key on an altar; take key to cause a methane gas vent to appear and a pilot light to ignite
    {{7, AMULET_LEVEL}, {80, 90},   10,     5,          0,                  (BP_ROOM | BP_PURGE_LIQUIDS | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM),  {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         FLOOR,      DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_FAR_FROM_ORIGIN)},
        {0,         MACHINE_METHANE_VENT_HIDDEN,DUNGEON,{1,1}, 1,       0,          -1,         0,              1,              0,          0,          MF_NEAR_ORIGIN},
        {0,         PILOT_LIGHT_DORMANT,DUNGEON,{1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_BUILD_IN_WALLS)}}},
    // Burning grass -- key on an altar; take key to cause pilot light to ignite grass in room
    {{1, 7},            {40, 110},  10,     6,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM | BP_OPEN_INTERIOR),  {
        {DF_SMALL_DEAD_GRASS,ALTAR_SWITCH_RETRACTING,DUNGEON,{1,1},1,   0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_FAR_FROM_ORIGIN)},
        {DF_DEAD_FOLIAGE,0,     SURFACE,        {2,3},      0,          0,          -1,         0,              1,              0,          0,          0},
        {0,         FOLIAGE,    SURFACE,        {1,4},      0,          0,          -1,         0,              1,              0,          0,          0},
        {0,         GRASS,      SURFACE,        {10,25},    0,          0,          -1,         0,              1,              0,          0,          0},
        {0,         DEAD_GRASS, SURFACE,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         PILOT_LIGHT_DORMANT,DUNGEON,{1,1},      1,          0,          -1,         0,              1,              0,          0,          MF_NEAR_ORIGIN | MF_BUILD_IN_WALLS}}},
    // Statuary -- key on an altar, area full of statues; take key to cause statues to burst and reveal monsters
    {{10, AMULET_LEVEL},{35, 90},   10,     2,          0,                  (BP_ADOPT_ITEM | BP_NO_INTERIOR_FLAG),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         STATUE_DORMANT,DUNGEON,     {3,5},      3,          0,          -1,         0,              2,              HORDE_MACHINE_STATUE,0, (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN)}}},
    // Guardian water puzzle -- key held by a guardian, flood trap in the room, glyphs scattered. Lure the guardian into the water to have him drop the key.
    {{4, AMULET_LEVEL}, {35, 70},   8,      4,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM), {
        {0,         0,          0,              {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          0,          -1,         MK_GUARDIAN,    2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_MONSTER_TAKE_ITEM)},
        {0,         FLOOD_TRAP,DUNGEON,         {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      4,          0,          -1,         0,              2,              0,          0,          (MF_EVERYWHERE | MF_NOT_IN_HALLWAY)}}},
    // Guardian gauntlet -- key in a room full of guardians, glyphs scattered and unavoidable.
    {{6, AMULET_LEVEL}, {50, 95},   10,     6,          0,                  (BP_ROOM | BP_ADOPT_ITEM),  {
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          0,              3,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {0,         0,          0,              {3,6},      3,          0,          -1,         MK_GUARDIAN,    2,              0,          0,          (MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,2},      1,          0,          -1,         MK_WINGED_GUARDIAN,2,           0,          0,          (MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         MACHINE_GLYPH,DUNGEON,      {10,15},   10,          0,          -1,         0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      0,          0,          -1,         0,              2,              0,          0,          (MF_EVERYWHERE | MF_PERMIT_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    // Guardian corridor -- key in a small room, with a connecting corridor full of glyphs, one guardian blocking the corridor.
    {{4, AMULET_LEVEL}, {85, 100},   5,     7,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),        {
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         MK_GUARDIAN,    3,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN  | MF_ALTERNATIVE)},
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         MK_WINGED_GUARDIAN,3,           0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN  | MF_ALTERNATIVE)},
        {0,         MACHINE_GLYPH,DUNGEON,      {3,5},      2,          0,          0,          0,              2,              0,          0,          MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              3,              0,          0,          MF_BUILD_AT_ORIGIN},
        {0,         WALL,DUNGEON,               {80,80},    1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_NOT_IN_HALLWAY | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
    // Sacrifice altar -- lure the chosen monster from elsewhere on the level onto the altar to release the key.
    {{4, AMULET_LEVEL}, {20, 60},   12,     6,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),        {
        {DF_BONES,  0,          0,              {3,4},      2,          0,          -1,         0,              1,              0,          0,          0},
        {0,         0,          0,              {1,1},      0,          0,          -1,         0,              2,              0,          0,          (MF_BUILD_IN_WALLS | MF_EVERYWHERE)},
        {DF_TRIGGER_AREA,SACRIFICE_ALTAR_DORMANT,DUNGEON,{1,1},1,       0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)},
        {0,         SACRIFICE_CAGE_DORMANT,DUNGEON,{1,1},   1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_NOT_IN_HALLWAY | MF_IMPREGNABLE)},
        {0,         DEMONIC_STATUE,DUNGEON,     {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY | MF_IMPREGNABLE)},
        {0,         STATUE_INSTACRACK,DUNGEON,  {1,1},      1,          0,          -1,         0,              2,              (HORDE_SACRIFICE_TARGET), 0, (MF_BUILD_ANYWHERE_ON_LEVEL | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY)}}},
    // Summoning circle -- key in a room with an eldritch totem, glyphs unavoidable. // DISABLED. (Not fun enough.)
    {{12, AMULET_LEVEL}, {50, 100}, 0,      2,          0,                  (BP_ROOM | BP_OPEN_INTERIOR | BP_ADOPT_ITEM),   {
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {DF_GLYPH_CIRCLE,0,     0,              {1,1},      1,          0,          -1,         MK_ELDRITCH_TOTEM,3,            0,          0,          (MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    // Beckoning obstacle -- key surrounded by glyphs in a room with a mirrored totem.
    {{5, AMULET_LEVEL}, {60, 100},  10,     4,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_ADOPT_ITEM), {
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN | MF_IN_VIEW_OF_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          0,          -1,         MK_MIRRORED_TOTEM,3,            0,          0,          (MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         MACHINE_GLYPH,DUNGEON,      {3,5},      2,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)}}},
    // Worms in the walls -- key on altar; take key to cause underworms to burst out of the walls
    {{12,AMULET_LEVEL}, {7, 7},     7,      2,          0,                  (BP_ADOPT_ITEM),    {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         WALL_MONSTER_DORMANT,DUNGEON,{5,8},     5,          0,          -1,         MK_UNDERWORM,   1,              0,          0,          (MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_NOT_ON_LEVEL_PERIMETER)}}},
    // Mud pit -- key on an altar, room full of mud, take key to cause bog monsters to spawn in the mud
    {{12, AMULET_LEVEL},{40, 90},   10,     3,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS),  {
        {DF_SWAMP,      0,      0,              {5,5},      0,          0,          -1,         0,              1,              0,          0,          0},
        {DF_SWAMP,  ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {DF_MUD_DORMANT,0,      0,              {3,4},      3,          0,          -1,         0,              1,              HORDE_MACHINE_MUD,0,    (MF_GENERATE_HORDE | MF_MONSTERS_DORMANT)}}},
    // Electric crystals -- key caged on an altar, darkened crystal globes around the room, lightning the globes to release the key.
    {{6, AMULET_LEVEL},{40, 60},    10,     5,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_PURGE_INTERIOR),  {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         ELECTRIC_CRYSTAL_OFF,DUNGEON,{3,4},     3,          0,          -1,         0,              3,              0,          0,          (MF_NOT_IN_HALLWAY | MF_IMPREGNABLE)},
        {0,         SACRED_GLYPH,  DUNGEON,     {1, 1},     1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         ALTAR_CAGE_RETRACTABLE,DUNGEON,{1,1},   1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         TURRET_LEVER, DUNGEON,      {7,9},      4,          0,          -1,         MK_SPARK_TURRET,3,              0,          0,          (MF_BUILD_IN_WALLS | MF_MONSTERS_DORMANT)}}},
    // Zombie crypt -- key on an altar; coffins scattered around; brazier in the room; take key to cause zombies to burst out of all of the coffins
    {{12, AMULET_LEVEL},{60, 90},   10,     8,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_PURGE_INTERIOR), {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {DF_BONES,  0,          0,              {3,4},      2,          0,          -1,         0,              1,              0,          0,          0},
        {DF_ASH,    0,          0,              {3,4},      2,          0,          -1,         0,              1,              0,          0,          0},
        {DF_AMBIENT_BLOOD,0,    0,              {1,2},      1,          0,          -1,         0,              1,              0,          0,          0},
        {DF_AMBIENT_BLOOD,0,    0,              {1,2},      1,          0,          -1,         0,              1,              0,          0,          0},
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         BRAZIER,    DUNGEON,        {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         COFFIN_CLOSED, DUNGEON,     {6,8},      1,          0,          0,          MK_ZOMBIE,      2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_MONSTERS_DORMANT)}}},
    // Haunted house -- key on an altar; take key to cause the room to darken, ectoplasm to cover everything and phantoms to appear
    {{16, AMULET_LEVEL},{45, 150},  10,     4,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS), {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         DARK_FLOOR_DORMANT,DUNGEON, {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         DARK_FLOOR_DORMANT,DUNGEON, {4,5},      4,          0,          -1,         MK_PHANTOM,     1,              0,          0,          (MF_MONSTERS_DORMANT)},
        {0,         HAUNTED_TORCH_DORMANT,DUNGEON,{5,10},   3,          0,          -1,         0,              2,              0,          0,          (MF_BUILD_IN_WALLS)}}},
    // Worm tunnels -- hidden lever causes tunnels to open up revealing worm areas and a key
    {{8, AMULET_LEVEL},{80, 175},   10,     6,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_MAXIMIZE_INTERIOR | BP_SURROUND_WITH_WALLS),  {
        {0,         ALTAR_INERT,DUNGEON,        {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {3,6},      3,          0,          -1,         MK_UNDERWORM,   1,              0,          0,          0},
        {0,         GRANITE,    DUNGEON,        {150,150},  1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_WORM_TUNNEL_MARKER_DORMANT,GRANITE,DUNGEON,{0,0},0,         0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE | MF_PERMIT_BLOCKING)},
        {DF_TUNNELIZE,WORM_TUNNEL_OUTER_WALL,DUNGEON,{1,1}, 1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)},
        {0,         WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
    // Gauntlet -- key on an altar; take key to cause turrets to emerge
    {{5, 24},           {35, 90},   10,     2,          0,                  (BP_ADOPT_ITEM | BP_NO_INTERIOR_FLAG),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)},
        {0,         TURRET_DORMANT,DUNGEON,     {4,6},      4,          0,          -1,         0,              2,              HORDE_MACHINE_TURRET,0, (MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN)}}},
    // Boss -- key is held by a boss atop a pile of bones in a secret room. A few fungus patches light up the area.
    {{5, AMULET_LEVEL}, {40, 100},  18,     3,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS), {
        {DF_BONES,  SECRET_DOOR,DUNGEON,        {1,1},      1,          0,          0,          0,              3,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {DF_LUMINESCENT_FUNGUS, STATUE_INERT,DUNGEON,{7,7}, 0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)},
        {DF_BONES,  0,          0,              {1,1},      1,          0,          -1,         0,              1,              HORDE_MACHINE_BOSS, 0,  (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_MONSTER_TAKE_ITEM | MF_GENERATE_HORDE | MF_MONSTER_SLEEPING)}}},

    // -- FLAVOR MACHINES --

    // Bloodwort -- bloodwort stalk, some pods, and surrounding grass
    {{1,DEEPEST_LEVEL}, {5, 5},     0,          2,      0,                  (BP_TREAT_AS_BLOCKING), {
        {DF_GRASS,  BLOODFLOWER_STALK, SURFACE, {1, 1},     1,          0,          -1,         0,              0,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_NOT_IN_HALLWAY)},
        {DF_BLOODFLOWER_PODS_GROW_INITIAL,0, 0, {1, 1},     1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_TREAT_AS_BLOCKING)}}},
    // Shrine -- safe haven constructed and abandoned by a past adventurer
    {{1,DEEPEST_LEVEL}, {15, 25},   0,          3,      0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR), {
        {0,         SACRED_GLYPH,  DUNGEON,     {1, 1},     1,          0,          -1,         0,              3,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         HAVEN_BEDROLL, SURFACE,     {1, 1},     1,          0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         BONES,      SURFACE,        {1, 1},     1,          (POTION|SCROLL|WEAPON|ARMOR|RING),-1,0, 2,              0,          0,          (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    // Idyll -- ponds and some grass and forest
    {{1,DEEPEST_LEVEL}, {80, 120},  0,      2,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_GRASS,  FOLIAGE,    SURFACE,        {3, 4},     3,          0,          -1,         0,              1,              0,          0,          0},
        {DF_DEEP_WATER_POOL,0,  0,              {2, 3},     2,          0,          -1,         0,              5,              0,          0,          (MF_NOT_IN_HALLWAY)}}},
    // Swamp -- mud, grass and some shallow water
    {{1,DEEPEST_LEVEL}, {50, 65},   0,      2,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_SWAMP,  0,          0,              {6, 8},     3,          0,          -1,         0,              1,              0,          0,          0},
        {DF_DEEP_WATER_POOL,0,  0,              {0, 1},     0,          0,          -1,         0,              3,              0,          0,          (MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)}}},
    // Camp -- hay, junk, urine, vomit
    {{1,DEEPEST_LEVEL}, {40, 50},   0,      4,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_HAY,    0,          0,              {1, 3},     1,          0,          -1,         0,              1,              0,          0,          (MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN)},
        {DF_JUNK,   0,          0,              {1, 2},     1,          0,          -1,         0,              3,              0,          0,          (MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN)},
        {DF_URINE,  0,          0,              {3, 5},     1,          0,          -1,         0,              1,              0,          0,          MF_IN_VIEW_OF_ORIGIN},
        {DF_VOMIT,  0,          0,              {0, 2},     0,          0,          -1,         0,              1,              0,          0,          MF_IN_VIEW_OF_ORIGIN}}},
    // Remnant -- carpet surrounded by ash and with some statues
    {{1,DEEPEST_LEVEL}, {80, 120},  0,      2,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_REMNANT, 0,         0,              {6, 8},     3,          0,          -1,         0,              1,              0,          0,          0},
        {0,         STATUE_INERT,DUNGEON,       {3, 5},     2,          0,          -1,         0,              1,              0,          0,          (MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)}}},
    // Dismal -- blood, bones, charcoal, some rubble
    {{1,DEEPEST_LEVEL}, {60, 70},   0,      3,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_AMBIENT_BLOOD, 0,   0,              {5,10},     3,          0,          -1,         0,              1,              0,          0,          MF_NOT_IN_HALLWAY},
        {DF_ASH,    0,          0,              {4, 8},     2,          0,          -1,         0,              1,              0,          0,          MF_NOT_IN_HALLWAY},
        {DF_BONES,  0,          0,              {3, 5},     2,          0,          -1,         0,              1,              0,          0,          MF_NOT_IN_HALLWAY}}},
    // Chasm catwalk -- narrow bridge over a chasm, possibly under fire from a turret or two
    {{1,DEEPEST_LEVEL-1},{40, 80},  0,      4,          0,                  (BP_REQUIRE_BLOCKING | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG), {
        {DF_CHASM_HOLE, 0,      0,              {80, 80},   1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_CATWALK_BRIDGE,0,   0,              {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         MACHINE_TRIGGER_FLOOR, DUNGEON, {0,1},  0,          0,          0,          0,              1,              0,          0,          (MF_NEAR_ORIGIN | MF_PERMIT_BLOCKING)},
        {0,         TURRET_DORMANT,DUNGEON,     {1, 2},     1,          0,          -1,         0,              2,              HORDE_MACHINE_TURRET,0, (MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN)}}},
    // Lake walk -- narrow bridge of shallow water through a lake, possibly under fire from a turret or two
    {{1,DEEPEST_LEVEL}, {40, 80},   0,      3,          0,                  (BP_REQUIRE_BLOCKING | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG), {
        {DF_LAKE_CELL,  0,      0,              {80, 80},   1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {0,         MACHINE_TRIGGER_FLOOR, DUNGEON, {0,1},  0,          0,          0,          0,              1,              0,          0,          (MF_NEAR_ORIGIN | MF_PERMIT_BLOCKING)},
        {0,         TURRET_DORMANT,DUNGEON,     {1, 2},     1,          0,          -1,         0,              2,              HORDE_MACHINE_TURRET,0, (MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN)}}},
    // Paralysis trap -- already-revealed pressure plate with a few hidden vents nearby.
    {{1,DEEPEST_LEVEL}, {35, 40},   0,      2,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         GAS_TRAP_PARALYSIS, DUNGEON, {1,2},     1,          0,          0,          0,              3,              0,          0,          (MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_PARALYSIS_VENT_HIDDEN,DUNGEON,{3, 4},2,     0,          0,          0,              3,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)}}},
    // Paralysis trap -- hidden pressure plate with a few vents nearby.
    {{1,DEEPEST_LEVEL}, {35, 40},   0,      2,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         GAS_TRAP_PARALYSIS_HIDDEN, DUNGEON, {1,2},1,        0,          0,          0,              3,              0,          0,          (MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_PARALYSIS_VENT_HIDDEN,DUNGEON,{3, 4},2,     0,          0,          0,              3,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)}}},
    // Statue comes alive -- innocent-looking statue that bursts to reveal a monster when the player approaches
    {{1,DEEPEST_LEVEL}, {5, 5},     0,      3,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         STATUE_DORMANT,DUNGEON,     {1, 1},     1,          0,          -1,         0,              1,              HORDE_MACHINE_STATUE,0, (MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         STATUE_DORMANT,DUNGEON,     {1, 1},     1,          0,          -1,         0,              1,              HORDE_MACHINE_STATUE,0, (MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_ALTERNATIVE | MF_NOT_ON_LEVEL_PERIMETER)},
        {0,         MACHINE_TRIGGER_FLOOR,DUNGEON,{0,0},    2,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)}}},
    // Worms in the walls -- step on trigger region to cause underworms to burst out of the walls
    {{1,DEEPEST_LEVEL}, {7, 7},     0,      2,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         WALL_MONSTER_DORMANT,DUNGEON,{1, 3},    1,          0,          -1,         MK_UNDERWORM,   1,              0,          0,          (MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_NOT_ON_LEVEL_PERIMETER)},
        {0,         MACHINE_TRIGGER_FLOOR,DUNGEON,{0,0},    2,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)}}},
    // Sentinels
    {{1,DEEPEST_LEVEL}, {40, 40},   0,      2,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         STATUE_INERT,DUNGEON,       {3, 3},     3,          0,          -1,         MK_SENTINEL,    2,              0,          0,          (MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING | MF_IN_VIEW_OF_ORIGIN)},
        {DF_ASH,    0,          0,              {2, 3},     0,          0,          -1,         0,              0,              0,          0,          0}}},
};


// Defines all creatures, which include monsters and the player:
creatureType monsterCatalog[NUMBER_MONSTER_KINDS] = {
    //  name            ch      color           HP      def     acc     damage          reg move    attack  blood           light isLarge     DFChance DFType         bolts       behaviorF, abilityF
    {0, "you",  G_PLAYER,       &playerInLightColor,30, 0,      100,    {1, 2, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_MALE | MONST_FEMALE)},

    {0, "rat",          G_RAT,    &gray,          6,      0,      80,     {1, 3, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      1,      DF_URINE,       {0}},
    {0, "kobold",       G_KOBOLD,    &goblinColor,   7,      0,      80,     {1, 4, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0}},
    {0, "jackal",       G_JACKAL,    &jackalColor,   8,      0,      70,     {2, 4, 1},      20, 50,     100,    DF_RED_BLOOD,   0,    false,      1,      DF_URINE,              {0}},
    {0, "eel",          G_EEL,    &eelColor,      18,     27,     100,    {3, 7, 2},      5,  50,     100,    0,              0,    false,      0,      0,              {0},
        (MONST_RESTRICTED_TO_LIQUID | MONST_IMMUNE_TO_WATER | MONST_SUBMERGES | MONST_FLITS | MONST_NEVER_SLEEPS)},
    {0, "monkey",       G_MONKEY,    &ogreColor,     12,     17,     100,    {1, 3, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      1,      DF_URINE,       {0},
        (0), (MA_HIT_STEAL_FLEE)},
    {0, "bloat",        G_BLOAT,    &poisonGasColor,4,      0,      100,    {0, 0, 0},      5,  100,    100,    DF_PURPLE_BLOOD,0,    false,      0,      DF_BLOAT_DEATH, {0},
        (MONST_FLIES | MONST_FLITS), (MA_KAMIKAZE | MA_DF_ON_DEATH)},
    {0, "pit bloat",    G_BLOAT,    &lightBlue,     4,      0,      100,    {0, 0, 0},      5,  100,    100,    DF_PURPLE_BLOOD,0,    false,      0,      DF_HOLE_POTION, {0},
        (MONST_FLIES | MONST_FLITS), (MA_KAMIKAZE | MA_DF_ON_DEATH)},
    {0, "goblin",       G_GOBLIN,    &goblinColor,   15,     10,     70,     {2, 5, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (0),  (MA_ATTACKS_PENETRATE | MA_AVOID_CORRIDORS)},
    {0, "goblin conjurer",G_GOBLIN_MAGIC,  &goblinConjurerColor, 10,10,    70,     {2, 4, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_MAINTAINS_DISTANCE | MONST_CAST_SPELLS_SLOWLY | MONST_CARRY_ITEM_25), (MA_CAST_SUMMON | MA_AVOID_CORRIDORS)},
    {0, "goblin mystic",G_GOBLIN_MAGIC,    &goblinMysticColor, 10, 10,     70,     {2, 4, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {BOLT_SHIELDING},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25), (MA_AVOID_CORRIDORS)},
    {0, "goblin totem", G_TOTEM, &orange,    30,     0,      0,      {0, 0, 0},      0,  100,    300,    DF_RUBBLE_BLOOD,IMP_LIGHT,    false,0,    0,              {BOLT_HASTE, BOLT_SPARK},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_WILL_NOT_USE_STAIRS), (0)},
    {0, "pink jelly",   G_JELLY,    &pinkJellyColor,50,     0,      85,     {1, 3, 1},      0,  100,    100,    DF_PURPLE_BLOOD,0,    true,       0,      0,              {0},
        (MONST_NEVER_SLEEPS), (MA_CLONE_SELF_ON_DEFEND)},
    {0, "toad",         G_TOAD,    &toadColor,     18,     0,      90,     {1, 4, 1},      10, 100,    100,    DF_GREEN_BLOOD, 0,    false,      0,      0,              {0},
        (0), (MA_HIT_HALLUCINATE)},
    {0, "vampire bat",  G_BAT,    &gray,          18,     25,     100,    {2, 6, 1},      20, 50,     100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_FLIES | MONST_FLITS), (MA_TRANSFERENCE)},
    {0, "arrow turret", G_TURRET,&black,     30,     0,      90,     {2, 6, 1},      0,  100,    250,    0,              0,    false,      0,      0,              {BOLT_DISTANCE_ATTACK},
        (MONST_TURRET), (0)},
    {0, "acid mound",   G_MOUND,    &acidBackColor, 15,     10,     70,     {1, 3, 1},      5,  100,    100,    DF_ACID_BLOOD,  0,    false,      0,      0,              {0},
        (MONST_DEFEND_DEGRADE_WEAPON), (MA_HIT_DEGRADE_ARMOR)},
    {0, "centipede",    G_CENTIPEDE,    &centipedeColor,20,     20,     80,     {4, 12, 1},     20, 100,    100,    DF_GREEN_BLOOD, 0,    false,      0,      0,              {0},
        (0), (MA_CAUSES_WEAKNESS)},
    {0, "ogre",         G_OGRE,    &ogreColor,     55,     60,     125,    {9, 13, 2},     20, 100,    200,    DF_RED_BLOOD,   0,    true,       0,      0,              {0},
        (MONST_MALE | MONST_FEMALE), (MA_AVOID_CORRIDORS | MA_ATTACKS_STAGGER)},
    {0, "bog monster",  G_BOG_MONSTER,    &krakenColor,   55,     60,     5000,   {3, 4, 1},      3,  200,    100,    0,              0,    true,       0,      0,              {0},
        (MONST_RESTRICTED_TO_LIQUID | MONST_SUBMERGES | MONST_FLITS | MONST_FLEES_NEAR_DEATH), (MA_SEIZES)},
    {0, "ogre totem",   G_TOTEM, &green,     70,     0,      0,      {0, 0, 0},      0,  100,    400,    DF_RUBBLE_BLOOD,LICH_LIGHT,    false,0,   0,              {BOLT_HEALING, BOLT_SLOW_2},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_WILL_NOT_USE_STAIRS), (0)},
    {0, "spider",       G_SPIDER,    &white,         20,     70,     90,     {3, 4, 2},      20, 100,    200,    DF_GREEN_BLOOD, 0,    false,      0,      0,              {BOLT_SPIDERWEB},
        (MONST_IMMUNE_TO_WEBS | MONST_CAST_SPELLS_SLOWLY | MONST_ALWAYS_USE_ABILITY), (MA_POISONS)},
    {0, "spark turret", G_TURRET, &lightningColor,80,0,      100,    {0, 0, 0},      0,  100,    150,    0,              SPARK_TURRET_LIGHT,    false, 0,  0,      {BOLT_SPARK},
        (MONST_TURRET), (0)},
    {0, "wisp",         G_WISP,    &wispLightColor,10,     90,     100,    {0, 0, 0},      5,  100,    100,    DF_ASH_BLOOD,   WISP_LIGHT,    false, 0,  0,              {0},
        (MONST_IMMUNE_TO_FIRE | MONST_FLIES | MONST_FLITS | MONST_NEVER_SLEEPS | MONST_FIERY | MONST_DIES_IF_NEGATED), (MA_HIT_BURN)},
    {0, "wraith",       G_WRAITH,    &wraithColor,   50,     60,     120,    {6, 13, 2},     5,  50,     100,    DF_GREEN_BLOOD, 0,    true,       0,      0,              {0},
        (MONST_FLEES_NEAR_DEATH)},
    {0, "zombie",       G_ZOMBIE,    &vomitColor,    80,     0,      120,    {7, 12, 1},     0,  100,    100,    DF_ROT_GAS_BLOOD,0,    true,      100,    DF_ROT_GAS_PUFF, {0}},
    {0, "troll",        G_TROLL,    &trollColor,    65,     70,     125,    {10, 15, 3},    1,  100,    100,    DF_RED_BLOOD,   0,    true,       0,      0,              {0},
        (MONST_MALE | MONST_FEMALE)},
    {0, "ogre shaman",  G_OGRE_MAGIC,    &green,         45,     40,     100,    {5, 9, 1},      20, 100,    200,    DF_RED_BLOOD,   0,    true,       0,      0,              {BOLT_HASTE, BOLT_SPARK},
        (MONST_MAINTAINS_DISTANCE | MONST_CAST_SPELLS_SLOWLY | MONST_MALE | MONST_FEMALE), (MA_CAST_SUMMON | MA_AVOID_CORRIDORS)},
    {0, "naga",         G_NAGA,    &trollColor,    75,     70,     150,    {7, 11, 4},     10, 100,    100,    DF_GREEN_BLOOD, 0,    true,       100,    DF_PUDDLE,      {0},
        (MONST_IMMUNE_TO_WATER | MONST_SUBMERGES | MONST_NEVER_SLEEPS | MONST_FEMALE), (MA_ATTACKS_ALL_ADJACENT)},
    {0, "salamander",   G_SALAMANDER,    &salamanderColor,60,    70,     150,    {5, 11, 3},     10, 100,    100,    DF_ASH_BLOOD,   SALAMANDER_LIGHT,    true,  100, DF_SALAMANDER_FLAME, {0},
        (MONST_IMMUNE_TO_FIRE | MONST_SUBMERGES | MONST_NEVER_SLEEPS | MONST_FIERY | MONST_MALE), (MA_ATTACKS_EXTEND)},
    {0, "explosive bloat",G_BLOAT,  &orange,        10,     0,      100,    {0, 0, 0},      5,  100,    100,    DF_RED_BLOOD,   EXPLOSIVE_BLOAT_LIGHT,    false,0, DF_BLOAT_EXPLOSION, {0},
        (MONST_FLIES | MONST_FLITS), (MA_KAMIKAZE | MA_DF_ON_DEATH)},
    {0, "dar blademaster",G_DAR_BLADEMASTER,  &purple,        35,     70,     160,    {5, 9, 2},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {BOLT_BLINKING},
        (MONST_CARRY_ITEM_25 | MONST_MALE | MONST_FEMALE), (MA_AVOID_CORRIDORS)},
    {0, "dar priestess", G_DAR_PRIESTESS,   &darPriestessColor,20,  60,     100,    {2, 5, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {BOLT_NEGATION, BOLT_HEALING, BOLT_HASTE, BOLT_SPARK},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25 | MONST_FEMALE), (MA_AVOID_CORRIDORS)},
    {0, "dar battlemage",G_DAR_BATTLEMAGE,   &darMageColor,  20,     60,     100,    {1, 3, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {BOLT_FIRE, BOLT_SLOW_2, BOLT_DISCORD},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25 | MONST_MALE | MONST_FEMALE), (MA_AVOID_CORRIDORS)},
    {0, "acidic jelly", G_JELLY,    &acidBackColor, 60,     0,      115,    {2, 6, 1},      0,  100,    100,    DF_ACID_BLOOD,  0,    true,       0,      0,              {0},
        (MONST_DEFEND_DEGRADE_WEAPON), (MA_HIT_DEGRADE_ARMOR | MA_CLONE_SELF_ON_DEFEND)},
    {0, "centaur",      G_CENTAUR,    &tanColor,      35,     50,     175,    {4, 8, 2},      20, 50,     100,    DF_RED_BLOOD,   0,    true,       0,      0,              {BOLT_DISTANCE_ATTACK},
        (MONST_MAINTAINS_DISTANCE | MONST_MALE), (0)},
    {0, "underworm",    G_UNDERWORM,    &wormColor,     80,     40,     160,    {18, 22, 2},    3,  150,    200,    DF_WORM_BLOOD,  0,    true,       0,      0,              {0},
        (MONST_NEVER_SLEEPS)},
    {0, "sentinel",     G_GUARDIAN, &sentinelColor, 50,0,      0,      {0, 0, 0},      0,  100,    175,    DF_RUBBLE_BLOOD,SENTINEL_LIGHT,    false,0,0,             {BOLT_HEALING, BOLT_SPARK},
        (MONST_TURRET | MONST_CAST_SPELLS_SLOWLY | MONST_DIES_IF_NEGATED), (0)},
    {0, "dart turret", G_TURRET, &centipedeColor,20, 0,      140,    {1, 2, 1},      0,  100,    250,    0,              0,    false,      0,      0,              {BOLT_POISON_DART},
        (MONST_TURRET), (MA_CAUSES_WEAKNESS)},
    {0, "kraken",       G_KRAKEN,    &krakenColor,   120,    0,      150,    {15, 20, 3},    1,  50,     100,    0,              0,    true,       0,      0,              {0},
        (MONST_RESTRICTED_TO_LIQUID | MONST_IMMUNE_TO_WATER | MONST_SUBMERGES | MONST_FLITS | MONST_NEVER_SLEEPS | MONST_FLEES_NEAR_DEATH), (MA_SEIZES)},
    {0, "lich",         G_LICH,    &white,         35,     80,     175,    {2, 6, 1},      0,  100,    100,    DF_ASH_BLOOD,   LICH_LIGHT,    true,  0,  0,              {BOLT_FIRE},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25 | MONST_NO_POLYMORPH), (MA_CAST_SUMMON)},
    {0, "phylactery",   G_EGG,&lichLightColor,30,    0,      0,      {0, 0, 0},      0,  100,    150,    DF_RUBBLE_BLOOD,LICH_LIGHT,    false, 0,  0,              {0},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED), (MA_CAST_SUMMON | MA_ENTER_SUMMONS)},
    {0, "pixie",        G_PIXIE,    &pixieColor,    10,     90,     100,    {1, 3, 1},      20, 50,     100,    DF_GREEN_BLOOD, PIXIE_LIGHT,    false, 0, 0,              {BOLT_NEGATION, BOLT_SLOW_2, BOLT_DISCORD, BOLT_SPARK},
        (MONST_MAINTAINS_DISTANCE | MONST_FLIES | MONST_FLITS | MONST_MALE | MONST_FEMALE), (0)},
    {0, "phantom",      G_PHANTOM,    &ectoplasmColor,35,     70,     160,    {12, 18, 4},    0,  50,     200,    DF_ECTOPLASM_BLOOD, 0,    true,   2,      DF_ECTOPLASM_DROPLET, {0},
        (MONST_INVISIBLE | MONST_FLITS | MONST_FLIES | MONST_IMMUNE_TO_WEBS)},
    {0, "flame turret", G_TURRET, &lavaForeColor,40, 0,      150,    {1, 2, 1},      0,  100,    250,    0,              LAVA_LIGHT,    false, 0,  0,              {BOLT_FIRE},
        (MONST_TURRET), (0)},
    {0, "imp",          G_IMP,    &pink,          35,     90,     225,    {4, 9, 2},      10, 100,    100,    DF_GREEN_BLOOD, IMP_LIGHT,    false,  0,  0,              {BOLT_BLINKING},
        (0), (MA_HIT_STEAL_FLEE)},
    {0, "fury",         G_FURY,    &darkRed,       19,     90,     200,    {6, 11, 4},     20, 50,     100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_NEVER_SLEEPS | MONST_FLIES)},
    {0, "revenant",     G_REVENANT,    &ectoplasmColor,30,     0,      200,    {15, 20, 5},    0,  100,    100,    DF_ECTOPLASM_BLOOD, 0,    true,   0,      0,              {0},
        (MONST_IMMUNE_TO_WEAPONS)},
    {0, "tentacle horror",G_TENTACLE_HORROR,  &centipedeColor,120,    95,     225,    {25, 35, 3},    1,  100,    100,    DF_PURPLE_BLOOD,0,    true,       0,      0,              {0}},
    {0, "golem",        G_GOLEM,    &gray,          400,    70,     225,    {4, 8, 1},      0,  100,    100,    DF_RUBBLE_BLOOD,0,    true,       0,      0,              {0},
        (MONST_REFLECT_4 | MONST_DIES_IF_NEGATED)},
    {0, "dragon",       G_DRAGON,    &dragonColor,   150,    90,     250,    {25, 50, 4},    20, 50,     200,    DF_GREEN_BLOOD, 0,    true,       0,      0,              {BOLT_DRAGONFIRE},
        (MONST_IMMUNE_TO_FIRE | MONST_CARRY_ITEM_100), (MA_ATTACKS_ALL_ADJACENT)},

    // bosses
    {0, "goblin warlord",G_GOBLIN_CHIEFTAN,   &blue,          30,     17,     100,    {3, 6, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25), (MA_CAST_SUMMON | MA_ATTACKS_PENETRATE | MA_AVOID_CORRIDORS)},
    {0, "black jelly",  G_JELLY,    &black,         120,    0,      130,    {3, 8, 1},      0,  100,    100,    DF_PURPLE_BLOOD,0,    true,       0,      0,              {0},
        (0), (MA_CLONE_SELF_ON_DEFEND)},
    {0, "vampire",      G_VAMPIRE,    &white,         75,     60,     120,    {4, 15, 2},     6,  50,     100,    DF_RED_BLOOD,   0,    true,       0,      DF_BLOOD_EXPLOSION, {BOLT_BLINKING, BOLT_DISCORD},
        (MONST_FLEES_NEAR_DEATH | MONST_MALE), (MA_TRANSFERENCE | MA_DF_ON_DEATH | MA_CAST_SUMMON | MA_ENTER_SUMMONS)},
    {0, "flamedancer",  G_FLAMEDANCER,    &white,         65,     80,     120,    {3, 8, 2},      0,  100,    100,    DF_EMBER_BLOOD, FLAMEDANCER_LIGHT,    true, 100,DF_FLAMEDANCER_CORONA, {BOLT_FIRE},
        (MONST_MAINTAINS_DISTANCE | MONST_IMMUNE_TO_FIRE | MONST_FIERY), (MA_HIT_BURN)},

    // special effect monsters
    {0, "spectral blade",G_WEAPON, &spectralBladeColor,1, 0, 150,    {1, 1, 1},      0,  50,     100,    0,              SPECTRAL_BLADE_LIGHT,    false,0,0,       {0},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_FLIES | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_IMMUNE_TO_WEBS | MONST_NOT_LISTED_IN_SIDEBAR)},
    {0, "spectral sword",G_WEAPON, &spectralImageColor, 1,0, 150,    {1, 1, 1},      0,  50,     100,    0,              SPECTRAL_IMAGE_LIGHT,    false,0,0,       {0},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_FLIES | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_IMMUNE_TO_WEBS)},
    {0, "stone guardian",G_GUARDIAN, &white,   1000,   0,      200,    {12, 17, 2},    0,  100,    100,    DF_RUBBLE,      0,    false,      100,      DF_GUARDIAN_STEP, {0},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_ALWAYS_HUNTING | MONST_IMMUNE_TO_FIRE | MONST_IMMUNE_TO_WEAPONS | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_REFLECT_4 | MONST_ALWAYS_USE_ABILITY | MONST_GETS_TURN_ON_ACTIVATION)},
    {0, "winged guardian",G_WINGED_GUARDIAN, &blue,   1000,   0,      200,    {12, 17, 2},    0,  100,    100,    DF_RUBBLE,      0,    false,      100,      DF_SILENT_GLYPH_GLOW, {BOLT_BLINKING},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_ALWAYS_HUNTING | MONST_IMMUNE_TO_FIRE | MONST_IMMUNE_TO_WEAPONS | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_REFLECT_4 | MONST_GETS_TURN_ON_ACTIVATION | MONST_ALWAYS_USE_ABILITY), (0)},
    {0, "guardian spirit",G_GUARDIAN, &spectralImageColor,1000,0,200,  {5, 12, 2},     0,  100,    100,    0,              SPECTRAL_IMAGE_LIGHT,    false,100,0,     {0},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_IMMUNE_TO_FIRE | MONST_IMMUNE_TO_WEAPONS | MONST_DIES_IF_NEGATED | MONST_REFLECT_4 | MONST_ALWAYS_USE_ABILITY)},
    {0, "Warden of Yendor",G_WARDEN, &yendorLightColor,1000,   0,    300,    {12, 17, 2},    0,  200,    200,    DF_RUBBLE,      YENDOR_LIGHT,    true,  100, 0,           {0},
        (MONST_NEVER_SLEEPS | MONST_ALWAYS_HUNTING | MONST_INVULNERABLE | MONST_NO_POLYMORPH)},
    {0, "eldritch totem",G_TOTEM, &glyphColor,80,    0,      0,      {0, 0, 0},      0,  100,    100,    DF_RUBBLE_BLOOD,0,    false,      0,      0,              {0},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS | MONST_GETS_TURN_ON_ACTIVATION | MONST_ALWAYS_USE_ABILITY), (MA_CAST_SUMMON)},
    {0, "mirrored totem",G_TOTEM, &beckonColor,80,   0,      0,      {0, 0, 0},      0,  100,    100,    DF_RUBBLE_BLOOD,0,    false,      100,    DF_MIRROR_TOTEM_STEP, {BOLT_BECKONING},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS | MONST_GETS_TURN_ON_ACTIVATION | MONST_ALWAYS_USE_ABILITY | MONST_REFLECT_4 | MONST_IMMUNE_TO_WEAPONS | MONST_IMMUNE_TO_FIRE), (0)},

    // legendary allies
    {0, "unicorn",      G_UNICORN, &white,   40,     60,     175,    {2, 10, 2},     20, 50,     100,    DF_RED_BLOOD,   UNICORN_LIGHT,    true, 1,DF_UNICORN_POOP, {BOLT_HEALING, BOLT_SHIELDING},
        (MONST_MAINTAINS_DISTANCE | MONST_MALE | MONST_FEMALE), (0)},
    {0, "ifrit",        G_IFRIT,    &ifritColor,    40,     75,     175,    {5, 13, 2},     1,  50,     100,    DF_ASH_BLOOD,   IFRIT_LIGHT,    true, 0,  0,              {BOLT_DISCORD},
        (MONST_IMMUNE_TO_FIRE | MONST_FLIES | MONST_MALE), (0)},
    {0, "phoenix",      G_PHOENIX,    &phoenixColor,  30,     70,     175,    {4, 10, 2},     0,  50,     100,    DF_ASH_BLOOD,   PHOENIX_LIGHT,    true, 0,0,              {0},
        (MONST_IMMUNE_TO_FIRE| MONST_FLIES | MONST_NO_POLYMORPH)},
    {0, "phoenix egg",  G_EGG,&phoenixColor, 50,     0,      0,      {0, 0, 0},      0,  100,    150,    DF_ASH_BLOOD,   PHOENIX_EGG_LIGHT,    false,  0,  0,      {0},
        (MONST_IMMUNE_TO_FIRE| MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_WILL_NOT_USE_STAIRS | MONST_NO_POLYMORPH | MONST_ALWAYS_HUNTING | MONST_IMMUNE_TO_WEAPONS), (MA_CAST_SUMMON | MA_ENTER_SUMMONS)},
    {0, "mangrove dryad",G_ANCIENT_SPIRIT,   &tanColor,      70,     60,     175,    {2, 8, 2},      6,  100,    100,    DF_ASH_BLOOD,   0,    true,       0,      0,              {BOLT_ANCIENT_SPIRIT_VINES},
        (MONST_IMMUNE_TO_WEBS | MONST_ALWAYS_USE_ABILITY | MONST_MAINTAINS_DISTANCE | MONST_NO_POLYMORPH | MONST_MALE | MONST_FEMALE), (0)},
};

const monsterWords monsterText[NUMBER_MONSTER_KINDS] = {
    {"A naked adventurer in an unforgiving place, bereft of equipment and confused about the circumstances.",
        "studying", "Studying",
        {"hit", {0}}},
    {"The rat is a scavenger of the shallows, perpetually in search of decaying animal matter.",
        "gnawing at", "Eating",
        {"scratches", "bites", {0}}},
    {"The kobold is a lizardlike humanoid of the upper dungeon.",
        "poking at", "Examining",
        {"clubs", "bashes", {0}}},
    {"The jackal prowls the caverns for intruders to rend with $HISHER powerful jaws.",
        "tearing at", "Eating",
        {"claws", "bites", "mauls", {0}}},
    {"The eel slips silently through the subterranean lake, waiting for unsuspecting prey to set foot in $HISHER dark waters.",
        "eating", "Eating",
        {"shocks", "bites", {0}}},
    {"Mischievous trickster that $HESHE is, the monkey lives to steal shiny trinkets from passing adventurers.",
        "examining", "Examining",
        {"tweaks", "bites", "punches", {0}}},
    {"A bladder of deadly gas buoys the bloat through the air, $HISHER thin veinous membrane ready to rupture at the slightest stress.",
        "gazing at", "Gazing",
        {"bumps", {0}},
        "bursts, leaving behind an expanding cloud of caustic gas!"},
    {"This rare subspecies of bloat is filled with a peculiar vapor that, if released, will cause the floor to vanish out from underneath $HIMHER.",
        "gazing at", "Gazing",
        {"bumps", {0}},
        "bursts, causing the floor underneath $HIMHER to disappear!"},
    {"A filthy little primate, the tribalistic goblin often travels in packs and carries a makeshift stone spear.",
        "chanting over", "Chanting",
        {"cuts", "stabs", "skewers", {0}}},
    {"This goblin is covered with glowing sigils that pulse with power. $HESHE can call into existence phantom blades to attack $HISHER foes.",
        "performing a ritual on", "Performing ritual",
        {"thumps", "whacks", "wallops", {0}},
        {0},
        "gestures ominously!"},
    {"This goblin carries no weapon, and $HISHER eyes sparkle with golden light. $HESHE can invoke a powerful shielding magic to protect $HISHER escorts from harm.",
        "performing a ritual on", "Performing ritual",
        {"slaps", "punches", "kicks", {0}}},
    {"Goblins have created this makeshift totem and imbued $HIMHER with a shamanistic power.",
        "gazing at", "Gazing",
        {"hits", {0}}},
    {"This mass of caustic pink goo slips across the ground in search of a warm meal.",
        "absorbing", "Feeding",
        {"smears", "slimes", "drenches"}},
    {"The enormous, warty toad secretes a powerful hallucinogenic slime to befuddle the senses of any creatures that come in contact with $HIMHER.",
        "eating", "Eating",
        {"slimes", "slams", {0}}},
    {"Often hunting in packs, leathery wings and keen senses guide the vampire bat unerringly to $HISHER prey.",
        "draining", "Feeding",
        {"nips", "bites", {0}}},
    {"A mechanical contraption embedded in the wall, the spring-loaded arrow turret will fire volley after volley of arrows at intruders.",
        "gazing at", "Gazing",
        {"shoots", {0}}},
    {"The acid mound squelches softly across the ground, leaving a trail of hissing goo in $HISHER path.",
        "liquefying", "Feeding",
        {"slimes", "douses", "drenches", {0}}},
    {"This monstrous centipede's incisors are imbued with a horrible venom that will slowly kill $HISHER prey.",
        "eating", "Eating",
        {"pricks", "stings", {0}}},
    {"This lumbering creature carries an enormous club that $HESHE can swing with incredible force.",
        "examining", "Studying",
        {"cudgels", "clubs", "batters", {0}}},
    {"The horrifying bog monster dwells beneath the surface of mud-filled swamps. When $HISHER prey ventures into the mud, the bog monster will ensnare the unsuspecting victim in $HISHER pale tentacles and squeeze its life away.",
        "draining", "Feeding",
        {"squeezes", "strangles", "crushes", {0}}},
    {"Ancient ogres versed in the eldritch arts have assembled this totem and imbued $HIMHER with occult power.",
        "gazing at", "Gazing",
        {"hits", {0}}},
    {"The spider's red eyes pierce the darkness in search of enemies to ensnare with $HISHER projectile webs and dissolve with deadly poison.",
        "draining", "Feeding",
        {"bites", "stings", {0}}},
    {"This contraption hums with electrical charge that $HISHER embedded crystals and magical sigils can direct at intruders in deadly arcs.",
        "gazing at", "Gazing",
        {"shocks", {0}}},
    {"An ethereal blue flame dances through the air, flickering and pulsing in time to an otherworldly rhythm.",
        "consuming", "Feeding",
        {"scorches", "burns", {0}}},
    {"The wraith's hollow eye sockets stare hungrily at the world from $HISHER emaciated frame, and $HISHER long, bloodstained nails grope ceaselessly at the air for a fresh victim.",
        "devouring", "Feeding",
        {"clutches", "claws", "bites", {0}}},
    {"The zombie is the accursed product of a long-forgotten ritual. Perpetually decaying flesh hangs from $HISHER bones in shreds and releases a flammable stench that will induce violent nausea with one whiff.",
        "rending", "Eating",
        {"hits", "bites", {0}}},
    {"An enormous, disfigured creature covered in phlegm and warts, the troll regenerates very quickly and attacks with astonishing strength. Many adventures have ended at $HISHER misshapen hands.",
        "eating", "Eating",
        {"cudgels", "clubs", "bludgeons", "pummels", "batters"}},
    {"This ogre is bent with age, but what $HESHE has lost in physical strength, $HESHE has more than gained in occult power.",
        "performing a ritual on", "Performing ritual",
        {"cudgels", "clubs", {0}},
        {0},
        "chants in a harsh, guttural tongue!"},
    {"The serpentine naga live beneath the subterranean waters and emerge to attack unsuspecting adventurers.",
        "studying", "Studying",
        {"claws", "bites", "tail-whips", {0}}},
    {"A serpent wreathed in flames and carrying a burning lash, salamanders dwell in lakes of fire and emerge when they sense a nearby victim, leaving behind a trail of glowing embers.",
        "studying", "Studying",
        {"whips", "lashes", {0}}},
    {"This rare subspecies of bloat is little more than a thin membrane surrounding a bladder of highly explosive gases. The slightest stress will cause $HIMHER to rupture in spectacular and deadly fashion.",
        "gazing at", "Gazing",
        {"bumps", {0}},
        "detonates with terrifying force!"},
    {"An elf of the deep, the dar blademaster leaps toward $HISHER enemies with frightening speed to engage in deadly swordplay.",
        "studying", "Studying",
        {"grazes", "cuts", "slices", "slashes", "stabs"}},
    {"The dar priestess carries a host of religious relics that jangle as $HESHE walks.",
        "praying over", "Praying",
        {"cuts", "slices", {0}}},
    {"The dar battlemage's eyes glow like embers and $HISHER hands radiate an occult heat.",
        "transmuting", "Transmuting",
        {"cuts", {0}}},
    {"A jelly subsisting on a diet of acid mounds will eventually express the characteristics of $HISHER prey, corroding any unprotected weapons or armor that come in contact with $HIMHER.",
        "transmuting", "Transmuting",
        {"burns", {0}}},
    {"Half man and half horse, the centaur is an expert with the bow and arrow -- hunter and steed fused into a single creature.",
        "studying", "Studying",
        {"shoots", {0}}},
    {"A strange and horrifying creature of the earth's deepest places, larger than an ogre but capable of squeezing through tiny openings. When hungry, the underworm will burrow behind the walls of a cavern and lurk dormant and motionless -- often for months -- until $HESHE can feel the telltale vibrations of nearby prey.",
        "consuming", "Consuming",
        {"slams", "bites", "tail-whips", {0}}},
    {"An ancient statue of an unrecognizable humanoid figure, the sentinel holds aloft a crystal that gleams with ancient warding magic. Sentinels are always found in groups, and each will attempt to repair any damage done to the others.",
        "focusing on", "Focusing",
        {"hits", {0}}},
    {"This spring-loaded contraption fires darts that are imbued with a strength-sapping poison.",
        "gazing at", "Gazing",
        {"pricks", {0}}},
    {"This tentacled nightmare will emerge from the subterranean waters to ensnare and devour any creature foolish enough to set foot into $HISHER lake.",
        "devouring", "Feeding",
        {"slaps", "smites", "batters", {0}}},
    {"The desiccated form of an ancient sorcerer, animated by dark arts and lust for power, commands the obedience of the infernal planes. $HISHER essence is anchored to reality by a phylactery that is always in $HISHER possession, and the lich cannot die unless $HISHER phylactery is destroyed.",
        "enchanting", "Enchanting",
        {"touches", {0}},
        {0},
        "rasps a terrifying incantation!"},
    {"This gem was the fulcrum of a dark rite, performed centuries ago, that bound the soul of an ancient and terrible sorcerer. Hurry and destroy the gem, before the lich can gather its power and regenerate its corporeal form!",
        "enchanting", "Enchanting",
        {"touches", {0}},
        {0},
        "swirls with dark sorcery as the lich regenerates its form!"},
    {"A tiny humanoid sparkles in the gloom, the hum of $HISHER beating wings punctuated by intermittent peals of high-pitched laughter. What $HESHE lacks in physical endurance, $HESHE makes up for with $HISHER wealth of mischievous magical abilities.",
        "sprinkling dust on", "Dusting",
        {"pokes", {0}}},
    {"A silhouette of mournful rage against an empty backdrop, the phantom slips through the dungeon invisibly in clear air, leaving behind glowing droplets of ectoplasm and the cries of $HISHER unsuspecting victims.",
        "permeating", "Permeating",
        {"hits", {0}}},
    {"This infernal contraption spits blasts of flame at intruders.",
        "incinerating", "Incinerating",
        {"pricks", {0}}},
    {"This trickster demon moves with astonishing speed and delights in stealing from $HISHER enemies and blinking away.",
        "dissecting", "Dissecting",
        {"slices", "cuts", {0}}},
    {"A creature of inchoate rage made flesh, the fury's moist wings beat loudly in the darkness.",
        "flagellating", "Flagellating",
        {"drubs", "fustigates", "castigates", {0}}},
    {"This unholy specter stalks the deep places of the earth without fear, impervious to conventional attacks.",
        "desecrating", "Desecrating",
        {"hits", {0}}},
    {"This seething, towering nightmare of fleshy tentacles slinks through the bowels of the world. The tentacle horror's incredible strength and regeneration make $HIMHER one of the most fearsome creatures of the dungeon.",
        "sucking on", "Consuming",
        {"slaps", "batters", "crushes", {0}}},
    {"A statue animated by an ancient and tireless magic, the golem does not regenerate and attacks with only moderate strength, but $HISHER stone form can withstand incredible damage before collapsing into rubble.",
        "cradling", "Cradling",
        {"backhands", "punches", "kicks", {0}}},
    {"An ancient serpent of the world's deepest places, the dragon's immense form belies its lightning-quick speed and testifies to $HISHER breathtaking strength. An undying furnace of white-hot flames burns within $HISHER scaly hide, and few could withstand a single moment under $HISHER infernal lash.",
        "consuming", "Consuming",
        {"claws", "tail-whips", "bites", {0}}},

    {"Taller, stronger and smarter than other goblins, the warlord commands the loyalty of $HISHER kind and can summon them into battle.",
        "chanting over", "Chanting",
        {"slashes", "cuts", "stabs", "skewers", {0}},
        {0},
        "lets loose a deafening war cry!"},
    {"This blob of jet-black goo is as rare as $HESHE is deadly. Few creatures of the dungeon can withstand $HISHER caustic assault. Beware.",
        "absorbing", "Feeding",
        {"smears", "slimes", "drenches"}},
    {"This vampire lives a solitary life deep underground, consuming any warm-blooded creature unfortunate to venture near $HISHER lair.",
        "draining", "Drinking",
        {"grazes", "bites", "buries $HISHER fangs in", {0}},
        {0},
        "spreads his cloak and bursts into a cloud of bats!"},
    {"An elemental creature from another plane of existence, the infernal flamedancer burns with such intensity that $HESHE is painful to behold.",
        "immolating", "Consuming",
        {"singes", "burns", "immolates", {0}}},

    {"Eldritch forces have coalesced to form this flickering, ethereal weapon.",
        "gazing at", "Gazing",
        {"nicks",  {0}}},
    {"Eldritch energies bound up in your equipment have leapt forth to project this spectral image.",
        "gazing at", "Gazing",
        {"hits",  {0}}},
    {"Guarding the room is a weathered stone statue of a knight carrying a battleaxe, connected to the glowing glyphs on the floor by invisible strands of enchantment.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},
    {"A statue of a sword-wielding angel surveys the room, connected to the glowing glyphs on the floor by invisible strands of enchantment.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},
    {"A spectral outline of a knight carrying a battleaxe casts an ethereal light on $HISHER surroundings.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},
    {"An immortal presence stalks through the dungeon, implacably hunting that which was taken... and the one who took it.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},
    {"This totem sits at the center of a summoning circle that radiates a strange energy.",
        "gazing at", "Gazing",
        {"strikes",  {0}},
        {0},
        "crackles with energy as you touch the glyph!"},
    {"A prism of shoulder-high mirrored surfaces gleams in the darkness.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},

    {"The unicorn's flowing mane and tail shine with rainbow light, $HISHER horn glows with healing and protective magic, and $HISHER eyes implore you to always chase your dreams. Unicorns are rumored to be attracted to virgins -- is there a hint of accusation in $HISHER gaze?",
        "consecrating", "Consecrating",
        {"pokes", "stabs", "gores", {0}}},
    {"A whirling desert storm given human shape, the ifrit's twin scimitars flicker in the darkness and $HISHER eyes burn with otherworldly zeal.",
        "absorbing", "Absorbing",
        {"cuts", "slashes", "lacerates", {0}}},
    {"This legendary bird shines with a brilliant light, and $HISHER wings crackle and pop like embers as they beat the air. When $HESHE dies, legend has it that an egg will form and a newborn phoenix will rise from its ashes.",
        "cremating", "Cremating",
        {"pecks", "scratches", "claws", {0}}},
    {"Cradled in a nest of cooling ashes, the translucent membrane of the phoenix egg reveals a yolk that glows brighter by the second.",
        "cremating", "Cremating",
        {"touches", {0}},
        {0},
        "bursts as a newborn phoenix rises from the ashes!"},
    {"This mangrove dryad is as old as the earth, and $HISHER gnarled figure houses an ancient power. When angered, $HESHE can call upon the forces of nature to bind $HISHER foes and tear them to shreds.",
        "absorbing", "Absorbing",
        {"whips", "lashes", "thrashes", "lacerates", {0}}},
};

const mutation mutationCatalog[NUMBER_MUTATORS] = {
    //Title         textColor       healthFactor    moveSpdMult attackSpdMult   defMult damMult DF% DFtype  light   monstFlags  abilityFlags    forbiddenFlags      forbiddenAbilities      canBeNegated
    {"explosive",   &orange,        50,             100,        100,            50,     100,    0,  DF_MUTATION_EXPLOSION, EXPLOSIVE_BLOAT_LIGHT, 0, MA_DF_ON_DEATH, MONST_SUBMERGES, 0,
        "A rare mutation will cause $HIMHER to explode violently when $HESHE dies.",    true},
    {"infested",    &lichenColor,   50,             100,        100,            50,     100,    0,  DF_MUTATION_LICHEN, 0, 0,   MA_DF_ON_DEATH, 0,               0,
        "$HESHE has been infested by deadly lichen spores; poisonous fungus will spread from $HISHER corpse when $HESHE dies.", true},
    {"agile",       &lightBlue,     100,            50,         100,            150,    100,    -1, 0,      0,      MONST_FLEES_NEAR_DEATH, 0, MONST_FLEES_NEAR_DEATH, 0,
        "A rare mutation greatly enhances $HISHER mobility.",   false},
    {"juggernaut",  &brown,         300,            200,        200,            75,     200,    -1, 0,      0,      0,          MA_ATTACKS_STAGGER, MONST_MAINTAINS_DISTANCE, 0,
        "A rare mutation has hardened $HISHER flesh, increasing $HISHER health and power but compromising $HISHER speed.",  false},
    {"grappling",   &tanColor,      150,            100,        100,            50,     100,    -1, 0,      0,      0,          MA_SEIZES,      MONST_MAINTAINS_DISTANCE, MA_SEIZES,
        "A rare mutation has caused suckered tentacles to sprout from $HISHER frame, increasing $HISHER health and allowing $HIMHER to grapple with $HISHER prey.", true},
    {"vampiric",    &red,           100,            100,        100,            100,    100,    -1, 0,      0,      0,          MA_TRANSFERENCE, MONST_MAINTAINS_DISTANCE, MA_TRANSFERENCE,
        "A rare mutation allows $HIMHER to heal $HIMSELFHERSELF with every attack.",    true},
    {"toxic",       &green,         100,            100,        200,            100,    20,     -1, 0,      0,      0,          (MA_CAUSES_WEAKNESS | MA_POISONS), MONST_MAINTAINS_DISTANCE, (MA_CAUSES_WEAKNESS | MA_POISONS),
        "A rare mutation causes $HIMHER to poison $HISHER victims and sap their strength with every attack.",   true},
    {"reflective",  &darkTurquoise, 100,            100,        100,            100,    100,    -1, 0,      0,      MONST_REFLECT_4, 0,         (MONST_REFLECT_4 | MONST_ALWAYS_USE_ABILITY), 0,
        "A rare mutation has coated $HISHER flesh with reflective scales.",     true},
};

const hordeType hordeCatalog[NUMBER_HORDES] = {
    // leader       #members    member list                             member numbers                  minL    maxL    freq    spawnsIn        machine         flags
    {MK_RAT,            0,      {0},                                    {{0}},                          1,      5,      150},
    {MK_KOBOLD,         0,      {0},                                    {{0}},                          1,      6,      150},
    {MK_JACKAL,         0,      {0},                                    {{0}},                          1,      3,      100},
    {MK_JACKAL,         1,      {MK_JACKAL},                            {{1, 3, 1}},                    3,      7,      50},
    {MK_EEL,            0,      {0},                                    {{0}},                          2,      17,     100,        DEEP_WATER},
    {MK_MONKEY,         0,      {0},                                    {{0}},                          2,      9,      50},
    {MK_BLOAT,          0,      {0},                                    {{0}},                          2,      13,     30},
    {MK_PIT_BLOAT,      0,      {0},                                    {{0}},                          2,      13,     10},
    {MK_BLOAT,          1,      {MK_BLOAT},                             {{0, 2, 1}},                    14,     26,     30},
    {MK_PIT_BLOAT,      1,      {MK_PIT_BLOAT},                         {{0, 2, 1}},                    14,     26,     10},
    {MK_EXPLOSIVE_BLOAT,0,      {0},                                    {{0}},                          10,     26,     10},
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          3,      10,     100},
    {MK_GOBLIN_CONJURER,0,      {0},                                    {{0}},                          3,      10,     60},
    {MK_TOAD,           0,      {0},                                    {{0}},                          4,      11,     100},
    {MK_PINK_JELLY,     0,      {0},                                    {{0}},                          4,      13,     100},
    {MK_GOBLIN_TOTEM,   1,      {MK_GOBLIN},                            {{2,4,1}},                      5,      13,     100,        0,              MT_CAMP_AREA,   HORDE_NO_PERIODIC_SPAWN},
    {MK_ARROW_TURRET,   0,      {0},                                    {{0}},                          5,      13,     100,        WALL,   0,                      HORDE_NO_PERIODIC_SPAWN},
    {MK_MONKEY,         1,      {MK_MONKEY},                            {{2,4,1}},                      5,      13,     20},
    {MK_VAMPIRE_BAT,    0,      {0},                                    {{0}},                          6,      13,     30},
    {MK_VAMPIRE_BAT,    1,      {MK_VAMPIRE_BAT},                       {{1,2,1}},                      6,      13,     70,      0,              0,              HORDE_NEVER_OOD},
    {MK_ACID_MOUND,     0,      {0},                                    {{0}},                          6,      13,     100},
    {MK_GOBLIN,         3,      {MK_GOBLIN, MK_GOBLIN_MYSTIC, MK_JACKAL},{{2, 3, 1}, {1,2,1}, {1,2,1}}, 6,      12,     40},
    {MK_GOBLIN_CONJURER,2,      {MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC}, {{0,1,1}, {1,1,1}},             7,      15,     40},
    {MK_CENTIPEDE,      0,      {0},                                    {{0}},                          7,      14,     100},
    {MK_BOG_MONSTER,    0,      {0},                                    {{0}},                          7,      14,     80,     MUD,            0,              HORDE_NEVER_OOD},
    {MK_OGRE,           0,      {0},                                    {{0}},                          7,      13,     100},
    {MK_EEL,            1,      {MK_EEL},                               {{2, 4, 1}},                    8,      22,     70,     DEEP_WATER},
    {MK_ACID_MOUND,     1,      {MK_ACID_MOUND},                        {{2, 4, 1}},                    9,      13,     30},
    {MK_SPIDER,         0,      {0},                                    {{0}},                          9,      16,     100},
    {MK_DAR_BLADEMASTER,1,      {MK_DAR_BLADEMASTER},                   {{0, 1, 1}},                    10,     14,     100},
    {MK_WILL_O_THE_WISP,0,      {0},                                    {{0}},                          10,     17,     100},
    {MK_WRAITH,         0,      {0},                                    {{0}},                          10,     17,     100},
    {MK_GOBLIN_TOTEM,   4,      {MK_GOBLIN_TOTEM, MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC, MK_GOBLIN}, {{1,2,1},{1,2,1},{1,2,1},{3,5,1}},10,17,80,0,MT_CAMP_AREA,  HORDE_NO_PERIODIC_SPAWN},
    {MK_SPARK_TURRET,   0,      {0},                                    {{0}},                          11,     18,     100,        WALL,   0,                      HORDE_NO_PERIODIC_SPAWN},
    {MK_ZOMBIE,         0,      {0},                                    {{0}},                          11,     18,     100},
    {MK_TROLL,          0,      {0},                                    {{0}},                          12,     19,     100},
    {MK_OGRE_TOTEM,     1,      {MK_OGRE},                              {{2,4,1}},                      12,     19,     60,     0,          0,                  HORDE_NO_PERIODIC_SPAWN},
    {MK_BOG_MONSTER,    1,      {MK_BOG_MONSTER},                       {{2,4,1}},                      12,     26,     100,        MUD},
    {MK_NAGA,           0,      {0},                                    {{0}},                          13,     20,     100,        DEEP_WATER},
    {MK_SALAMANDER,     0,      {0},                                    {{0}},                          13,     20,     100,        LAVA},
    {MK_OGRE_SHAMAN,    1,      {MK_OGRE},                              {{1, 3, 1}},                    14,     20,     100},
    {MK_CENTAUR,        1,      {MK_CENTAUR},                           {{1, 1, 1}},                    14,     21,     100},
    {MK_ACID_JELLY,     0,      {0},                                    {{0}},                          14,     21,     100},
    {MK_DART_TURRET,    0,      {0},                                    {{0}},                          15,     22,     100,        WALL,   0,                      HORDE_NO_PERIODIC_SPAWN},
    {MK_PIXIE,          0,      {0},                                    {{0}},                          14,     21,     80},
    {MK_FLAME_TURRET,   0,      {0},                                    {{0}},                          14,     24,     100,        WALL,   0,                      HORDE_NO_PERIODIC_SPAWN},
    {MK_DAR_BLADEMASTER,2,      {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS}, {{0, 1, 1}, {0, 1, 1}},         15,     17,     100},
    {MK_PINK_JELLY,     2,      {MK_PINK_JELLY, MK_DAR_PRIESTESS},      {{0, 1, 1}, {1, 2, 1}},         17,     23,     70},
    {MK_KRAKEN,         0,      {0},                                    {{0}},                          15,     30,     100,        DEEP_WATER},
    {MK_PHANTOM,        0,      {0},                                    {{0}},                          16,     23,     100},
    {MK_WRAITH,         1,      {MK_WRAITH},                            {{1, 4, 1}},                    16,     23,     80},
    {MK_IMP,            0,      {0},                                    {{0}},                          17,     24,     100},
    {MK_DAR_BLADEMASTER,3,      {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE},{{1,2,1},{1,1,1},{1,1,1}},18,25,100},
    {MK_FURY,           1,      {MK_FURY},                              {{2, 4, 1}},                    18,     26,     80},
    {MK_REVENANT,       0,      {0},                                    {{0}},                          19,     27,     100},
    {MK_GOLEM,          0,      {0},                                    {{0}},                          21,     30,     100},
    {MK_TENTACLE_HORROR,0,      {0},                                    {{0}},                          22,     DEEPEST_LEVEL-1,        100},
    {MK_PHYLACTERY,     0,      {0},                                    {{0}},                          22,     DEEPEST_LEVEL-1,        100},
    {MK_DRAGON,         0,      {0},                                    {{0}},                          24,     DEEPEST_LEVEL-1,        70},
    {MK_DRAGON,         1,      {MK_DRAGON},                            {{1,1,1}},                      27,     DEEPEST_LEVEL-1,        30},
    {MK_GOLEM,          3,      {MK_GOLEM, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE}, {{1, 2, 1}, {0,1,1},{0,1,1}},27,DEEPEST_LEVEL-1,   80},
    {MK_GOLEM,          1,      {MK_GOLEM},                             {{5, 10, 2}},                   30,     DEEPEST_LEVEL-1,    20},
    {MK_KRAKEN,         1,      {MK_KRAKEN},                            {{5, 10, 2}},                   30,     DEEPEST_LEVEL-1,    100,        DEEP_WATER},
    {MK_TENTACLE_HORROR,2,      {MK_TENTACLE_HORROR, MK_REVENANT},      {{1, 3, 1}, {2, 4, 1}},         32,     DEEPEST_LEVEL-1,    20},
    {MK_DRAGON,         1,      {MK_DRAGON},                            {{3, 5, 1}},                    34,     DEEPEST_LEVEL-1,    20},

    // summons
    {MK_GOBLIN_CONJURER,1,      {MK_SPECTRAL_BLADE},                    {{3, 5, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED | HORDE_DIES_ON_LEADER_DEATH},
    {MK_OGRE_SHAMAN,    1,      {MK_OGRE},                              {{1, 1, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_VAMPIRE,        1,      {MK_VAMPIRE_BAT},                       {{3, 3, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_LICH,           1,      {MK_PHANTOM},                           {{2, 3, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_LICH,           1,      {MK_FURY},                              {{2, 3, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_PHYLACTERY,     1,      {MK_LICH},                              {{1,1,1}},                      0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_GOBLIN_CHIEFTAN,2,      {MK_GOBLIN_CONJURER, MK_GOBLIN},        {{1,1,1}, {3,4,1}},             0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED | HORDE_SUMMONED_AT_DISTANCE},
    {MK_PHOENIX_EGG,    1,      {MK_PHOENIX},                           {{1,1,1}},                      0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_ELDRITCH_TOTEM, 1,      {MK_SPECTRAL_BLADE},                    {{4, 7, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED | HORDE_DIES_ON_LEADER_DEATH},
    {MK_ELDRITCH_TOTEM, 1,      {MK_FURY},                              {{2, 3, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED | HORDE_DIES_ON_LEADER_DEATH},

    // captives
    {MK_MONKEY,         1,      {MK_KOBOLD},                            {{1, 2, 1}},                    1,      5,      10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_GOBLIN,         1,      {MK_GOBLIN},                            {{1, 2, 1}},                    3,      7,      10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_OGRE,           1,      {MK_GOBLIN},                            {{3, 5, 1}},                    4,      10,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_GOBLIN_MYSTIC,  1,      {MK_KOBOLD},                            {{3, 7, 1}},                    5,      11,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_OGRE,           1,      {MK_OGRE},                              {{1, 2, 1}},                    8,      15,     20,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_TROLL,          1,      {MK_TROLL},                             {{1, 2, 1}},                    14,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_CENTAUR,        1,      {MK_TROLL},                             {{1, 2, 1}},                    12,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_TROLL,          2,      {MK_OGRE, MK_OGRE_SHAMAN},              {{2, 3, 1}, {0, 1, 1}},         17,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_BLADEMASTER,1,      {MK_TROLL},                             {{1, 2, 1}},                    12,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_NAGA,           1,      {MK_SALAMANDER},                        {{1, 2, 1}},                    14,     20,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_SALAMANDER,     1,      {MK_NAGA},                              {{1, 2, 1}},                    13,     20,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_TROLL,          1,      {MK_SALAMANDER},                        {{1, 2, 1}},                    13,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_IMP,            1,      {MK_FURY},                              {{2, 4, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_PIXIE,          1,      {MK_IMP, MK_PHANTOM},                   {{1, 2, 1}, {1, 2, 1}},         14,     21,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_BLADEMASTER,1,      {MK_FURY},                              {{2, 4, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_BLADEMASTER,1,      {MK_IMP},                               {{2, 3, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_PRIESTESS,  1,      {MK_FURY},                              {{2, 4, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_BATTLEMAGE, 1,      {MK_IMP},                               {{2, 3, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_TENTACLE_HORROR,3,      {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE},{{1,2,1},{1,1,1},{1,1,1}},20,26,10,   0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_GOLEM,          3,      {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE},{{1,2,1},{1,1,1},{1,1,1}},18,25,10,   0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},

    // bosses
    {MK_GOBLIN_CHIEFTAN,2,      {MK_GOBLIN_MYSTIC, MK_GOBLIN, MK_GOBLIN_TOTEM}, {{1,1,1}, {2,3,1}, {2,2,1}},2,  10,     50,     0,          0,                  HORDE_MACHINE_BOSS},
    {MK_BLACK_JELLY,    0,      {0},                                    {{0}},                          5,      15,     50,     0,          0,                  HORDE_MACHINE_BOSS},
    {MK_VAMPIRE,        0,      {0},                                    {{0}},                          10,     DEEPEST_LEVEL,  50,  0,     0,                  HORDE_MACHINE_BOSS},
    {MK_FLAMEDANCER,    0,      {0},                                    {{0}},                          10,     DEEPEST_LEVEL,  50,  0,     0,                  HORDE_MACHINE_BOSS},

    // machine water monsters
    {MK_EEL,            0,      {0},                                    {{0}},                          2,      7,      100,        DEEP_WATER, 0,                  HORDE_MACHINE_WATER_MONSTER},
    {MK_EEL,            1,      {MK_EEL},                               {{2, 4, 1}},                    5,      15,     100,        DEEP_WATER, 0,                  HORDE_MACHINE_WATER_MONSTER},
    {MK_KRAKEN,         0,      {0},                                    {{0}},                          12,     DEEPEST_LEVEL,  100,    DEEP_WATER, 0,              HORDE_MACHINE_WATER_MONSTER},
    {MK_KRAKEN,         1,      {MK_EEL},                               {{1, 2, 1}},                    12,     DEEPEST_LEVEL,  80, DEEP_WATER, 0,              HORDE_MACHINE_WATER_MONSTER},

    // dungeon captives -- no captors
    {MK_OGRE,           0,      {0},                                    {{0}},                          4,      13,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_NAGA,           0,      {0},                                    {{0}},                          12,     20,     50,     0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_MYSTIC,  0,      {0},                                    {{0}},                          2,      8,      100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_TROLL,          0,      {0},                                    {{0}},                          10,     20,     50,     0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BLADEMASTER,0,      {0},                                    {{0}},                          8,      16,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_DAR_PRIESTESS,  0,      {0},                                    {{0}},                          8,      14,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_WRAITH,         0,      {0},                                    {{0}},                          11,     20,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_GOLEM,          0,      {0},                                    {{0}},                          17,     23,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_TENTACLE_HORROR,0,      {0},                                    {{0}},                          20,     AMULET_LEVEL,100,0,         0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_DRAGON,         0,      {0},                                    {{0}},                          23,     AMULET_LEVEL,100,0,         0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},

    // machine statue monsters
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          1,      6,      100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_OGRE,           0,      {0},                                    {{0}},                          6,      12,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_WRAITH,         0,      {0},                                    {{0}},                          10,     17,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_NAGA,           0,      {0},                                    {{0}},                          12,     19,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_TROLL,          0,      {0},                                    {{0}},                          14,     21,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_GOLEM,          0,      {0},                                    {{0}},                          21,     30,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_DRAGON,         0,      {0},                                    {{0}},                          29,     DEEPEST_LEVEL,  100,    STATUE_DORMANT, 0,          HORDE_MACHINE_STATUE},
    {MK_TENTACLE_HORROR,0,      {0},                                    {{0}},                          29,     DEEPEST_LEVEL,  100,    STATUE_DORMANT, 0,          HORDE_MACHINE_STATUE},

    // machine turrets
    {MK_ARROW_TURRET,   0,      {0},                                    {{0}},                          5,      13,     100,        TURRET_DORMANT, 0,              HORDE_MACHINE_TURRET},
    {MK_SPARK_TURRET,   0,      {0},                                    {{0}},                          11,     18,     100,        TURRET_DORMANT, 0,              HORDE_MACHINE_TURRET},
    {MK_DART_TURRET,    0,      {0},                                    {{0}},                          15,     22,     100,        TURRET_DORMANT, 0,              HORDE_MACHINE_TURRET},
    {MK_FLAME_TURRET,   0,      {0},                                    {{0}},                          17,     24,     100,        TURRET_DORMANT, 0,              HORDE_MACHINE_TURRET},

    // machine mud monsters
    {MK_BOG_MONSTER,    0,      {0},                                    {{0}},                          12,     26,     100,        MACHINE_MUD_DORMANT, 0,         HORDE_MACHINE_MUD},
    {MK_KRAKEN,         0,      {0},                                    {{0}},                          17,     26,     30,     MACHINE_MUD_DORMANT, 0,         HORDE_MACHINE_MUD},

    // kennel monsters
    {MK_MONKEY,         0,      {0},                                    {{0}},                          1,      5,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          1,      8,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_CONJURER,0,      {0},                                    {{0}},                          2,      9,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_MYSTIC,  0,      {0},                                    {{0}},                          2,      9,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_OGRE,           0,      {0},                                    {{0}},                          7,      17,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_TROLL,          0,      {0},                                    {{0}},                          12,     21,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_NAGA,           0,      {0},                                    {{0}},                          13,     23,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_SALAMANDER,     0,      {0},                                    {{0}},                          9,      20,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_IMP,            0,      {0},                                    {{0}},                          15,     26,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_PIXIE,          0,      {0},                                    {{0}},                          11,     21,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BLADEMASTER,0,      {0},                                    {{0}},                          9,      AMULET_LEVEL, 100, MONSTER_CAGE_CLOSED, 0,      HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_DAR_PRIESTESS,  0,      {0},                                    {{0}},                          12,     AMULET_LEVEL, 100, MONSTER_CAGE_CLOSED, 0,      HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BATTLEMAGE, 0,      {0},                                    {{0}},                          13,     AMULET_LEVEL, 100, MONSTER_CAGE_CLOSED, 0,      HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},

    // vampire bloodbags
    {MK_MONKEY,         0,      {0},                                    {{0}},                          1,      5,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          1,      8,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_CONJURER,0,      {0},                                    {{0}},                          2,      9,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_MYSTIC,  0,      {0},                                    {{0}},                          2,      9,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_OGRE,           0,      {0},                                    {{0}},                          5,      15,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_TROLL,          0,      {0},                                    {{0}},                          10,     19,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_NAGA,           0,      {0},                                    {{0}},                          9,      20,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_IMP,            0,      {0},                                    {{0}},                          15,     AMULET_LEVEL,100,MONSTER_CAGE_CLOSED, 0,            HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_PIXIE,          0,      {0},                                    {{0}},                          11,     21,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BLADEMASTER,0,      {0},                                    {{0}},                          9,      AMULET_LEVEL,100,MONSTER_CAGE_CLOSED, 0,            HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_DAR_PRIESTESS,  0,      {0},                                    {{0}},                          12,     AMULET_LEVEL,100,MONSTER_CAGE_CLOSED, 0,            HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BATTLEMAGE, 0,      {0},                                    {{0}},                          13,     AMULET_LEVEL,100,MONSTER_CAGE_CLOSED, 0,            HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},

    // key thieves
    {MK_MONKEY,         0,      {0},                                    {{0}},                          1,      14,     100,     0,          0,                  HORDE_MACHINE_THIEF},
    {MK_IMP,            0,      {0},                                    {{0}},                          15,     DEEPEST_LEVEL,  100, 0,      0,                  HORDE_MACHINE_THIEF},

    // sacrifice victims
    {MK_MONKEY,         0,      {0},                                    {{0}},                          1,      5,      100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          3,      10,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_OGRE,           0,      {0},                                    {{0}},                          7,      13,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_TROLL,          0,      {0},                                    {{0}},                          12,     19,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_WRAITH,         0,      {0},                                    {{0}},                          10,     17,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_NAGA,           0,      {0},                                    {{0}},                          13,     20,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_DAR_BLADEMASTER,0,      {0},                                    {{0}},                          10,     20,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_GOLEM,          0,      {0},                                    {{0}},                          22,     DEEPEST_LEVEL,100,  STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_REVENANT,       0,      {0},                                    {{0}},                          22,     DEEPEST_LEVEL,100,  STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_TENTACLE_HORROR,0,      {0},                                    {{0}},                          22,     DEEPEST_LEVEL,100,  STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},

    // legendary allies
    {MK_UNICORN,        0,      {0},                                    {{0}},                          1,      DEEPEST_LEVEL,  100, 0,     0,                  HORDE_MACHINE_LEGENDARY_ALLY | HORDE_ALLIED_WITH_PLAYER},
    {MK_IFRIT,          0,      {0},                                    {{0}},                          1,      DEEPEST_LEVEL,  100,    0,      0,                  HORDE_MACHINE_LEGENDARY_ALLY | HORDE_ALLIED_WITH_PLAYER},
    {MK_PHOENIX_EGG,    0,      {0},                                    {{0}},                          1,      DEEPEST_LEVEL,  100,    0,      0,                  HORDE_MACHINE_LEGENDARY_ALLY | HORDE_ALLIED_WITH_PLAYER},
    {MK_ANCIENT_SPIRIT, 0,      {0},                                    {{0}},                          1,      DEEPEST_LEVEL,  100,    0,      0,                  HORDE_MACHINE_LEGENDARY_ALLY | HORDE_ALLIED_WITH_PLAYER},

    // goblin warren
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          1,      10,     100,     0,              0,              HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN_CONJURER,0,      {0},                                    {{0}},                          1,      10,     60,      0,              0,              HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN_TOTEM,   1,      {MK_GOBLIN},                            {{2,4,1}},                      5,      13,     100,        0,              MT_CAMP_AREA,   HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN,         3,      {MK_GOBLIN, MK_GOBLIN_MYSTIC, MK_JACKAL},{{2, 3, 1}, {1,2,1}, {1,2,1}}, 6,      12,     40,      0,              0,              HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN_CONJURER,2,      {MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC}, {{0,1,1}, {1,1,1}},             7,      15,     40,      0,              0,              HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN_TOTEM,   4,      {MK_GOBLIN_TOTEM, MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC, MK_GOBLIN}, {{1,2,1},{1,2,1},{1,2,1},{3,5,1}},10,17,80,0,MT_CAMP_AREA,  HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN,         1,      {MK_GOBLIN},                            {{1, 2, 1}},                    3,      7,      10,     0,              0,              HORDE_MACHINE_GOBLIN_WARREN | HORDE_LEADER_CAPTIVE},
};

const monsterClass monsterClassCatalog[MONSTER_CLASS_COUNT] = {
    // name             frequency   maxDepth    member list
    {"abomination",     10,         -1,         {MK_BOG_MONSTER, MK_UNDERWORM, MK_KRAKEN, MK_TENTACLE_HORROR}},
    {"dar",             10,         22,         {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE}},
    {"animal",          10,         10,         {MK_RAT, MK_MONKEY, MK_JACKAL, MK_EEL, MK_TOAD, MK_VAMPIRE_BAT, MK_CENTIPEDE, MK_SPIDER}},
    {"goblin",          10,         10,         {MK_GOBLIN, MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC, MK_GOBLIN_TOTEM, MK_GOBLIN_CHIEFTAN, MK_SPECTRAL_BLADE}},
    {"ogre",            10,         16,         {MK_OGRE, MK_OGRE_SHAMAN, MK_OGRE_TOTEM}},
    {"dragon",          10,         -1,         {MK_DRAGON}},
    {"undead",          10,         -1,         {MK_ZOMBIE, MK_WRAITH, MK_VAMPIRE, MK_PHANTOM, MK_LICH, MK_REVENANT}},
    {"jelly",           10,         15,         {MK_PINK_JELLY, MK_BLACK_JELLY, MK_ACID_JELLY}},
    {"turret",          5,          18,         {MK_ARROW_TURRET, MK_SPARK_TURRET, MK_DART_TURRET, MK_FLAME_TURRET}},
    {"infernal",        10,         -1,         {MK_FLAMEDANCER, MK_IMP, MK_REVENANT, MK_FURY, MK_PHANTOM, MK_IFRIT}},
    {"mage",            10,         -1,         {MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC, MK_OGRE_SHAMAN, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE, MK_PIXIE, MK_LICH}},
    {"waterborne",      10,         17,         {MK_EEL, MK_NAGA, MK_KRAKEN}},
    {"airborne",        10,         15,         {MK_VAMPIRE_BAT, MK_WILL_O_THE_WISP, MK_PIXIE, MK_PHANTOM, MK_FURY, MK_IFRIT, MK_PHOENIX}},
    {"fireborne",       10,         12,         {MK_WILL_O_THE_WISP, MK_SALAMANDER, MK_FLAMEDANCER, MK_PHOENIX}},
    {"troll",           10,         15,         {MK_TROLL}},
};

// ITEMS

char itemTitles[NUMBER_SCROLL_KINDS][30];

const char itemCategoryNames[NUMBER_ITEM_CATEGORIES][7] = {
        "food",
        "weapon",
        "armor",
        "potion",
        "scroll",
        "staff",
        "wand",
        "ring",
        "charm",
        "gold",
        "amulet",
        "gem",
        "key"
};

const char titlePhonemes[NUMBER_TITLE_PHONEMES][30] = {
    "glorp",
    "snarg",
    "gana",
    "flin",
    "herba",
    "pora",
    "nuglo",
    "greep",
    "nur",
    "lofa",
    "poder",
    "nidge",
    "pus",
    "wooz",
    "flem",
    "bloto",
    "porta",
    "ermah",
    "gerd",
    "nurt",
    "flurx",
};

char itemColors[NUMBER_ITEM_COLORS][30];

const char itemColorsRef[NUMBER_ITEM_COLORS][30] = {
    "crimson",
    "scarlet",
    "orange",
    "yellow",
    "green",
    "blue",
    "indigo",
    "violet",
    "puce",
    "mauve",
    "burgundy",
    "turquoise",
    "aquamarine",
    "gray",
    "pink",
    "white",
    "lavender",
    "tan",
    "brown",
    "cyan",
    "black"
};

char itemWoods[NUMBER_ITEM_WOODS][30];

const char itemWoodsRef[NUMBER_ITEM_WOODS][30] = {
    "teak",
    "oak",
    "redwood",
    "rowan",
    "willow",
    "mahogany",
    "pinewood",
    "maple",
    "bamboo",
    "ironwood",
    "pearwood",
    "birch",
    "cherry",
    "eucalyptus",
    "walnut",
    "cedar",
    "rosewood",
    "yew",
    "sandalwood",
    "hickory",
    "hemlock",
};

char itemMetals[NUMBER_ITEM_METALS][30];

const char itemMetalsRef[NUMBER_ITEM_METALS][30] = {
    "bronze",
    "steel",
    "brass",
    "pewter",
    "nickel",
    "copper",
    "aluminum",
    "tungsten",
    "titanium",
    "cobalt",
    "chromium",
    "silver",
};

char itemGems[NUMBER_ITEM_GEMS][30];

const char itemGemsRef[NUMBER_ITEM_GEMS][30] = {
    "diamond",
    "opal",
    "garnet",
    "ruby",
    "amethyst",
    "topaz",
    "onyx",
    "tourmaline",
    "sapphire",
    "obsidian",
    "malachite",
    "aquamarine",
    "emerald",
    "jade",
    "alexandrite",
    "agate",
    "bloodstone",
    "jasper"
};

//typedef struct itemTable {
//  char *name;
//  char *flavor;
//  short frequency;
//  short marketValue;
//  short number;
//  randomRange range;
//} itemTable;

const itemTable keyTable[NUMBER_KEY_TYPES] = {
    {"door key",            "", "", 1, 0,   0, {0,0,0}, true, false, "The notches on this ancient iron key are well worn; its leather lanyard is battered by age. What door might it open?"},
    {"cage key",            "", "", 1, 0,   0, {0,0,0}, true, false, "The rust accreted on this iron key has been stained with flecks of blood; it must have been used recently. What cage might it open?"},
    {"crystal orb",         "", "", 1, 0,   0, {0,0,0}, true, false, "A faceted orb, seemingly cut from a single crystal, sparkling and perpetually warm to the touch. What manner of device might such an object activate?"},
};

const itemTable foodTable[NUMBER_FOOD_KINDS] = {
    {"ration of food",      "", "", 3, 25,  1800, {0,0,0}, true, false, "A ration of food. Was it left by former adventurers? Is it a curious byproduct of the subterranean ecosystem?"},
    {"mango",               "", "", 1, 15,  1550, {0,0,0}, true, false, "An odd fruit to be found so deep beneath the surface of the earth, but only slightly less filling than a ration of food."}
};

const itemTable weaponTable[NUMBER_WEAPON_KINDS] = {
    {"dagger",              "", "", 10, 190,        12, {3, 4,  1},     true, false, "A simple iron dagger with a well-worn wooden handle. Daggers will deal quintuple damage upon a successful sneak attack instead of triple damage."},
    {"sword",               "", "", 10, 440,        14, {7, 9,  1},     true, false, "The razor-sharp length of steel blade shines reassuringly."},
    {"broadsword",          "", "", 10, 990,        19, {14, 22, 1},    true, false, "This towering blade inflicts heavy damage by investing its heft into every cut."},

    {"whip",                "", "", 10, 440,        14, {3, 5,  1},     true, false, "The lash from this coil of braided leather can tear bark from trees, and it will reach opponents up to five spaces away."},
    {"rapier",              "", "", 10, 440,        15, {3, 5,  1},     true, false, "This blade is thin and flexible, designed for deft and rapid maneuvers. It inflicts less damage than comparable weapons, but permits you to attack twice as quickly. If there is one space between you and an enemy and you step directly toward it, you will perform a devastating lunge attack, which deals treble damage and never misses."},
    {"flail",               "", "", 10, 440,        17, {10,13, 1},     true, false, "This spiked iron ball can be whirled at the end of its chain in synchronicity with your movement, allowing you a free attack whenever moving between two spaces that are adjacent to an enemy."},

    {"mace",                "", "", 10, 660,        16, {16, 20, 1},    true, false, "The iron flanges at the head of this weapon inflict substantial damage with every weighty blow. Because of its heft, it takes an extra turn to recover when it hits, and will push your opponent backward if there is room."},
    {"war hammer",          "", "", 10, 1100,       20, {25, 35, 1},    true, false, "Few creatures can withstand the crushing blow of this towering mass of lead and steel, but only the strongest of adventurers can effectively wield it. Because of its heft, it takes an extra turn to recover when it hits, and will push your opponent backward if there is room."},

    {"spear",               "", "", 10, 330,        13, {4, 5, 1},      true, false, "A slender wooden rod tipped with sharpened iron. The reach of the spear permits you to simultaneously attack an adjacent enemy and the enemy directly behind it."},
    {"war pike",            "", "", 10, 880,        18, {11, 15, 1},    true, false, "A long steel pole ending in a razor-sharp point. The reach of the pike permits you to simultaneously attack an adjacent enemy and the enemy directly behind it."},

    {"axe",                 "", "", 10, 550,        15, {7, 9, 1},      true, false, "The blunt iron edge on this axe glints in the darkness. The arc of its swing permits you to attack all adjacent enemies simultaneously."},
    {"war axe",             "", "", 10, 990,        19, {12, 17, 1},    true, false, "The enormous steel head of this war axe puts considerable heft behind each stroke. The arc of its swing permits you to attack all adjacent enemies simultaneously."},

    {"dart",                "", "", 0,  15,         10, {2, 4,  1},     true, false, "These simple metal spikes are weighted to fly true and sting their prey with a flick of the wrist."},
    {"incendiary dart",     "", "", 10, 25,         12, {1, 2,  1},     true, false, "The barbed spike on each of these darts is designed to stick to its target while the compounds strapped to its length explode into flame."},
    {"javelin",             "", "", 10, 40,         15, {3, 11, 3},     true, false, "This length of metal is weighted to keep the spike at its tip foremost as it sails through the air."},
};

const itemTable armorTable[NUMBER_ARMOR_KINDS] = {
    {"leather armor",   "", "", 10, 250,        10, {30,30,0},      true, false, "This lightweight armor offers basic protection."},
    {"scale mail",      "", "", 10, 350,        12, {40,40,0},      true, false, "Bronze scales cover the surface of treated leather, offering greater protection than plain leather with minimal additional weight."},
    {"chain mail",      "", "", 10, 500,        13, {50,50,0},      true, false, "Interlocking metal links make for a tough but flexible suit of armor."},
    {"banded mail",     "", "", 10, 800,        15, {70,70,0},      true, false, "Overlapping strips of metal horizontally encircle a chain mail base, offering an additional layer of protection at the cost of greater weight."},
    {"splint mail",     "", "", 10, 1000,       17, {90,90,0},      true, false, "Thick plates of metal are embedded into a chain mail base, providing the wearer with substantial protection."},
    {"plate armor",     "", "", 10, 1300,       19, {110,110,0},    true, false, "Enormous plates of metal are joined together into a suit that provides unmatched protection to any adventurer strong enough to bear its staggering weight."}
};

const char weaponRunicNames[NUMBER_WEAPON_RUNIC_KINDS][30] = {
    "speed",
    "quietus",
    "paralysis",
    "multiplicity",
    "slowing",
    "confusion",
    "force",
    "slaying",
    "mercy",
    "plenty"
};

const char armorRunicNames[NUMBER_ARMOR_ENCHANT_KINDS][30] = {
    "multiplicity",
    "mutuality",
    "absorption",
    "reprisal",
    "immunity",
    "reflection",
    "respiration",
    "dampening",
    "burden",
    "vulnerability",
    "immolation",
};

itemTable scrollTable[NUMBER_SCROLL_KINDS] = {
    {"enchanting",          itemTitles[0], "",  0,  550,    0,{0,0,0}, false, false, "This ancient enchanting sorcery will imbue a single item with a powerful and permanent magical charge. A staff will increase in power and in number of charges; a weapon will inflict more damage and find its mark more easily; a suit of armor will deflect attacks more often; the magic of a ring will intensify; and a wand will gain expendable charges in the least amount that such a wand can be found with. Weapons and armor will also require less strength to use, and any curses on the item will be lifted."}, // frequency is dynamically adjusted
    {"identify",            itemTitles[1], "",  30, 300,    0,{0,0,0}, false, false, "This scrying magic will permanently reveal all of the secrets of a single item."},
    {"teleportation",       itemTitles[2], "",  10, 500,    0,{0,0,0}, false, false, "This escape spell will instantly relocate you to a random location on the dungeon level. It can be used to escape a dangerous situation with luck. The unlucky reader might find himself in an even more dangerous place."},
    {"remove curse",        itemTitles[3], "",  15, 150,    0,{0,0,0}, false, false, "This redemption spell will instantly strip from the reader's weapon, armor, rings and carried items any evil enchantments that might prevent the wearer from removing them."},
    {"recharging",          itemTitles[4], "",  12, 375,    0,{0,0,0}, false, false, "The power bound up in this parchment will instantly recharge all of your staffs and charms."},
    {"protect armor",       itemTitles[5], "",  10, 400,    0,{0,0,0}, false, false, "This ceremonial shielding magic will permanently proof your armor against degradation by acid."},
    {"protect weapon",      itemTitles[6], "",  10, 400,    0,{0,0,0}, false, false, "This ceremonial shielding magic will permanently proof your weapon against degradation by acid."},
    {"sanctuary",           itemTitles[7], "",  10, 500,    0,{0,0,0}, false, false, "This protection rite will imbue the area with powerful warding glyphs, when released over plain ground. Monsters will not willingly set foot on the affected area."},
    {"magic mapping",       itemTitles[8], "",  12, 500,    0,{0,0,0}, false, false, "This powerful scouting magic will etch a purple-hued image of crystal clarity into your memory, alerting you to the precise layout of the level and revealing all hidden secrets."},
    {"negation",            itemTitles[9], "",  8,  400,    0,{0,0,0}, false, false, "When this powerful anti-magic is released, all creatures (including yourself) and all items lying on the ground within your field of view will be exposed to its blast and stripped of magic. Creatures animated purely by magic will die. Potions, scrolls, items being held by other creatures and items in your inventory will not be affected."},
    {"shattering",          itemTitles[10],"",  8,  500,    0,{0,0,0}, false, false, "This strange incantation will alter the physical structure of nearby stone, causing it to evaporate into the air over the ensuing minutes."},
    {"discord",             itemTitles[11], "", 8,  400,    0,{0,0,0}, false, false, "This scroll will unleash a powerful blast of mind magic. Any creatures within line of sight will turn against their companions and attack indiscriminately for 30 turns."},
    {"aggravate monsters",  itemTitles[12], "", 15, 50,     0,{0,0,0}, false, false, "This scroll will unleash a piercing shriek that will awaken all monsters and alert them to the reader's location."},
    {"summon monsters",     itemTitles[13], "", 10, 50,     0,{0,0,0}, false, false, "This summoning incantation will call out to creatures in other planes of existence, drawing them through the fabric of reality to confront the reader."},
};

itemTable potionTable[NUMBER_POTION_KINDS] = {
    {"life",                itemColors[1], "",  0,  500,    0,{0,0,0}, false, false, "A swirling elixir that will instantly heal you, cure you of ailments, and permanently increase your maximum health."}, // frequency is dynamically adjusted
    {"strength",            itemColors[2], "",  0,  400,    0,{0,0,0}, false, false, "This powerful medicine will course through your muscles, permanently increasing your strength by one point."}, // frequency is dynamically adjusted
    {"telepathy",           itemColors[3], "",  20, 350,    0,{0,0,0}, false, false, "This mysterious liquid will attune your mind to the psychic signature of distant creatures. Its effects will not reveal inanimate objects, such as totems, turrets and traps."},
    {"levitation",          itemColors[4], "",  15, 250,    0,{0,0,0}, false, false, "This curious liquid will cause you to hover in the air, able to drift effortlessly over lava, water, chasms and traps. Flames, gases and spiderwebs fill the air, and cannot be bypassed while airborne. Creatures that dwell in water or mud will be unable to attack you while you levitate."},
    {"detect magic",        itemColors[5], "",  20, 500,    0,{0,0,0}, false, false, "This mysterious brew will sensitize your mind to the radiance of magic. Items imbued with helpful enchantments will be marked with a full sigil; items corrupted by curses or designed to bring misfortune upon the bearer will be marked with a hollow sigil. The Amulet of Yendor will be revealed by its unique aura."},
    {"speed",               itemColors[6], "",  10, 500,    0,{0,0,0}, false, false, "Quaffing the contents of this flask will enable you to move at blinding speed for several minutes."},
    {"fire immunity",       itemColors[7], "",  15, 500,    0,{0,0,0}, false, false, "This potion will render you impervious to heat and permit you to wander through fire and lava and ignore otherwise deadly bolts of flame. It will not guard against the concussive impact of an explosion, however."},
    {"invisibility",        itemColors[8], "",  15, 400,    0,{0,0,0}, false, false, "Drinking this potion will render you temporarily invisible. Enemies more than two spaces away will be unable to track you."},
    {"caustic gas",         itemColors[9], "",  15, 200,    0,{0,0,0}, false, false, "Uncorking or shattering this pressurized glass will cause its contents to explode into a deadly cloud of caustic purple gas. You might choose to fling this potion at distant enemies instead of uncorking it by hand."},
    {"paralysis",           itemColors[10], "", 10, 250,    0,{0,0,0}, false, false, "Upon exposure to open air, the liquid in this flask will vaporize into a numbing pink haze. Anyone who inhales the cloud will be paralyzed instantly, unable to move for some time after the cloud dissipates. This item can be thrown at distant enemies to catch them within the effect of the gas."},
    {"hallucination",       itemColors[11], "", 10, 500,    0,{0,0,0}, false, false, "This flask contains a vicious and long-lasting hallucinogen. Under its dazzling effect, you will wander through a rainbow wonderland, unable to discern the form of any creatures or items you see."},
    {"confusion",           itemColors[12], "", 15, 450,    0,{0,0,0}, false, false, "This unstable chemical will quickly vaporize into a glittering cloud upon contact with open air, causing any creature that inhales it to lose control of the direction of its movements until the effect wears off (although its ability to aim projectile attacks will not be affected). Its vertiginous intoxication can cause creatures and adventurers to careen into one another or into chasms or lava pits, so extreme care should be taken when under its effect. Its contents can be weaponized by throwing the flask at distant enemies."},
    {"incineration",        itemColors[13], "", 15, 500,    0,{0,0,0}, false, false, "This flask contains an unstable compound which will burst violently into flame upon exposure to open air. You might throw the flask at distant enemies -- or into a deep lake, to cleanse the cavern with scalding steam."},
    {"darkness",            itemColors[14], "", 7,  150,    0,{0,0,0}, false, false, "Drinking this potion will plunge you into darkness. At first, you will be completely blind to anything not illuminated by an independent light source, but over time your vision will regain its former strength. Throwing the potion will create a cloud of supernatural darkness, and enemies will have difficulty seeing or following you if you take refuge under its cover."},
    {"descent",             itemColors[15], "", 15, 500,    0,{0,0,0}, false, false, "When this flask is uncorked by hand or shattered by being thrown, the fog that seeps out will temporarily cause the ground in the vicinity to vanish."},
    {"creeping death",      itemColors[16], "", 7,  450,    0,{0,0,0}, false, false, "When the cork is popped or the flask is thrown, tiny spores will spill across the ground and begin to grow a deadly lichen. Anything that touches the lichen will be poisoned by its clinging tendrils, and the lichen will slowly grow to fill the area. Fire will purge the infestation."},
};

itemTable wandTable[NUMBER_WAND_KINDS] = {
    {"teleportation",   itemMetals[0], "",  3,  800,    BOLT_TELEPORT,      {3,5,1}, false, false, "This wand will teleport a creature to a random place on the level. Aquatic or mud-bound creatures will be rendered helpless on dry land."},
    {"slowness",        itemMetals[1], "",  3,  800,    BOLT_SLOW,          {2,5,1}, false, false, "This wand will cause a creature to move at half its ordinary speed for 30 turns."},
    {"polymorphism",    itemMetals[2], "",  3,  700,    BOLT_POLYMORPH,     {3,5,1}, false, false, "This mischievous magic will transform a creature into another creature at random. Beware: the tamest of creatures might turn into the most fearsome. The horror of the transformation will turn an allied victim against you."},
    {"negation",        itemMetals[3], "",  3,  550,    BOLT_NEGATION,      {4,6,1}, false, false, "This powerful anti-magic will strip a creature of a host of magical traits, including flight, invisibility, acidic corrosiveness, telepathy, magical speed or slowness, hypnosis, magical fear, immunity to physical attack, fire resistance and the ability to blink. Spellcasters will lose their magical abilities and magical totems will be rendered inert. Creatures animated purely by magic will die."},
    {"domination",      itemMetals[4], "",  1,  1000,   BOLT_DOMINATION,    {1,2,1}, false, false, "This wand can forever bind an enemy to the caster's will, turning it into a steadfast ally. However, the magic works only against enemies that are near death."},
    {"beckoning",       itemMetals[5], "",  3,  500,    BOLT_BECKONING,     {2,4,1}, false, false, "The force of this wand will draw the targeted creature into direct proximity."},
    {"plenty",          itemMetals[6], "",  2,  700,    BOLT_PLENTY,        {1,2,1}, false, false, "The creature at the other end of this wand, friend or foe, will be beside itself -- literally! This mischievous cloning magic splits the body and life essence of its target into two. Both the creature and its clone will be weaker than the original."},
    {"invisibility",    itemMetals[7], "",  3,  100,    BOLT_INVISIBILITY,  {3,5,1}, false, false, "This wand will render a creature temporarily invisible to the naked eye. Only with telepathy or in the silhouette of a thick gas will an observer discern the creature's hazy outline."},
    {"empowerment",     itemMetals[8], "",  1,  100,    BOLT_EMPOWERMENT,   {1,1,1}, false, false, "This sacred magic will permanently improve the mind and body of any monster it hits. A wise adventurer will use it on allies, making them stronger in combat and able to learn a new talent from a fallen foe. If the bolt is reflected back at you, it will have no effect."},
};

itemTable staffTable[NUMBER_STAFF_KINDS] = {
    {"lightning",       itemWoods[0], "",   15, 1300,   BOLT_LIGHTNING,     {2,4,1}, false, false, "This staff conjures forth deadly arcs of electricity to damage to any number of creatures in a straight line."},
    {"firebolt",        itemWoods[1], "",   15, 1300,   BOLT_FIRE,          {2,4,1}, false, false, "This staff unleashes bursts of magical fire. It will ignite flammable terrain and burn any creature that it hits. Creatures with an immunity to fire will be unaffected by the bolt."},
    {"poison",          itemWoods[3], "",   10, 1200,   BOLT_POISON,        {2,4,1}, false, false, "The vile blast of this twisted staff will imbue its target with a deadly venom. Each turn, a creature that is poisoned will suffer one point of damage per dose of poison it has received, and poisoned creatures will not regenerate lost health until the poison clears."},
    {"tunneling",       itemWoods[4], "",   10, 1000,   BOLT_TUNNELING,     {2,4,1}, false, false, "Bursts of magic from this staff will pass harmlessly through creatures but will reduce obstructions to rubble."},
    {"blinking",        itemWoods[5], "",   11, 1200,   BOLT_BLINKING,      {2,4,1}, false, false, "This staff will allow you to teleport in the chosen direction. Creatures and inanimate obstructions will block the teleportation."},
    {"entrancement",    itemWoods[6], "",   6,  1000,   BOLT_ENTRANCEMENT,  {2,4,1}, false, false, "This staff will send creatures into a temporary trance, causing them to mindlessly mirror your movements. You can use the effect to cause one creature to attack another or to step into hazardous terrain, but the spell will be broken if you attack the creature under the effect."},
    {"obstruction",     itemWoods[7], "",   10, 1000,   BOLT_OBSTRUCTION,   {2,4,1}, false, false, "This staff will conjure a mass of impenetrable green crystal, preventing anything from moving through the affected area and temporarily entombing anything that is already there. The crystal will dissolve into the air as time passes. Higher level staffs will create larger obstructions."},
    {"discord",         itemWoods[8], "",   10, 1000,   BOLT_DISCORD,       {2,4,1}, false, false, "This staff will alter the perception of a creature and cause it to lash out indiscriminately. Strangers and allies alike will turn on the victim."},
    {"conjuration",     itemWoods[9], "",   8,  1000,   BOLT_CONJURATION,   {2,4,1}, false, false, "A flick of this staff will summon a number of phantom blades to fight on your behalf."},
    {"healing",         itemWoods[10], "",  5,  1100,   BOLT_HEALING,       {2,4,1}, false, false, "This staff will heal any creature, friend or foe. Unfortunately, you cannot use this or any staff on yourself except by reflecting the bolt."},
    {"haste",           itemWoods[11], "",  5,  900,    BOLT_HASTE,         {2,4,1}, false, false, "This staff will temporarily double the speed of any creature, friend or foe. Unfortunately, you cannot use this or any staff on yourself except by reflecting the bolt."},
    {"protection",      itemWoods[12], "",  5,  900,    BOLT_SHIELDING,     {2,4,1}, false, false, "This staff will bathe a creature in a protective light that will absorb all damage until it is depleted. Unfortunately, you cannot use this or any staff on yourself except by reflecting the bolt."},
};

itemTable ringTable[NUMBER_RING_KINDS] = {
    {"clairvoyance",    itemGems[0], "",    1,  900,    0,{1,3,1}, false, false, "This ring of eldritch scrying will permit you to see through nearby walls and doors, within a radius determined by the level of the ring. A cursed ring of clairvoyance will blind you to your immediate surroundings."},
    {"stealth",         itemGems[1], "",    1,  800,    0,{1,3,1}, false, false, "This ring of silent passage will reduce your stealth range, making enemies less likely to notice you and more likely to lose your trail. Staying motionless and lurking in the shadows will make you even harder to spot. Cursed rings of stealth will increase your stealth range, making you easier to spot and to track."},
    {"regeneration",    itemGems[2], "",    1,  750,    0,{1,3,1}, false, false, "This ring of sacred life will allow you to recover lost health at an accelerated rate. Cursed rings will decrease or even halt your natural regeneration."},
    {"transference",    itemGems[3], "",    1,  750,    0,{1,3,1}, false, false, "This ring of blood magic will heal you in proportion to the damage you inflict on others. Cursed rings will cause you to lose health when inflicting damage."},
    {"light",           itemGems[4], "",    1,  600,    0,{1,3,1}, false, false, "This ring of preternatural vision will allow you to see farther in the dimming light of the deeper dungeon levels. It will not make you more noticeable to enemies."},
    {"awareness",       itemGems[5], "",    1,  700,    0,{1,3,1}, false, false, "This ring of effortless vigilance will enable you to notice hidden secrets (traps, secret doors and hidden levers) more often and from a greater distance. Cursed rings of awareness will dull your senses, making it harder to notice secrets without actively searching for them."},
    {"wisdom",          itemGems[6], "",    1,  700,    0,{1,3,1}, false, false, "This ring of arcane power will cause your staffs to recharge at an accelerated rate. Cursed rings of wisdom will cause your staffs to recharge more slowly."},
    {"reaping",         itemGems[7], "",    1,  700,    0,{1,3,1}, false, false, "This ring of blood magic will recharge your staffs and charms every time you hit an enemy. Cursed rings of reaping will drain your staffs and charms with every hit."},
};

itemTable charmTable[NUMBER_CHARM_KINDS] = {
    {"health",          "", "", 5,  900,    0,{1,2,1}, true, false, "A handful of dried bloodwort and mandrake root has been bound together with leather cord and imbued with a powerful healing magic."},
    {"protection",      "", "", 5,  800,    0,{1,2,1}, true, false, "Four copper rings have been joined into a tetrahedron. The construct is oddly warm to the touch."},
    {"haste",           "", "", 5,  750,    0,{1,2,1}, true, false, "Various animals have been etched into the surface of this brass bangle. It emits a barely audible hum."},
    {"fire immunity",   "", "", 3,  750,    0,{1,2,1}, true, false, "Eldritch flames flicker within this polished crystal bauble."},
    {"invisibility",    "", "", 5,  700,    0,{1,2,1}, true, false, "A jade figurine depicts a strange humanoid creature. It has a face on both sides of its head, but all four eyes are closed."},
    {"telepathy",       "", "", 3,  700,    0,{1,2,1}, true, false, "Seven tiny glass eyes roll freely within this glass sphere. Somehow, they always come to rest facing outward."},
    {"levitation",      "", "", 1,  700,    0,{1,2,1}, true, false, "Sparkling dust and fragments of feather waft and swirl endlessly inside this small glass sphere."},
    {"shattering",      "", "", 1,  700,    0,{1,2,1}, true, false, "This turquoise crystal, fixed to a leather lanyard, hums with an arcane energy that sets your teeth on edge."},
    {"guardian",        "", "", 5,  700,    0,{1,2,1}, true, false, "When you touch this tiny granite statue, a rhythmic booming echoes in your mind."},
//    {"fear",            "", "",   3,  700,    0,{1,2,1}, true, false, "When you gaze into the murky interior of this obsidian cube, you feel as though something predatory is watching you."},
    {"teleportation",   "", "", 4,  700,    0,{1,2,1}, true, false, "The surface of this nickel sphere has been etched with a perfect grid pattern. Somehow, the squares of the grid are all exactly the same size."},
    {"recharging",      "", "", 5,  700,    0,{1,2,1}, true, false, "A strip of bronze has been wound around a rough wooden sphere. Each time you touch it, you feel a tiny electric shock."},
    {"negation",        "", "", 5,  700,    0,{1,2,1}, true, false, "A featureless gray disc hangs from a lanyard. When you touch it, your hand and arm go numb."},
};

const bolt boltCatalog[NUMBER_BOLT_KINDS] = {
    {{0}},
    //name                      bolt description                ability description                         char    foreColor       backColor           boltEffect      magnitude       pathDF      targetDF    forbiddenMonsterFlags       flags
    {"teleportation spell",     "casts a teleport spell",       "can teleport other creatures",             0,      NULL,           &blue,              BE_TELEPORT,    10,             0,          0,          MONST_IMMOBILE,             (BF_TARGET_ENEMIES)},
    {"slowing spell",           "casts a slowing spell",        "can slow $HISHER enemies",                 0,      NULL,           &green,             BE_SLOW,        10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"polymorph spell",         "casts a polymorphism spell",   "can polymorph other creatures",            0,      NULL,           &purple,            BE_POLYMORPH,   10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"negation magic",          "casts a negation spell",       "can cast negation",                        0,      NULL,           &pink,              BE_NEGATION,    10,             0,          0,          0,                          (BF_TARGET_ENEMIES)},
    {"domination spell",        "casts a domination spell",     "can dominate other creatures",             0,      NULL,           &dominationColor,   BE_DOMINATION,  10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"beckoning spell",         "casts a beckoning spell",      "can cast beckoning",                       0,      NULL,           &beckonColor,       BE_BECKONING,   10,             0,          0,          MONST_IMMOBILE,             (BF_TARGET_ENEMIES)},
    {"spell of plenty",         "casts a spell of plenty",      "can duplicate other creatures",            0,      NULL,           &rainbow,           BE_PLENTY,      10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES | BF_NOT_LEARNABLE)},
    {"invisibility magic",      "casts invisibility magic",     "can turn creatures invisible",             0,      NULL,           &darkBlue,          BE_INVISIBILITY, 10,            0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES)},
    {"empowerment sorcery",     "casts empowerment",            "can cast empowerment",                     0,      NULL,           &empowermentColor,  BE_EMPOWERMENT, 10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES | BF_NOT_LEARNABLE)},
    {"lightning",               "casts lightning",              "can hurl lightning bolts",                 0,      NULL,           &lightningColor,    BE_DAMAGE,      10,             0,          0,          0,                          (BF_PASSES_THRU_CREATURES | BF_TARGET_ENEMIES | BF_ELECTRIC)},
    {"flame",                   "casts a gout of flame",        "can hurl gouts of flame",                  0,      NULL,           &fireBoltColor,     BE_DAMAGE,      4,              0,          0,          MONST_IMMUNE_TO_FIRE,       (BF_TARGET_ENEMIES | BF_FIERY)},
    {"poison ray",              "casts a poison ray",           "can cast poisonous bolts",                 0,      NULL,           &poisonColor,       BE_POISON,      10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"tunneling magic",         "casts tunneling",              "can tunnel",                               0,      NULL,           &brown,             BE_TUNNELING,   10,             0,          0,          0,                          (BF_PASSES_THRU_CREATURES)},
    {"blink trajectory",        "blinks",                       "can blink",                                0,      NULL,           &white,             BE_BLINKING,    5,              0,          0,          0,                          (BF_HALTS_BEFORE_OBSTRUCTION)},
    {"entrancement ray",        "casts entrancement",           "can cast entrancement",                    0,      NULL,           &yellow,            BE_ENTRANCEMENT,10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"obstruction magic",       "casts obstruction",            "can cast obstruction",                     0,      NULL,           &forceFieldColor,   BE_OBSTRUCTION, 10,             0,          0,          0,                          (BF_HALTS_BEFORE_OBSTRUCTION)},
    {"spell of discord",        "casts a spell of discord",     "can cast discord",                         0,      NULL,           &discordColor,      BE_DISCORD,     10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"conjuration magic",       "casts a conjuration bolt",     "can cast conjuration",                     0,      NULL,           &spectralBladeColor, BE_CONJURATION,10,             0,          0,          MONST_IMMUNE_TO_WEAPONS,    (BF_HALTS_BEFORE_OBSTRUCTION | BF_TARGET_ENEMIES)},
    {"healing magic",           "casts healing",                "can heal $HISHER allies",                  0,      NULL,           &darkRed,           BE_HEALING,     5,              0,          0,          0,                          (BF_TARGET_ALLIES)},
    {"haste spell",             "casts a haste spell",          "can haste $HISHER allies",                 0,      NULL,           &orange,            BE_HASTE,       2,              0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES)},
    {"slowing spell",           "casts a slowing spell",        "can slow $HISHER enemies",                 0,      NULL,           &green,             BE_SLOW,        2,              0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"protection magic",        "casts protection",             "can cast protection",                      0,      NULL,           &shieldingColor,    BE_SHIELDING,   5,              0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES)},
    {"spiderweb",               "launches a sticky web",        "can launch sticky webs",                   '*',    &white,         NULL,               BE_NONE,        10,             DF_WEB_SMALL, DF_WEB_LARGE, (MONST_IMMOBILE | MONST_IMMUNE_TO_WEBS),   (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE)},
    {"spark",                   "shoots a spark",               "can throw sparks of lightning",            0,      NULL,           &lightningColor,    BE_DAMAGE,      1,              0,          0,          0,                          (BF_PASSES_THRU_CREATURES | BF_TARGET_ENEMIES | BF_ELECTRIC)},
    {"dragonfire",              "breathes a gout of white-hot flame", "can breathe gouts of white-hot flame", 0,    NULL,           &dragonFireColor,   BE_DAMAGE,      18,             DF_OBSIDIAN, 0,         MONST_IMMUNE_TO_FIRE,       (BF_TARGET_ENEMIES | BF_FIERY | BF_NOT_LEARNABLE)},
    {"arrow",                   "shoots an arrow",              "attacks from a distance",                  G_WEAPON,&gray,         NULL,               BE_ATTACK,      1,              0,          0,          MONST_IMMUNE_TO_WEAPONS,    (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE)},
    {"poisoned dart",           "fires a dart",                 "fires strength-sapping darts",             G_WEAPON,&centipedeColor,NULL,              BE_ATTACK,      1,              0,          0,          0,                          (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE)},
    {"growing vines",           "releases carnivorous vines into the ground", "conjures carnivorous vines", G_GRASS,&tanColor,      NULL,               BE_NONE,        5,              DF_ANCIENT_SPIRIT_GRASS, DF_ANCIENT_SPIRIT_VINES, (MONST_INANIMATE | MONST_IMMUNE_TO_WEBS),   (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS)},
    {"whip",                    "whips",                        "wields a whip",                            '*',    &tanColor,      NULL,               BE_ATTACK,      1,              0,          0,          MONST_IMMUNE_TO_WEAPONS,    (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE | BF_DISPLAY_CHAR_ALONG_LENGTH)},
};

const feat featTable[FEAT_COUNT] = {
    {"Pure Mage",       "Ascend without using fists or a weapon.", true},
    {"Pure Warrior",    "Ascend without using a staff, wand or charm.", true},
    {"Pacifist",        "Ascend without attacking a creature.", true},
    {"Archivist",       "Ascend without drinking a potion or reading a scroll.", true},
    {"Companion",       "Journey with an ally through 20 depths.", false},
    {"Specialist",      "Enchant an item up to or above +16.", false},
    {"Jellymancer",     "Obtain at least 90 jelly allies simultaneously.", false},
    {"Indomitable",     "Ascend without taking damage.", true},
    {"Mystic",          "Ascend without eating.", true},
    {"Dragonslayer",    "Kill a dragon with a melee attack.", false},
    {"Paladin",         "Ascend without attacking an unaware or fleeing creature.", true},
};

const char monsterBehaviorFlagDescriptions[32][COLS] = {
    "is invisible",                             // MONST_INVISIBLE
    "is an inanimate object",                   // MONST_INANIMATE
    "cannot move",                              // MONST_IMMOBILE
    "",                                         // MONST_CARRY_ITEM_100
    "",                                         // MONST_CARRY_ITEM_25
    "",                                         // MONST_ALWAYS_HUNTING
    "flees at low health",                      // MONST_FLEES_NEAR_DEATH
    "",                                         // MONST_ATTACKABLE_THRU_WALLS
    "corrodes weapons when hit",                // MONST_DEFEND_DEGRADE_WEAPON
    "is immune to weapon damage",               // MONST_IMMUNE_TO_WEAPONS
    "flies",                                    // MONST_FLIES
    "moves erratically",                        // MONST_FLITS
    "is immune to fire",                        // MONST_IMMUNE_TO_FIRE
    "",                                         // MONST_CAST_SPELLS_SLOWLY
    "cannot be entangled",                      // MONST_IMMUNE_TO_WEBS
    "can reflect magic spells",                 // MONST_REFLECT_4
    "never sleeps",                             // MONST_NEVER_SLEEPS
    "burns unceasingly",                        // MONST_FIERY
    "is invulnerable",                          // MONST_INVULNERABLE
    "is at home in water",                      // MONST_IMMUNE_TO_WATER
    "cannot venture onto dry land",             // MONST_RESTRICTED_TO_LIQUID
    "submerges",                                // MONST_SUBMERGES
    "keeps $HISHER distance",                   // MONST_MAINTAINS_DISTANCE
    "",                                         // MONST_WILL_NOT_USE_STAIRS
    "is animated purely by magic",              // MONST_DIES_IF_NEGATED
    "",                                         // MONST_MALE
    "",                                         // MONST_FEMALE
    "",                                         // MONST_NOT_LISTED_IN_SIDEBAR
    "moves only when activated",                // MONST_GETS_TURN_ON_ACTIVATION
};

const char monsterAbilityFlagDescriptions[32][COLS] = {
    "can induce hallucinations",                // MA_HIT_HALLUCINATE
    "can steal items",                          // MA_HIT_STEAL_FLEE
    "lights enemies on fire when $HESHE hits",  // MA_HIT_BURN
    "can possess $HISHER summoned allies",      // MA_ENTER_SUMMONS
    "corrodes armor when $HESHE hits",          // MA_HIT_DEGRADE_ARMOR
    "can summon allies",                        // MA_CAST_SUMMON
    "immobilizes $HISHER prey",                 // MA_SEIZES
    "injects poison when $HESHE hits",          // MA_POISONS
    "",                                         // MA_DF_ON_DEATH
    "divides in two when struck",               // MA_CLONE_SELF_ON_DEFEND
    "dies when $HESHE attacks",                 // MA_KAMIKAZE
    "recovers health when $HESHE inflicts damage",// MA_TRANSFERENCE
    "saps strength when $HESHE inflicts damage",// MA_CAUSE_WEAKNESS

    "attacks up to two opponents in a line",    // MA_ATTACKS_PENETRATE
    "attacks all adjacent opponents at once",   // MA_ATTACKS_ALL_ADJACENT
    "attacks with a whip",                      // MA_ATTACKS_EXTEND
    "pushes opponents backward when $HESHE hits", // MA_ATTACKS_STAGGER
    "avoids attacking in corridors in a group", // MA_AVOID_CORRIDORS
};

const char monsterBookkeepingFlagDescriptions[32][COLS] = {
    "",                                         // MB_WAS_VISIBLE
    "is telepathically bonded with you",        // MB_TELEPATHICALLY_REVEALED
    "",                                         // MB_PREPLACED
    "",                                         // MB_APPROACHING_UPSTAIRS
    "",                                         // MB_APPROACHING_DOWNSTAIRS
    "",                                         // MB_APPROACHING_PIT
    "",                                         // MB_LEADER
    "",                                         // MB_FOLLOWER
    "",                                         // MB_CAPTIVE
    "has been immobilized",                     // MB_SEIZED
    "is currently holding $HISHER prey immobile",// MB_SEIZING
    "is submerged",                             // MB_SUBMERGED
    "",                                         // MB_JUST_SUMMONED
    "",                                         // MB_WILL_FLASH
    "is anchored to reality by $HISHER summoner",// MB_BOUND_TO_LEADER
    "is marked for demonic sacrifice",          // MB_MARKED_FOR_SACRIFICE
};
