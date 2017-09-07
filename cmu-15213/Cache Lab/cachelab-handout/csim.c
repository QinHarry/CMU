#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>
#include <math.h>
#include "cachelab.h"

typedef unsigned long long int memoryAddress;


struct cacheParameter {
    int s;
    int b;
    int E;
    int S;
    int B;
    int hits;
    int misses;
    int evicts;
};

struct line {
    int latestUsed;
    int valid;
    memoryAddress tag;
    char *block;
};

struct cacheSet {
    struct line *lines;
};

struct cache {
    struct cacheSet *sets;
};

struct cache initCahce(long long numberOfSets, int numberOfLines, long long sizeOfBlock) {
    struct cache newCache;
    struct cacheSet set;
    struct line l;
    int indexOfSet;
    int indexOfLine;

    newCache.sets = (struct cacheSet*) malloc(sizeof(struct cacheSet) * numberOfSets);
    for (indexOfSet = 0; indexOfSet < numberOfSets; indexOfSet++) {
        set.lines = (struct line*) malloc(sizeof(struct line) * numberOfLines);
        newCache.sets[indexOfSet] = set;
        for (indexOfLine = 0; indexOfLine < numberOfLines; indexOfLine++) {
            l.latestUsed = 0;
            l.valid = 0;
            l.tag = 0;
            set.lines[indexOfLine] = l;
        }
    }
    return newCache;
}

void cleanFunction(struct cache myCache, long long numberOfSets, int numberOfLines, long long sizeOfBlock) {
    int indexOfSet;
    for (indexOfSet = 0; indexOfSet < numberOfSets; indexOfSet++) {
        struct cacheSet set = myCache.sets[indexOfSet];
        if (set.lines != NULL) free(set.lines);
    }
    if (myCache.sets != NULL) free(myCache.sets);
}

int detectEmptyLine(struct cacheSet exampleSet, struct cacheParameter exampleParameter) {
    int numberOfLines = exampleParameter.E;
    int index;
    struct line l;
    for (index = 0; index < numberOfLines; index++) {
        l = exampleSet.lines[index];
        if (l.valid == 0) {
            return index;
        }
    }
    return -1;
}

int detectEvictLine(struct cacheSet exampleSet, struct cacheParameter exampleParameter, int *usedLines) {
    int numberOfLines = exampleParameter.E;
    int maxFreqUsage = exampleSet.lines[0].latestUsed;
    int minFreqUsage = exampleSet.lines[0].latestUsed;
    int minFreqUsage_index = 0;
    int indexOfLine;

    for (indexOfLine = 1; indexOfLine < numberOfLines; indexOfLine++) {
        if (minFreqUsage > exampleSet.lines[indexOfLine].latestUsed) {
            minFreqUsage_index = indexOfLine;
            minFreqUsage = exampleSet.lines[indexOfLine].latestUsed;
        }

        if (maxFreqUsage < exampleSet.lines[indexOfLine].latestUsed) {
            maxFreqUsage = exampleSet.lines[indexOfLine].latestUsed;
        }
    }
    usedLines[0] = minFreqUsage;
    usedLines[1] = maxFreqUsage;
    return minFreqUsage_index;
}

struct cacheParameter accessTheCacheData(struct cache myCache, struct cacheParameter exampleParameter, memoryAddress address) {
    int indexOfLine;
    int checkFullCache = 1;

    int numberOfLines = exampleParameter.E;
    int previousHit = exampleParameter.hits;

    int tagSize = (64 - (exampleParameter.s + exampleParameter.b));
    memoryAddress inputTag = address >> (exampleParameter.s + exampleParameter.b);
    unsigned long long temp = address << (tagSize);
    unsigned long long indexOfSet = temp >> (tagSize + exampleParameter.b);

    struct cacheSet exampleSet = myCache.sets[indexOfSet];

    for (indexOfLine = 0; indexOfLine < numberOfLines; indexOfLine++) {
        if (exampleSet.lines[indexOfLine].valid) {
            if (exampleSet.lines[indexOfLine].tag == inputTag) {
                exampleSet.lines[indexOfLine].latestUsed++;
                exampleParameter.hits++;
            }
        }else if (!(exampleSet.lines[indexOfLine].valid) && (checkFullCache)) {
            checkFullCache = 0;
        }
    }

    if (previousHit == exampleParameter.hits) {
        exampleParameter.misses++;
    }else {
        return exampleParameter;
    }

    int *usedLines = (int *)malloc(sizeof(int) * 2);
    int minFreqUsage_index = detectEvictLine(exampleSet, exampleParameter, usedLines);

    if (checkFullCache) {
        exampleParameter.evicts++;
        exampleSet.lines[minFreqUsage_index].tag = inputTag;
        exampleSet.lines[minFreqUsage_index].latestUsed = usedLines[1] + 1;
    }else {
        int empty_index = detectEmptyLine(exampleSet, exampleParameter);
        exampleSet.lines[empty_index].tag = inputTag;
        exampleSet.lines[empty_index].valid = 1;
        exampleSet.lines[empty_index].latestUsed = usedLines[1] + 1;
    }

    free(usedLines);
    return exampleParameter;
}


int main(int argc, char **argv)
{
    struct cache myCache;
    struct cacheParameter exampleParameter;
    bzero(&exampleParameter, sizeof(exampleParameter));
    long long numberOfSets;
    long long sizeOfBlock;
    FILE *openTrace;
    char instructionInTraceFile;
    memoryAddress address;
    int size;
    char *trace_file;
    char c;

    while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch(c) {
            case 's':
                exampleParameter.s = atoi(optarg);
                break;
            case 'E':
                exampleParameter.E = atoi(optarg);
                break;
            case 'b':
                exampleParameter.b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'h':
                exit(0);
            default:
                exit(1);
        }
    }

    numberOfSets = pow(2.0, exampleParameter.s);
    sizeOfBlock = pow(2.0, exampleParameter.b);
    exampleParameter.hits = 0;
    exampleParameter.misses = 0;
    exampleParameter.evicts = 0;
    myCache = initCahce(numberOfSets, exampleParameter.E, sizeOfBlock);

    openTrace = fopen(trace_file, "r");
    if (openTrace != NULL) {
        while (fscanf(openTrace, " %c %llx,%d", &instructionInTraceFile, &address, &size) == 3) {
            switch(instructionInTraceFile) {
                case 'I':
                    break;
                case 'L':
                    exampleParameter = accessTheCacheData(myCache, exampleParameter, address);
                    break;
                case 'S':
                    exampleParameter = accessTheCacheData(myCache, exampleParameter, address);
                case 'M':
                    exampleParameter = accessTheCacheData(myCache, exampleParameter, address);
                    exampleParameter = accessTheCacheData(myCache, exampleParameter, address);
                    break;
                default:
                    break;
            }
        }
    }

    printSummary(exampleParameter.hits, exampleParameter.misses, exampleParameter.evicts);
    cleanFunction(myCache, numberOfSets, exampleParameter.E, sizeOfBlock);
    fclose(openTrace);

    return 0;
}
