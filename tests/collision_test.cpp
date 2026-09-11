#include "core/Game.hpp"
#include "core/GameBoard.hpp"
#include "core/Types.hpp"
#include "features/Collision.hpp"
#include "features/Tetromino.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void expect(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void testCanPlaceOnEmptyBoard() {
    tetris::Tetromino factory;
    tetris::Collision collision;
    tetris::GameBoard board;

    const tetris::TetrominoType types[] = {
        tetris::TetrominoType::I,
        tetris::TetrominoType::O,
        tetris::TetrominoType::T,
        tetris::TetrominoType::S,
        tetris::TetrominoType::Z,
        tetris::TetrominoType::J,
        tetris::TetrominoType::L,
    };

    for (auto type : types) {
        expect(collision.canPlace(board, factory.createPiece(type)),
               "spawn piece must fit an empty board");
    }
}

void testWallAndFloorCollisionRejected() {
    tetris::Tetromino factory;
    tetris::Collision collision;
    tetris::GameBoard board;
    const auto piece = factory.createPiece(tetris::TetrominoType::I);

    expect(!collision.canPlace(board, translated(piece, -4, 0)),
           "piece must collide with the left wall");
    expect(!collision.canPlace(board, translated(piece, 4, 0)),
           "piece must collide with the right wall");
    expect(!collision.canPlace(board, translated(piece, 0, 19)),
           "piece must collide with the floor");
    expect(!collision.canPlace(board, translated(piece, 0, -2)),
           "piece must collide with the ceiling");

    expect(collision.canPlace(board, translated(piece, -3, 0)),
           "piece flush with the left wall is valid");
    expect(collision.canPlace(board, translated(piece, 3, 0)),
           "piece flush with the right wall is valid");
    expect(collision.canPlace(board, translated(piece, 0, 18)),
           "piece resting on the floor is valid");
}

void testBlockCollisionRejected() {
    tetris::Tetromino factory;
    tetris::Collision collision;
    tetris::GameBoard board;
    board.setCell(4, 2, tetris::CellState::I);

    const auto piece = factory.createPiece(tetris::TetrominoType::T);
    expect(!collision.canPlace(board, piece),
           "piece overlapping an occupied cell must be rejected");
    expect(collision.canPlace(board, translated(piece, -1, 0)),
           "piece beside the occupied cell must be accepted");
}

void testLockPieceStoresTypeAndColor() {
    tetris::Tetromino factory;
    tetris::Collision collision;
    tetris::GameBoard board;

    const auto piece = translated(
        factory.createPiece(tetris::TetrominoType::J), 0, 18);
    collision.lockPiece(board, piece);

    int occupiedCells = 0;
    for (int y = 0; y < board.height(); ++y) {
        for (int x = 0; x < board.width(); ++x) {
            if (tetris::isOccupied(board.getCell(x, y))) {
                ++occupiedCells;
                expect(board.getCell(x, y) == tetris::cellStateFor(piece.type),
                       "locked cell must store the piece type");
            }
        }
    }
    expect(occupiedCells == 4, "locking must write exactly four cells");
}

void testClearWithoutFullLinesKeepsBoard() {
    tetris::Collision collision;
    tetris::GameBoard board;
    board.setCell(0, 19, tetris::CellState::Z);
    board.setCell(9, 15, tetris::CellState::T);

    expect(collision.clearCompletedLines(board) == 0,
           "no full row must clear nothing");
    expect(board.getCell(0, 19) == tetris::CellState::Z,
           "partial row must stay untouched");
    expect(board.getCell(9, 15) == tetris::CellState::T,
           "isolated block must stay untouched");
}

void testClearSingleLineShiftsRowsDown() {
    tetris::Collision collision;
    tetris::GameBoard board;

    for (int x = 0; x < board.width(); ++x) {
        board.setCell(x, 19, tetris::CellState::J);
    }
    board.setCell(0, 18, tetris::CellState::Z);

    expect(collision.clearCompletedLines(board) == 1,
           "one full row must be cleared");
    expect(board.getCell(0, 19) == tetris::CellState::Z,
           "row above must fall down and keep its color");
    expect(board.getCell(5, 19) == tetris::CellState::Empty,
           "rest of the fallen row must be empty");
    expect(board.getCell(0, 18) == tetris::CellState::Empty,
           "row above the cleared row must become empty");
}

void testClearMultipleConsecutiveLines() {
    tetris::Collision collision;
    tetris::GameBoard board;

    for (int x = 0; x < board.width(); ++x) {
        board.setCell(x, 17, tetris::CellState::S);
        board.setCell(x, 18, tetris::CellState::T);
        board.setCell(x, 19, tetris::CellState::Z);
    }
    board.setCell(0, 16, tetris::CellState::I);
    board.setCell(9, 16, tetris::CellState::L);

    expect(collision.clearCompletedLines(board) == 3,
           "all consecutive full rows must be cleared");
    expect(board.getCell(0, 19) == tetris::CellState::I &&
               board.getCell(9, 19) == tetris::CellState::L,
               "surviving blocks must keep their colors after falling");
    expect(board.getCell(1, 19) == tetris::CellState::Empty,
           "fallen row must be empty beside surviving blocks");
    for (int y = 16; y <= 18; ++y) {
        for (int x = 0; x < board.width(); ++x) {
            expect(board.getCell(x, y) == tetris::CellState::Empty,
                   "cleared region must be empty");
        }
    }
}

void testClearTopLineEmptiesReplacementCells() {
    tetris::Collision collision;
    tetris::GameBoard board;

    for (int x = 0; x < board.width(); ++x) {
        board.setCell(x, 0, tetris::CellState::L);
    }
    board.setCell(0, 1, tetris::CellState::T);

    expect(collision.clearCompletedLines(board) == 1,
           "top row must be cleared");
    expect(board.getCell(0, 1) == tetris::CellState::T,
           "row below the cleared top row must not shift");
    expect(board.getCell(5, 0) == tetris::CellState::Empty,
           "replacement cells with nothing above must be empty");
}

void testGameTickLocksPieceIntoBoard() {
    tetris::Game game;

    while (game.tick()) {
    }

    const tetris::ActivePiece locked = game.activePiece();
    for (const tetris::Position& block : locked.blocks) {
        expect(game.board().getCell(block.x, block.y) ==
                   tetris::cellStateFor(locked.type),
               "gravity must lock the piece at its final cells");
    }
    expect(!game.moveCurrentPiece(0, 1), "locked piece must rest on the floor");
}

void testGameMovementRejectsBlockedCandidates() {
    tetris::Game game;

    while (game.tick()) {
    }
    expect(!game.moveCurrentPiece(0, 1),
           "tick must not move a piece below the floor");

    const int beforeY = game.activePiece().blocks[0].y;
    expect(!game.moveCurrentPiece(0, 5),
           "candidate crossing the floor must be rejected atomically");
    expect(game.activePiece().blocks[0].y == beforeY,
           "rejected candidate must not mutate the piece");
}

}  // namespace

int main() {
    try {
        testCanPlaceOnEmptyBoard();
        testWallAndFloorCollisionRejected();
        testBlockCollisionRejected();
        testLockPieceStoresTypeAndColor();
        testClearWithoutFullLinesKeepsBoard();
        testClearSingleLineShiftsRowsDown();
        testClearMultipleConsecutiveLines();
        testClearTopLineEmptiesReplacementCells();
        testGameTickLocksPieceIntoBoard();
        testGameMovementRejectsBlockedCandidates();
        std::cout << "collision_test: all passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "collision_test failed: " << ex.what() << '\n';
        return 1;
    }
}
