#include <vector>
#include <string>
#include <queue>

#include "dijkstra.h"
#include "direction.h"
#include "util.h"
#include "config.h"
#include "robot.h"
#include "node.h"

robot_t bot;

class Scheduler
{
private:
    Direction dir;
    int currentPosition, prevPosition = -1;
    int startPosition;
    Node *nodeVec;
    int *path; 
    int pathSize;
    Port port3;
public:
    Scheduler(int startPosition);
    ~Scheduler();
    void setUp(Box *boxes, unsigned int numBoxes);
    std::priority_queue<Box, std::vector<Box>, boxComparision> queue;
    std::vector<std::string> getRoute(Box box, Port port);
    std::vector<std::string> getRoute();
    std::vector<std::string> getPaco(int destNode);
    std::vector<Port> deliveryPorts;
    std::vector<Port> deliveryNodesA;
    std::vector<Port> deliveryNodesB;
    Box getBox();
    Port getAvailablePort(Box currentBox);
    Node* getNodeVec();
    void resetPosition();
    int * getPath();
    int getPathSize();
    int flag = 0;
};

Scheduler::Scheduler(int startPosition) {
    currentPosition = startPosition;
    this->startPosition = startPosition;
}

Scheduler::~Scheduler() {
    free(nodeVec);
}

void Scheduler::setUp(Box *boxes, unsigned int numBoxes) {
    for (auto deliveryNode : deliveryNodes) {
        Port port;
        port.pos = deliveryNode;
        port.occupied = false;
        deliveryPorts.push_back(port);        
    }

    for (auto deliveryNodeA : processNodesA) {
        Port port1;
        port1.pos = deliveryNodeA;
        port1.occupied = false;
        deliveryNodesA.push_back(port1);        
    }
    
    for (auto deliveryNodeB : processNodesB) {
        Port port2;
        port2.pos = deliveryNodeB;
        port2.occupied = false;
        deliveryNodesB.push_back(port2);        
    }

    for (size_t i = 0; i < numBoxes; i++)
    {   
        queue.push(boxes[i]);
    }
    

}

std::vector<std::string> Scheduler::getRoute(Box box, Port destPort) {
    Dijkstra dijkstra = Dijkstra(N_NODES);
    
    // TODO: Chose which port to go based on the color
    if(destPort.occupied == true){
        if(box.color == BLUE){
            for(int i = 0; i<deliveryPorts.size(); i++){
                if (deliveryPorts[i].pos == destPort.pos)
                    deliveryPorts[i].occupied = destPort.occupied; 
            }
        }

        if(box.color == RED){   
            for(int i = 0; i<deliveryNodesA.size(); i++){
                if (deliveryNodesA[i].pos == destPort.pos)
                    deliveryNodesA[i].occupied = destPort.occupied; 
            }
        }
            
        if(box.color == GREEN){
            for(int i = 0; i<deliveryNodesB.size(); i++){
                if (deliveryNodesB[i].pos == destPort.pos)
                    deliveryNodesB[i].occupied = destPort.occupied; 
            }
        }
        destPort = getAvailablePort(box);

        Serial.println();
        Serial.print("Dropping at: ");
        Serial.print(destPort.pos);
        Serial.print(".occupied = ");
        Serial.println(destPort.occupied);
    }
    else {
        Serial.println();
        Serial.print("Dropping at: ");
        Serial.print(destPort.pos);
        Serial.print(".occupied = ");
        Serial.println(destPort.occupied);
    }

    // Serial.printf("No de destino: ");
    // Serial.println(destPort.pos);
    // If box is waiting then we need to pick it up
    dijkstra.findPath(graph, currentPosition, destPort.pos);
        
    path = dijkstra.getPathArray();
    currentPosition = path[dijkstra.getPathSize() - 1];
    pathSize = dijkstra.getPathSize();
    // Serial.printf("Minimun Path is: [ ");
    // int a= dijkstra.getPathSize();
    // for(int i = 0; i < a; i++){
    //     Serial.print(path[i]);
    //     Serial.printf(" ");
    // }
    // Serial.println("]");

    nodeVec = (Node *)calloc(N_NODES, sizeof(Node));
    dir.nodeVector(nodeVec);

    int size = dijkstra.getPathSize();
    std::vector<std::string> dirVec = dir.definePath(path, size, nodeVec);

    if (box.color != BLUE) {
        Box newBox;
        if (box.color == GREEN) newBox.color = BLUE;

        else if (box.color == RED) newBox.color = GREEN;
        
        newBox.num = box.num;
        newBox.pos = currentPosition+1;
        newBox.status = WAITING;

        queue.push(newBox);
    }
      
    return dirVec;
}

std::vector<std::string> Scheduler::getRoute() {
    Dijkstra dijkstra = Dijkstra(N_NODES);

    if(queue.empty()){
        Serial.print("FILA -= VAZIA =- BRO ");
    }
    
    Box box = queue.top();
    queue.pop();



    // If box is waiting then we need to pick it up
    dijkstra.findPath(graph, currentPosition, box.pos);

    for(auto & node : Scheduler::deliveryNodesB)
        if(node.pos + 1 == box.pos)
            node.occupied = false;
        
    path = dijkstra.getPathArray();
    currentPosition = path[dijkstra.getPathSize() - 1];

    nodeVec = (Node *)calloc(N_NODES, sizeof(Node));
    dir.nodeVector(nodeVec);

    int size = dijkstra.getPathSize();
    pathSize = dijkstra.getPathSize();
    std::vector<std::string> dirVec = dir.definePath(path, size, nodeVec);
      
    return dirVec;
}

std::vector<std::string> Scheduler::getPaco(int destNode) {

    Dijkstra dijkstra = Dijkstra(N_NODES);

    Serial.printf("destNode %d\n", destNode);
    Serial.printf("currentPosition %d\n", currentPosition);
    Serial.printf("prevPosition %d\n", prevPosition);
    // If box is waiting then we need to pick it up
    dijkstra.findPath(graph, currentPosition, destNode);

    path = dijkstra.getPathArray();
    int size = dijkstra.getPathSize();

    switch (destNode){
        //Incoming warehouse
        case 0:
        case 1:
        case 2:
        case 3:
        //Machine A
        case 15:
        case 16:
        case 22:
        case 23:
        //Machine B
        case 11:
        case 12:
        case 18:
        case 19:
        //Outgoing Warehouse
        case 31:
        case 32:
        case 33:
        case 34:

            currentPosition = path[size - 2];
            prevPosition = path[size - 1];

        break;
    
        default:

            currentPosition = path[size - 1];
            prevPosition = path[size - 2];

        break;
    }

    // if(destNode == 0 || destNode == 31){

    //     currentPosition = path[size - 2];
    //     prevPosition = path[size - 1];

    // }else{

    //     currentPosition = path[size - 1];
    //     prevPosition = path[size - 2];

    // }

    nodeVec = (Node *)calloc(N_NODES, sizeof(Node));

    dir.nodeVector(nodeVec);

    pathSize = dijkstra.getPathSize();

    std::vector<std::string> dirVec = dir.definePath(path, size, nodeVec);

    if(path[0] == prevPosition) robot.setState(TURNBACK);

    Serial.printf("destNode2 %d\n", destNode);
    Serial.printf("currentPosition2 %d\n", currentPosition);
    Serial.printf("prevPosition2 %d\n", prevPosition);     
    
    return dirVec;
}

Box Scheduler::getBox() {
    return queue.top();
}

// Port Scheduler::getAvailablePort() {
//     for (auto port: deliveryPorts) {
//         if (port.occupied == false)
//             return port;
//     }
// }

Port Scheduler::getAvailablePort(Box currentBox) {
    
    if(currentBox.color == BLUE){
        for (Port port: deliveryPorts) {
            if (port.occupied == false)
                return port;
        }
    } else if(currentBox.color == GREEN){
        for (Port port1: deliveryNodesB) {
            if (port1.occupied == false)
                return port1;
        }
    } else if(currentBox.color == RED){
        for (Port port2: deliveryNodesA) {
            if (port2.occupied == false)
                return port2;
        }
    } 
    
    return port3;
}

Node* Scheduler::getNodeVec() {
    return nodeVec;
}

int * Scheduler::getPath() {
    return path;
}

int Scheduler::getPathSize(){
    return pathSize;
}

void Scheduler::resetPosition(){
    this->currentPosition = this->startPosition;
    this->prevPosition = -1;
    while(!queue.empty()){
        queue.pop();
    }
}