#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "access/hash.h"
#include <string.h>

PG_FUNCTION_INFO_V1(hash_kmer);

// Hash function for hash index support
Datum
hash_kmer(PG_FUNCTION_ARGS)
{
    text *kmer = PG_GETARG_TEXT_PP(0);
    char *str = text_to_cstring(kmer);
    Datum result = hash_any((unsigned char *)str, strlen(str));
    pfree(str);
    PG_RETURN_DATUM(result);
}
