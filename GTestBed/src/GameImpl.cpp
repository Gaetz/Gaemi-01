//
// Created by gaetz on 25/09/2021.
//

#include "GameImpl.h"
#include <Locator.h>

using game::GameImpl;

void GameImpl::load() {
    Locator::memory().logMemoryUsage();
}

void GameImpl::update(u32 dt) {

}

void GameImpl::draw() {

}

void GameImpl::close() {

}