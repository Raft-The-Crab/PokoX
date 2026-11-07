/**
 * @file model.cpp
 * @brief Model component implementation.
 */

#include "poko/world/model.h"
#include "poko/world/entity.h"
#include "poko/render/mesh.h"
#include "poko/render/material.h"

namespace poko::world {

Model::Model() = default;

void Model::set_mesh(render::Mesh* mesh) {
    mesh_ = mesh;
}

render::Mesh* Model::get_mesh() {
    return mesh_;
}

const render::Mesh* Model::get_mesh() const {
    return mesh_;
}

void Model::set_material(render::Material* material) {
    material_ = material;
}

render::Material* Model::get_material() {
    return material_;
}

const render::Material* Model::get_material() const {
    return material_;
}

void Model::set_visible(bool visible) {
    visible_ = visible;
}

bool Model::is_visible() const {
    return visible_;
}

void Model::on_add(Entity* entity) {
    (void)entity;
}

void Model::on_remove(Entity* entity) {
    (void)entity;
}

void Model::on_update(float delta_time) {
    (void)delta_time;
}

} // namespace poko::world
