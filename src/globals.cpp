#include "globals.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

QString g_loadedROM;
QString g_appPath;
QString g_header;
QString g_header2;
QString g_header3;

int g_point2MapBankPointers = 0;
int g_mapBankPointers = 0;
int g_bankPointer = 0;
int g_headerPointer = 0;
int g_mapBank = 0;
int g_mapNumber = 0;

int g_mapFooter = 0;
int g_mapEvents = 0;
int g_mapLevelScripts = 0;
int g_mapConnectionHeader = 0;

int g_mapHeight = 0;
int g_mapWidth = 0;
int g_borderPointer = 0;
int g_mapDataPointer = 0;
int g_primaryTilesetPointer = 0;
int g_secondaryTilesetPointer = 0;
int g_borderHeight = 0;
int g_borderWidth = 0;

int g_primaryTilesetCompression = 0;
int g_secondaryTilesetCompression = 0;
int g_primaryImagePointer = 0;
int g_secondaryImagePointer = 0;
int g_primaryPalPointer = 0;
int g_secondaryPalPointer = 0;
int g_primaryBlockSetPointer = 0;
int g_secondaryBlockSetPointer = 0;
int g_primaryBehaviourPointer = 0;
int g_secondaryBehaviourPointer = 0;

QString g_primaryPals;
QString g_secondaryPals;
QString g_primaryTilesImg;
QString g_secondaryTilesImg;
QString g_primaryBlocks;
QString g_secondaryBlocks;
QString g_primaryBehaviors;
QString g_secondaryBehaviors;

QString g_borderData;
QString g_mapPermData;
QString g_exportName;
QString g_selectedPath;

QStringList g_mapNames;

QString getConfigFileLocation()
{
    if (!g_loadedROM.isEmpty()) {
        QString romJson = g_loadedROM.left(g_loadedROM.length() - 4) + ".json";
        if (QFileInfo::exists(romJson))
            return romJson;
    }
    return g_appPath + "config.json";
}
