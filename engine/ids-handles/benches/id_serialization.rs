//! Performance benchmarks for ID serialization
//!
//! Run with: cargo bench --bench id_serialization

use criterion::{black_box, criterion_group, criterion_main, Criterion, BenchmarkId};
use poko_ids_handles::{Id, IdType, IdBytes, IdBatch};

fn bench_id_creation(c: &mut Criterion) {
    let mut group = c.benchmark_group("id_creation");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            b.iter(|| {
                for _ in 0..size {
                    black_box(Id::new(IdType::Workspace));
                }
            });
        });
    }
    group.finish();
}

fn bench_id_to_bytes(c: &mut Criterion) {
    let mut group = c.benchmark_group("id_to_bytes");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let ids: Vec<_> = (0..size).map(|_| Id::new(IdType::Workspace)).collect();

            b.iter(|| {
                for id in &ids {
                    black_box(IdBytes::from(id));
                }
            });
        });
    }
    group.finish();
}

fn bench_id_from_bytes(c: &mut Criterion) {
    let mut group = c.benchmark_group("id_from_bytes");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let bytes: Vec<_> = (0..size)
                .map(|_| IdBytes::from(&Id::new(IdType::Workspace)))
                .collect();

            b.iter(|| {
                for bytes in &bytes {
                    black_box(Id::try_from(bytes).unwrap());
                }
            });
        });
    }
    group.finish();
}

fn bench_id_serialization_json(c: &mut Criterion) {
    let mut group = c.benchmark_group("id_serialization_json");
    for size in [100, 1_000, 10_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let ids: Vec<_> = (0..size).map(|_| Id::new(IdType::Workspace)).collect();

            b.iter(|| {
                black_box(serde_json::to_string(&ids).unwrap());
            });
        });
    }
    group.finish();
}

fn bench_id_deserialization_json(c: &mut Criterion) {
    let mut group = c.benchmark_group("id_deserialization_json");
    for size in [100, 1_000, 10_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let ids: Vec<_> = (0..size).map(|_| Id::new(IdType::Workspace)).collect();
            let json = serde_json::to_string(&ids).unwrap();

            b.iter(|| {
                black_box(serde_json::from_str::<Vec<Id>>(&json).unwrap());
            });
        });
    }
    group.finish();
}

fn bench_id_batch_serialization(c: &mut Criterion) {
    let mut group = c.benchmark_group("id_batch_serialization");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let ids: Vec<_> = (0..size).map(|_| Id::new(IdType::Workspace)).collect();
            let batch = IdBatch::new(&ids);

            b.iter(|| {
                black_box(batch.to_bytes());
            });
        });
    }
    group.finish();
}

fn bench_id_batch_deserialization(c: &mut Criterion) {
    let mut group = c.benchmark_group("id_batch_deserialization");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let ids: Vec<_> = (0..size).map(|_| Id::new(IdType::Workspace)).collect();
            let batch = IdBatch::new(&ids);
            let bytes = batch.to_bytes();

            b.iter(|| {
                black_box(IdBatch::from_bytes(&bytes).unwrap());
            });
        });
    }
    group.finish();
}

fn bench_id_parsing(c: &mut Criterion) {
    let mut group = c.benchmark_group("id_parsing");
    for size in [100, 1_000, 10_000, 50_000].iter() {
        group.bench_with_input(BenchmarkId::from_parameter(size), size, |b, &size| {
            let strings: Vec<_> = (0..size)
                .map(|_| Id::new(IdType::Workspace).to_string())
                .collect();

            b.iter(|| {
                for s in &strings {
                    black_box(s.parse::<Id>().unwrap());
                }
            });
        });
    }
    group.finish();
}

criterion_group!(
    benches,
    bench_id_creation,
    bench_id_to_bytes,
    bench_id_from_bytes,
    bench_id_serialization_json,
    bench_id_deserialization_json,
    bench_id_batch_serialization,
    bench_id_batch_deserialization,
    bench_id_parsing
);
criterion_main!(benches);
