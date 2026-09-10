#include "features/Tetromino.hpp"

#include <random>

namespace tetris {

ActivePiece Tetromino::createPiece(TetrominoType type) const {
    ActivePiece piece;
    piece.type = type;
    piece.rotation = RotationState::Spawn;

    switch (type) {
        case TetrominoType::I:
            piece.origin = {4, 1};
            piece.blocks = {{{3, 1}, {4, 1}, {5, 1}, {6, 1}}};
            break;
        case TetrominoType::O:
            piece.origin = {4, 0};
            piece.blocks = {{{4, 0}, {5, 0}, {4, 1}, {5, 1}}};
            break;
        case TetrominoType::T:
            piece.origin = {4, 1};
            piece.blocks = {{{3, 1}, {4, 1}, {5, 1}, {4, 2}}};
            break;
        case TetrominoType::S:
            piece.origin = {4, 1};
            piece.blocks = {{{4, 1}, {5, 1}, {3, 2}, {4, 2}}};
            break;
        case TetrominoType::Z:
            piece.origin = {4, 1};
            piece.blocks = {{{3, 1}, {4, 1}, {4, 2}, {5, 2}}};
            break;
        case TetrominoType::J:
            piece.origin = {4, 1};
            piece.blocks = {{{3, 0}, {3, 1}, {4, 1}, {5, 1}}};
            break;
        case TetrominoType::L:
            piece.origin = {4, 1};
            piece.blocks = {{{5, 0}, {3, 1}, {4, 1}, {5, 1}}};
            break;
    }

    return piece;
}

ActivePiece Tetromino::createPiece() {
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_int_distribution<int> dist(0, 6);

    const int index = dist(rng);
    const TetrominoType type = static_cast<TetrominoType>(index);
    return createPiece(type);
}

ActivePiece Tetromino::getRotated(const ActivePiece& piece) const {
    ActivePiece result = piece;

    // 1) Advance orientation: Spawn -> Right -> Reverse -> Left -> Spawn.
    switch (piece.rotation) {
        case RotationState::Spawn:
            result.rotation = RotationState::Right;
            break;
        case RotationState::Right:
            result.rotation = RotationState::Reverse;
            break;
        case RotationState::Reverse:
            result.rotation = RotationState::Left;
            break;
        case RotationState::Left:
            result.rotation = RotationState::Spawn;
            break;
    }

    // 2) O is a square: rotation state changes, but blocks/origin do not.
    if (piece.type == TetrominoType::O) {
        return result;
    }

    // 3) Clockwise rotate each block around origin.
    // With y growing downward, offset (dx, dy) maps to (-dy, dx).
    for (Position& block : result.blocks) {
        const int dx = block.x - piece.origin.x;
        const int dy = block.y - piece.origin.y;
        block.x = piece.origin.x - dy;
        block.y = piece.origin.y + dx;
    }

    // Origin is the rotation pivot; leave it as-is.
    return result;
}

}  // namespace tetris