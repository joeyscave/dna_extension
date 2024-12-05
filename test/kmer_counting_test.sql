-- Test k-mer counting functionality

-- Start with a clean state
DROP TABLE IF EXISTS dna_test;
DROP TABLE IF EXISTS kmer_test;
DROP EXTENSION IF EXISTS dna_extension CASCADE;
CREATE EXTENSION dna_extension;

-- Create test table
CREATE TABLE dna_test (
    id serial PRIMARY KEY,
    sequence dna
);

-- Insert test data
INSERT INTO dna_test (sequence) VALUES
    ('ACGTACGT'),
    ('ACGTACGTACGT'),
    ('AAACCCGGG');

-- Test k-mer counting
-- Count all 3-mers
SELECT k.kmer, count(*)
FROM dna_test d,
     generate_kmers(d.sequence, 3) AS k(kmer)
GROUP BY k.kmer
ORDER BY count(*) DESC;

-- Test k-mer statistics
WITH kmers AS (
    SELECT k.kmer, count(*) as count
    FROM dna_test d,
         generate_kmers(d.sequence, 3) AS k(kmer)
    GROUP BY k.kmer
)
SELECT sum(count) AS total_count,
       count(*) AS distinct_count,
       count(*) FILTER (WHERE count = 1) AS unique_count
FROM kmers;

-- Test k-mer hash functionality by creating a hash index
CREATE TABLE kmer_test (
    id serial PRIMARY KEY,
    kmer kmer
);

CREATE INDEX kmer_hash_idx ON kmer_test USING hash (kmer);

-- Insert some test k-mers
INSERT INTO kmer_test (kmer)
SELECT k.kmer
FROM generate_kmers('ACGTACGT', 3) AS k(kmer);

-- Test the hash index with a simple equality query
EXPLAIN ANALYZE
SELECT * FROM kmer_test WHERE kmer = 'ACG'::kmer;

-- Clean up
DROP TABLE IF EXISTS dna_test;
DROP TABLE IF EXISTS kmer_test;
DROP EXTENSION IF EXISTS dna_extension CASCADE;
