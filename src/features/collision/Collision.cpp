#include "features/Collision.hpp"

namespace tetris {

bool Collision::canPlace(const GameBoard& board, const ActivePiece& piece) const {
    for (const Position& block : piece.blocks) {
        if (!board.isInside(block.x, block.y)) {
            return false;
        }

        if (isOccupied(board.getCell(block.x, block.y))) {
            return false;
        }
    }

    return true;
}

void Collision::lockPiece(GameBoard& board, const ActivePiece& piece) const {
    const CellState state = cellStateFor(piece.type);

    for (const Position& block : piece.blocks) {
        board.setCell(block.x, block.y, state);
    }
}

int Collision::clearCompletedLines(GameBoard& board) const {
    int writeRow = board.height() - 1;
    int clearedLines = 0;

    for (int row = board.height() - 1; row >= 0; --row) {
        bool full = true;
        for (int col = 0; col < board.width(); ++col) {
            if (!isOccupied(board.getCell(col, row))) {
                full = false;
                break;
            }
        }

        if (!full) {
            if (writeRow != row) {
                for (int col = 0; col < board.width(); ++col) {
                    board.setCell(col, writeRow, board.getCell(col, row));
                }
            }
            --writeRow;
        } else {
            ++clearedLines;
        }
    }

    for (int row = writeRow; row >= 0; --row) {
        for (int col = 0; col < board.width(); ++col) {
            board.setCell(col, row, CellState::Empty);
        }
    }

    return clearedLines;
}

}  // namespace tetris
