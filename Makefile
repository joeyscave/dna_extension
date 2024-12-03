MODULES = dna_extension
EXTENSION = dna_extension
DATA = sql/dna_extension--1.0.sql

MODULE_big = dna_extension
OBJS = src/dna_extension.o src/index.o
PG_CPPFLAGS = -I$(libpq_srcdir)

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)