#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//! var substitutions:
//! consensusParams.alwaysUpdateDiffChangeTarget  https://github.com/DigiByte-Core/digibyte/blob/develop/src/chainparams.cpp#L146
//! consensusParams.nDiffChangeTarget             https://github.com/DigiByte-Core/digibyte/blob/develop/src/chainparams.cpp#L316
//! consensusParams.patchBlockRewardDuration      https://github.com/DigiByte-Core/digibyte/blob/develop/src/chainparams.cpp#L319
//! consensusParams.workComputationChangeTarget   https://github.com/DigiByte-Core/digibyte/blob/develop/src/chainparams.cpp#L353
//! consensusParams.patchBlockRewardDuration2     https://github.com/DigiByte-Core/digibyte/blob/develop/src/chainparams.cpp#L321

typedef int64_t CAmount;
const int64_t COIN = 100000000;

CAmount GetDGBSubsidy(int nHeight)
{
    CAmount qSubsidy;

    if (nHeight < 400000 /*consensusParams.alwaysUpdateDiffChangeTarget*/) {
        qSubsidy = 8000 * COIN;
        int blocks = nHeight - 67200 /* consensusParams.nDiffChangeTarget*/;
        int weeks = (blocks / 10080 /*consensusParams.patchBlockRewardDuration*/) + 1;
        //decrease reward by 0.5% every 10080 blocks
        for (int i = 0; i < weeks; i++)
            qSubsidy -= (qSubsidy / 200);
    } else if (nHeight < 1430000 /*consensusParams.workComputationChangeTarget*/) {
        qSubsidy = 2459 * COIN;
        int blocks = nHeight - 400000 /*consensusParams.alwaysUpdateDiffChangeTarget*/;
        int weeks = (blocks / 80160 /*consensusParams.patchBlockRewardDuration2*/) + 1;
        //decrease reward by 1% every month
        for (int i = 0; i < weeks; i++)
            qSubsidy -= (qSubsidy / 100);
    } else {
        //hard fork point: 1.43M
        //subsidy at hard fork: 2157
        //monthly decay factor: 98884/100000
        //last block number: 41668798
        //expected years after hard fork: 19.1395

        qSubsidy = 2157 * COIN / 2;
        int64_t blocks = nHeight - 1430000 /*consensusParams.workComputationChangeTarget*/;
        int64_t months = blocks * 15 / (3600 * 24 * 365 / 12);
        for (int64_t i = 0; i < months; i++) {
            qSubsidy *= 98884;
            qSubsidy /= 100000;
        }
    }

    return qSubsidy;
}

CAmount GetBlockSubsidy(int nHeight)
{
    CAmount nSubsidy = COIN;
    if (nHeight < 67200 /* consensusParams.nDiffChangeTarget*/) {
        //this is pre-patch, reward is 8000.
        nSubsidy = 8000 * COIN;

        if (nHeight < 1440) //1440
        {
            nSubsidy = 72000 * COIN;
        } else if (nHeight < 5760) //5760
        {
            nSubsidy = 16000 * COIN;
        }

    } else {
        //patch takes effect after 67,200 blocks solved
        nSubsidy = GetDGBSubsidy(nHeight);
    }

    //make sure the reward is at least 1 DGB
    if (nSubsidy < COIN) {
        nSubsidy = COIN;
    }

    return nSubsidy;
}

int main()
{
    int height = 0;
    CAmount totalSubsidy = 0;
    CAmount lastSubsidy = 0;
    CAmount thisSubsidy = 0;

    while (true) {
        ++height;
        thisSubsidy = GetBlockSubsidy(height);
        totalSubsidy += thisSubsidy;
        if (lastSubsidy != thisSubsidy) {
            printf("height %08d subsidy %0.8f totalsub %0.8f\n", height, (double)thisSubsidy / COIN, (double)totalSubsidy / COIN);
            lastSubsidy = thisSubsidy;
        }
    }

    return 0;
}
