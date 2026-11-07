/**
 * @file main.cpp
 * @brief M10 Demo: Skinned GLB character with animation and character controller.
 *
 * Loads the RobloxRigsSuite R6 rig (Character_Rig.glb), builds a
 * render::SkinnedModel, attaches it to a world entity via the
 * world::SkinnedModel component, and drives it with a
 * CharacterController. WASD moves, Space jumps, Shift runs.
 *
 * Usage: poko_m10_demo [--frames N] [--glb path] [--tga path]
 */

#define SDL_MAIN_HANDLED

#include "poko/core/engine.h"
#include "poko/render/gltf_loader.h"
#include "poko/render/skinned_model.h"
#include "poko/render/pipeline.h"
#include "poko/render/context.h"
#include "poko/render/swapchain.h"
#include "poko/render/vertex_buffer.h"
#include "poko/render/index_buffer.h"
#include "poko/render/texture.h"
#include "poko/render/material.h"
#include "poko/render/constant_buffer.h"
#include "poko/world/world.h"
#include "poko/world/scene.h"
#include "poko/world/entity.h"
#include "poko/world/transform.h"
#include "poko/world/character.h"
#include "poko/world/skinned_model.h"
#include "poko/math/mat4.h"
#include "poko/math/vec3.h"
#include "poko/platform/window.h"

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace math = poko::math;

namespace {

// ---------------------------------------------------------------------------
// Ground plane: unlit quad with its own MVP constant buffer
// ---------------------------------------------------------------------------

constexpr char kGroundVS[] = R"(#version 330 core
layout(location = 0) in vec3 aPosition;
layout(std140) uniform g_GroundMVP { mat4 uMVP; };
void main() { gl_Position = uMVP * vec4(aPosition, 1.0); }
)";

constexpr char kGroundPS[] = R"(#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(0.32, 0.34, 0.38, 1.0); }
)";

struct GroundMVP { math::Mat4 mvp; };

class Ground {
public:
    bool initialize(poko::render::Device* device) {
        // 20x20 quad at y=0, two triangles
        static const float verts[] = {
            -10.f, 0.f, -10.f,   10.f, 0.f, -10.f,   10.f, 0.f, 10.f,
            -10.f, 0.f, -10.f,   10.f, 0.f, 10.f,   -10.f, 0.f, 10.f,
        };
        vb_desc_.data = verts;
        vb_desc_.size = sizeof(verts);
        vb_desc_.stride = 3 * sizeof(float);
        vb_desc_.name = "GroundVB";
        if (vb_.initialize(device, vb_desc_) != poko::render::BufferErrorCode::Success) return false;

        attr_.semantic = poko::render::VertexSemantic::Position;
        attr_.format = poko::render::VertexFormat::Float3;
        attr_.offset = 0;
        attr_.input_slot = 0;
        layout_.attributes = &attr_;
        layout_.attribute_count = 1;
        layout_.stride = 3 * sizeof(float);

        res_.name = "g_GroundMVP";
        res_.type = poko::render::ShaderResourceType::ConstantBuffer;
        res_.shader_stages = 1;
        res_.array_size = 1;
        sig_.name = "GroundSignature";
        sig_.resources = &res_;
        sig_.resource_count = 1;

        vs_.type = poko::render::ShaderType::Vertex;
        vs_.language = poko::render::ShaderLanguage::GLSL;
        vs_.source = kGroundVS;
        vs_.entry_point = "main";
        ps_.type = poko::render::ShaderType::Pixel;
        ps_.language = poko::render::ShaderLanguage::GLSL;
        ps_.source = kGroundPS;
        ps_.entry_point = "main";

        pipe_desc_.name = "GroundPipeline";
        pipe_desc_.vertex_shader = vs_;
        pipe_desc_.pixel_shader = ps_;
        pipe_desc_.color_format = poko::render::TextureFormat::RGBA8_UNORM;
        pipe_desc_.depth_format = poko::render::TextureFormat::D32_FLOAT;
        pipe_desc_.topology = poko::render::PrimitiveTopology::TriangleList;
        pipe_desc_.depth_stencil.depth_enable = true;
        pipe_desc_.depth_stencil.depth_write_enable = true;
        pipe_desc_.depth_stencil.depth_func = poko::render::ComparisonFunc::Less;
        pipe_desc_.rasterizer.cull_mode = poko::render::CullMode::None;
        pipe_desc_.vertex_layout = layout_;
        pipe_desc_.resource_signature = sig_;
        if (pipe_.initialize(device, pipe_desc_) != poko::render::PipelineErrorCode::Success) {
            std::fprintf(stderr, "Ground pipeline: %s\n", pipe_.get_last_error());
            return false;
        }

        cb_desc_.data = nullptr;
        cb_desc_.size = sizeof(GroundMVP);
        cb_desc_.name = "GroundMVP";
        if (cb_.initialize(device, cb_desc_) != poko::render::ConstantBufferErrorCode::Success) return false;

        cb_bind_.buffer = &cb_;
        cb_bind_.shader_variable = "g_GroundMVP";
        mat_desc_.name = "GroundMaterial";
        mat_desc_.pipeline = &pipe_;
        mat_desc_.constant_buffer_count = 1;
        mat_desc_.constant_buffer_bindings = &cb_bind_;
        if (mat_.initialize(device, mat_desc_) != poko::render::MaterialErrorCode::Success) {
            std::fprintf(stderr, "Ground material: %s\n", mat_.get_last_error());
            return false;
        }
        return true;
    }

    void render(poko::render::Context* ctx, const math::Mat4& mvp) {
        mvp_data_.mvp = mvp;
        cb_.update(&mvp_data_, sizeof(GroundMVP), 0);
        ctx->set_pipeline_state(&pipe_);
        ctx->set_material(&mat_);
        ctx->set_vertex_buffer(&vb_, 0);
        ctx->draw(6);
    }

private:
    poko::render::VertexBuffer vb_;
    poko::render::BufferDesc vb_desc_;
    poko::render::VertexAttribute attr_;
    poko::render::VertexLayout layout_;
    poko::render::ShaderResource res_;
    poko::render::ResourceSignatureDesc sig_;
    poko::render::ShaderDesc vs_, ps_;
    poko::render::PipelineState pipe_;
    poko::render::PipelineDesc pipe_desc_;
    poko::render::ConstantBuffer cb_;
    poko::render::ConstantBufferDesc cb_desc_;
    poko::render::ConstantBufferBinding cb_bind_;
    poko::render::MaterialDesc mat_desc_;
    poko::render::Material mat_;
    GroundMVP mvp_data_{};
};

} // namespace

int main(int argc, char** argv) {
    std::printf("PokoX M10 Demo - Skinned GLB Character\n");
    std::printf("=====================================\n\n");

    uint64_t max_frames = 0;
    const char* glb_path = "RobloxRigsSuite/rigs/Character_Rig.glb";
    const char* tga_path = "assets/characters/rrs/RRS-Tex.tga";

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            max_frames = static_cast<uint64_t>(std::atoi(argv[++i]));
        } else if (std::strcmp(argv[i], "--glb") == 0 && i + 1 < argc) {
            glb_path = argv[++i];
        } else if (std::strcmp(argv[i], "--tga") == 0 && i + 1 < argc) {
            tga_path = argv[++i];
        }
    }

    // ---- Load glTF asset ----
    std::printf("Loading %s ...\n", glb_path);
    poko::render::GltfLoader loader;
    poko::render::GltfAsset asset;
    auto load_rc = loader.load(glb_path, asset);
    if (load_rc != poko::render::GltfLoaderError::Success) {
        std::fprintf(stderr, "GLB load failed: %s\n", loader.get_last_error());
        return 1;
    }
    std::printf("  primitives: %zu  materials: %zu  skins: %zu  animations: %zu  nodes: %zu\n",
                asset.primitives.size(), asset.materials.size(), asset.skins.size(),
                asset.animations.size(), asset.nodes.size());

    // ---- Engine ----
    poko::core::EngineConfig config;
    config.window_width = 1280;
    config.window_height = 720;
    config.window_title = "PokoX M10 - Skinned Character";
    config.headless = false;
    config.max_frames = max_frames;

    poko::core::Engine engine;
    if (engine.initialize(config) != poko::core::ErrorCode::Success) {
        std::fprintf(stderr, "Failed to initialize engine\n");
        return 1;
    }
    auto* device = engine.device();
    auto* context = engine.context();
    auto* swap_chain = engine.swap_chain();
    auto* window = engine.window();
    if (!device || !device->is_valid() || !context || !context->is_valid() ||
        !swap_chain || !swap_chain->is_valid() || !window) {
        std::fprintf(stderr, "Failed to get valid engine objects\n");
        engine.shutdown();
        return 1;
    }

    // ---- Skinned model ----
    std::printf("Building skinned model...\n");
    poko::render::SkinnedModelDesc model_desc;
    model_desc.name = "RRSCharacter";
    model_desc.texture_path = tga_path;
    model_desc.animation = "Idle";
    model_desc.loop = true;

    poko::render::SkinnedModel character;
    auto model_rc = character.initialize(device, asset, model_desc);
    if (model_rc != poko::render::SkinnedModelError::Success) {
        std::fprintf(stderr, "SkinnedModel failed: %s\n", character.get_last_error());
        engine.shutdown();
        return 1;
    }
    std::printf("  primitives: %u  bones: %u\n",
                character.get_primitive_count(), character.get_bone_count());

    // ---- Ground ----
    Ground ground;
    if (!ground.initialize(device)) {
        std::fprintf(stderr, "Ground initialization failed\n");
        engine.shutdown();
        return 1;
    }

    // ---- World / entity ----
    poko::world::World world;
    poko::world::WorldConfig world_config;
    bool world_ok = world.initialize(world_config);
    std::printf("world init: %d\n", world_ok ? 1 : 0);
    auto* scene = world.create_scene("Main");
    std::printf("scene: %p entities=%u\n", (void*)scene,
                scene ? scene->get_entity_count() : 0);
    auto* entity = scene->create_entity("Hero");
    // RRS rig is authored ~5 units tall (stud scale); scale to ~1.75 m
    entity->get_transform()->set_scale(math::Vec3(0.35f, 0.35f, 0.35f));
    entity->get_transform()->set_position(math::Vec3(0.f, 0.0f, 0.f));

    auto* controller = entity->add_component<poko::world::CharacterController>();
    // NB: do not call initialize() here — it would reset entity_ (set by on_add)
    controller->set_move_speed(4.0f);
    controller->set_jump_velocity(7.0f);
    controller->set_gravity(-18.0f);
    controller->set_ground_height(0.0f);

    auto* skinned_comp = entity->add_component<poko::world::SkinnedModel>();
    skinned_comp->set_model(&character);

    // ---- Camera ----
    const math::Vec3 eye(0.f, 1.7f, 4.2f);
    const math::Vec3 target(0.f, 1.0f, 0.f);
    const math::Vec3 up(0.f, 1.f, 0.f);
    math::Mat4 view = math::Mat4::look_at(eye, target, up);

    auto compute_vp = [&](uint32_t w, uint32_t h) {
        float aspect = static_cast<float>(w) / static_cast<float>(h > 0 ? h : 1);
        math::Mat4 proj = math::Mat4::perspective(1.0f, aspect, 0.1f, 100.0f);
        return proj * view;
    };

    // ---- Main loop ----
    std::printf("Starting render loop (WASD move, Space jump, Shift run)...\n");
    bool running = true;
    uint64_t frame = 0;
    uint64_t last_ticks = SDL_GetTicks();
    const char* current_anim = "Idle";
    bool jump_was_down = false;

    while (running) {
        poko::platform::WindowEvent event;
        while (window->poll_event(&event)) {
            if (event.type == poko::platform::WindowEventType::Quit ||
                event.type == poko::platform::WindowEventType::Close) {
                running = false;
            } else if (event.type == poko::platform::WindowEventType::Resize &&
                       event.width > 0 && event.height > 0) {
                swap_chain->resize(event.width, event.height);
            }
        }
        if (!running) break;

        uint64_t now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;
        if (dt > 0.1f) dt = 0.1f;

        // Input
        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        bool w = keys[SDL_SCANCODE_W], a = keys[SDL_SCANCODE_A];
        bool s = keys[SDL_SCANCODE_S], d = keys[SDL_SCANCODE_D];
        bool shift = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
        bool space = keys[SDL_SCANCODE_SPACE];

        math::Vec3 wish(0.f, 0.f, 0.f);
        // Camera looks -Z; W = -Z forward
        if (w) wish.z -= 1.f;
        if (s) wish.z += 1.f;
        if (a) wish.x -= 1.f;
        if (d) wish.x += 1.f;
        if (wish.x != 0.f || wish.z != 0.f) wish = wish.normalized();

        controller->set_move_speed(shift ? 7.0f : 4.0f);
        controller->move_with_acceleration(wish, dt, 12.0f);
        if (space && !jump_was_down) {
            controller->jump();
        }
        jump_was_down = space;

        world.update(dt);

        if (frame % 60 == 0) {
            math::Vec3 p = entity->get_transform()->get_position();
            std::printf("[dbg f%llu] pos=(%.2f,%.2f,%.2f) grounded=%d vel_y=%.2f\n",
                        static_cast<unsigned long long>(frame), p.x, p.y, p.z,
                        controller->is_grounded() ? 1 : 0,
                        controller->get_velocity().y);
        }

        // Animation state selection
        const char* desired = "Idle";
        if (!controller->is_grounded()) {
            desired = "Jump";
        } else {
            math::Vec3 vel = controller->get_velocity();
            float hspeed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
            if (hspeed > 0.5f) {
                desired = (shift ? "Run" : "Walk");
            }
        }
        if (std::strcmp(desired, current_anim) != 0) {
            int32_t idx = asset.find_animation(desired);
            if (idx >= 0) {
                character.set_animation(desired);
                current_anim = desired;
                std::printf("[frame %llu] anim -> %s\n",
                            static_cast<unsigned long long>(frame), desired);
            }
        }
        character.update(dt);

        // Render
        uint32_t width = window->width();
        uint32_t height = window->height();
        math::Mat4 vp = compute_vp(width, height);
        math::Mat4 ground_mvp = vp;  // identity world

        context->set_viewport({0.f, 0.f, static_cast<float>(width),
                               static_cast<float>(height), 0.f, 1.f});
        context->set_render_target(swap_chain);
        context->clear_render_target({0.53f, 0.72f, 0.88f, 1.f});  // sky blue
        context->clear_depth(1.0f, 0);

        ground.render(context, ground_mvp);

        math::Mat4 entity_world = entity->get_transform()->get_world_matrix();
        character.set_world_matrix(entity_world);
        character.render(context, vp);

        context->flush();
        swap_chain->present();
        ++frame;

        if (max_frames > 0 && frame >= max_frames) break;
        SDL_Delay(8);
    }

    std::printf("Rendered %llu frames\n", static_cast<unsigned long long>(frame));
    engine.shutdown();
    std::printf("\nDemo finished successfully\n");
    return 0;
}
