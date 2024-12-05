-- Test script for DNA extension functions
\set ECHO all

-- Start with a clean state
DROP TABLE IF EXISTS test_dna;
DROP TABLE IF EXISTS test_kmers;
DROP EXTENSION IF EXISTS dna_extension CASCADE;
CREATE EXTENSION dna_extension;

-- Create test tables
CREATE TABLE test_dna (
    id serial primary key,
    sequence dna
);

CREATE TABLE test_kmers (
    id serial primary key,
    kmer kmer
);

-- Test data insertion
INSERT INTO test_dna (sequence) VALUES ('ACGTACGT');
INSERT INTO test_kmers (kmer) VALUES ('ACGT'), ('CGTA'), ('GTAC'), ('TACG');

-- Test length functions
SELECT 'Testing length functions' as test;

SELECT id, sequence, length(sequence) as len
FROM test_dna;

SELECT id, kmer, length(kmer) as len
FROM test_kmers;

SELECT length('ACGTN'::qkmer) as qkmer_len;

-- Test generate_kmers function
SELECT 'Testing generate_kmers function' as test;

-- Test with k=4
SELECT k.kmer
FROM generate_kmers('ACGTACGT'::dna, 4) AS k(kmer)

-- Test with k=3
SELECT k.kmer
FROM generate_kmers('ACGTACGT'::dna, 3) AS k(kmer)

-- Test error case: k > sequence length
SELECT k.kmer
FROM generate_kmers('ACGT'::dna, 5) AS k(kmer);

-- Test equals function and operator
SELECT 'Testing equals function and operator' as test;

-- Using function
SELECT id, kmer
FROM test_kmers
WHERE equals(kmer, 'ACGT'::kmer);

-- Using operator
SELECT id, kmer
FROM test_kmers
WHERE kmer = 'ACGT'::kmer;

-- Test starts_with function and operator
SELECT 'Testing starts_with function and operator' as test;

-- Using function
SELECT id, kmer
FROM test_kmers
WHERE starts_with('AC'::kmer, kmer);

-- Using operator
SELECT id, kmer
FROM test_kmers
WHERE kmer ^@ 'AC'::kmer;

-- Test error case: prefix longer than kmer
SELECT starts_with('ACGTX'::kmer, 'ACGT'::kmer);

-- Test contains function and operator
SELECT 'Testing contains function and operator' as test;

-- Using function
SELECT id, kmer
FROM test_kmers
WHERE contains('ACNT'::qkmer, kmer);

-- Using operator
SELECT id, kmer
FROM test_kmers
WHERE 'ACNT'::qkmer @> kmer;

-- Test error case: pattern and kmer length mismatch
SELECT contains('ACNTT'::qkmer, 'ACGT'::kmer);

-- Clean up
DROP TABLE IF EXISTS test_dna;
DROP TABLE IF EXISTS test_kmers;
DROP EXTENSION IF EXISTS dna_extension CASCADE;