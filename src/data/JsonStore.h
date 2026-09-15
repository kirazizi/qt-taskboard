#pragma once

#include <QString>

class Board;
class BoardManager;

/**
 * JsonStore -- persistence layer. Saves/loads Boards to/from a JSON file.
 *
 * Design choices:
 *   - All methods are static -- JsonStore has no state of its own.
 *   - Uses only Qt built-in JSON API: QJsonDocument, QJsonObject, QJsonArray.
 *
 * File format evolution:
 *   - Single-board (Tier 1/2): { "tasks": [...] }
 *   - Multi-board  (Tier 3):   { "version": 2, "boards": [ { "id", "name", "tasks": [...] }, ... ] }
 *   JsonStore::load(BoardManager) auto-migrates the old single-board format.
 */
class JsonStore
{
public:
    // ── Single-board API (kept for compat with undo/redo helpers) ─────────────
    static bool save(const Board &board, const QString &filePath);
    static bool load(Board &board, const QString &filePath);

    // ── Multi-board API (Tier 3 Item 11) ─────────────────────────────────────
    static bool save(const BoardManager &manager, const QString &filePath);
    static bool load(BoardManager &manager, const QString &filePath);
};
