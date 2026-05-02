#pragma once
#include <QString>
#include <QByteArray>
#include <cstdint>

// Read `length` bytes from file at 0-based offset `start`.
// Returns uppercase hex string, 2 chars per byte, no spaces.
QString readHex(const QString& filePath, int start, int length);

// Write hex string as raw bytes to file at 0-based offset `start`.
void writeHex(const QString& filePath, int start, const QString& data);

// Reverse bytes in hex string (e.g. "01020304" -> "04030201").
QString reverseHex(const QString& hexData);

// Format a byte value as a 2-char uppercase hex string.
QString makeProperByte(uint8_t byte);

// Convert raw byte array to uppercase hex string.
QString byteArrayToHexString(const QByteArray& data);

// Convert hex string to raw byte array.
QByteArray hexStringToByteArray(const QString& hex);

// Read a little-endian uint32 from a file at 0-based offset, return as int.
int readInt32LE(const QString& filePath, int offset);

// Read a little-endian uint32 from a file at 0-based offset, subtract 0x8000000.
int readROMPointer(const QString& filePath, int offset);
