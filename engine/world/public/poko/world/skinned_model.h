/**
 * @file skinned_model.h
 * @brief SkinnedModel component for attaching skinned meshes to entities.
 *
 * Holds a pointer to a render::SkinnedModel (owned externally). The entity's
 * world transform is applied after skinning when the scene renderer draws
 * the component.
 *
 * Usage example:
 * @code
 * Entity* entity = scene->create_entity("Hero");
 * SkinnedModel* skinned = entity->add_component<SkinnedModel>();
 * skinned->set_model(&character_model);
 * @endcode
 */

#ifndef POKO_WORLD_SKINNED_MODEL_H
#define POKO_WORLD_SKINNED_MODEL_H

#include "component.h"
#include <cstdint>

namespace poko::render {
class SkinnedModel;
}

namespace poko::world {

/**
 * @brief SkinnedModel component for GPU-skinned mesh rendering.
 */
class SkinnedModel : public Component {
public:
    SkinnedModel();
    ~SkinnedModel() = default;

    /**
     * @brief Sets the skinned model to render.
     *
     * The model must remain valid for the component's lifetime.
     */
    void set_model(render::SkinnedModel* model);

    /**
     * @brief Returns the skinned model.
     */
    render::SkinnedModel* get_model();

    /**
     * @brief Returns the skinned model (const).
     */
    const render::SkinnedModel* get_model() const;

    /**
     * @brief Sets whether the model is visible.
     */
    void set_visible(bool visible);

    /**
     * @brief Returns true if the model is visible.
     */
    bool is_visible() const;

    // Component lifecycle hooks
    void on_add(Entity* entity) override;
    void on_remove(Entity* entity) override;
    void on_update(float delta_time) override;

private:
    render::SkinnedModel* model_ = nullptr;
    bool visible_ = true;
};

} // namespace poko::world

#endif // POKO_WORLD_SKINNED_MODEL_H
