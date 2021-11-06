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

    Locator::assets().loadTexture("../../assets/default1.png", "default1");
    Locator::assets().createMaterial("green", "default", "default1");
}

void GameImpl::update(u32 dt) {
    timeMs += dt;
    if(timeMs > 1000) {
        knot.material = &Locator::assets().getMaterial("green");
    }
}

void GameImpl::draw() {

}

void GameImpl::close() {

}