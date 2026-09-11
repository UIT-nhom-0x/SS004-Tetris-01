#include "core/Game.hpp"

#include "features/Collision.hpp"
#include "features/Tetromino.hpp"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <thread>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace tetris {

namespace {

bool standardOutputIsTerminal() {
#if defined(_WIN32)
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}

/// Uses a separate terminal screen so rendered frames never enter scrollback.
class TerminalScreen {
public:
    TerminalScreen() : active_(standardOutputIsTerminal()) {
        if (active_) {
            std::cout << "\x1B[?1049h\x1B[?25l" << std::flush;
        }
    }

    ~TerminalScreen() {
        restore();
    }

    TerminalScreen(const TerminalScreen&) = delete;
    TerminalScreen& operator=(const TerminalScreen&) = delete;

    void restore() {
        if (active_) {
            std::cout << "\x1B[?25h\x1B[?1049l" << std::flush;
            active_ = false;
        }
    }

private:
    bool active_;
};

// Keeps the core loop runnable until Huy's Tetromino generator is integrated.
ActivePiece makeTemporaryActivePiece() {
    return ActivePiece{
        TetrominoType::O,
        RotationState::Spawn,
        {4, 0},
        {{{4, 0}, {5, 0}, {4, 1}, {5, 1}}}};
}

// Exercises the preview contract without implementing random piece generation.
ActivePiece makeTemporaryNextPiece() {
    return ActivePiece{
        TetrominoType::T,
        RotationState::Spawn,
        {4, 1},
        {{{3, 1}, {4, 1}, {5, 1}, {4, 2}}}};
}

}  // namespace

Game::Game()
    : activePiece_(makeTemporaryActivePiece()),
      nextPiece_(makeTemporaryNextPiece()) {}

void Game::run() {
    using Clock = std::chrono::steady_clock;

    TerminalScreen terminalScreen;
    render();
    // steady_clock prevents system clock changes from affecting gravity timing.
    auto nextFall = Clock::now() + std::chrono::milliseconds(FALL_INTERVAL_MS);

    while (running_) {
        bool stateChanged = false;
        const InputAction action = input_.pollAction();
        if (action != InputAction::None) {
            stateChanged = handleInput(action);
        }

        const auto now = Clock::now();
        if (action == InputAction::Restart) {
            // A restarted piece always receives a complete first fall interval.
            nextFall = now + std::chrono::milliseconds(FALL_INTERVAL_MS);
        } else if (running_ && now >= nextFall) {
            stateChanged = tick() || stateChanged;
            nextFall = now + std::chrono::milliseconds(FALL_INTERVAL_MS);
        }

        if (running_ && stateChanged) {
            render();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_MS));
    }

    terminalScreen.restore();
    std::cout << "Game closed.\n";
}

bool Game::moveCurrentPiece(int dx, int dy) {
    const ActivePiece candidate = translated(activePiece_, dx, dy);

    if (!collision_.canPlace(board_, candidate)) {
        return false;
    }

    activePiece_ = candidate;
    return true;
}

bool Game::rotateCurrentPiece() {
    Tetromino tetromino;
    const ActivePiece candidate = tetromino.getRotated(activePiece_);

    if (!collision_.canPlace(board_, candidate)) {
        return false;
    }

    activePiece_ = candidate;
    return true;
}

bool Game::tick() {
    if (moveCurrentPiece(0, 1)) {
        return true;
    }

    collision_.lockPiece(board_, activePiece_);
    collision_.clearCompletedLines(board_);
    activePiece_ = nextPiece_;
    Tetromino tetromino;
    nextPiece_ = tetromino.createPiece();
    // TODO(Gam): update the score using the cleared-line count.
    // TODO(Khanh): set Game Over when the next piece cannot spawn.
    return true;
}

void Game::restart() {
    board_.reset();
    activePiece_ = makeTemporaryActivePiece();
    nextPiece_ = makeTemporaryNextPiece();
    running_ = true;

    // TODO(Gam): reset Scoring during integration.
    // TODO(Khanh): reset GameState during integration.
    // TODO(Huy): replace both temporary pieces using the Tetromino generator.
}

const GameBoard& Game::board() const {
    return board_;
}

const ActivePiece& Game::activePiece() const {
    return activePiece_;
}

const ActivePiece& Game::nextPiece() const {
    return nextPiece_;
}

bool Game::handleInput(InputAction action) {
    switch (action) {
        case InputAction::MoveLeft:
            return moveCurrentPiece(-1, 0);
        case InputAction::MoveRight:
            return moveCurrentPiece(1, 0);
        case InputAction::MoveDown:
            return moveCurrentPiece(0, 1);
        case InputAction::Rotate:
            return rotateCurrentPiece();
        case InputAction::Restart:
            restart();
            return true;
        case InputAction::Quit:
            running_ = false;
            return false;
        case InputAction::None:
            return false;
    }

    return false;
}

void Game::render() const {
    // Redraw in place within the alternate screen owned by run().
    const bool useTerminalFeatures = standardOutputIsTerminal();
    if (useTerminalFeatures) {
        std::cout << "\x1B[2J\x1B[H";
    }

    // TODO(Gam): replace zero with Scoring::getScore() during integration.
    // TODO(Khanh): replace false with GameState::isGameOver().
    std::cout << renderer_.buildFrame(
        board_, activePiece_, nextPiece_, 0, false, useTerminalFeatures)
              << std::flush;
}

}  // namespace tetris
