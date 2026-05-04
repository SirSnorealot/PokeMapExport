#include "image.h"
#include "globals.h"
#include "hex.h"
#include "lz77.h"

#include <vector>

// ---- loadPalette -----------------------------------------------------------------

std::array<QColor, 16> loadPalette(const QByteArray& bits)
{
    std::array<QColor, 16> colors;
    for (int i = 0; i < 16; i++) {
        int c1 = static_cast<unsigned char>(bits[i * 2]);
        int c2 = static_cast<unsigned char>(bits[i * 2 + 1]);
        int temp = c2 * 256 + c1;
        int r = (temp & 0x1F) * 8;
        int g = ((temp >> 5) & 0x1F) * 8;
        int b = ((temp >> 10) & 0x1F) * 8;
        colors[i] = QColor(r, g, b);
    }
    return colors;
}

// ---- loadPaletteFromROM ----------------------------------------------------------

std::array<QColor, 16> loadPaletteFromROM(QFile& file)
{
    QByteArray data = file.read(32); // 16 colors × 2 bytes each
    if (data.size() < 32)
        data.resize(32); // pad if short
    return loadPalette(data);
}

// ---- loadSprite ------------------------------------------------------------------

QImage loadSprite(const QByteArray& bits, const std::array<QColor, 16>& palette,
                  int width, int height)
{
    QImage img(width, height, QImage::Format_ARGB32);
    img.fill(0);

    int i = 0;
    for (int y1 = 0; y1 <= height - 8; y1 += 8) {
        for (int x1 = 0; x1 <= width - 8; x1 += 8) {
            for (int y2 = 0; y2 < 8; y2++) {
                for (int x2 = 0; x2 < 8; x2 += 2) {
                    if (i >= bits.size()) break;
                    unsigned char temp = static_cast<unsigned char>(bits[i++]);
                    img.setPixel(x1 + x2,     y1 + y2, palette[temp & 0x0F].rgba());
                    img.setPixel(x1 + x2 + 1, y1 + y2, palette[(temp >> 4) & 0x0F].rgba());
                }
            }
        }
    }
    return img;
}

// ---- loadSpriteToBits ------------------------------------------------------------
// Converts 4bpp GBA tile data to Windows-DIB 4bpp format (swapped nibbles).
// GBA:    byte = (right_pixel << 4) | left_pixel
// WinDIB: byte = (left_pixel  << 4) | right_pixel
// Output is then expanded to 8bpp for QImage::Format_Indexed8.

QByteArray loadSpriteToBits(const QByteArray& bits,
                             const std::array<QColor, 16>& /*palette*/,
                             int width, int height)
{
    // We return one byte per pixel (expanded from 4bpp to 8bpp indexed)
    // Each 4bpp byte gives 2 pixels.
    int numPixels = width * height;
    QByteArray output(numPixels, static_cast<char>(0));

    int i    = 0; // source byte index
    int pIdx = 0; // destination pixel index

    for (int y1 = 0; y1 <= height - 8; y1 += 8) {
        for (int x1 = 0; x1 <= width - 8; x1 += 8) {
            for (int y2 = 0; y2 < 8; y2++) {
                for (int x2 = 0; x2 < 8; x2 += 2) {
                    if (i >= bits.size()) break;
                    unsigned char temp = static_cast<unsigned char>(bits[i++]);
                    int leftPix  = temp & 0x0F;
                    int rightPix = (temp >> 4) & 0x0F;
                    int outIdx = (y1 + y2) * width + (x1 + x2);
                    if (outIdx < numPixels)
                        output[outIdx]     = static_cast<char>(leftPix);
                    if (outIdx + 1 < numPixels)
                        output[outIdx + 1] = static_cast<char>(rightPix);
                    (void)pIdx;
                }
            }
        }
    }
    return output;
}

// ---- helpers for LZ77 decompression from file ----------------------------------

static QByteArray decompressLZ77FromFile(const QString& path, int offset)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    f.seek(offset);
    QByteArray compressed = f.readAll();
    f.close();
    if (compressed.isEmpty()) return {};

    const uint8_t* src = reinterpret_cast<const uint8_t*>(compressed.constData());
    int uncompLen = src[1] | (src[2] << 8) | (src[3] << 16);
    if (uncompLen <= 0 || uncompLen > 0x200000) return {};

    QByteArray dest(uncompLen, '\0');
    lz77DecompressRaw(src, reinterpret_cast<uint8_t*>(dest.data()));
    return dest;
}

static QByteArray decompressLZ77FromROM(int imageOffset)
{
    return decompressLZ77FromFile(g_loadedROM, imageOffset);
}

QByteArray decompressFromROM(int offset)
{
    return decompressLZ77FromROM(offset);
}

// ---- loadTilesToBitsForImage / 2 ------------------------------------------------

QByteArray loadTilesToBitsForImage(const QString& imgFile,
                                   const std::array<QColor, 16>& palette,
                                   bool isCompressed, int h, int w, int /*bytesNum*/)
{
    QFile f(imgFile);
    if (!f.open(QIODevice::ReadOnly)) return {};
    QByteArray bytes = f.readAll();
    f.close();

    if (isCompressed) {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(bytes.constData());
        if (bytes.size() < 4) return {};
        int uncompLen = src[1] | (src[2] << 8) | (src[3] << 16);
        QByteArray dest(uncompLen, '\0');
        lz77DecompressRaw(src, reinterpret_cast<uint8_t*>(dest.data()));
        bytes = dest;
    }

    return loadSpriteToBits(bytes, palette, w, h);
}

QByteArray loadTilesToBitsForImage2(const QString& imgFile,
                                    const std::array<QColor, 16>& palette,
                                    bool isCompressed, int h, int w, int bytesNum)
{
    return loadTilesToBitsForImage(imgFile, palette, isCompressed, h, w, bytesNum);
}

// ---- mapTilesCompressedToHexString (EM/RS, 0x4000 bytes) -----------------------

QString mapTilesCompressedToHexString(int imageOffset, int /*palOffset*/)
{
    QByteArray image = decompressLZ77FromROM(imageOffset);
    if (image.isEmpty()) return {};

    return byteArrayToHexString(lz77CompressBytes(image));
}

// ---- mapTilesCompressedToHexStringFRPrim (FR primary, 0x5000 bytes) -------------

QString mapTilesCompressedToHexStringFRPrim(int imageOffset, int /*palOffset*/)
{
    QByteArray image = decompressLZ77FromROM(imageOffset);
    if (image.isEmpty()) return {};

    return byteArrayToHexString(lz77CompressBytes(image));
}

// ---- mapTilesCompressedToHexStringFRSec (FR secondary, 0x3000 bytes) ------------

QString mapTilesCompressedToHexStringFRSec(int imageOffset, int /*palOffset*/)
{
    QByteArray image = decompressLZ77FromROM(imageOffset);
    if (image.isEmpty()) return {};

    return byteArrayToHexString(lz77CompressBytes(image));
}

// ---- mapTilesCompressedToHexStringFRPrim2 / Sec2 --------------------------------
// Reads a LZ77-compressed .bin file, decompresses it, returns raw uncompressed
// hex string (no re-compression).

QString mapTilesCompressedToHexStringFRPrim2(int /*offset*/, int /*dummy*/,
                                              const QString& tileFile)
{
    QFile f(tileFile);
    if (!f.open(QIODevice::ReadOnly)) return {};
    QByteArray data = f.readAll();
    f.close();

    // Decompress LZ77-compressed data
    if (data.size() < 4) return {};
    const uint8_t* src = reinterpret_cast<const uint8_t*>(data.constData());
    int uncompLen = src[1] | (src[2] << 8) | (src[3] << 16);
    if (uncompLen <= 0) return {};
    QByteArray dest(uncompLen, '\0');
    lz77DecompressRaw(src, reinterpret_cast<uint8_t*>(dest.data()));
    return byteArrayToHexString(dest);
}

QString mapTilesCompressedToHexStringFRSec2(int offset, int dummy,
                                             const QString& tileFile)
{
    return mapTilesCompressedToHexStringFRPrim2(offset, dummy, tileFile);
}

// ---- colorToPalText --------------------------------------------------------------
// Converts 16 QColors to JASC-PAL format (used by paint programs and the decomp).

QString colorToPalText(const std::array<QColor, 16>& colors)
{
    QString text = "JASC-PAL\n0100\n16\n";
    for (const QColor& c : colors) {
        text += QString("%1 %2 %3\n").arg(c.red()).arg(c.green()).arg(c.blue());
    }
    return text;
}
