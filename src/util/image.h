#pragma once
#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QFile>
#include <QString>
#include <array>

// Decompress LZ77-compressed tile data from the loaded ROM at the given offset.
QByteArray decompressFromROM(int offset);

// Convert 32-byte GBA RGB555 palette (16 entries × 2 bytes) to 16 QColors.
std::array<QColor, 16> loadPalette(const QByteArray& bits);

// Read 16 GBA palette colors from the current file position (advances 32 bytes).
std::array<QColor, 16> loadPaletteFromROM(QFile& file);

// Render GBA 4bpp tile data to a QImage (ARGB32).
// Tiles are 8×8 pixels arranged in row-major order across the image.
QImage loadSprite(const QByteArray& bits, const std::array<QColor, 16>& palette,
                  int width, int height);

// Convert GBA 4bpp tile data to Windows-style indexed nibble data (nibbles swapped).
// Used to create palette-indexed PNG bitmaps.
QByteArray loadSpriteToBits(const QByteArray& bits,
                             const std::array<QColor, 16>& palette,
                             int width, int height);

// Load tile data from a file (LZ77-decompressing if isCompressed), convert to indexed
// pixel data suitable for a QImage::Format_Indexed8 image.
QByteArray loadTilesToBitsForImage(const QString& imgFile,
                                   const std::array<QColor, 16>& palette,
                                   bool isCompressed, int h, int w, int bytesNum);

QByteArray loadTilesToBitsForImage2(const QString& imgFile,
                                    const std::array<QColor, 16>& palette,
                                    bool isCompressed, int h, int w, int bytesNum);

// Decompress tileset image from the loaded ROM, generate tile preview (EM/RS).
// Returns LZ77-recompressed hex string of the 0x4000-byte tile data.
QString mapTilesCompressedToHexString(int imageOffset, int palOffset);

// Decompress FR/LG primary tileset (0x5000 bytes). Returns LZ77-compressed hex.
QString mapTilesCompressedToHexStringFRPrim(int imageOffset, int palOffset);

// Decompress FR/LG secondary tileset (0x3000 bytes). Returns LZ77-compressed hex.
QString mapTilesCompressedToHexStringFRSec(int imageOffset, int palOffset);

// Read FR primary tile data from a .bin file (LZ77-compressed), decompress,
// return raw (uncompressed) hex string.
QString mapTilesCompressedToHexStringFRPrim2(int offset, int dummy, const QString& tileFile);

// Same for FR secondary tile data.
QString mapTilesCompressedToHexStringFRSec2(int offset, int dummy, const QString& tileFile);

// Convert 16 QColors to a JASC-PAL text string.
QString colorToPalText(const std::array<QColor, 16>& colors);
