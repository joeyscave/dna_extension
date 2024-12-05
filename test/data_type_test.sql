-- Drop the tables first
DROP TABLE IF EXISTS dna_sequences;
DROP TABLE IF EXISTS kmers;
DROP TABLE IF EXISTS qkmers;

-- Finally drop the extension
DROP EXTENSION IF EXISTS dna_extension CASCADE;

-- First, create the extension
CREATE EXTENSION dna_extension;

-- Test DNA type
CREATE TABLE dna_sequences (
    id serial primary key,
    sequence dna
);

-- Test valid DNA sequences (should work)
INSERT INTO dna_sequences (sequence) VALUES ('ACGT');
INSERT INTO dna_sequences (sequence) VALUES ('GA');
INSERT INTO dna_sequences (sequence) VALUES ('gattaca');  -- lowercase should work too
INSERT INTO dna_sequences (sequence) VALUES ('ACGTACGTACGTACGT');  -- longer sequence
INSERT INTO dna_sequences (sequence) VALUES ('AAAAAAAAAAAAAAAAAAAA');  -- homopolymer
INSERT INTO dna_sequences (sequence) VALUES ('CGCGCGCGCG');  -- repeating pattern

-- Test invalid DNA sequences (should fail)
INSERT INTO dna_sequences (sequence) VALUES ('ACGTX');  -- invalid character X
INSERT INTO dna_sequences (sequence) VALUES ('AC GT');  -- space not allowed
INSERT INTO dna_sequences (sequence) VALUES ('AC-GT');  -- hyphen not allowed
INSERT INTO dna_sequences (sequence) VALUES ('1234');   -- numbers not allowed
INSERT INTO dna_sequences (sequence) VALUES ('');  -- empty sequence not allowed

-- Test K-mer type
CREATE TABLE kmers (
    id serial primary key,
    kmer_seq kmer
);

-- Test valid k-mers (should work)
INSERT INTO kmers (kmer_seq) VALUES ('ACGT');
INSERT INTO kmers (kmer_seq) VALUES ('GATTACA');
INSERT INTO kmers (kmer_seq) VALUES ('A');  -- single nucleotide
INSERT INTO kmers (kmer_seq) VALUES ('ACGTACGTACGTACGT');  -- 16-mer
INSERT INTO kmers (kmer_seq) VALUES ('AAAAAAAAAA');  -- homopolymer 10-mer
INSERT INTO kmers (kmer_seq) VALUES ('CGCGCG');  -- alternating pattern

-- Test invalid k-mers (should fail)
INSERT INTO kmers (kmer_seq) VALUES ('AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA');  -- too long
INSERT INTO kmers (kmer_seq) VALUES ('AC GT');  -- space not allowed
INSERT INTO kmers (kmer_seq) VALUES ('ACGTX');  -- invalid character
INSERT INTO kmers (kmer_seq) VALUES ('');  -- empty sequence not allowed

-- Test Query K-mer type
CREATE TABLE qkmers (
    id serial primary key,
    pattern qkmer
);

-- Test valid query k-mers (should work)
-- Basic patterns (standard bases)
INSERT INTO qkmers (pattern) VALUES ('ACGT');  -- standard bases uppercase
INSERT INTO qkmers (pattern) VALUES ('acgt');  -- standard bases lowercase
INSERT INTO qkmers (pattern) VALUES ('AcGt');  -- standard bases mixed case

-- Single ambiguity code tests (not at start)
INSERT INTO qkmers (pattern) VALUES ('AAGT');  -- no ambiguity
INSERT INTO qkmers (pattern) VALUES ('ANGT');  -- single N
INSERT INTO qkmers (pattern) VALUES ('ARGT');  -- single R (A/G)
INSERT INTO qkmers (pattern) VALUES ('AYGT');  -- single Y (C/T)
INSERT INTO qkmers (pattern) VALUES ('AMGT');  -- single M (A/C)
INSERT INTO qkmers (pattern) VALUES ('AKGT');  -- single K (G/T)
INSERT INTO qkmers (pattern) VALUES ('ASGT');  -- single S (G/C)
INSERT INTO qkmers (pattern) VALUES ('AWGT');  -- single W (A/T)

-- Single three-base codes (not at start)
INSERT INTO qkmers (pattern) VALUES ('ABGT');  -- single B (C/G/T)
INSERT INTO qkmers (pattern) VALUES ('ADGT');  -- single D (A/G/T)
INSERT INTO qkmers (pattern) VALUES ('AHGT');  -- single H (A/C/T)
INSERT INTO qkmers (pattern) VALUES ('AVGT');  -- single V (A/C/G)

-- Safe patterns with two ambiguity codes
INSERT INTO qkmers (pattern) VALUES ('ACGTA');  -- control pattern
INSERT INTO qkmers (pattern) VALUES ('ACNTA');  -- N with spacing
INSERT INTO qkmers (pattern) VALUES ('ACRTA');  -- R with spacing
INSERT INTO qkmers (pattern) VALUES ('ACYTA');  -- Y with spacing
INSERT INTO qkmers (pattern) VALUES ('ACMTA');  -- M with spacing
INSERT INTO qkmers (pattern) VALUES ('ACKTA');  -- K with spacing
-- Known working longer patterns
INSERT INTO qkmers (pattern) VALUES ('ACGTRYMK');  -- 8-mer with two-base codes
INSERT INTO qkmers (pattern) VALUES ('GTACBDHV');  -- 8-mer with three-base codes
INSERT INTO qkmers (pattern) VALUES ('B');  -- 8-mer with three-base codes

-- Test invalid query k-mers (should fail)
INSERT INTO qkmers (pattern) VALUES ('ACGTX');  -- invalid character X
INSERT INTO qkmers (pattern) VALUES ('AC GT');  -- space not allowed
INSERT INTO qkmers (pattern) VALUES ('AC-GT');  -- hyphen not allowed
INSERT INTO qkmers (pattern) VALUES ('ACGT?');  -- invalid wildcard character
INSERT INTO qkmers (pattern) VALUES ('');  -- empty sequence not allowed
INSERT INTO qkmers (pattern) VALUES ('ACGT_');  -- underscore not allowed
INSERT INTO qkmers (pattern) VALUES ('ACGT.');  -- dot not allowed

-- View the results
SELECT * FROM dna_sequences;
SELECT * FROM kmers;
SELECT * FROM qkmers ORDER BY id;

-- Clean up
DROP TABLE IF EXISTS dna_sequences;
DROP TABLE IF EXISTS kmers;
DROP TABLE IF EXISTS qkmers;
DROP EXTENSION IF EXISTS dna_extension CASCADE;