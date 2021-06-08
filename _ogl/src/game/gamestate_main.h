#ifndef GAMESTATE_MAIN_H
#define GAMESTATE_MAIN_H

#include "../engine/gamestate.h"
#include "../engine/spritebatch.h"
#include "../engine/sprite.h"

#define SPEED 500

// Main scene of the game, contains all the game's logic
class GameStateMain : public GameState
{
public:
	/*GameStateMain(
		std::shared_ptr<SpriteRenderer>, 
		std::shared_ptr<GeometryRenderer>
	);*/
	GameStateMain(std::shared_ptr<SpriteRenderer>);
	virtual ~GameStateMain();

	void load();
	void clean();
	void pause();
	void resume();
	void handleEvent(const InputState &);
	void update(unsigned int dt);
	void draw();
	void setGame(Game *);



private:
	Game *game;
	Spritebatch spritebatch;
    std::shared_ptr<SpriteRenderer> sRenderer;

    Sprite sprite;

	int getRand(int a, int b);

/*
	int moveLeftKey;
	int moveRightKey;
	int rotateKey;
	int fallKey;

	std::shared_ptr<SpriteRenderer> sRenderer;
	std::shared_ptr<GeometryRenderer> gRenderer;

	Pieces *pieces;
	Board *board;
	unsigned int counter;
	int screenHeight;

	Piece nextPiece;

	void drawPiece(Piece piece);
	void drawBoard();
*/
};

#endif
