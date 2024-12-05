#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "catalog/pg_type.h"
#include "funcapi.h"
#include <string.h>

// Function declarations
PG_FUNCTION_INFO_V1(length_dna);
PG_FUNCTION_INFO_V1(length_kmer);
PG_FUNCTION_INFO_V1(length_qkmer);
PG_FUNCTION_INFO_V1(generate_kmers);
PG_FUNCTION_INFO_V1(equals_kmer);
PG_FUNCTION_INFO_V1(starts_with_kmer);
PG_FUNCTION_INFO_V1(contains_kmer);

// Length functions
Datum
length_dna(PG_FUNCTION_ARGS)
{
    text *dna_text = PG_GETARG_TEXT_PP(0);
    int32 result = VARSIZE_ANY_EXHDR(dna_text);
    PG_RETURN_INT32(result);
}

Datum
length_kmer(PG_FUNCTION_ARGS)
{
    text *kmer_text = PG_GETARG_TEXT_PP(0);
    int32 result = VARSIZE_ANY_EXHDR(kmer_text);
    PG_RETURN_INT32(result);
}

Datum
length_qkmer(PG_FUNCTION_ARGS)
{
    text *qkmer_text = PG_GETARG_TEXT_PP(0);
    int32 result = VARSIZE_ANY_EXHDR(qkmer_text);
    PG_RETURN_INT32(result);
}

// Generate k-mers function
typedef struct {
    char *dna_seq;
    int k;
    int curr_pos;
} generate_kmers_fctx;

Datum
generate_kmers(PG_FUNCTION_ARGS)
{
    FuncCallContext *funcctx;
    generate_kmers_fctx *fctx;
    
    if (SRF_IS_FIRSTCALL())
    {
        MemoryContext oldcontext;
        text *dna_text = PG_GETARG_TEXT_PP(0);
        int k = PG_GETARG_INT32(1);
        char *dna_seq = text_to_cstring(dna_text);
        int seq_len = strlen(dna_seq);
        
        if (k <= 0 || k > seq_len)
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("k must be positive and not greater than sequence length")));
        
        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
        
        fctx = (generate_kmers_fctx *) palloc(sizeof(generate_kmers_fctx));
        fctx->dna_seq = pstrdup(dna_seq);
        fctx->k = k;
        fctx->curr_pos = 0;
        
        funcctx->user_fctx = fctx;
        funcctx->max_calls = seq_len - k + 1;
        
        MemoryContextSwitchTo(oldcontext);
    }
    
    funcctx = SRF_PERCALL_SETUP();
    fctx = funcctx->user_fctx;
    
    if (funcctx->call_cntr < funcctx->max_calls)
    {
        char *kmer = palloc(fctx->k + 1);
        memcpy(kmer, fctx->dna_seq + fctx->curr_pos, fctx->k);
        kmer[fctx->k] = '\0';
        fctx->curr_pos++;
        
        text *result = cstring_to_text(kmer);
        pfree(kmer);
        
        SRF_RETURN_NEXT(funcctx, PointerGetDatum(result));
    }
    else
    {
        pfree(fctx->dna_seq);
        pfree(fctx);
        SRF_RETURN_DONE(funcctx);
    }
}

// Equals function
Datum
equals_kmer(PG_FUNCTION_ARGS)
{
    text *kmer1 = PG_GETARG_TEXT_PP(0);
    text *kmer2 = PG_GETARG_TEXT_PP(1);
    
    int len1 = VARSIZE_ANY_EXHDR(kmer1);
    int len2 = VARSIZE_ANY_EXHDR(kmer2);
    
    if (len1 != len2)
        PG_RETURN_BOOL(false);
    
    bool result = (memcmp(VARDATA_ANY(kmer1), VARDATA_ANY(kmer2), len1) == 0);
    PG_RETURN_BOOL(result);
}

// Starts with function
Datum
starts_with_kmer(PG_FUNCTION_ARGS)
{
    text *prefix = PG_GETARG_TEXT_PP(0);
    text *kmer = PG_GETARG_TEXT_PP(1);
    
    int prefix_len = VARSIZE_ANY_EXHDR(prefix);
    int kmer_len = VARSIZE_ANY_EXHDR(kmer);
    
    if (prefix_len > kmer_len)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("prefix length cannot be greater than kmer length")));
    
    bool result = (memcmp(VARDATA_ANY(prefix), VARDATA_ANY(kmer), prefix_len) == 0);
    PG_RETURN_BOOL(result);
}

// Contains function
Datum
contains_kmer(PG_FUNCTION_ARGS)
{
    text *pattern = PG_GETARG_TEXT_PP(0);
    text *kmer = PG_GETARG_TEXT_PP(1);
    
    int pattern_len = VARSIZE_ANY_EXHDR(pattern);
    int kmer_len = VARSIZE_ANY_EXHDR(kmer);
    
    if (pattern_len != kmer_len)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("pattern and kmer must have the same length")));
    
    char *pattern_str = VARDATA_ANY(pattern);
    char *kmer_str = VARDATA_ANY(kmer);
    
    bool matches = true;
    for (int i = 0; i < pattern_len; i++)
    {
        if (pattern_str[i] != 'N' && pattern_str[i] != kmer_str[i])
        {
            matches = false;
            break;
        }
    }
    
    PG_RETURN_BOOL(matches);
}
