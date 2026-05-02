#pragma once
#include <cstdint>
#include <QByteArray>

// Decompress GBA LZ77 data into a raw output buffer.
// source: compressed bytes (4-byte header: 0x10 + 3-byte LE uncompressed size).
// dest  : caller-allocated buffer of at least the uncompressed size.
// Returns the uncompressed byte count.
int lz77DecompressRaw(const uint8_t* source, uint8_t* dest);

// Compress raw data to GBA LZ77 format using a simple greedy algorithm.
// dest: caller-allocated output buffer.
// Returns the compressed byte count.
int lz77CompressRaw(const uint8_t* source, int sourceLen, uint8_t* dest);

// Decompress GBA LZ77 data (QByteArray interface).
QByteArray lz77DecompressBytes(const QByteArray& data);

// Compress data to GBA LZ77.
QByteArray lz77CompressBytes(const QByteArray& data);
