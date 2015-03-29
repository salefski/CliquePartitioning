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
#include <map>

const bool inline noisy() { return true; };

class graphDatabaseClass {
    
    const int maxVertexNameLength = 256;
    const unsigned int discoveredBit = (1 << 0);
    const unsigned int processedBit = (1 << 1);
    
    //
    // vertex key
    //
    typedef std::string vertexKeyT;

    int edgeCount = 0; // number of edges in the graph
    //
    // Adacency list records for edges
    //
	struct edgeRecord {
        vertexKeyT vertexID;
		int weight = 0;
		unsigned int edgeBits = 0;
		edgeRecord* next;
        //
		// default constructor
        //
		edgeRecord() {
            vertexID.clear();
			weight = 0;
			edgeBits = 0;
			next = nullptr;
		}
        //
		// copy constructor
        //
		edgeRecord(edgeRecord &other) {
			vertexID.assign(other.vertexID);
			weight = other.weight;
			edgeBits = other.edgeBits;
			next = nullptr;
		}
	};
    //
    // vertecies are string key with vertex information
    //
    struct vertexRecord {
        // vertex name is the key
        int degree; // # edges in edgeList, convenience
        unsigned int vertexBits=0;
        edgeRecord* edgeList;
        // empty constructor
        vertexRecord() : degree(0), vertexBits(0), edgeList(nullptr) {
            // empty
        };
        // duplicate constructor
        vertexRecord(const vertexRecord &other) :
        degree(other.degree),
        vertexBits(other.vertexBits),
        edgeList(nullptr) {
            // empty
        }
    };
    
    struct vertexKeyCompare {
        bool operator()(const std::string &lhs, const std::string &rhs) const {
            return(lhs.compare(rhs) < 0);
        }
    };
    //
    // map of vertex names to vertex records
    //
    typedef std::map<std::string, vertexRecord, vertexKeyCompare> verticiesT;
    verticiesT verticies;
    
    void insertEdgeUtility(vertexKeyT x, vertexKeyT y, int w);
    bool deleteEdgeUtility(vertexKeyT x, vertexKeyT y);
    void deepCopy(const graphDatabaseClass& src);
    void clearEdgeLists();
    
public:
   
	// constructor
	graphDatabaseClass(); 
	// contstructor to copy from an existing graph
	graphDatabaseClass(const graphDatabaseClass& other); 
	// copy operator
	graphDatabaseClass& operator= (const graphDatabaseClass &rhs);
    ~graphDatabaseClass();
    
    int numVerticies();
    int numEdges();
    bool directed;
    
    void initializeGraph(int numVerticies, bool isDirected);
    bool readGraph(FILE* ifile);
    bool writeGraph(FILE* ofile);
    bool deleteEdge(vertexKeyT x, vertexKeyT y);
    void insertEdge(vertexKeyT x, vertexKeyT y, int w);
    
	void disconnectVertex(vertexKeyT x);
    bool deleteVertex(vertexKeyT x);
    void commonNeighbor(vertexKeyT si, vertexKeyT sj, std::vector<vertexKeyT>& returnVal);
};

#endif /* defined(__CliquePartitioning__graphdb__) */
