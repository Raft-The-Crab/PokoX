//! Performance benchmarks for HandleRegistry
//!
//! Run with: cargo bench --bench registry

use criterion::{black_box, criterion_group, criterion_main, BenchmarkId, Criterion};
use poko_ids_handles::HandleRegistry;

fn bench_allocate(c: &mut Criterion) {
    let mut group = c.benchmark_group("allocate");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            b.iter(|| {
                let mut registry: HandleRegistry<u32> = HandleRegistry::new();
                for _ in 0..size {
                    black_box(registry.allocate(black_box(42)).unwrap());
                }
            });
        });
    }
    group.finish();
}

fn bench_get(c: &mut Criterion) {
    let mut group = c.benchmark_group("get");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry: HandleRegistry<u32> = HandleRegistry::new();
            let handles: Vec<_> = (0..size).map(|_| registry.allocate(42).unwrap()).collect();

            b.iter(|| {
                for handle in &handles {
                    black_box(registry.get(*handle).unwrap());
                }
            });
        });
    }
    group.finish();
}

fn bench_remove(c: &mut Criterion) {
    let mut group = c.benchmark_group("remove");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            b.iter(|| {
                let mut registry: HandleRegistry<u32> = HandleRegistry::new();
                let handles: Vec<_> = (0..size).map(|_| registry.allocate(42).unwrap()).collect();

                for handle in handles {
                    black_box(registry.remove(handle).unwrap());
                }
            });
        });
    }
    group.finish();
}

fn bench_allocate_remove_cycle(c: &mut Criterion) {
    let mut group = c.benchmark_group("allocate_remove_cycle");
    for size in [100, 1_000, 10_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            b.iter(|| {
                let mut registry: HandleRegistry<u32> = HandleRegistry::new();
                for _ in 0..size {
                    let handle = registry.allocate(42).unwrap();
                    black_box(registry.remove(handle).unwrap());
                }
            });
        });
    }
    group.finish();
}

fn bench_find(c: &mut Criterion) {
    let mut group = c.benchmark_group("find");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry: HandleRegistry<u32> = HandleRegistry::new();
            for i in 0..size {
                registry.allocate(i as u32).unwrap();
            }

            b.iter(|| {
                black_box(registry.find(|&v| v == size as u32 / 2));
            });
        });
    }
    group.finish();
}

fn bench_retain(c: &mut Criterion) {
    let mut group = c.benchmark_group("retain");
    for size in [100, 1_000, 10_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            b.iter(|| {
                let mut registry: HandleRegistry<u32> = HandleRegistry::new();
                for i in 0..size {
                    registry.allocate(i as u32).unwrap();
                }
                black_box(registry.retain(|&v| v % 2 == 0));
            });
        });
    }
    group.finish();
}

fn bench_handles_iteration(c: &mut Criterion) {
    let mut group = c.benchmark_group("handles_iteration");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry: HandleRegistry<u32> = HandleRegistry::new();
            for _ in 0..size {
                registry.allocate(42).unwrap();
            }

            b.iter(|| {
                black_box(registry.handles().count());
            });
        });
    }
    group.finish();
}

fn bench_contains(c: &mut Criterion) {
    let mut group = c.benchmark_group("contains");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let mut registry: HandleRegistry<u32> = HandleRegistry::new();
            for i in 0..size {
                registry.allocate(i as u32).unwrap();
            }

            b.iter(|| {
                black_box(registry.contains(|&v| v == size as u32 / 2));
            });
        });
    }
    group.finish();
}

criterion_group!(
    benches,
    bench_allocate,
    bench_get,
    bench_remove,
    bench_allocate_remove_cycle,
    bench_find,
    bench_retain,
    bench_handles_iteration,
    bench_contains
);
criterion_main!(benches);
