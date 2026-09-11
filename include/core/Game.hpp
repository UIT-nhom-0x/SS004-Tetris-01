#pragma once

#include "core/ConsoleRenderer.hpp"
#include "features/Collision.hpp"
#include "core/GameBoard.hpp"
#include "core/Input.hpp"
#include "core/Types.hpp"

namespace tetris {

/// Owns the core game loop and orchestrates board, input, and feature modules.
class Game {
public:
    static constexpr int FALL_INTERVAL_MS = 500;
    static constexpr int LOOP_SLEEP_MS = 16;

    Game();

    /// Runs until the player quits. Gravity advances independently of input.
    void run();

    /// Applies a translation only when every candidate block is placeable.
    /// Occupied-cell validation is delegated to Collision.
    bool moveCurrentPiece(int dx, int dy);

    /// Advances gravity by one row; returns true when game state changed.
    bool tick();

    /// Restores a new-game state without restarting the process.
    void restart();

    /// Exposes read-only state for feature integration and tests.
    [[nodiscard]] const GameBoard& board() const;
    [[nodiscard]] const ActivePiece& activePiece() const;
    [[nodiscard]] const ActivePiece& nextPiece() const;

private:
    bool handleInput(InputAction action);
    void render() const;

    GameBoard board_;
    ActivePiece activePiece_;
    ActivePiece nextPiece_;
    Collision collision_;
    Input input_;
    ConsoleRenderer renderer_;
    bool running_{true};
};

}  // namespace tetris
