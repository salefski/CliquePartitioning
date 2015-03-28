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
	verticies.resize(src.verticies.size());
	auto dstVertex = verticies.begin();
	auto srcVertex = src.verticies.begin();
	while((dstVertex < verticies.end()) && (srcVertex < src.verticies.end())) {
		dstVertex->vertexName.assign(srcVertex->vertexName);
		dstVertex->vertexBits = srcVertex->vertexBits;
		// copy the edge list
		if (srcVertex->edgeList) {
			// copy first edge
			dstVertex->edgeList = new edgeRecord(*srcVertex->edgeList);
			// copy the remaining edges
			edgeRecord* dstEdge = dstVertex->edgeList;
			edgeRecord* srcEdge = srcVertex->edgeList;
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
	for (auto &vtr : verticies) {
		edgeRecord* e = vtr.edgeList;
		while (e != nullptr) {
			edgeToDelete = e;
			e = e->next;
			delete edgeToDelete;
		}
		vtr.edgeList = nullptr;
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
// destructor
graphDatabaseClass::~graphDatabaseClass() {
	clearEdgeLists();
}

// copy operator
graphDatabaseClass& graphDatabaseClass::operator= (const graphDatabaseClass &rhs) {
	if (this != &rhs) {
		clearEdgeLists();
		verticies.clear();
		deepCopy(rhs);
	}
	return *this;;
};


void graphDatabaseClass::initializeGraph(int numVerticies, bool isDirected) {
    directed = isDirected;
	clearEdgeLists();
	verticies.clear();
	verticies.resize(numVerticies);
};

int graphDatabaseClass::lookupVertexIndex (std::string vertexNameToFind) {
    int i=0;
    for(auto &itr : verticies) {
        if(itr.vertexName.compare(vertexNameToFind) == 0) {
            return(i);
        }
        ++i;
    }
    return(-1);
}

void graphDatabaseClass::insertEdgeUtility(int x, int y, int w) {
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

void graphDatabaseClass::insertEdge(int x, int y, int w) {
	insertEdgeUtility(x, y, w);
	insertEdgeUtility(y, x, w);
	if (noisy()) {
		printf("adding edge %i to %i\n", x, y);
	}
}

bool graphDatabaseClass::deleteEdgeUtility(int x, int y) {
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
			printf("deleteEdgeUtility(): Not able to find an edge to delete from %i to %i\n", x, y);
		}
	}
	return returnVal;
}

bool graphDatabaseClass::deleteEdge(int x, int y) {
	bool returnVal1 = false;
	bool returnVal2 = false;
	returnVal1 = deleteEdgeUtility(x, y);
	returnVal2 = deleteEdgeUtility(y, x);
	return (returnVal1 && returnVal2);
}

bool graphDatabaseClass::deleteVertex(int x) {
	if (noisy()) {
		printf("deleting vertex %i\n", x);
	}
    if(verticies[x].edgeList != nullptr) {
        printf("Deleting a vertex with non-empty edgelist\n");
    }
	// erase from the list of verticies
	verticies.erase(verticies.begin()+x);
	return true;
}

//
// deletes all the edges to and from from vertex x
//
void graphDatabaseClass::disconnectVertex(int x) {
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
//
// merges two verticies. Assumes they are already disconnected
//
int graphDatabaseClass::mergeVerticies(int x, int y) {
    if(x>y) {
        int tmp = x;
        x = y;
        y = tmp;
    }
	if (noisy()) {
		printf("merging vertex %s and %s ", verticies[x].vertexName.c_str(), verticies[y].vertexName.c_str());
	}
    verticies[x].vertexName = verticies[x].vertexName + " & " + verticies[y].vertexName;
    verticies[x].vertexBits = verticies[x].vertexBits | verticies[y].vertexBits;
	if (noisy()) {
		printf("into vertex %s\n", verticies[x].vertexName.c_str());
	}
	deleteVertex(y);
    return x;
}

void graphDatabaseClass::commonNeighbor(int si, int sj, std::vector<int>& returnVal) {
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
    int x=0, y=0; // vertices in edge (x,y)
    int w=0; // weight of edge
    char fromVertexName[maxVertexNameLength];
    char toVertexName[maxVertexNameLength];
    
    fscanf(f, "%i %i %i", &numVerticies, &numEdges, &isDirected);
    initializeGraph(numVerticies, (isDirected==1));
    // read in the vertex names
    for(int i=0; i<numVerticies; ++i) {
        fscanf(f, "%s", fromVertexName);
        verticies[i].vertexName.assign(fromVertexName);
    }
    
    // Read in the edges.
    while(!feof(f)) {
        if(3==fscanf(f, "%s %s %i", fromVertexName, toVertexName, &w)) {
            x = lookupVertexIndex(fromVertexName);
            y = lookupVertexIndex(toVertexName);
            insertEdge(x, y, w); // all edges are explicit, so directed == true
        } else {
            printf("Error reading graph file\n");
            return false;
        }
    }
    return true;
}

bool graphDatabaseClass::writeGraph (FILE* f) {

    fprintf(f, "%lu %d %i\n", verticies.size(), edgeCount, (directed?1:0));
    
    // print the vertex names
    for(auto vtr : verticies) {
        fprintf(f, "%s\n", vtr.vertexName.c_str());
    };
    
    for(int x = 0; x < verticies.size(); ++x) {
        for(edgeRecord* e = verticies[x].edgeList; e != nullptr; e=e->next) {
            fprintf(f, "%s ", verticies[x].vertexName.c_str());
            fprintf(f, "%s ", verticies[e->vertexID].vertexName.c_str());
            fprintf(f, "%i\n", e->weight);
        }
    }
    return true;
}
