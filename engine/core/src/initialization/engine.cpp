/**
 * @file engine.cpp
 * @brief Engine public interface implementation.
 */

#include "poko/core/engine.h"

#include "engine_impl.h"
#include "poko/render/device.h"

namespace poko::core {

Engine::Engine() : impl_(new (std::nothrow) Impl()) {}

Engine::~Engine() {
    delete impl_;
}

ErrorCode Engine::initialize(const EngineConfig& config) {
    if (!impl_) {
        return ErrorCode::InitFailed;
    }
    return impl_->initialize(config) ? ErrorCode::Success : ErrorCode::InitFailed;
}

ErrorCode Engine::run() {
    if (!impl_) {
        return ErrorCode::InvalidState;
    }
    return impl_->run() == 0 ? ErrorCode::Success : ErrorCode::Unknown;
}

void Engine::shutdown() {
    if (impl_) {
        impl_->shutdown();
    }
}

bool Engine::is_initialized() const {
    return impl_ && impl_->initialized;
}

render::Device* Engine::device() {
    return impl_ ? &impl_->device : nullptr;
}

render::Context* Engine::context() {
    return impl_ ? &impl_->context : nullptr;
}

render::SwapChain* Engine::swap_chain() {
    return impl_ ? &impl_->swap_chain : nullptr;
}

platform::Window* Engine::window() {
    return impl_ ? &impl_->window : nullptr;
}

} // namespace poko::core
