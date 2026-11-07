/**
 * @file mesh.cpp
 * @brief Mesh implementation.
 */

#include "poko/render/mesh.h"
#include "poko/render/vertex_buffer.h"
#include "poko/render/index_buffer.h"

#include <cstring>
#include <string>

namespace poko::render {

struct Mesh::Impl {
    VertexBuffer vertex_buffer;
    IndexBuffer index_buffer;
    VertexLayout layout;
    uint32_t vertex_count = 0;
    uint32_t index_count = 0;
    std::string name;
    std::string last_error;
};

Mesh::Mesh() : impl_(new Impl()) {}

Mesh::~Mesh() {
    delete impl_;
}

MeshErrorCode Mesh::initialize(Device* device, const MeshDesc& desc) {
    if (!device) {
        impl_->last_error = "Device is null";
        return MeshErrorCode::InvalidDevice;
    }

    if (!desc.layout) {
        impl_->last_error = "Vertex layout is null";
        return MeshErrorCode::InvalidLayout;
    }

    if (!desc.vertex_data || desc.vertex_count == 0 || desc.vertex_stride == 0) {
        impl_->last_error = "Invalid vertex data parameters";
        return MeshErrorCode::InvalidVertexData;
    }

    if (!desc.index_data || desc.index_count == 0) {
        impl_->last_error = "Invalid index data parameters";
        return MeshErrorCode::InvalidIndexData;
    }

    impl_->layout = *desc.layout;
    impl_->vertex_count = desc.vertex_count;
    impl_->index_count = desc.index_count;
    impl_->name = desc.name ? desc.name : "Mesh";

    // Create vertex buffer
    BufferDesc vb_desc;
    vb_desc.data = desc.vertex_data;
    vb_desc.size = desc.vertex_count * desc.vertex_stride;
    vb_desc.stride = desc.vertex_stride;
    vb_desc.name = (impl_->name + "_VB").c_str();

    if (impl_->vertex_buffer.initialize(device, vb_desc) != BufferErrorCode::Success) {
        impl_->last_error = std::string("Vertex buffer creation failed: ") +
                           impl_->vertex_buffer.get_last_error();
        return MeshErrorCode::BufferCreationFailed;
    }

    // Create index buffer
    IndexBufferDesc ib_desc;
    ib_desc.data = desc.index_data;
    ib_desc.size = desc.index_count * (desc.index_format == IndexFormat::UInt16 ? 2 : 4);
    ib_desc.index_count = desc.index_count;
    ib_desc.format = desc.index_format;
    ib_desc.name = (impl_->name + "_IB").c_str();

    if (impl_->index_buffer.initialize(device, ib_desc) != IndexBufferErrorCode::Success) {
        impl_->last_error = std::string("Index buffer creation failed: ") +
                           impl_->index_buffer.get_last_error();
        return MeshErrorCode::BufferCreationFailed;
    }

    return MeshErrorCode::Success;
}

bool Mesh::is_valid() const {
    return impl_->vertex_buffer.is_valid() && impl_->index_buffer.is_valid();
}

VertexBuffer* Mesh::get_vertex_buffer() {
    return &impl_->vertex_buffer;
}

const VertexBuffer* Mesh::get_vertex_buffer() const {
    return &impl_->vertex_buffer;
}

IndexBuffer* Mesh::get_index_buffer() {
    return &impl_->index_buffer;
}

const IndexBuffer* Mesh::get_index_buffer() const {
    return &impl_->index_buffer;
}

const VertexLayout* Mesh::get_layout() const {
    return &impl_->layout;
}

uint32_t Mesh::get_vertex_count() const {
    return impl_->vertex_count;
}

uint32_t Mesh::get_index_count() const {
    return impl_->index_count;
}

const char* Mesh::get_name() const {
    return impl_->name.c_str();
}

const char* Mesh::get_last_error() const {
    return impl_->last_error.c_str();
}

} // namespace poko::render
