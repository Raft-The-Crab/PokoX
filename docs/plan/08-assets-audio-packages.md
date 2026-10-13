# Assets, Audio, and Packages

## Import pipeline
1. Validate extension, size, and source.
2. Parse with bounded resources.
3. Convert to canonical intermediate data.
4. Optimize meshes/textures/audio and generate LOD/collision data where needed.
5. Process images through Cloudinary (resize, compress, format conversion).
6. Cache derived output; hashes may support internal deduplication but are not public version IDs.
7. Package manifest, checksums, compatibility metadata and signature policy.
8. Upload raw assets to Cloudflare R2, processed images to Cloudinary.
9. Atomically update published database pointer.

## Storage Strategy
- **Cloudflare R2**: Raw assets (meshes, audio, packages, source images)
- **Cloudinary**: Processed images (resized textures, thumbnails, icons)
- **Free tier**: 25 credits/month (approximately 25GB storage + 25GB bandwidth)

## Cloudinary Integration

### Use Cases
- **Texture optimization**: Resize, compress, format conversion (WebP, AVIF)
- **Thumbnails**: Generate multiple sizes for UI and discovery
- **Icons**: Process icons for UI elements
- **Avatar images**: Profile pictures and avatars
- **Screenshots**: Process game screenshots for discovery

### Processing Pipeline
1. Upload source image to Cloudinary (or upload to R2 and trigger Cloudinary fetch)
2. Apply transformations (resize, compress, format conversion)
3. Generate multiple variants (thumbnail, medium, large)
4. Store processed versions in Cloudinary
5. Store CDN URLs in asset metadata
6. Serve processed images from Cloudinary CDN

### Transformation Rules
- **Textures**: Power-of-two dimensions, WebP format, quality 85%
- **Thumbnails**: 256x256 max, WebP format, quality 80%
- **Icons**: 64x64 max, PNG format (preserve transparency)
- **Screenshots**: 1920x1080 max, WebP format, quality 85%

### Fallback Strategy
- If Cloudinary credits exhausted: Serve raw images from R2
- If Cloudinary service unavailable: Serve raw images from R2
- Monitor Cloudinary usage and alert before credit exhaustion

## Candidate dependencies
Review versions, build status, and licenses before adopting `fastgltf`, `zstd`, meshoptimizer, KTX/Basis, FreeType/HarfBuzz. Keep Poko-owned APIs around integrations.

## PKX / PNV
- [ ] Define package schema/version and compatibility rules.
- [ ] Separate client/server packages if `.pkxclient` and `.pkxserver` remain the chosen outputs.
- [ ] Include manifest, size limits, checksums, compatibility and signing policy.
- [ ] Never serialize raw pointers or runtime handles.
- [ ] Use monotonically sortable numeric version IDs, not `1.12.12` as the internal unique ID and not a hash as version number.
- [ ] Test corruption, truncation, rollback, and incompatible package rejection.

## Audio
- [ ] Start with miniaudio behind a Poko audio API.
- [ ] Define voice lifetime, streaming, spatial attenuation, listener, mixer/bus, memory limits.
- [ ] Profile voice count and streaming.
- [ ] Replace miniaudio only if a measured requirement justifies it.

## Gate
Repeatable import behavior, safe rejection of corrupt packages, tested rollback, and no leaked audio voices during unload/shutdown.
