#include "features/Tetromino.hpp"
#include "core/Types.hpp"

#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

void expect(bool ok, const std::string& msg) {
    if (!ok) throw std::runtime_error(msg);
}

bool hasFourDistinctBlocks(const tetris::ActivePiece& p) {
    std::set<std::pair<int, int>> s;
    for (const auto& b : p.blocks) {
        s.insert({b.x, b.y});
    }
    return s.size() == 4;
}

void testCreateEachType() {
    tetris::Tetromino factory;
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
        const auto piece = factory.createPiece(type);
        expect(piece.type == type, "type mismatch");
        expect(piece.rotation == tetris::RotationState::Spawn, "must spawn");
        expect(hasFourDistinctBlocks(piece), "need 4 distinct blocks");
    }
}

void testRandomTypesAreValid() {
    tetris::Tetromino factory;
    for (int i = 0; i < 50; ++i) {
        const auto piece = factory.createPiece();
        const int v = static_cast<int>(piece.type);
        expect(v >= 0 && v <= 6, "random type out of range");
        expect(hasFourDistinctBlocks(piece), "random piece invalid");
    }
}

void testRotateFourTimesReturnsOriginalBlocks() {
    tetris::Tetromino factory;
    const auto original = factory.createPiece(tetris::TetrominoType::T);
    auto p = original;
    for (int i = 0; i < 4; ++i) {
        p = factory.getRotated(p);
    }
    expect(p.blocks == original.blocks, "4 rotates must restore blocks");
    expect(p.rotation == original.rotation, "4 rotates must restore rotation");
}

void testRotateOUnchangedShape() {
    tetris::Tetromino factory;
    const auto original = factory.createPiece(tetris::TetrominoType::O);
    const auto rotated = factory.getRotated(original);
    expect(rotated.blocks == original.blocks, "O shape must not change");
}

void testGetRotatedDoesNotMutateInput() {
    tetris::Tetromino factory;
    const auto original = factory.createPiece(tetris::TetrominoType::J);
    auto copy = original;
    (void)factory.getRotated(original);
    expect(original.blocks == copy.blocks, "input must stay unchanged");
    expect(original.rotation == copy.rotation, "input rotation unchanged");
}

}  // namespace

int main() {
    try {
        testCreateEachType();
        testRandomTypesAreValid();
        testRotateFourTimesReturnsOriginalBlocks();
        testRotateOUnchangedShape();
        testGetRotatedDoesNotMutateInput();
        std::cout << "tetromino_test: all passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "tetromino_test failed: " << ex.what() << '\n';
        return 1;
    }
}