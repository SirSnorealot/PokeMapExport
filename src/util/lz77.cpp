#include "lz77.h"
#include <vector>
#include <algorithm>
#include <cstring>

// ---- lz77DecompressRaw -------------------------------------------------------

int lz77DecompressRaw(const uint8_t* source, uint8_t* dest)
{
    uint32_t header = source[0]
                    | (static_cast<uint32_t>(source[1]) << 8)
                    | (static_cast<uint32_t>(source[2]) << 16)
                    | (static_cast<uint32_t>(source[3]) << 24);

    int xLen   = static_cast<int>(header >> 8);
    int retLen = xLen;
    int xIn    = 4;
    int xOut   = 0;

    while (xLen > 0) {
        uint8_t d = source[xIn++];

        for (int i = 0; i < 8; i++) {
            if (d & 0x80) {
                int data   = (source[xIn] << 8) | source[xIn + 1];
                xIn += 2;
                int length = (data >> 12) + 3;
                int offset = data & 0x0FFF;
                int winOff = xOut - offset - 1;
                for (int j = 0; j < length; j++) {
                    dest[xOut++] = dest[winOff++];
                    if (--xLen == 0)
                        return retLen;
                }
            } else {
                dest[xOut++] = source[xIn++];
                if (--xLen == 0)
                    return retLen;
            }
            d = static_cast<uint8_t>((d << 1) & 0xFF);
        }
    }
    return retLen;
}

// ---- lz77CompressRaw -------------------------------------------------------

int lz77CompressRaw(const uint8_t* source, int sourceLen, uint8_t* dest)
{
    dest[0] = 0x10;
    dest[1] = static_cast<uint8_t>(sourceLen & 0xFF);
    dest[2] = static_cast<uint8_t>((sourceLen >> 8) & 0xFF);
    dest[3] = static_cast<uint8_t>((sourceLen >> 16) & 0xFF);

    int xOut = 4;
    int xIn  = 0;

    while (xIn < sourceLen) {
        int flagPos   = xOut++;
        uint8_t flags = 0;

        for (int bit = 0; bit < 8 && xIn < sourceLen; bit++) {
            flags <<= 1;

            int bestLen = 0, bestOff = 0;
            int winStart = xIn - 4096;
            if (winStart < 0) winStart = 0;

            for (int w = winStart; w < xIn; w++) {
                int matchLen = 0;
                while (matchLen < 18
                       && (xIn + matchLen) < sourceLen
                       && source[w + matchLen] == source[xIn + matchLen])
                {
                    matchLen++;
                }
                if (matchLen > bestLen) {
                    bestLen = matchLen;
                    bestOff = xIn - w - 1;
                }
            }

            if (bestLen >= 3) {
                flags |= 1;
                int enc = ((bestLen - 3) << 12) | (bestOff & 0x0FFF);
                dest[xOut++] = static_cast<uint8_t>((enc >> 8) & 0xFF);
                dest[xOut++] = static_cast<uint8_t>(enc & 0xFF);
                xIn += bestLen;
            } else {
                dest[xOut++] = source[xIn++];
            }
        }

        dest[flagPos] = flags;
    }

    return xOut;
}

// ---- lz77DecompressBytes -----------------------------------------------------

static void lz77AmmendArray(uint8_t* bytes, int& index, int start, int length)
{
    for (int a = 0; a < length; a++) {
        if (index + a < (1 << 24) && start + a >= 0)
            bytes[index + a] = bytes[start + a];
    }
    index += length - 1;
}

QByteArray lz77DecompressBytes(const QByteArray& inputData)
{
    const uint8_t* Data = reinterpret_cast<const uint8_t*>(inputData.constData());

    if (Data[0] != 0x10)
        return QByteArray();

    int dataLength = Data[1] | (Data[2] << 8) | (Data[3] << 16);
    std::vector<uint8_t> out(dataLength, 0);

    int offset = 4;
    int i      = 0;
    int pos    = 8;
    uint8_t watchBits[8] = {};

    while (i < dataLength) {
        if (pos != 8) {
            if (watchBits[pos] == 0) {
                out[i] = Data[offset];
            } else {
                uint8_t r0 = Data[offset];
                uint8_t r1 = Data[offset + 1];
                int length = r0 >> 4;
                int start  = ((r0 & 0x0F) << 8) | r1;
                int from   = i - start - 1;
                lz77AmmendArray(out.data(), i, from, length + 3);
                offset++;
            }
            offset++;
            i++;
            pos++;
        } else {
            uint8_t w = Data[offset++];
            for (int b = 7; b >= 0; b--)
                watchBits[7 - b] = (w >> b) & 1;
            pos = 0;
        }
    }

    return QByteArray(reinterpret_cast<const char*>(out.data()), dataLength);
}

// ---- lz77CompressBytes -----------------------------------------------------

static int lz77SearchBytes(const uint8_t* D, int index, int dataLen, int maxLen, int& bestMatch)
{
    int found = -1;
    bestMatch = 0;

    if (index + 2 >= dataLen)
        return -1;

    for (int pos = 2; pos <= maxLen && bestMatch != 18; pos++) {
        int back = index - pos;
        if (back < 0) break;

        if (D[back] != D[index] || D[back + 1] != D[index + 1])
            continue;

        if (index > 2) {
            if (D[back + 2] != D[index + 2])
                continue;

            int m   = 2;
            bool ok = true;
            while (ok && m < 18 && (m + index) < dataLen - 1) {
                m++;
                if (D[index + m] != D[back + m])
                    ok = false;
            }
            if (m > bestMatch) {
                bestMatch = m;
                found     = pos;
            }
        } else {
            bestMatch = -1;
            return pos;
        }
    }
    return found;
}

QByteArray lz77CompressBytes(const QByteArray& inputData)
{
    const uint8_t* D = reinterpret_cast<const uint8_t*>(inputData.constData());
    int dataLen      = inputData.size();

    std::vector<uint8_t> bytes;
    bytes.reserve(dataLen + 64);

    bytes.push_back(0x10);
    bytes.push_back(static_cast<uint8_t>(dataLen & 0xFF));
    bytes.push_back(static_cast<uint8_t>((dataLen >> 8) & 0xFF));
    bytes.push_back(static_cast<uint8_t>((dataLen >> 16) & 0xFF));

    std::vector<uint8_t> preBytes;
    preBytes.push_back(D[0]);
    if (dataLen > 1) preBytes.push_back(D[1]);

    uint8_t watch = 0;
    int shortPos  = 2;
    int actualPos = 2;

    while (actualPos < dataLen) {
        if (shortPos == 8) {
            bytes.push_back(watch);
            for (uint8_t b : preBytes) bytes.push_back(b);
            watch = 0;
            preBytes.clear();
            shortPos = 0;
        } else {
            int match      = -1;
            int bestLength = 0;

            if (actualPos + 1 < dataLen) {
                int maxSearch = std::min(4096, actualPos);
                match = lz77SearchBytes(D, actualPos, dataLen, maxSearch, bestLength);
            }

            if (match == -1) {
                preBytes.push_back(D[actualPos]);
                watch = static_cast<uint8_t>((watch << 1) & 0xFF);
                actualPos++;
            } else {
                int length;
                int start = match;

                if (bestLength == -1) {
                    length = -1;
                    bool compatible = true;
                    while (compatible && length < 18
                           && (length + actualPos) < dataLen - 1) {
                        length++;
                        if (D[actualPos + length] != D[actualPos - start + length])
                            compatible = false;
                    }
                } else {
                    length = bestLength;
                }

                int enc = ((length - 3) << 12) | (start - 1);
                preBytes.push_back(static_cast<uint8_t>((enc >> 8) & 0xFF));
                preBytes.push_back(static_cast<uint8_t>(enc & 0xFF));

                actualPos += length;
                watch = static_cast<uint8_t>(((watch << 1) + 1) & 0xFF);
            }

            shortPos++;
        }
    }

    if (shortPos != 0) {
        watch = static_cast<uint8_t>((watch << (8 - shortPos)) & 0xFF);
        bytes.push_back(watch);
        for (uint8_t b : preBytes) bytes.push_back(b);
    }

    return QByteArray(reinterpret_cast<const char*>(bytes.data()),
                      static_cast<int>(bytes.size()));
}
