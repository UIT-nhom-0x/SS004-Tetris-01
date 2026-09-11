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

void testGameTickLocksPieceAndPromotesNextPiece() {
    tetris::Game game;
    const tetris::ActivePiece expectedNext = game.nextPiece();

    for (int tick = 0; tick < tetris::GameBoard::HEIGHT; ++tick) {
        game.tick();
        if (game.activePiece().type == expectedNext.type) {
            break;
        }
    }

    expect(game.activePiece().type == expectedNext.type,
           "blocked piece must switch to the next piece");
    expect(game.activePiece().rotation == expectedNext.rotation,
           "promoted piece must preserve its rotation");
    expect(game.activePiece().origin == expectedNext.origin,
           "promoted piece must preserve its origin");
    expect(game.activePiece().blocks == expectedNext.blocks,
           "promoted piece must preserve its blocks");

    const tetris::Position lockedBlocks[] = {
        {4, 18},
        {5, 18},
        {4, 19},
        {5, 19},
    };
    for (const tetris::Position& block : lockedBlocks) {
        expect(game.board().getCell(block.x, block.y) == tetris::CellState::O,
               "previous piece must lock before the next piece is promoted");
    }
}

void testGameMovementRejectsBlockedCandidates() {
    tetris::Game game;
    expect(game.moveCurrentPiece(0, 18),
           "piece must move to the bottom for test setup");

    const auto beforeBlocks = game.activePiece().blocks;
    expect(!game.moveCurrentPiece(0, 1),
           "piece must not move below the floor");
    expect(game.activePiece().blocks == beforeBlocks,
           "rejected candidate must not mutate the piece");
}

void testGameRotationAppliesTetrominoCandidate() {
    tetris::Game game;
    tetris::Tetromino tetromino;
    const tetris::ActivePiece original = game.activePiece();
    const tetris::ActivePiece expected = tetromino.getRotated(original);

    expect(game.rotateCurrentPiece(),
           "rotation input must apply a placeable candidate");
    expect(game.activePiece().rotation == expected.rotation,
           "rotation input must update rotation state");
    expect(game.activePiece().blocks == expected.blocks,
           "rotation input must use Tetromino block coordinates");
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
        testGameTickLocksPieceAndPromotesNextPiece();
        testGameMovementRejectsBlockedCandidates();
        testGameRotationAppliesTetrominoCandidate();
        std::cout << "collision_test: all passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "collision_test failed: " << ex.what() << '\n';
        return 1;
    }
}
