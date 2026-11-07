/**
 * @file skinned_model.cpp
 * @brief SkinnedModel component implementation.
 */

#include "poko/world/skinned_model.h"
#include "poko/world/entity.h"
#include "poko/render/skinned_model.h"

namespace poko::world {

SkinnedModel::SkinnedModel() = default;

void SkinnedModel::set_model(render::SkinnedModel* model) {
    model_ = model;
}

render::SkinnedModel* SkinnedModel::get_model() {
    return model_;
}

const render::SkinnedModel* SkinnedModel::get_model() const {
    return model_;
}

void SkinnedModel::set_visible(bool visible) {
    visible_ = visible;
}

bool SkinnedModel::is_visible() const {
    return visible_;
}

void SkinnedModel::on_add(Entity* entity) {
    (void)entity;
}

void SkinnedModel::on_remove(Entity* entity) {
    (void)entity;
}

void SkinnedModel::on_update(float delta_time) {
    if (model_) {
        model_->update(delta_time);
    }
}

} // namespace poko::world
