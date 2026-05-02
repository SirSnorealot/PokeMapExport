#pragma once
#include <QByteArray>
#include <QColor>
#include <QImage>
#include <array>
#include <vector>

// Render all metatiles (blocks) as a grid image.
// primaryTiles / secondaryTiles : raw decompressed 4bpp GBA tile data.
// allMetatiles  : primary block data concatenated with secondary block data.
// numMetatiles  : total number of metatiles to render.
// palettes      : 13 combined palettes (0..numPriPals-1 primary, rest secondary).
// primaryTileCount : tiles in the primary tileset (512 for RS/EM, 640 for FR/LG).
// tilesPerRow   : metatile columns in the output image.
QImage renderBlockSheet(
    const QByteArray& primaryTiles,
    const QByteArray& secondaryTiles,
    const QByteArray& allMetatiles,
    int numMetatiles,
    const std::vector<std::array<QColor, 16>>& palettes,
    int primaryTileCount,
    int tilesPerRow = 16);

// Render the full map as a QImage.
// mapData : raw bytes from map.bin (2 bytes per cell, u16 LE).
QImage renderMapImage(
    const QByteArray& mapData,
    int mapWidth, int mapHeight,
    const QByteArray& primaryTiles,
    const QByteArray& secondaryTiles,
    const QByteArray& allMetatiles,
    const std::vector<std::array<QColor, 16>>& palettes,
    int primaryTileCount);
