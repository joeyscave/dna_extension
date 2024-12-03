#include "postgres.h"
#include "access/spgist.h"
#include "access/stratnum.h"
#include "utils/builtins.h"
#include "catalog/pg_type.h"

PG_MODULE_MAGIC;



// Input and output functions for the kmer type
PG_FUNCTION_INFO_V1(kmer_in);
Datum kmer_in(PG_FUNCTION_ARGS) {
    char *str = PG_GETARG_CSTRING(0);
    if (strlen(str) > 32) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                        errmsg("k-mer length must not exceed 32")));
    }
    Kmer *kmer = palloc(sizeof(Kmer));
    strncpy(kmer->value, str, 32);
    PG_RETURN_POINTER(kmer);
}

PG_FUNCTION_INFO_V1(kmer_out);
Datum kmer_out(PG_FUNCTION_ARGS) {
    Kmer *kmer = (Kmer *) PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING(pstrdup(kmer->value));
}

// Equality operator
PG_FUNCTION_INFO_V1(kmer_eq);
Datum kmer_eq(PG_FUNCTION_ARGS) {
    Kmer *a = (Kmer *) PG_GETARG_POINTER(0);
    Kmer *b = (Kmer *) PG_GETARG_POINTER(1);
    PG_RETURN_BOOL(strcmp(a->value, b->value) == 0);
}

// SP-GiST choose method
PG_FUNCTION_INFO_V1(spgist_kmer_choose);
Datum spgist_kmer_choose(PG_FUNCTION_ARGS) {
    SpGistChooseResult *result = (SpGistChooseResult *) palloc(sizeof(SpGistChooseResult));
    Kmer *kmer = (Kmer *) DatumGetPointer(PG_GETARG_DATUM(0));
    uint32 level = PG_GETARG_UINT32(1);

    if (level < strlen(kmer->value)) {
        result->resultType = SpGistTypeInner;
        result->nextLevel = level + 1;
        result->chosen = kmer->value[level];
    } else {
        result->resultType = SpGistTypeLeaf;
    }
    PG_RETURN_POINTER(result);
}

// SP-GiST picksplit method
PG_FUNCTION_INFO_V1(spgist_kmer_picksplit);
Datum spgist_kmer_picksplit(PG_FUNCTION_ARGS) {
    SpGistPickSplitResult *result = (SpGistPickSplitResult *) palloc(sizeof(SpGistPickSplitResult));
    SpGistSplitTuple *splits;
    int i;

    int nTuples = PG_GETARG_INT32(2); // Number of tuples
    SpGistLeafTuple *tuples = PG_GETARG_POINTER(1); // Tuple data

    // Allocate space for result splits
    splits = palloc(nTuples * sizeof(SpGistSplitTuple));

    // Simple character-level splitting at the first differing character
    for (i = 0; i < nTuples; i++) {
        Kmer *kmer = (Kmer *) DatumGetPointer(tuples[i]->value);
        int level = 0;
        while (level < strlen(kmer->value) && kmer->value[level] != '\0') {
            splits[i].prefixChar = kmer->value[level];
            level++;
        }
        splits[i].value = tuples[i]->value;
    }

    result->splits = splits;
    result->nNodes = nTuples; // Each split creates a node
    PG_RETURN_POINTER(result);
}

// SP-GiST inner consistent method
PG_FUNCTION_INFO_V1(spgist_kmer_inner_consistent);
Datum spgist_kmer_inner_consistent(PG_FUNCTION_ARGS) {
    SpGistInnerConsistentResult *result = (SpGistInnerConsistentResult *) palloc(sizeof(SpGistInnerConsistentResult));
    MemoryContext oldCtx;
    Kmer *query = (Kmer *) DatumGetPointer(PG_GETARG_DATUM(0));
    uint32 level = PG_GETARG_UINT32(1);

    oldCtx = MemoryContextSwitchTo(CurrentMemoryContext);

    // Assume every branch is potentially consistent until deeper checks are performed
    result->nNodes = 1;
    result->nodeNumbers = palloc(sizeof(int));
    result->nodeNumbers[0] = query->value[level];
    MemoryContextSwitchTo(oldCtx);

    PG_RETURN_POINTER(result);
}