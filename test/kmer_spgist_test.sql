-- Comprehensive test script for SP-GiST k-mer index functionality
\set ECHO all

-- Create extension if not exists
DROP EXTENSION IF EXISTS dna_extension CASCADE;
CREATE EXTENSION dna_extension;

-- Create test table
DROP TABLE IF EXISTS test_kmers;
CREATE TABLE test_kmers (
    id serial PRIMARY KEY,
    sequence dna,
    kmer kmer
);

-- Insert test data
INSERT INTO test_kmers (sequence, kmer)
SELECT 
    'ACGTACGT'::dna AS sequence,
    k AS kmer
FROM generate_kmers('ACGTACGT'::dna, 4) k;

INSERT INTO test_kmers (sequence, kmer)
SELECT 
    'AAGGTTCC'::dna AS sequence,
    k AS kmer
FROM generate_kmers('AAGGTTCC'::dna, 4) k;

-- Create SP-GiST index
CREATE INDEX idx_kmer_spgist ON test_kmers USING spgist (kmer);

-- Test 1: Exact match queries
\echo 'Test 1: Exact match queries'
EXPLAIN ANALYZE
SELECT * FROM test_kmers WHERE kmer = 'ACGT'::kmer;

-- Test 2: Prefix match queries
\echo 'Test 2: Prefix match queries'
EXPLAIN ANALYZE
SELECT * FROM test_kmers WHERE kmer ^@ 'ACG'::kmer;

-- Test 3: Pattern match queries
\echo 'Test 3: Pattern match queries'
EXPLAIN ANALYZE
SELECT * FROM test_kmers WHERE 'ANGT'::qkmer @> kmer;

-- Test 4: Multiple pattern matches
\echo 'Test 4: Multiple pattern matches'
EXPLAIN ANALYZE
SELECT * FROM test_kmers WHERE 'NNGT'::qkmer @> kmer;

-- Test 5: No matches
\echo 'Test 5: No matches'
EXPLAIN ANALYZE
SELECT * FROM test_kmers WHERE kmer = 'TTTT'::kmer;

-- Test 6: Index usage verification
\echo 'Test 6: Index usage verification'
SET enable_seqscan = off;
EXPLAIN ANALYZE
SELECT * FROM test_kmers WHERE kmer = 'ACGT'::kmer;
SET enable_seqscan = on;

-- Test 7: Performance comparison
\echo 'Test 7: Performance comparison with sequential scan'
-- With index
EXPLAIN ANALYZE
SELECT * FROM test_kmers WHERE kmer ^@ 'ACG'::kmer;
-- Without index (force sequential scan)
SET enable_indexscan = off;
EXPLAIN ANALYZE
SELECT * FROM test_kmers WHERE kmer ^@ 'ACG'::kmer;
SET enable_indexscan = on;

-- Test 8: Complex pattern matching
\echo 'Test 8: Complex pattern matching'
EXPLAIN ANALYZE
SELECT * FROM test_kmers WHERE 'NNNN'::qkmer @> kmer;

-- Test 9: Edge cases
\echo 'Test 9: Edge cases'
-- Single nucleotide k-mer
SELECT * FROM test_kmers WHERE kmer = 'A'::kmer;
-- Pattern with all wildcards
SELECT * FROM test_kmers WHERE 'NNNN'::qkmer @> kmer;

-- Cleanup
DROP TABLE test_kmers;
DROP EXTENSION dna_extension;
