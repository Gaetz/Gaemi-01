//
// Created by gaetz on 25/09/2021.
//

#include "GameImpl.h"
#include "Engine.h"

using game::GameImpl;

void GameImpl::load() {
    engine::Engine::getState().memoryManager.logMemoryUsage();
}

void GameImpl::update(u32 dt) {

}

void GameImpl::draw() {

}

void GameImpl::cleanup() {

}