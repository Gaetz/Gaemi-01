//
// Created by gaetz on 25/09/2021.
//

#include "GameImpl.h"
#include <Locator.h>
#include <Transformations.h>
#include <Engine.h>

using game::GameImpl;
using engine::Engine;

void GameImpl::load() {
    //Locator::assets().loadTexture("../../default.png", "default");
    Locator::assets().loadMesh("../../assets/knot.obj", "knot");

    knot.mesh = &Locator::assets().getMesh("knot");
    knot.material = &Locator::assets().getMaterial("default");
    knot.transform = engine::math::translate(Mat4{1.0f}, Vec3{0, 0, -20});
    Engine::addToScene(knot);
}

void GameImpl::update(u32 dt) {

}

void GameImpl::draw() {

}

void GameImpl::close() {

}