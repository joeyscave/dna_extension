MODULES = dna_extension
EXTENSION = dna_extension
DATA = dna_extension--1.0.sql

MODULE_big = dna_extension
OBJS = src/data_type.o src/functions.o src/kmer_hash.o src/kmer_spgist.o
PG_CPPFLAGS = -I$(libpq_srcdir)

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)