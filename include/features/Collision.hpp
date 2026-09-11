#pragma once

#include "core/GameBoard.hpp"
#include "core/Types.hpp"

namespace tetris {

/// Validates piece placement and owns board mutation for locking/line clearing.
class Collision {
public:
    /// Returns true when all four blocks are inside the board and on empty cells.
    [[nodiscard]] bool canPlace(
        const GameBoard& board,
        const ActivePiece& piece) const;

    /// Stores all four blocks with the piece type used for terminal color.
    /// The piece must be placeable.
    void lockPiece(GameBoard& board, const ActivePiece& piece) const;

    /// Removes every completed row and returns the number of removed rows.
    int clearCompletedLines(GameBoard& board) const;
};

}  // namespace tetris
