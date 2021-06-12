//
// Created by gaetz on 08/06/2021.
//

#ifdef __linux__

#include <SDL2/SDL_events.h>

#elif _WIN32
#include <SDL_events.h>
#endif

#include "../engine/Game.h"

using game::Game;

Game::Game() : isRunning(true) {}

Game::~Game() {
}


void Game::load() {

}

void Game::processInputs() {

}

void Game::update(unsigned int dt) {

}

void Game::draw() {

}

void Game::cleanup() {

}