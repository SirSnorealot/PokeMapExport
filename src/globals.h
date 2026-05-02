#pragma once
#include <QString>
#include <QStringList>

// Global state (equivalent to mMain.vb module)

extern QString g_loadedROM;
extern QString g_appPath;
extern QString g_header;    // 4-char game code read from ROM offset 0xAC
extern QString g_header2;   // first 3 chars: "BPR", "BPG", "BPE", "AXP", "AXV"
extern QString g_header3;   // 4th char: region "E"=US, "J"=Japan, etc.

extern int g_point2MapBankPointers;
extern int g_mapBankPointers;
extern int g_bankPointer;
extern int g_headerPointer;
extern int g_mapBank;
extern int g_mapNumber;

extern int g_mapFooter;
extern int g_mapEvents;
extern int g_mapLevelScripts;
extern int g_mapConnectionHeader;

extern int g_mapHeight;
extern int g_mapWidth;
extern int g_borderPointer;
extern int g_mapDataPointer;
extern int g_primaryTilesetPointer;
extern int g_secondaryTilesetPointer;
extern int g_borderHeight;
extern int g_borderWidth;

extern int g_primaryTilesetCompression;
extern int g_secondaryTilesetCompression;
extern int g_primaryImagePointer;
extern int g_secondaryImagePointer;
extern int g_primaryPalPointer;
extern int g_secondaryPalPointer;
extern int g_primaryBlockSetPointer;
extern int g_secondaryBlockSetPointer;
extern int g_primaryBehaviourPointer;
extern int g_secondaryBehaviourPointer;

extern QString g_primaryPals;
extern QString g_secondaryPals;
extern QString g_primaryTilesImg;
extern QString g_secondaryTilesImg;
extern QString g_primaryBlocks;
extern QString g_secondaryBlocks;
extern QString g_primaryBehaviors;
extern QString g_secondaryBehaviors;

extern QString g_borderData;
extern QString g_mapPermData;
extern QString g_exportName;
extern QString g_selectedPath;

extern QStringList g_mapNames;

// Returns path of the JSON config file to use for the currently loaded ROM.
// Falls back to config.json in the app directory.
QString getConfigFileLocation();
