#pragma once
#include <QByteArray>
#include <QString>

// Convert a GBA-encoded byte array (Pokemon character encoding) to a readable QString.
// Stops at the 0xFF string terminator.
QString decodePokeText(const QByteArray& bytes, bool japanese = false);
