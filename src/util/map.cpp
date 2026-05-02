#include "map.h"
#include <cstdint>

// ---- drawTile -------------------------------------------------------------------
// Draw one 8x8 GBA 4bpp tile from tileData[tileIdx*32] into dest at (destX,destY).
// hflip/vflip mirror the source pixel coordinates before writing to destination.

static void drawTile(QImage& dest, int destX, int destY,
                     const QByteArray& tileData, int tileIdx,
                     const std::vector<std::array<QColor, 16>>& palettes,
                     int palIdx, bool hflip, bool vflip, bool skipTransparent)
{
    if (palIdx < 0 || palIdx >= static_cast<int>(palettes.size())) return;
    const auto& pal = palettes[palIdx];

    int base = tileIdx * 32;
    if (base < 0 || base + 32 > static_cast<int>(tileData.size())) return;

    // GBA 4bpp layout: 8 rows × 4 bytes; each byte = 2 pixels (lo nibble = left).
    for (int sy = 0; sy < 8; sy++) {
        int dy = vflip ? (7 - sy) : sy;
        for (int sx = 0; sx < 8; sx++) {
            int dx = hflip ? (7 - sx) : sx;
            uint8_t byte = static_cast<uint8_t>(tileData[base + sy * 4 + sx / 2]);
            int colorIdx = (sx & 1) ? (byte >> 4) & 0x0F : byte & 0x0F;
            // Color index 0 is transparent on GBA; skip it for the top layer
            // so it shows through to the bottom layer below.
            if (skipTransparent && colorIdx == 0) continue;
            dest.setPixel(destX + dx, destY + dy, pal[colorIdx].rgba());
        }
    }
}

// ---- drawMetatile ---------------------------------------------------------------
// Draw a 16x16 metatile (2 layers of 2x2 tiles) at (destX,destY).
// Metatile layout in allMetatiles: 8 × u16 per metatile (16 bytes).
//   Entries 0-3 : bottom layer  [col0,row0][col1,row0][col0,row1][col1,row1]
//   Entries 4-7 : top layer     [col0,row0][col1,row0][col0,row1][col1,row1]
// Tile entry u16: bits 0-9 tile index, 10 hflip, 11 vflip, 12-15 palette index.

static void drawMetatile(QImage& dest, int destX, int destY,
                         int metatileIdx,
                         const QByteArray& primaryTiles,
                         const QByteArray& secondaryTiles,
                         const QByteArray& allMetatiles,
                         const std::vector<std::array<QColor, 16>>& palettes,
                         int primaryTileCount)
{
    int base = metatileIdx * 16;
    if (base < 0 || base + 16 > static_cast<int>(allMetatiles.size())) return;

    for (int layer = 0; layer < 2; layer++) {
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 2; col++) {
                int entryOffset = base + (layer * 4 + row * 2 + col) * 2;
                uint16_t entry = static_cast<uint8_t>(allMetatiles[entryOffset])
                               | (static_cast<uint8_t>(allMetatiles[entryOffset + 1]) << 8);

                int  tileIdx = entry & 0x3FF;
                bool hflip   = (entry >> 10) & 1;
                bool vflip   = (entry >> 11) & 1;
                int  palIdx  = (entry >> 12) & 0xF;

                const QByteArray* tileData;
                int localIdx;
                if (tileIdx < primaryTileCount) {
                    tileData = &primaryTiles;
                    localIdx = tileIdx;
                } else {
                    tileData = &secondaryTiles;
                    localIdx = tileIdx - primaryTileCount;
                }

                drawTile(dest, destX + col * 8, destY + row * 8,
                         *tileData, localIdx, palettes, palIdx, hflip, vflip,
                         /*skipTransparent=*/ layer == 1);
            }
        }
    }
}

// ---- renderBlockSheet -----------------------------------------------------------

QImage renderBlockSheet(const QByteArray& primaryTiles,
                        const QByteArray& secondaryTiles,
                        const QByteArray& allMetatiles,
                        int numMetatiles,
                        const std::vector<std::array<QColor, 16>>& palettes,
                        int primaryTileCount,
                        int tilesPerRow)
{
    if (numMetatiles <= 0 || tilesPerRow <= 0) return {};

    int numRows = (numMetatiles + tilesPerRow - 1) / tilesPerRow;
    QImage img(tilesPerRow * 16, numRows * 16, QImage::Format_ARGB32);
    img.fill(0xFF000000);

    for (int i = 0; i < numMetatiles; i++) {
        int col = i % tilesPerRow;
        int row = i / tilesPerRow;
        drawMetatile(img, col * 16, row * 16, i,
                     primaryTiles, secondaryTiles, allMetatiles,
                     palettes, primaryTileCount);
    }
    return img;
}

// ---- renderMapImage -------------------------------------------------------------

QImage renderMapImage(const QByteArray& mapData,
                      int mapWidth, int mapHeight,
                      const QByteArray& primaryTiles,
                      const QByteArray& secondaryTiles,
                      const QByteArray& allMetatiles,
                      const std::vector<std::array<QColor, 16>>& palettes,
                      int primaryTileCount)
{
    if (mapWidth <= 0 || mapHeight <= 0) return {};

    QImage img(mapWidth * 16, mapHeight * 16, QImage::Format_ARGB32);
    img.fill(0xFF000000);

    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            int cellOffset = (y * mapWidth + x) * 2;
            if (cellOffset + 2 > static_cast<int>(mapData.size())) continue;

            uint16_t cell = static_cast<uint8_t>(mapData[cellOffset])
                          | (static_cast<uint8_t>(mapData[cellOffset + 1]) << 8);
            int metatileId = cell & 0x03FF;

            drawMetatile(img, x * 16, y * 16, metatileId,
                         primaryTiles, secondaryTiles, allMetatiles,
                         palettes, primaryTileCount);
        }
    }
    return img;
}
