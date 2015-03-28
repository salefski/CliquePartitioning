//
//  main.cpp
//  CliquePartitioning
//
//  Created by Bill Salefski on 25-Mar-15.
//  Copyright (c) 2015 Bill Salefski. All rights reserved.
//
#include <iostream>
#include <cstdio>
#include "graphdb.h"
#include <unistd.h>

int main(int argc, const char * argv[]) {
    graphDatabaseClass g;
    graphDatabaseClass gPrime;
    const char * ifname = "/Users/Bill/Google Drive/Computer and Network/MacProjects/CliquePartitioning/CliquePartitioning/testData01.txt";
    
    const char * ofname = "/Users/Bill/Google Drive/Computer and Network/MacProjects/CliquePartitioning/CliquePartitioning/output.txt";
    const std::string startNodeName("v1");
    std::vector<int> commonNeighbors;
    
    FILE* inFile=nullptr, *outFile=nullptr;
    
    inFile = fopen(ifname, "r");
    if(inFile == nullptr) {
        printf("Unable to open\n");
        return 1;
    }
    g.readGraph(inFile);
    fclose(inFile);
    
    outFile = fopen(ofname, "w");
    if(outFile == nullptr) {
        printf("Unable to open output file\n");
        return 1;
    }
    g.writeGraph(stdout);
    fclose(outFile);
    
    gPrime = g;
    printf("\n\ngPrime is\n");
    gPrime.writeGraph(stdout);
    gPrime.commonNeighbor(0, 2, commonNeighbors);
    gPrime.disconnectVertex(0);
    gPrime.disconnectVertex(4);
    printf("\n\nEdited gPrime is\n");
    gPrime.writeGraph(stdout);
    printf("\n\n");
    printf("\n\noriginal g is\n");
    g.writeGraph(stdout);
    return 0;
};
