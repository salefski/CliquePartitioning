//
//  graphdb.cpp
//  CliquePartitioning
//
//  Created by Bill Salefski on 25-Mar-15.
//  Copyright (c) 2015 Bill Salefski. All rights reserved.
//

#include "graphdb.h"

// copies the verticies and their edgelists
void graphDatabaseClass::deepCopy(const graphDatabaseClass& src) {
    edgeCount = src.edgeCount;
    directed = src.directed;
	verticies = src.verticies; // copy the verticies
	auto dstVertex = verticies.begin();
	auto srcVertex = src.verticies.begin();
	while((dstVertex != verticies.end()) && (srcVertex != src.verticies.end())) {
		dstVertex->second.vertexBits = srcVertex->second.vertexBits;
		// copy the edge list
		if (srcVertex->second.edgeList) {
			// copy first edge
			dstVertex->second.edgeList = new edgeRecord(*srcVertex->second.edgeList);
			// copy the remaining edges
			edgeRecord* dstEdge = dstVertex->second.edgeList;
			edgeRecord* srcEdge = srcVertex->second.edgeList;
			while(srcEdge->next != nullptr) {
				dstEdge->next = new edgeRecord(*srcEdge->next);
				srcEdge = srcEdge->next;
				dstEdge = dstEdge->next;
			}
		}
		++dstVertex;
		++srcVertex;
	}
}

//
// deletes all the edges
//
void graphDatabaseClass::clearEdgeLists() {
	edgeRecord* edgeToDelete = nullptr;
    verticiesT::iterator vtr;
    for (vtr = verticies.begin(); vtr != verticies.end(); ++vtr) {
		edgeRecord* e = vtr->second.edgeList;
		while (e != nullptr) {
			edgeToDelete = e;
			e = e->next;
			delete edgeToDelete;
		}
		vtr->second.edgeList = nullptr;
	}
}

graphDatabaseClass::graphDatabaseClass() {
	verticies.clear();
	edgeCount = 0;
};
// constructor to copy from an existing grapy
graphDatabaseClass::graphDatabaseClass(const graphDatabaseClass& src) {
	deepCopy(src);
};

// copy operator
graphDatabaseClass& graphDatabaseClass::operator= (const graphDatabaseClass &rhs) {
	if (this != &rhs) {
		clearEdgeLists();
		verticies.clear();
		deepCopy(rhs);
	}
	return *this;;
};

graphDatabaseClass::~graphDatabaseClass() {
};

void graphDatabaseClass::initializeGraph(int numVerticies, bool isDirected) {
    directed = isDirected;
	clearEdgeLists();
	verticies.clear();
};


void graphDatabaseClass::insertEdgeUtility(vertexKeyT x, vertexKeyT y, int w) {
	edgeRecord* newEdge = new edgeRecord;
    newEdge->vertexID = y;
    newEdge->weight = w;
	if (verticies[x].edgeList) {
        edgeRecord* e = verticies[x].edgeList;
        for (; e->next != nullptr; e = e->next);
		e->next = newEdge;
		++verticies[x].degree;
	} else {
		verticies[x].edgeList = newEdge;
		verticies[x].degree = 1;
	}
    ++edgeCount;
}

void graphDatabaseClass::insertEdge(vertexKeyT x, vertexKeyT y, int w) {
	insertEdgeUtility(x, y, w);
	insertEdgeUtility(y, x, w);
	if (noisy()) {
		printf("adding edge %s to %s\n", x.c_str(), y.c_str());
	}
}

bool graphDatabaseClass::deleteEdgeUtility(vertexKeyT x, vertexKeyT y) {
	bool returnVal = false;
	if (verticies[x].edgeList->vertexID == y) {
		// delete the head of the edgeList
        edgeRecord* edgeToDelete = verticies[x].edgeList;
		verticies[x].edgeList = verticies[x].edgeList->next;
        delete edgeToDelete;
        --edgeCount;
		--verticies[x].degree;
		returnVal = true;
	} else {
		// search the edge list for the edge to delete
		edgeRecord* e = verticies[x].edgeList;
		do {
			if ((e->next != nullptr) && (e->next->vertexID == y)) {
				edgeRecord* edgeToDelete = e->next;
				e->next = e->next->next;
				delete edgeToDelete;
				returnVal = true;
                --edgeCount;
				--verticies[x].degree;
				break;
			} else {
				e = e->next;
			}
		} while (e != nullptr);
	}
	if (noisy()) {
		if (returnVal == false) {
			printf("deleteEdgeUtility(): Not able to find an edge to delete from %s to %s\n", x.c_str(), y.c_str());
		}
	}
	return returnVal;
}

bool graphDatabaseClass::deleteEdge(vertexKeyT x, vertexKeyT y) {
	bool returnVal1 = false;
	bool returnVal2 = false;
	returnVal1 = deleteEdgeUtility(x, y);
	returnVal2 = deleteEdgeUtility(y, x);
	return (returnVal1 && returnVal2);
}

bool graphDatabaseClass::deleteVertex(vertexKeyT x) {
	if (noisy()) {
		printf("deleting vertex %s\n", x.c_str());
	}
    if(verticies[x].edgeList != nullptr) {
        printf("Deleting a vertex with non-empty edgelist\n");
    }
	// erase from the list of verticies
	verticies.erase(x);
	return true;
}

//
// deletes all the edges to and from from vertex x
//
void graphDatabaseClass::disconnectVertex(vertexKeyT x) {
	edgeRecord* e = verticies[x].edgeList;
	edgeRecord* edgeToDelete = nullptr;
	while (e != nullptr) {
		edgeToDelete = e;
		deleteEdgeUtility(e->vertexID, x); // delete the back edge
		e = e->next;
		delete edgeToDelete;
		--edgeCount;
	}
	verticies[x].edgeList = nullptr;
	verticies[x].degree = 0;
}

void graphDatabaseClass::commonNeighbor(vertexKeyT si, vertexKeyT sj, std::vector<vertexKeyT>& returnVal) {
    returnVal.clear();
    // go through each edgeList for si and sj to determine which edges have the same destination
    for(edgeRecord* esi = verticies[si].edgeList; esi != nullptr; esi = esi->next) {
        for(edgeRecord* esj = verticies[sj].edgeList; esj != nullptr; esj = esj->next) {
            if(esi->vertexID == esj->vertexID) {
                returnVal.push_back(esi->vertexID);
            }
        }
    }
}

// file format
// 1: numVertices numEedges isDirected
// 2-#verticies: vertexName
// #verticies: fromVertex toVertex weight
//
bool graphDatabaseClass::readGraph(FILE* f) {
    int numVerticies=0;
    int numEdges=0; // counter for # edges, inserteEdge increments g->nedges
    int isDirected=0;
    int w=0; // weight of edge
    
    fscanf(f, "%i %i %i", &numVerticies, &numEdges, &isDirected);
    initializeGraph(numVerticies, (isDirected==1));
    // read in the vertex names
    for(int i=0; i<numVerticies; ++i) {
        verticiesT::iterator vtr;
        char vertexToInsertCstr[maxVertexNameLength];
        vertexKeyT vertexToInsert;
        fscanf(f, "%s", vertexToInsertCstr);
        vertexToInsert.assign(vertexToInsertCstr);
        vtr = verticies.find(vertexToInsertCstr);
        if(vtr != verticies.end()) {
            printf("readGraph(): duplicate vertex name %s; not inserted again\n", vertexToInsert.c_str());
            continue;
        }
        verticies[vertexToInsert]; // inserts vertex
    }
    
    // Read in the edges.
    while(!feof(f)) {
        char fromVertexNameCstr[maxVertexNameLength];
        char toVertexNameCstr[maxVertexNameLength];
        vertexKeyT fromVertexKey, toVertexKey;
        if(3==fscanf(f, "%s %s %i", fromVertexNameCstr, toVertexNameCstr, &w)) {
            fromVertexKey.assign(fromVertexNameCstr);
            toVertexKey.assign(toVertexNameCstr);
            insertEdge(fromVertexKey, toVertexKey , w); // all edges are explicit, so directed == true
        } else {
            printf("Error reading graph file\n");
            return false;
        }
    }
    return true;
}

bool graphDatabaseClass::writeGraph (FILE* f) {

    fprintf(f, "%lu %d %i\n", verticies.size(), edgeCount, (directed?1:0));
    verticiesT::iterator vtr;

    // print the vertex names
    for(vtr=verticies.begin(); vtr != verticies.end(); ++vtr) {
        fprintf(f, "%s\n", vtr->first.c_str());
    };
    
    for(vtr=verticies.begin(); vtr != verticies.end(); ++vtr) {
        for(edgeRecord* e = vtr->second.edgeList; e != nullptr; e=e->next) {
            fprintf(f, "%s ", vtr->first.c_str());
            fprintf(f, "%s ", e->vertexID.c_str());
            fprintf(f, "%i\n", e->weight);
        }
    }
    return true;
}
