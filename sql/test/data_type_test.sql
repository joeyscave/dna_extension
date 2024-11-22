-- Drop the tables first
DROP TABLE IF EXISTS dna_sequences;
DROP TABLE IF EXISTS kmers;
DROP TABLE IF EXISTS qkmers;

-- Finally drop the extension
DROP EXTENSION IF EXISTS dna_extension;

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

-- Test invalid DNA sequence (should fail)
INSERT INTO dna_sequences (sequence) VALUES ('ACGTX');

-- Test K-mer type
CREATE TABLE kmers (
    id serial primary key,
    kmer_seq kmer
);

-- Test valid k-mers (should work)
INSERT INTO kmers (kmer_seq) VALUES ('ACGT');
INSERT INTO kmers (kmer_seq) VALUES ('GATTACA');

-- Test invalid k-mer (too long, should fail)
INSERT INTO kmers (kmer_seq) VALUES ('AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA');

-- Test Query K-mer type
CREATE TABLE qkmers (
    id serial primary key,
    pattern qkmer
);

-- Test valid query k-mers (should work)
INSERT INTO qkmers (pattern) VALUES ('ACGT');
INSERT INTO qkmers (pattern) VALUES ('ANGTA');
INSERT INTO qkmers (pattern) VALUES ('KMBD');

-- Test invalid query k-mer (should fail)
INSERT INTO qkmers (pattern) VALUES ('ACGTX');

-- View the results
SELECT * FROM dna_sequences;
SELECT * FROM kmers;
SELECT * FROM qkmers;