//
// Created by gaetz on 08/06/2021.
//


#include <SDL2/SDL_events.h>
#include "../engine/Game.h"

Game::Game() : isRunning(true) {}

Game::~Game()
{
}


void Game::load()
{

}

void Game::handleInputs()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        // Close the window when user clicks the X button or alt-f4
        if (e.type == SDL_QUIT) isRunning = false;
    }
}

void Game::update(unsigned int dt)
{

}

void Game::draw()
{

}
void Game::cleanup()
{

}