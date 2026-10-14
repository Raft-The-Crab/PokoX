//! Performance benchmarks for object model hierarchy operations
//!
//! Run with: cargo bench --bench hierarchy

use criterion::{black_box, criterion_group, criterion_main, BenchmarkId, Criterion};
use poko_object_model::ObjectRegistry;

fn bench_hierarchy_creation(c: &mut Criterion) {
    let mut group = c.benchmark_group("hierarchy_creation");
    for size in [10, 50, 100, 500].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            b.iter(|| {
                let mut registry = ObjectRegistry::new();
                let workspace = registry.create_workspace("Workspace".to_string()).unwrap();

                for i in 0..size {
                    let model = registry
                        .create_model(workspace.data.id, format!("Model{}", i))
                        .unwrap();
                    for j in 0..10 {
                        black_box(
                            registry
                                .create_part(model.data.id, format!("Part{}", j))
                                .unwrap(),
                        );
                    }
                }
            });
        });
    }
    group.finish();
}

fn bench_traverse_depth_first(c: &mut Criterion) {
    let mut group = c.benchmark_group("traverse_depth_first");
    for size in [10, 50, 100, 500].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry = ObjectRegistry::new();
            let workspace = registry.create_workspace("Workspace".to_string()).unwrap();

            for i in 0..size {
                let model = registry
                    .create_model(workspace.data.id, format!("Model{}", i))
                    .unwrap();
                for j in 0..10 {
                    registry
                        .create_part(model.data.id, format!("Part{}", j))
                        .unwrap();
                }
            }

            b.iter(|| {
                let mut count = 0;
                registry
                    .traverse_depth_first(workspace.data.id, |_| {
                        count += 1;
                        true
                    })
                    .unwrap();
                black_box(count);
            });
        });
    }
    group.finish();
}

fn bench_traverse_breadth_first(c: &mut Criterion) {
    let mut group = c.benchmark_group("traverse_breadth_first");
    for size in [10, 50, 100, 500].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry = ObjectRegistry::new();
            let workspace = registry.create_workspace("Workspace".to_string()).unwrap();

            for i in 0..size {
                let model = registry
                    .create_model(workspace.data.id, format!("Model{}", i))
                    .unwrap();
                for j in 0..10 {
                    registry
                        .create_part(model.data.id, format!("Part{}", j))
                        .unwrap();
                }
            }

            b.iter(|| {
                let mut count = 0;
                registry
                    .traverse_breadth_first(workspace.data.id, |_| {
                        count += 1;
                        true
                    })
                    .unwrap();
                black_box(count);
            });
        });
    }
    group.finish();
}

fn bench_get_descendants(c: &mut Criterion) {
    let mut group = c.benchmark_group("get_descendants");
    for size in [10, 50, 100, 500].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry = ObjectRegistry::new();
            let workspace = registry.create_workspace("Workspace".to_string()).unwrap();

            for i in 0..size {
                let model = registry
                    .create_model(workspace.data.id, format!("Model{}", i))
                    .unwrap();
                for j in 0..10 {
                    registry
                        .create_part(model.data.id, format!("Part{}", j))
                        .unwrap();
                }
            }

            b.iter(|| {
                black_box(registry.get_descendants(workspace.data.id).unwrap());
            });
        });
    }
    group.finish();
}

fn bench_get_ancestors(c: &mut Criterion) {
    let mut group = c.benchmark_group("get_ancestors");
    for depth in [5, 10, 20, 50].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(depth), depth, |b, &depth| {
            let mut registry = ObjectRegistry::new();
            let workspace = registry.create_workspace("Workspace".to_string()).unwrap();

            // Create a deep chain
            let mut parent_id = workspace.data.id;
            for i in 0..depth {
                let model = registry
                    .create_model(parent_id, format!("Model{}", i))
                    .unwrap();
                parent_id = model.data.id;
            }

            b.iter(|| {
                black_box(registry.get_ancestors(parent_id).unwrap());
            });
        });
    }
    group.finish();
}

fn bench_validate_hierarchy(c: &mut Criterion) {
    let mut group = c.benchmark_group("validate_hierarchy");
    for size in [10, 50, 100, 500].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry = ObjectRegistry::new();
            let workspace = registry.create_workspace("Workspace".to_string()).unwrap();

            for i in 0..size {
                let model = registry
                    .create_model(workspace.data.id, format!("Model{}", i))
                    .unwrap();
                for j in 0..10 {
                    registry
                        .create_part(model.data.id, format!("Part{}", j))
                        .unwrap();
                }
            }

            b.iter(|| {
                black_box(registry.validate_hierarchy().unwrap());
            });
        });
    }
    group.finish();
}

fn bench_serialize_hierarchy(c: &mut Criterion) {
    let mut group = c.benchmark_group("serialize_hierarchy");
    for size in [10, 50, 100, 500].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry = ObjectRegistry::new();
            let workspace = registry.create_workspace("Workspace".to_string()).unwrap();

            for i in 0..size {
                let model = registry
                    .create_model(workspace.data.id, format!("Model{}", i))
                    .unwrap();
                for j in 0..10 {
                    registry
                        .create_part(model.data.id, format!("Part{}", j))
                        .unwrap();
                }
            }

            b.iter(|| {
                black_box(registry.serialize_hierarchy().unwrap());
            });
        });
    }
    group.finish();
}

fn bench_deserialize_hierarchy(c: &mut Criterion) {
    let mut group = c.benchmark_group("deserialize_hierarchy");
    for size in [10, 50, 100, 500].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry1 = ObjectRegistry::new();
            let workspace = registry1.create_workspace("Workspace".to_string()).unwrap();

            for i in 0..size {
                let model = registry1
                    .create_model(workspace.data.id, format!("Model{}", i))
                    .unwrap();
                for j in 0..10 {
                    registry1
                        .create_part(model.data.id, format!("Part{}", j))
                        .unwrap();
                }
            }

            let json = registry1.serialize_hierarchy().unwrap();

            b.iter(|| {
                let mut registry2 = ObjectRegistry::new();
                black_box(registry2.deserialize_hierarchy(&json).unwrap());
            });
        });
    }
    group.finish();
}

criterion_group!(
    benches,
    bench_hierarchy_creation,
    bench_traverse_depth_first,
    bench_traverse_breadth_first,
    bench_get_descendants,
    bench_get_ancestors,
    bench_validate_hierarchy,
    bench_serialize_hierarchy,
    bench_deserialize_hierarchy
);
criterion_main!(benches);
