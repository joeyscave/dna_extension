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
    
    // Validate DNA sequence
    validate_dna_sequence(str, false);
    
    // Convert to uppercase for consistency
    char *result = pstrdup(str);
    for (int i = 0; result[i]; i++) {
        result[i] = toupper(result[i]);
    }
    
    elog(DEBUG1, "dna_in: returning processed sequence '%s'", result);
    PG_RETURN_CSTRING(result);
}

Datum
dna_out(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    elog(DEBUG1, "dna_out: returning output sequence '%s'", str);
    PG_RETURN_CSTRING(pstrdup(str));
}

// K-mer type functions
Datum
kmer_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    elog(DEBUG1, "kmer_in: processing input k-mer '%s'", str);
    
    // Check length constraint (max 32)
    if (strlen(str) > 32) {
        elog(WARNING, "kmer_in: k-mer length %zu exceeds maximum allowed length of 32", strlen(str));
        ereport(ERROR,
            (errcode(ERRCODE_STRING_DATA_RIGHT_TRUNCATION),
             errmsg("k-mer length cannot exceed 32 nucleotides")));
    }
    
    // Validate DNA sequence
    validate_dna_sequence(str, false);
    
    // Convert to uppercase for consistency
    char *result = pstrdup(str);
    for (int i = 0; result[i]; i++) {
        result[i] = toupper(result[i]);
    }
    
    elog(DEBUG1, "kmer_in: returning processed k-mer '%s'", result);
    PG_RETURN_CSTRING(result);
}

Datum
kmer_out(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    elog(DEBUG1, "kmer_out: returning output k-mer '%s'", str);
    PG_RETURN_CSTRING(pstrdup(str));
}

typedef struct {
    char sequence[32];
} Qkmer;

// Query K-mer type functions
Datum
qkmer_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    elog(DEBUG1, "qkmer_in: processing input query k-mer '%s'", str);
    
    // Check length constraint (max 32)
    if (strlen(str) > 32) {
        elog(WARNING, "qkmer_in: query k-mer length %zu exceeds maximum allowed length of 32", strlen(str));
        ereport(ERROR,
            (errcode(ERRCODE_STRING_DATA_RIGHT_TRUNCATION),
             errmsg("query k-mer length cannot exceed 32 nucleotides")));
    }
    
    // Validate DNA sequence with IUPAC codes
    validate_dna_sequence(str, true);
    
    // Convert to uppercase for consistency
    char *result = pstrdup(str);
    for (int i = 0; result[i]; i++) {
        result[i] = toupper(result[i]);
    }
    
    elog(DEBUG1, "qkmer_in: returning processed query k-mer '%s'", result);
    PG_RETURN_CSTRING(result);
    // Qkmer *qkmer = (Qkmer *) palloc(sizeof(Qkmer));
    // strcpy(qkmer->sequence, result);
    // PG_RETURN_POINTER(qkmer);
}

Datum
qkmer_out(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    elog(DEBUG1, "qkmer_out: returning output query k-mer '%s'", str);
    PG_RETURN_CSTRING(pstrdup(str));
    // Qkmer *qkmer = (Qkmer *) PG_GETARG_POINTER(0);
    // char *result = pstrdup(qkmer->sequence);
    // PG_RETURN_CSTRING(result);
}