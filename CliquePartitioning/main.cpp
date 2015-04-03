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
#include <map>
#include <limits>
#include <assert.h>

void TsengAlgorithm (const graphDatabaseClass& g,
                     std::vector<graphDatabaseClass::vertexKeyT>& cliques
                     )
{
    graphDatabaseClass gPrime(g); // work on copy
    cliques.clear();
	int loops = 0;
	while (gPrime.numEdges() > 0) {
		if (noisy()) {
			printf("TsengAlgorithm(): Loop %i\n", ++loops);
			gPrime.checkGraph();
			gPrime.writeGraph(stdout);
		}
        graphDatabaseClass::vertexKeyT si, sj, newVertexKey;
        graphDatabaseClass::verticiesT::iterator sitr;
        std::vector<graphDatabaseClass::vertexKeyT> commonNeighbors;
        //
        // Find two verticies with most common neighboers. In case of tie, take
        // verticies with maximum sum of degrees
        //
        int sumOfDegrees = -1;
        int mostCommons = -1;
        bool foundNewVertex = false;
        for(sitr=gPrime.verticies.begin(); sitr != gPrime.verticies.end(); ++sitr) {
            for(auto e = sitr->second.edgeList; e != nullptr; e = e->next) {
                gPrime.commonNeighbor(sitr->first, e->vertexID, commonNeighbors);
                int testSumOfDegrees = sitr->second.degree + gPrime.verticies[e->vertexID].degree;
                if((int)commonNeighbors.size() > mostCommons) {
                    foundNewVertex = true;
                } else if((int)commonNeighbors.size() == mostCommons) {
					// tie breaker
                    if(sumOfDegrees < testSumOfDegrees) {
                        foundNewVertex = true;
                    }
                }
                if(foundNewVertex) {
                    mostCommons = (int) commonNeighbors.size();
                    si = sitr->first;
                    sj = e->vertexID;
                    sumOfDegrees = testSumOfDegrees;
                    foundNewVertex = false;
                }
            }
        }
        if(noisy()) {
            printf("TsengAlgorithm(): Most Commons is %s and %s with %i common neighbors\n", si.c_str(), sj.c_str(), mostCommons);
        }
        //
        // Update gPrime by
        //   1. deleting all edges linking si and sj in graph
        //   2. merging verticies si and sj into one super node
        //   3. linking in the supernode with the common neighbor links
        //
        gPrime.commonNeighbor(si, sj, commonNeighbors);
        gPrime.disconnectVertex(si); gPrime.verticies.erase(si);
        gPrime.disconnectVertex(sj); gPrime.verticies.erase(sj);
        newVertexKey = si + "," + sj;
        gPrime.insertVertex(newVertexKey);
		// insert edges to the common neighbers
        for(const auto &toVertex : commonNeighbors) {
            gPrime.insertEdge(newVertexKey, toVertex, 1);
        }
    }
    //
    // All edges gone from gPrime, what remains are the
    // clique super nodes
    //
    for(const auto &vtr : gPrime.verticies) {
        cliques.push_back(vtr.first);
    }
}

inline int maxInt() { return std::numeric_limits<int>::max(); }

void BhaskerAlgorithm(const graphDatabaseClass& g,
	std::vector<graphDatabaseClass::vertexKeyT>& cliques
	)
{
	graphDatabaseClass gPrime(g); // work on copy
	cliques.clear();
	int loops = 0;
	while (gPrime.edgeCount > 0) {
        graphDatabaseClass::vertexKeyT p, q, newVertexKey;
        graphDatabaseClass::verticiesT::iterator pitr, qitr;
        std::vector<graphDatabaseClass::vertexKeyT> commonNeighbors;

		if (noisy()) {
			printf("BhaskerAlgorithm(): Loop %i\n", ++loops);
			gPrime.checkGraph();
			gPrime.writeGraph(stdout);
        }
		int pDegree = maxInt();
		// find the vertex with the smallest non-zero degree, call it p
		// there must be at least one since there is at least one edge in gPrime
		for (const auto &vtr : gPrime.verticies) {
			if ((vtr.second.degree > 0) && (vtr.second.degree < pDegree)) {
				p = vtr.first;
				pDegree = vtr.second.degree;
			}
		}
		assert(!p.empty()); // there should have been at least one vertex of non-zero degree
		assert(pDegree > 0);
		// find q
		int qDegree = maxInt();
		int qCommonNeighbors = -1;
		// q is
		//  1. not the same as p
		//  2. neighbor of p with smallest degree
		//  3. tie breaker: has most common neighbors with p
		//  4. tie breaker: first one we find
    	for (auto e = gPrime.verticies[p].edgeList; e != nullptr; e = e->next) {
			if (p.compare(e->vertexID) == 0) {
				// we found the back pointer to p
				continue;
			}
			gPrime.commonNeighbor(p, e->vertexID, commonNeighbors);
			bool newqFound = false;
			// if degree is smaller, e is our new q
			if (gPrime.verticies[e->vertexID].degree < qDegree) {
				newqFound = true;
			} else if (gPrime.verticies[e->vertexID].degree == qDegree) {
				// tiebreaker: if e has more common neighbors with p
				if ((int) commonNeighbors.size() > qCommonNeighbors) {
					newqFound = true;
				}
			}
			if (newqFound) {
                q = e->vertexID;
				qCommonNeighbors = (int) commonNeighbors.size();
				newqFound = false;
			}
		}
		gPrime.commonNeighbor(p, q, commonNeighbors);
        gPrime.disconnectVertex(p); gPrime.verticies.erase(p);
        gPrime.disconnectVertex(q); gPrime.verticies.erase(q);
		newVertexKey = p + "," + q;
		gPrime.insertVertex(newVertexKey);
		// insert edges to the common neighbers
		for (const auto &toVertex : commonNeighbors) {
			gPrime.insertEdge(newVertexKey, toVertex, 1);
		}
	}
	//
	// All edges gone from gPrime, what remains are the
	// clique super nodes
	//
	for (const auto &vtr : gPrime.verticies) {
		cliques.push_back(vtr.first);
	}
}

int main(int argc, const char * argv[]) {
    graphDatabaseClass g;
    graphDatabaseClass gPrime;
    std::string s1("v1");
    std::string s2("v3");
    const char * ifname = "/Users/Bill/Google Drive/Computer and Network/MacProjects/CliquePartitioning/CliquePartitioning/testData03.txt";
    
    const char * ofname = "/Users/Bill/Google Drive/Computer and Network/MacProjects/CliquePartitioning/CliquePartitioning/output.txt";
    std::vector<std::string> cliques;
    
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
	
	g.checkGraph();

	TsengAlgorithm(g, cliques);
	if (noisy()) {
		int numCliques = 0;
		printf("Tseng Algorithm cliques are: ");
		for (const auto &vtr : cliques) {
			printf("%i. %s\n", ++numCliques, vtr.c_str());
		}
	}

	BhaskerAlgorithm(g, cliques);
	if (noisy()) {
		int numCliques = 0;
		printf("Bhasker Algorithm cliques are: ");
		for (const auto &vtr : cliques) {
			printf("%i. %s\n", ++numCliques, vtr.c_str());
		}
	}

    return 0;
};
