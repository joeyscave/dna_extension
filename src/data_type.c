#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "libpq/pqformat.h"
#include <string.h>
#include <ctype.h>
#include "utils/elog.h"

PG_MODULE_MAGIC;

// Function declarations
PG_FUNCTION_INFO_V1(dna_in);
PG_FUNCTION_INFO_V1(dna_out);
PG_FUNCTION_INFO_V1(kmer_in);
PG_FUNCTION_INFO_V1(kmer_out);
PG_FUNCTION_INFO_V1(qkmer_in);
PG_FUNCTION_INFO_V1(qkmer_out);

// Helper function to validate DNA sequence
static void validate_dna_sequence(const char *str, bool allow_iupac) {
    elog(DEBUG1, "validate_dna_sequence: validating sequence '%s' with IUPAC mode %s", 
         str, allow_iupac ? "true" : "false");
    
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        char c = toupper(str[i]);
        if (allow_iupac) {
            // IUPAC nucleotide codes
            if (strchr("ACGTUWSMKRYBDHVN", c) == NULL) {
                elog(WARNING, "validate_dna_sequence: invalid IUPAC nucleotide code '%c' at position %d", 
                     str[i], i);
                ereport(ERROR,
                    (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                     errmsg("invalid IUPAC nucleotide code: %c", str[i])));
            }
        } else {
            // Standard nucleotides only
            if (strchr("ACGT", c) == NULL) {
                elog(WARNING, "validate_dna_sequence: invalid nucleotide '%c' at position %d", 
                     str[i], i);
                ereport(ERROR,
                    (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                     errmsg("invalid nucleotide: %c", str[i])));
            }
        }
    }
    elog(DEBUG1, "validate_dna_sequence: validation successful");
}

Datum
dna_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    elog(DEBUG1, "dna_in: processing input sequence '%s'", str);
    
    // Check for empty input
    if (str == NULL || *str == '\0') {
        elog(WARNING, "dna_in: empty input sequence");
        ereport(ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
             errmsg("DNA sequence cannot be empty")));
    }
    
    // Validate DNA sequence
    validate_dna_sequence(str, false);
    
    // Calculate required size
    int len = strlen(str);
    text *result = (text *) palloc(VARHDRSZ + len);
    SET_VARSIZE(result, VARHDRSZ + len);
    
    // Copy data and convert to uppercase
    char *dst = VARDATA(result);
    for (int i = 0; i < len; i++) {
        dst[i] = toupper(str[i]);
    }
    
    elog(DEBUG1, "dna_in: returning processed sequence of length %d", len);
    PG_RETURN_TEXT_P(result);
}

Datum
dna_out(PG_FUNCTION_ARGS)
{
    text *t = PG_GETARG_TEXT_PP(0);
    char *result = palloc(VARSIZE_ANY_EXHDR(t) + 1);
    memcpy(result, VARDATA_ANY(t), VARSIZE_ANY_EXHDR(t));
    result[VARSIZE_ANY_EXHDR(t)] = '\0';
    elog(DEBUG1, "dna_out: returning output sequence '%s'", result);
    PG_RETURN_CSTRING(result);
}

// K-mer type functions
Datum
kmer_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    elog(DEBUG1, "kmer_in: processing input k-mer '%s'", str);
    
    // Check for empty input
    if (str == NULL || *str == '\0') {
        elog(WARNING, "kmer_in: empty input k-mer");
        ereport(ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
             errmsg("k-mer sequence cannot be empty")));
    }
    
    // Check length constraint (max 32)
    if (strlen(str) > 32) {
        elog(WARNING, "kmer_in: k-mer too long: %zu", strlen(str));
        ereport(ERROR,
            (errcode(ERRCODE_STRING_DATA_RIGHT_TRUNCATION),
             errmsg("k-mer sequence too long (maximum length is 32)")));
    }
    
    // Validate DNA sequence
    validate_dna_sequence(str, false);
    
    // Calculate required size
    int len = strlen(str);
    text *result = (text *) palloc(VARHDRSZ + len);
    SET_VARSIZE(result, VARHDRSZ + len);
    
    // Copy data and convert to uppercase
    char *dst = VARDATA(result);
    for (int i = 0; i < len; i++) {
        dst[i] = toupper(str[i]);
    }
    
    elog(DEBUG1, "kmer_in: returning processed k-mer of length %d", len);
    PG_RETURN_TEXT_P(result);
}

Datum
kmer_out(PG_FUNCTION_ARGS)
{
    text *t = PG_GETARG_TEXT_PP(0);
    char *result = palloc(VARSIZE_ANY_EXHDR(t) + 1);
    memcpy(result, VARDATA_ANY(t), VARSIZE_ANY_EXHDR(t));
    result[VARSIZE_ANY_EXHDR(t)] = '\0';
    elog(DEBUG1, "kmer_out: returning output k-mer '%s'", result);
    PG_RETURN_CSTRING(result);
}

// Query K-mer type functions
Datum
qkmer_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    elog(DEBUG1, "qkmer_in: processing input query k-mer '%s'", str);
    
    // Check for empty input
    if (str == NULL || *str == '\0') {
        elog(WARNING, "qkmer_in: empty input query k-mer");
        ereport(ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
             errmsg("query k-mer sequence cannot be empty")));
    }
    
    // Check length constraint (max 32)
    if (strlen(str) > 32) {
        elog(WARNING, "qkmer_in: query k-mer too long: %zu", strlen(str));
        ereport(ERROR,
            (errcode(ERRCODE_STRING_DATA_RIGHT_TRUNCATION),
             errmsg("query k-mer sequence too long (maximum length is 32)")));
    }
    
    // Validate DNA sequence
    validate_dna_sequence(str, true);  // Allow IUPAC codes
    
    // Calculate required size
    int len = strlen(str);
    text *result = (text *) palloc(VARHDRSZ + len);
    SET_VARSIZE(result, VARHDRSZ + len);
    
    // Copy data and convert to uppercase
    char *dst = VARDATA(result);
    for (int i = 0; i < len; i++) {
        dst[i] = toupper(str[i]);
    }
    
    elog(DEBUG1, "qkmer_in: returning processed query k-mer of length %d", len);
    PG_RETURN_TEXT_P(result);
}

Datum
qkmer_out(PG_FUNCTION_ARGS)
{
    text *t = PG_GETARG_TEXT_PP(0);
    char *result = palloc(VARSIZE_ANY_EXHDR(t) + 1);
    memcpy(result, VARDATA_ANY(t), VARSIZE_ANY_EXHDR(t));
    result[VARSIZE_ANY_EXHDR(t)] = '\0';
    elog(DEBUG1, "qkmer_out: returning output query k-mer '%s'", result);
    PG_RETURN_CSTRING(result);
}