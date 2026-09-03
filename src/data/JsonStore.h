#pragma once

#include <QString>

class Board;

/**
 * JsonStore -- persistence layer. Saves/loads a Board to/from a JSON file.
 *
 * Design choices:
 *   - All methods are static -- JsonStore has no state of its own.
 *     It is a namespaced collection of utility functions, not an object.
 *   - Does NOT inherit QObject -- no signals needed here.
 *   - Uses only Qt built-in JSON API: QJsonDocument, QJsonObject, QJsonArray.
 *     No third-party library needed.
 */
class JsonStore
{
public:
    // Serialise board and write to filePath.
    // Returns true on success, false on I/O error.
    static bool save(const Board &board, const QString &filePath);

    // Read filePath and populate board with loaded tasks.
    // Returns true on success, or false if file doesn't exist / is corrupted.
    static bool load(Board &board, const QString &filePath);
};
