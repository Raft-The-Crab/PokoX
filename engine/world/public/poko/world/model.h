/**
 * @file model.h
 * @brief Model component for attaching meshes to entities.
 *
 * A Model component holds a mesh and optional material, allowing entities
 * to be rendered. The mesh is typically loaded via ObjLoader.
 *
 * Usage example:
 * @code
 * // Load mesh
 * ObjLoader loader;
 * MeshDesc mesh_desc;
 * loader.load("character.obj", device, mesh_desc);
 * Mesh mesh;
 * mesh.initialize(device, mesh_desc);
 * ObjLoader::cleanup(mesh_desc);
 *
 * // Attach to entity
 * Entity* entity = scene->create_entity("Character");
 * Model* model = entity->add_component<Model>();
 * model->set_mesh(&mesh);
 * model->set_material(&material);
 * @endcode
 */

#ifndef POKO_WORLD_MODEL_H
#define POKO_WORLD_MODEL_H

#include "component.h"
#include <cstdint>

namespace poko::render {
class Mesh;
class Material;
} // namespace poko::render

namespace poko::world {

/**
 * @brief Model component for mesh rendering.
 */
class Model : public Component {
public:
    Model();
    ~Model() = default;

    /**
     * @brief Sets the mesh for this model.
     *
     * The mesh must remain valid for the model's lifetime.
     *
     * @param mesh Pointer to the mesh (owned externally).
     */
    void set_mesh(render::Mesh* mesh);

    /**
     * @brief Returns the mesh.
     */
    render::Mesh* get_mesh();

    /**
     * @brief Returns the mesh (const).
     */
    const render::Mesh* get_mesh() const;

    /**
     * @brief Sets the material for this model.
     *
     * The material must remain valid for the model's lifetime.
     *
     * @param material Pointer to the material (owned externally).
     */
    void set_material(render::Material* material);

    /**
     * @brief Returns the material.
     */
    render::Material* get_material();

    /**
     * @brief Returns the material (const).
     */
    const render::Material* get_material() const;

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
    render::Mesh* mesh_ = nullptr;
    render::Material* material_ = nullptr;
    bool visible_ = true;
};

} // namespace poko::world

#endif // POKO_WORLD_MODEL_H
