# DNA Extension for PostgreSQL

A PostgreSQL extension for efficient DNA sequence storage and k-mer based searching. This extension implements custom data types and indexing methods optimized for DNA sequence operations.

The extension provides functions for DNA sequence manipulation (`length()`, `generate_kmers()`), k-mer comparison operations (`=`, `^@`, `@>` operators), and indexing support (Hash and SP-GiST) for efficient searching and grouping operations.

## Features

- Custom `dna` data type for storing DNA sequences
- Custom `kmer` data type for k-mer representation
- Custom `qkmer` data type for query k-mer representation
- Various DNA sequence manipulation functions
- Hash index support for efficient k-mer grouping
- SP-GiST index support for efficient k-mer searching

## Prerequisites

- PostgreSQL development environment
- GCC compiler
- PostgreSQL server (version 15 or higher recommended)

## Installation

1. Clone the repository:
```bash
git clone https://github.com/joeyscave/dna_extension.git
cd dna_extension
```

2. Build and install the extension:
```bash
make clean install
```

3. Create the extension in your PostgreSQL database:
```sql
CREATE EXTENSION dna_extension;
```

## Example Usage

### Basic DNA Sequence Operations
```sql
-- Create a table for DNA sequences
CREATE TABLE sequences (
    id SERIAL PRIMARY KEY,
    sequence dna
);

-- Insert DNA sequences
INSERT INTO sequences (sequence) VALUES ('ATCG');
INSERT INTO sequences (sequence) VALUES ('GCTA');

-- Get sequence lengths
SELECT id, sequence, length(sequence) FROM sequences;
```

### K-mer Generation and Storage
```sql
-- Create a table for k-mers
CREATE TABLE kmers (
    id SERIAL PRIMARY KEY,
    seq_id INTEGER REFERENCES sequences(id),
    kmer_seq kmer
);

-- Generate and store 3-mers from sequences
INSERT INTO kmers (seq_id, kmer_seq)
SELECT s.id, k.kmer_seq
FROM sequences s,
     generate_kmers(s.sequence, 3) AS k;

-- Create indexes for efficient searching
CREATE INDEX kmer_hash_idx ON kmers USING hash (kmer_seq);
CREATE INDEX kmer_spgist_idx ON kmers USING spgist (kmer_seq);
```

### Pattern Matching Queries
```sql
-- Find k-mers starting with 'AT'
SELECT * FROM kmers WHERE kmer_seq ^@ 'AT'::kmer;

-- Find k-mers containing 'CG'
SELECT * FROM kmers WHERE 'CG'::qkmer @> kmer_seq;

-- Group similar k-mers
SELECT kmer_seq, COUNT(*) 
FROM kmers 
GROUP BY kmer_seq 
HAVING COUNT(*) > 1;
```

## Test Suite

The extension includes comprehensive test files in the `test/` directory:

1. **data_type_test.sql**
   - Tests for DNA, k-mer, and query k-mer data types
   - Input/output function validation
   - Type conversion and validation tests

2. **functions_test.sql**
   - Tests for sequence manipulation functions
   - Length calculation validation
   - K-mer generation tests

3. **kmer_counting_test.sql**
   - Tests for k-mer frequency analysis
   - Group by operations with hash index

4. **kmer_spgist_test.sql**
   - SP-GiST index functionality tests
   - Prefix matching performance tests
   - Pattern matching validation

## Project Structure

- `src/`: Source code implementation
  - `data_type.c`: DNA data type implementation
  - `functions.c`: DNA analysis functions
  - `kmer_hash.c`: K-mer hashing functionality
  - `kmer_spgist.c`: SP-GiST index implementation
- `test/`: Test files
- `dna_extension--1.0.sql`: SQL function definitions
- `dna_extension.control`: Extension control file
- `Makefile`: Build configuration
