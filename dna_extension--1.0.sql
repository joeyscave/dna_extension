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

-- Length functions
CREATE OR REPLACE FUNCTION length(dna)
RETURNS integer
AS '$libdir/dna_extension', 'length_dna'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION length(kmer)
RETURNS integer
AS '$libdir/dna_extension', 'length_kmer'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION length(qkmer)
RETURNS integer
AS '$libdir/dna_extension', 'length_qkmer'
LANGUAGE C IMMUTABLE STRICT;

-- Generate k-mers function
CREATE OR REPLACE FUNCTION generate_kmers(sequence dna, k integer)
RETURNS SETOF kmer
AS '$libdir/dna_extension', 'generate_kmers'
LANGUAGE C IMMUTABLE STRICT;

-- Equals function and operator
CREATE OR REPLACE FUNCTION equals(k1 kmer, k2 kmer)
RETURNS boolean
AS '$libdir/dna_extension', 'equals_kmer'
LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR = (
    LEFTARG = kmer,
    RIGHTARG = kmer,
    PROCEDURE = equals,
    COMMUTATOR = =
);

-- Starts with function and operator
CREATE OR REPLACE FUNCTION starts_with(prefix kmer, k kmer)
RETURNS boolean
AS '$libdir/dna_extension', 'starts_with_kmer'
LANGUAGE C IMMUTABLE STRICT;

-- Function that swaps the arguments of starts_with
CREATE OR REPLACE FUNCTION starts_with_reversed(k kmer, prefix kmer)
RETURNS boolean AS $$
BEGIN
    RETURN starts_with(prefix, k);
END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;

CREATE OPERATOR ^@ (
    LEFTARG = kmer,
    RIGHTARG = kmer,
    PROCEDURE = starts_with_reversed
);

-- Contains function and operator
CREATE OR REPLACE FUNCTION contains(pattern qkmer, k kmer)
RETURNS boolean
AS '$libdir/dna_extension', 'contains_kmer'
LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR @> (
    LEFTARG = qkmer,
    RIGHTARG = kmer,
    PROCEDURE = contains
);

-- Hash index support for efficient Group
CREATE FUNCTION hash_kmer(kmer)
RETURNS integer
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR CLASS kmer_hash_ops
DEFAULT FOR TYPE kmer USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   hash_kmer(kmer);

-- SP-GiST support functions
CREATE FUNCTION kmer_spgist_config(internal, internal)
RETURNS void
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION kmer_spgist_choose(internal, internal)
RETURNS void
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION kmer_spgist_picksplit(internal, internal)
RETURNS void
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION kmer_spgist_inner_consistent(internal, internal)
RETURNS void
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION kmer_spgist_leaf_consistent(internal, internal)
RETURNS bool
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION kmer_spgist_compress(internal)
RETURNS internal
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION kmer_spgist_decompress(internal)
RETURNS internal
AS '$libdir/dna_extension'
LANGUAGE C IMMUTABLE STRICT;

-- SP-GiST index support
CREATE OPERATOR CLASS kmer_spgist_ops
DEFAULT FOR TYPE kmer USING spgist AS
    OPERATOR    1   = (kmer, kmer),
    OPERATOR    2   ^@ (kmer, kmer),
    OPERATOR    3   @> (qkmer, kmer),
    FUNCTION    1   kmer_spgist_config(internal, internal),
    FUNCTION    2   kmer_spgist_choose(internal, internal),
    FUNCTION    3   kmer_spgist_picksplit(internal, internal),
    FUNCTION    4   kmer_spgist_inner_consistent(internal, internal),
    FUNCTION    5   kmer_spgist_leaf_consistent(internal, internal);
    -- FUNCTION    6   kmer_spgist_compress(internal),
    -- FUNCTION    7   kmer_spgist_decompress(internal);