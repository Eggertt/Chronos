#pragma once

#include <stdlib.h>
#include <vector>
#include <iostream>
#include "config.h"

class Dijkstra{
    private:
        unsigned int numNodes;
        unsigned int pathSize;
        int* pathArray;
        float* acumWeight;
        float* prev;
        bool* wasVisited; 
        std::vector<int> toWeight;
        std::vector<int> toVisit;

    public:
        Dijkstra(unsigned int numNodes) {
            this->numNodes = numNodes;
            this->pathSize = 0;
            this->pathArray = NULL;
            this->acumWeight = (float*)calloc(numNodes, sizeof(float));
            this->prev = (float*)calloc(numNodes, sizeof(float));
            this->wasVisited = (bool*)calloc(numNodes, sizeof(bool));
            
            for (unsigned int i = 0; i < this->numNodes; i++) {
                acumWeight[i] = -1.0F;
                prev[i] = -1.0F;
                wasVisited[i] = false;
            }

            this->toVisit.clear();
            this->toWeight.clear();
        }

        ~Dijkstra() {
            this->toVisit.clear();
            this->toWeight.clear();
            free(this->acumWeight);
            free(this->prev);
            free(this->wasVisited);
            free(this->pathArray);
        }

        bool isInToVisit(int node) {
            for (int i = 0; i < toVisit.size(); i++) {
                if (toVisit.at(i) == node) return true;
            }

            return false;
        }

        void setAsVisited(int node) {
            this->wasVisited[node] = true;
            this->toVisit.erase(this->toVisit.begin());
        }

        int* findPath(const int graph[N_NODES][N_NODES], int origin, int destination) {
            int visiting = origin;
            this->acumWeight[origin] = 0;
            toVisit.push_back(origin);

            if (origin != destination) {
                while(!this->toVisit.empty()) {
                    // Going through all the nodes to get the neighbors of the visiting node
                    for(int i = 0; i < this->numNodes; i++) {
                        if(graph[visiting][i] > 0) {
                            this->toWeight.push_back(i);
                                                                            // std::cout << "\n\n______" << "Visiting: " << visiting << "______\n";
                                                                            // printVectorContents(toWeight);
                            // Put the neighbor nodes that wasn`t been visited yet to be visited
                            if(!isInToVisit(i) && !wasVisited[i])
                                this->toVisit.push_back(i);
                        }
                    }

                    // Updating the weight of the neighbors of the visiting node
                    while(!this->toWeight.empty()) {
                        int weighting = toWeight.at(0);
                        
                        float sourceAccum = this->acumWeight[visiting];
                        float targetAccum = this->acumWeight[weighting];
                        float distance = graph[visiting][weighting];
                        
                        // If a node was already visited but your acumWeight was updated to a lower one, it needs to be visited once again
                        if((sourceAccum + distance < targetAccum || targetAccum == -1) && wasVisited[weighting]) {
                            this->acumWeight[weighting] = sourceAccum + distance;
                            this->prev[weighting] = visiting;
                            this->toVisit.push_back(weighting);
                                                                            // std::cout << "                                                                  Node " << weighting << " needs to be updated!\n";
                        }
                        else if((sourceAccum + distance < targetAccum || targetAccum == -1) && !wasVisited[weighting]) {
                            this->acumWeight[weighting] = sourceAccum + distance;
                            this->prev[weighting] = visiting;
                        }

                        this->toWeight.erase(this->toWeight.begin());
                    }

                    setAsVisited(visiting); //already removes from toVisit
                    if(!toVisit.empty()) visiting = toVisit.at(0);
                    //printWasVisited();
                    //printAccumWeight();
                }
            }

            pathArray = (int*)malloc(sizeof(int));
            pathArray[0] = destination;
            //pathArray[3] = destination;
            pathSize++;
            // int minAcumulatedNode;
            int prevNode;
            int now = destination;
            while(pathArray[pathSize-1] != origin){
                // int min = INT32_MAX;
                for(int i = 0; i < numNodes; i++){
                    if(prev[now] == i){
                        prevNode = i;
                                                                            // std::cout << "prev(" << now << "): " << prevNode << "\n";
                                                                            // if(prevNode != origin){
                                                                            //     std::cout << "acumWeight: " << acumWeight[i] << "\n\n";
                                                                            // }
                                                                            // else std::cout << "acumWeight: We arrived at the origin!\n";
                    } 
                    // if(graph[now][i] != 0){
                    //     if(acumWeight[i] < min){
                    //         minAcumulatedNode = i;
                    //         std::cout << i << ": ";
                    //         min = acumWeight[i];
                    //         std::cout << min << "\n";
                    //     }
                    // } 
                }
                pathSize++;
                pathArray = (int*)realloc(pathArray, pathSize*sizeof(int));
                // pathArray[pathSize-1] = minAcumulatedNode;
                pathArray[pathSize-1] = prevNode;
                //pathArray[0] = minAcumulatedNode;
                now = prevNode;
            }
                                                                            // std::cout << "________________________\n";

            int flipping;
            for(int i = 0; i < pathSize/2 ; i++){
                flipping = pathArray[pathSize -1 - i];
                pathArray[pathSize -1 -i] = pathArray[i];
                pathArray[i] = flipping;
            }
            return pathArray;
        }
        
        void printVectorContents(std::vector<int> vector){
            for(int i = 0 ; i < vector.size(); i++){
                std::cout << i << ": " << vector.at(i) << "\n";
            }
            printAccumWeight();
            std::cout << "________________________\n";
        }

        void printWasVisited(){
            std::cout << "was visited : |";
            for(int i = 0; i < numNodes; i++){
                std::cout << i << "  ";
            }
            std::cout << "|\n";
            std::cout << "              |";
            for(int i = 0; i < numNodes; i++){
                std::cout << wasVisited[i] << "  ";
            }
            std::cout << "|\n\n";
        }

        void printAccumWeight(){
            int isHigh = 0;
            std::cout << "AccumWeight : |";
            for(int i = 0; i < numNodes; i++){
                if(acumWeight[i] < 0 && i < 10) {
                    std::cout << " " << i << "  ";
                }
                else {
                    if(acumWeight[i] > 9 && i < 10) std::cout << " " << i << "  ";
                    else std::cout << i << "  ";
                }
            }
            std::cout << "|\n";
            std::cout << "              |";
            for(int i = 0; i < numNodes; i++){
                if (i == 10) {
                    if(acumWeight[i] > -1 && acumWeight[i] < 10) std::cout << " " << acumWeight[i] << "  ";
                    else std::cout << acumWeight[i] << "  ";
                    isHigh = 1;
                }
                else if(isHigh == 0) std::cout << acumWeight[i] << "  ";
                else {
                    if(acumWeight[i] > 9 || acumWeight[i] < 0) std::cout << acumWeight[i] << "  ";
                    else std::cout << " " << acumWeight[i] << "  ";
                }
            }
            std::cout << "|\n\n";
        }

        int getPathSize(){
            return this->pathSize;
        }

        int* getPathArray(){
            return this->pathArray;
        }
    
};
