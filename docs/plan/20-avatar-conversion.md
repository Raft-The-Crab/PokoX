# Avatar Conversion and Compatibility

## Reference dimensions
R6-like planning reference:
- Overall envelope: 4 studs wide × 5 tall × 2 deep.
- Head: 2 × 1 × 2.
- Torso: 2 × 2 × 1.
- Limbs: 1 × 2 × 1.

These are conversion references, not a guarantee that every source rig exactly matches them.

## Pipeline
Inspect source → validate skeleton/units → normalize axes/scale → map canonical rig → convert pivots/parts → convert attachments → convert animation channels/timing → validate materials/weights where supported → generate warnings/report → preview in Studio.

## Tasks
- [ ] Canonical rig schema and stable names.
- [ ] Versioned conversion profile.
- [ ] Coordinate/pivot/joint conventions.
- [ ] Attachment names and transform semantics.
- [ ] Golden rigs and animation clips.
- [ ] Report unmapped bones, missing attachments, unsupported curves and scale changes.
- [ ] Visual regression renders for standard poses.
- [ ] Preserve original source and conversion settings.
- [ ] Document unsupported source features and lossy conversion.

## Gate
Reference rig, joints, attachments and sample animations convert repeatably with documented warnings and visual tests. Never delete source when derived conversion fails.
