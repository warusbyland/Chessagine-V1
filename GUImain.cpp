#include <SFML/Graphics.hpp>
#include <map>
#include <chrono>

#include "src/Core/MoveGen/moveGen.h"
#include "src/Core/Evaluation/Eval.h"
#include "src/Chessagine.h"

const int TILE_SIZE = 160; // Size of each square in pixels
const int TILE_SIZE_HALF = TILE_SIZE / 2;
const int BOARD_SIZE = 8; // 8x8 board
const int WINDOW_SIZE = TILE_SIZE * BOARD_SIZE;

const bool displayWhiteSide = true; // Show white as the current player (flips board)
std::string pieces[PIECE_NB] = {"wp", "wh", "wb", "wr", "wq", "wk", 
                        "bp", "bh", "bb", "br", "bq", "bk"}; // Index -> string
std::map<Piece, sf::Texture> pieceTextures;
std::map<Piece, sf::Sprite> pieceSprites; // String -> image

// Performance tester
// Usage: measureTime([&]() { func(arg); });
template <typename Func, typename... Args>
void measureTime(Func&& func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();

    std::forward<Func>(func)(std::forward<Args>(args)...);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Execution Time: " << duration.count() << " microseconds\n";
}
// Usage: measureTimeMult([&]() { func(arg); }, 100);
template <typename Func, typename... Args>
void measureTimeMult(Func&& func, Args&&... args, int epoch) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < epoch; i++) {
        std::forward<Func>(func)(std::forward<Args>(args)...);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << epoch << "x Execution Time: " << duration.count() << " microseconds\n";
    std::cout << "Execution Time each: " << duration.count() / epoch << " microseconds\n";
}

// Draw squares of the board
void drawBoardSquares(sf::RenderWindow& window) {
    sf::RectangleShape square(sf::Vector2f(TILE_SIZE, TILE_SIZE));

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            bool isLightSquare = (row + col) % 2 == 0;

            if (isLightSquare)
                square.setFillColor(sf::Color(240, 217, 181)); // light 
            else
                square.setFillColor(sf::Color(181, 136, 99)); // black

            square.setPosition(col * TILE_SIZE, row * TILE_SIZE);
            window.draw(square);
        }
    }
}
void drawPiece(sf::RenderWindow& window, sf::Sprite& sprite, int sq) {
    int ltr = displayWhiteSide ? 7 -(sq % BOARD_SIZE) : sq % BOARD_SIZE;
    // if want to display white, flip the board in the x axis
    int num = displayWhiteSide ? 7 - (sq / BOARD_SIZE) : sq / BOARD_SIZE;

    sprite.setPosition(ltr * TILE_SIZE, num * TILE_SIZE);
    window.draw(sprite);
}
void drawBoardPieces(sf::RenderWindow& window, Board board) {
    BB joined = board.getAllOccupancy();

    while (joined) {
        int sq = popLSB(joined);
        
        drawPiece(window, pieceSprites[board.getPiece(sq)], sq);
    }
}

std::pair<int, int> getBoardCoordinates(sf::RenderWindow& window, const sf::Vector2i& mousePos, bool displayWhiteSide) {
    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
    int file = static_cast<int>(worldPos.x) / TILE_SIZE;
    int rank = static_cast<int>(worldPos.y) / TILE_SIZE;

    if (displayWhiteSide) {
        file = 7 - file;
        rank = 7 - rank;
    }

    return { file, rank };
}
sf::Vector2f alignToTile(const sf::Vector2f& pos) {
    int alignedX = static_cast<int>(pos.x / TILE_SIZE) * TILE_SIZE;
    int alignedY = static_cast<int>(pos.y / TILE_SIZE) * TILE_SIZE;
    return sf::Vector2f(static_cast<float>(alignedX), static_cast<float>(alignedY));
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1800, 1500), "Chess", sf::Style::Default);
    window.setFramerateLimit(60);

    // This view keeps the board fixed-size and centered
    sf::View boardView(sf::FloatRect(0, 0, WINDOW_SIZE, WINDOW_SIZE));
    boardView.setViewport(sf::FloatRect(
        (1.f - (float)WINDOW_SIZE / window.getSize().x) / 2.f,
        (1.f - (float)WINDOW_SIZE / window.getSize().y) / 2.f,
        (float)WINDOW_SIZE / window.getSize().x,
        (float)WINDOW_SIZE / window.getSize().y
    ));

    // Render Chess pieces
    // ------------------------------------------------------------------------------------
    for (int i = 0; i < PIECE_NB; i++) {
        sf::Texture texture;
        if (!texture.loadFromFile("img/" + pieces[i] + ".png")) {
            return -1;  // Failed to load
        }

        pieceTextures[static_cast<Piece>(i)] = texture;

        sf::Sprite sprite;
        sprite.setTexture(pieceTextures[static_cast<Piece>(i)]);
        sprite.setScale(2, 2);
        pieceSprites[static_cast<Piece>(i)] = sprite;
    }

    MoveGen::initRays(); // Initialize rays for slider pieces

    // Piece dragging setup
    int selectedSq = 64;
    sf::RectangleShape selectedSquare(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    Piece pieceDragging = PIECE_NB;
    bool isDragging = false;
    Board pos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Moves ml;



    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if ((e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) || 
                 e.type == sf::Event::Closed)
                window.close();

            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
                auto [file, rank] = getBoardCoordinates(window, mousePos, displayWhiteSide);
                selectedSq = rank * 8 + file;

                // Make sure selected is a piece
                pieceDragging = pos.getPiece(selectedSq);
                if (pieceDragging != PIECE_NB) { isDragging = true; }

                // Make a square to cover the old piece
                bool isLightSquare = (rank + file) % 2 == 0;
                selectedSquare.setFillColor(isLightSquare 
                    ? sf::Color(240, 217, 181) 
                    : sf::Color(181, 136, 99));

                selectedSquare.setPosition(alignToTile(worldPos));

            } else if (isDragging && e.type == sf::Event::MouseButtonReleased 
                                  && e.mouseButton.button == sf::Mouse::Left) {
                isDragging = false;
                
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                auto [file, rank] = getBoardCoordinates(window, mousePos, displayWhiteSide);
                if (file >= 0 && file < BOARD_SIZE && rank >= 0 && rank < BOARD_SIZE) {
                    int targetSq = rank * 8 + file;
                    pos.playerMove(selectedSq, targetSq, QUIET);
                    std::cout << Eval::eval(pos)<< " value\n";

                    GameState state = pos.getGameState();
                    if (state == CHECKMATE) std::cout << "checkmate!\n";
                    else if (state == STALEMATE) std::cout << "draw!\n";
                    // MoveGen::perftDebug(pos, 6);
                }
            }
        }

        window.clear();
        window.clear(sf::Color(100, 100, 100)); // gray background
        window.setView(boardView);  
        drawBoardSquares(window);

        drawBoardPieces(window, pos);


        // Piece dragging logic 
        if (isDragging) {
            // Draw over the piece that is dragging
            window.draw(selectedSquare);

            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f pos = window.mapPixelToCoords(mousePos);
                
            sf::Sprite& sprite = pieceSprites[pieceDragging];
            sprite.setPosition(pos.x - TILE_SIZE_HALF, pos.y - TILE_SIZE_HALF);
            window.draw(sprite);
        }

        window.display();
    }

    return 0;
}
