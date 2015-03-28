//
//  graphdb.h
//  CliquePartitioning
//
//  Created by Bill Salefski on 25-Mar-15.
//  Copyright (c) 2015 Bill Salefski. All rights reserved.
//

#ifndef __CliquePartitioning__graphdb__
#define __CliquePartitioning__graphdb__

#include <stdio.h>
#include <vector>
#include <string>

const bool inline noisy() { return true; };

class graphDatabaseClass {
    
    const int maxVertexNameLength = 256;
    const unsigned int discoveredBit = (1 << 0);
    const unsigned int processedBit = (1 << 1);
    
    int edgeCount = 0; // number of edges in the graph
    
	struct edgeRecord {
        int vertexID=0;
		int weight = 0;
		unsigned int edgeBits = 0;
		edgeRecord* next;
		// default constructor
		edgeRecord() {
			vertexID = 0;
			weight = 0;
			edgeBits = 0;
			next = nullptr;
		}
		// copy constructor
		edgeRecord(edgeRecord &other) {
			vertexID = other.vertexID;
			weight = other.weight;
			edgeBits = other.edgeBits;
			next = nullptr;
		}
	};
	
	struct vertexRecord {
        std::string vertexName;
		int degree; // # edges in edgeList, convenience
        unsigned int vertexBits=0;
		edgeRecord* edgeList;
		// empty constructor
		vertexRecord() : vertexName(), degree(0), vertexBits(0), edgeList(nullptr) {
			// empty
		};
		// duplicate constructor
        vertexRecord(const vertexRecord &other) :
			vertexName(other.vertexName),
			degree(other.degree),
			vertexBits(other.vertexBits),
			edgeList(nullptr) {
			// empty
		}
    };
 
    typedef std::vector<vertexRecord> verticiesT;
    verticiesT verticies;
       
    void insertEdgeUtility(int x, int y, int w);
    bool deleteEdgeUtility(int x, int y);
    void deepCopy(const graphDatabaseClass& src);
    void clearEdgeLists();
    
public:
    
	// constructor
	graphDatabaseClass(); 
	// contstructor to copy from an existing graph
	graphDatabaseClass(const graphDatabaseClass& other); 
	// destructor
	~graphDatabaseClass();
	// copy operator
	graphDatabaseClass& operator= (const graphDatabaseClass &rhs);

    int numVerticies();
    int numEdges();
    bool directed;
    
    void initializeGraph(int numVerticies, bool isDirected);
    bool readGraph(FILE* ifile);
    bool writeGraph(FILE* ofile);
    int lookupVertexIndex (std::string vertexNameToFind);
    bool deleteEdge(int x, int y);
    void insertEdge(int x, int y, int w);
    
	void disconnectVertex(int x);
	int mergeVerticies(int x, int y);
    bool deleteVertex(int x);
    void commonNeighbor(int si, int sj, std::vector<int>& returnVal);
};

#endif /* defined(__CliquePartitioning__graphdb__) */
