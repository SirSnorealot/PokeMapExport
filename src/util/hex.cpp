#include "hex.h"
#include <QFile>

QString readHex(const QString& filePath, int start, int length)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    if (!f.seek(start)) {
        f.close();
        return QString();
    }
    QByteArray data = f.read(length);
    f.close();

    QString hex;
    hex.reserve(data.size() * 2);
    for (unsigned char b : data) {
        hex += QString("%1").arg(b, 2, 16, QChar('0')).toUpper();
    }
    return hex;
}

void writeHex(const QString& filePath, int start, const QString& data)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadWrite))
        return;
    f.seek(start);
    f.write(hexStringToByteArray(data));
    f.close();
}

QString reverseHex(const QString& hexData)
{
    // Swap bytes: "01020304" -> "04030201"
    QString result;
    result.reserve(hexData.size());
    for (int i = hexData.size() - 2; i >= 0; i -= 2) {
        result += hexData.mid(i, 2);
    }
    return result;
}

QString makeProperByte(uint8_t byte)
{
    return QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
}

QString byteArrayToHexString(const QByteArray& data)
{
    QString hex;
    hex.reserve(data.size() * 2);
    for (unsigned char b : data) {
        hex += QString("%1").arg(b, 2, 16, QChar('0')).toUpper();
    }
    return hex;
}

QByteArray hexStringToByteArray(const QString& hex)
{
    QByteArray result;
    result.reserve(hex.size() / 2);
    for (int i = 0; i + 1 < hex.size(); i += 2) {
        bool ok;
        uint8_t byte = static_cast<uint8_t>(hex.mid(i, 2).toUInt(&ok, 16));
        result.append(static_cast<char>(byte));
    }
    return result;
}

int readInt32LE(const QString& filePath, int offset)
{
    QString hex = readHex(filePath, offset, 4);
    QString rev = reverseHex(hex);
    bool ok;
    return static_cast<int>(rev.toUInt(&ok, 16));
}

int readROMPointer(const QString& filePath, int offset)
{
    return readInt32LE(filePath, offset) - 0x8000000;
}
