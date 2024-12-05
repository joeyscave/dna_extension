#include "postgres.h"

#include "access/spgist.h"
#include "utils/builtins.h"
#include "utils/datum.h"
#include "utils/memutils.h"
#include "catalog/pg_type.h"

/* Node structure for the trie */
typedef struct KmerNode
{
    char letter;         /* Current letter in the k-mer */
    int level;          /* Level in the trie (position in k-mer) */
} KmerNode;

/* Functions for SP-GiST interface */
Datum kmer_spgist_config(PG_FUNCTION_ARGS);
Datum kmer_spgist_choose(PG_FUNCTION_ARGS);
Datum kmer_spgist_picksplit(PG_FUNCTION_ARGS);
Datum kmer_spgist_inner_consistent(PG_FUNCTION_ARGS);
Datum kmer_spgist_leaf_consistent(PG_FUNCTION_ARGS);
Datum kmer_spgist_compress(PG_FUNCTION_ARGS);
Datum kmer_spgist_decompress(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(kmer_spgist_config);
PG_FUNCTION_INFO_V1(kmer_spgist_choose);
PG_FUNCTION_INFO_V1(kmer_spgist_picksplit);
PG_FUNCTION_INFO_V1(kmer_spgist_inner_consistent);
PG_FUNCTION_INFO_V1(kmer_spgist_leaf_consistent);
PG_FUNCTION_INFO_V1(kmer_spgist_compress);
PG_FUNCTION_INFO_V1(kmer_spgist_decompress);

/* Configuration function */
Datum
kmer_spgist_config(PG_FUNCTION_ARGS)
{
    spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
    
    cfg->prefixType = TEXTOID;  /* We'll store prefixes as text */
    cfg->labelType = TEXTOID;   /* Node labels are text */
    cfg->canReturnData = true;  /* We can store data in leaf nodes */
    cfg->longValuesOK = true;   /* Allow long k-mers */
    
    PG_RETURN_VOID();
}

/* Choose function - determines which node to traverse */
Datum
kmer_spgist_choose(PG_FUNCTION_ARGS)
{
    spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
    spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
    char *kmer = VARDATA_ANY(in->datum);
    KmerNode *node;
    
    if (in->allTheSame)
    {
        out->resultType = spgMatchNode;
        out->result.matchNode.nodeN = 0;
        PG_RETURN_VOID();
    }
    
    node = (KmerNode *) DatumGetPointer(in->nodeLabels[0]);
    if (node == NULL)
    {
        /* Root node case */
        out->resultType = spgMatchNode;
        out->result.matchNode.nodeN = (int) kmer[0] - 'A';
    }
    else
    {
        /* Non-root node case */
        out->resultType = spgMatchNode;
        out->result.matchNode.nodeN = (int) kmer[node->level + 1] - 'A';
    }
    
    PG_RETURN_VOID();
}

/* Split function - handles node splits */
Datum
kmer_spgist_picksplit(PG_FUNCTION_ARGS)
{
    spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
    spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
    int i;
    
    /* Initialize output */
    out->nNodes = 4;  /* One node for each possible DNA base */
    out->nodeLabels = palloc(sizeof(Datum) * out->nNodes);
    out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
    out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);
    
    /* Create node labels */
    for (i = 0; i < 4; i++)
    {
        KmerNode *node = palloc(sizeof(KmerNode));
        node->letter = 'A' + i;
        node->level = in->level;
        out->nodeLabels[i] = PointerGetDatum(node);
    }
    
    /* Map tuples to nodes */
    for (i = 0; i < in->nTuples; i++)
    {
        char *kmer = VARDATA_ANY(in->datums[i]);
        out->mapTuplesToNodes[i] = (int) kmer[in->level] - 'A';
        out->leafTupleDatums[i] = in->datums[i];
    }
    
    PG_RETURN_VOID();
}

/* Inner consistent function - handles non-leaf node traversal */
Datum
kmer_spgist_inner_consistent(PG_FUNCTION_ARGS)
{
    spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
    spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
    int i;
    
    out->nNodes = 0;
    out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
    
    for (i = 0; i < in->nNodes; i++)
    {
        KmerNode *node = (KmerNode *) DatumGetPointer(in->nodeLabels[i]);
        bool match = true;
        int j;
        
        /* Check each qual */
        for (j = 0; j < in->nkeys; j++)
        {
            Datum query = in->scankeys[j].sk_argument;
            char *queryStr = VARDATA_ANY(query);
            
            switch (in->scankeys[j].sk_strategy)
            {
                case 1:  /* Equality */
                    if (node->level < VARSIZE_ANY_EXHDR(query) &&
                        queryStr[node->level] != node->letter)
                        match = false;
                    break;
                case 2:  /* Prefix match */
                    if (node->level < VARSIZE_ANY_EXHDR(query) &&
                        queryStr[node->level] != node->letter)
                        match = false;
                    break;
                case 3:  /* Pattern match */
                    if (node->level < VARSIZE_ANY_EXHDR(query) &&
                        queryStr[node->level] != 'N' &&
                        queryStr[node->level] != node->letter)
                        match = false;
                    break;
            }
            
            if (!match)
                break;
        }
        
        if (match)
            out->nodeNumbers[out->nNodes++] = i;
    }
    
    PG_RETURN_VOID();
}

/* Leaf consistent function - handles leaf node matching */
Datum
kmer_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
    spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
    spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
    char *kmer = VARDATA_ANY(in->leafDatum);
    bool match = true;
    int i;
    
    /* Check each qual */
    for (i = 0; i < in->nkeys; i++)
    {
        Datum query = in->scankeys[i].sk_argument;
        char *queryStr = VARDATA_ANY(query);
        int queryLen = VARSIZE_ANY_EXHDR(query);
        int kmerLen = VARSIZE_ANY_EXHDR(in->leafDatum);
        
        switch (in->scankeys[i].sk_strategy)
        {
            case 1:  /* Equality */
                if (queryLen != kmerLen || memcmp(queryStr, kmer, queryLen) != 0)
                    match = false;
                break;
            case 2:  /* Prefix match */
                if (queryLen > kmerLen || memcmp(queryStr, kmer, queryLen) != 0)
                    match = false;
                break;
            case 3:  /* Pattern match */
                if (queryLen != kmerLen)
                {
                    match = false;
                    break;
                }
                for (int j = 0; j < queryLen; j++)
                {
                    if (queryStr[j] != 'N' && queryStr[j] != kmer[j])
                    {
                        match = false;
                        break;
                    }
                }
                break;
        }
        
        if (!match)
            break;
    }
    
    out->recheck = false;
    
    PG_RETURN_BOOL(match);
}

/* Compress function - compresses leaf data */
Datum
kmer_spgist_compress(PG_FUNCTION_ARGS)
{
    Datum datum = PG_GETARG_DATUM(0);
    PG_RETURN_DATUM(datum);
}

/* Decompress function - decompresses leaf data */
Datum
kmer_spgist_decompress(PG_FUNCTION_ARGS)
{
    Datum datum = PG_GETARG_DATUM(0);
    PG_RETURN_DATUM(datum);
}
