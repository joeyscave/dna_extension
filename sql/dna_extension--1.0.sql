-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION dna_extension" to load this file. \quit

-- DNA Sequence Type
CREATE TYPE dna;

CREATE OR REPLACE FUNCTION dna_in(cstring)
RETURNS dna
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION dna_out(dna)
RETURNS cstring
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE dna (
    INTERNALLENGTH = VARIABLE,
    INPUT = dna_in,
    OUTPUT = dna_out,
    STORAGE = EXTENDED
);

-- K-mer Type
CREATE TYPE kmer;

CREATE OR REPLACE FUNCTION kmer_in(cstring)
RETURNS kmer
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION kmer_out(kmer)
RETURNS cstring
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE kmer (
    INTERNALLENGTH = VARIABLE,
    INPUT = kmer_in,
    OUTPUT = kmer_out,
    STORAGE = EXTENDED
);

-- Query K-mer Type
CREATE OR REPLACE FUNCTION qkmer_in(cstring)
RETURNS qkmer
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION qkmer_out(qkmer)
RETURNS cstring
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE qkmer (
    INTERNALLENGTH = VARIABLE,
    INPUT = qkmer_in,
    OUTPUT = qkmer_out,
    STORAGE = EXTENDED
);