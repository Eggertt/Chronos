// #include "direction.h"

// Direction::Direction() {
//     nNodes = N_NODES;
// }

// char Direction::direction(Node *no1, Node *no2) {   
//     // printf("1-x:%d-y:%d\n", no1->xpos, no1->ypos);
//     // printf("2-x:%d-y:%d\n", no2->xpos, no2->ypos);
//         if (no1->xpos < no2->xpos)
//         return 'd';
//     else if (no1->xpos > no2->xpos)
//         return 'u';
//     else if (no1->ypos > no2->ypos)
//         return 'l';
//     else if (no1->ypos < no2->ypos)
//         return 'r';
//     return 'e';
// }

// const char* Direction::mode(Node *intermedio){
//     if ((direction(intermedio->prev, intermedio) == 'u' && direction(intermedio, intermedio->next) == 'u') ||
//         (direction(intermedio->prev, intermedio) == 'd' && direction(intermedio, intermedio->next) == 'd') ||
//         (direction(intermedio->prev, intermedio) == 'l' && direction(intermedio, intermedio->next) == 'l') ||
//         (direction(intermedio->prev, intermedio) == 'r' && direction(intermedio, intermedio->next) == 'r'))
//         return "Line";
//     else

//         if ((direction(intermedio->prev, intermedio) == 'u' && direction(intermedio, intermedio->next) == 'l') ||
//             (direction(intermedio->prev, intermedio) == 'l' && direction(intermedio, intermedio->next) == 'd') ||
//             (direction(intermedio->prev, intermedio) == 'd' && direction(intermedio, intermedio->next) == 'r') ||
//             (direction(intermedio->prev, intermedio) == 'r' && direction(intermedio, intermedio->next) == 'u'))
//         return "Left";
//     else

//         if ((direction(intermedio->prev, intermedio) == 'u' && direction(intermedio, intermedio->next) == 'r') ||
//             (direction(intermedio->prev, intermedio) == 'r' && direction(intermedio, intermedio->next) == 'd') ||
//             (direction(intermedio->prev, intermedio) == 'd' && direction(intermedio, intermedio->next) == 'l') ||
//             (direction(intermedio->prev, intermedio) == 'l' && direction(intermedio, intermedio->next) == 'u'))
//         return "Right";
//     else

//         return "Error";
// }

// void Direction::findEdges(int no1) {
//     printf("Adj de no %d: ", no1);
//     for (int i=0; i<35; i++){
//         if(graph[no1][i] == 1){
//             printf("%d ", i);
//         }
//     }
//     printf("\n");
// }

// void Direction::nodeVector(Node *nodeVec){  //Define os ponteiros para os nos anteriores e proximos a NULL e as coordenadas
//     for(int i=0; i<nNodes; i++){
//         nodeVec[i].prev = NULL;
//         nodeVec[i].next = NULL;
//         nodeVec[i].xpos = pos[0][i];
//         nodeVec[i].ypos = pos[1][i];
//     //    if(i == 4 || i == 10 || i == 14 || i == 21 || i == 25 || i == 31) printf("\n");
//         // printf("node[%i]:  pos = %d,%d\n", i, nodeVec[i].xpos, nodeVec[i].ypos);
//     }
// }

// std::vector<std::string> Direction::definePath(int *path, int size, Node *nodeVec){
    
//     int cont1 = 1, cont2 = 1;
//     std::vector<std::string> dirVec;
    
//     printf("Comecar\n");
//     for(int i=0; i<size; i++){
//         if (i == 0) {
//             nodeVec[path[i]].prev = NULL;
//             nodeVec[path[i]].next = &nodeVec[path[i+1]];
//             // printf("No inicial\n");
//         }
//         else if (i == (size)-1){
//             nodeVec[path[i]].prev = &nodeVec[path[i-1]];
//             nodeVec[path[i]].next = NULL;
//             // printf("No final\n");
//         }
//         else{
//             nodeVec[path[i]].prev = &nodeVec[path[i-1]];
//             nodeVec[path[i]].next = &nodeVec[path[i+1]];
//             // printf("No ao meio\n");
//         }
        
//         if(nodeVec[path[i-1]].prev != NULL && nodeVec[path[i]].next != NULL && i>2){ //Estamos em algum no entre o quarto e o penultimo
//         //                                                                             Como i>2 estamos em algum no entre o quarto e o penultimo (pois o ultimo tem .next = NULL)
//         //                                                                             Atribuimos apenas o modo do anterior a ele 
//             // printf("AQUI AO MEIO\n");
//             // printf("i = %d\n", i);
//             if (strcmp(mode(&nodeVec[path[i-1]]), "Error") != 0){
//                 //Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i-1], path[i], direction(&nodeVec[path[i-1]], &nodeVec[path[i]]));
//                 cont2++;
//                 //Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i], path[i+1],direction(&nodeVec[path[i]], &nodeVec[path[i+1]]));
//                 //Serial.printf("Modo %d = %s\n\n", path[i], mode(&nodeVec[path[i]]));
//                 dirVec.push_back(mode(&nodeVec[path[i]]));
//             }
//             // else{
//             //     Serial.printf("Modo %d = %s\n\n", cont1, mode(&nodeVec[path[i-1]]));
//             //     dirVec[i-1]
//             //     break;
//             // }
//             cont1++;
//             cont2 = 1;
//         }
//         else if(nodeVec[path[i]].prev != NULL && nodeVec[path[i]].next != NULL && i==2){ //Estamos no terceiro no
//             // printf("ESTAMOS NO 3\n");
//             // printf("i = %d\n", i);
//             //printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i-2], path[i-1], direction(&nodeVec[path[i-2]], &nodeVec[path[i-1]]));
//             cont2++;
//             //printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i-1], path[i],direction(&nodeVec[path[i-1]], &nodeVec[path[i]]));
//             //printf("Modo %d = %s\n\n", path[i-1], mode(&nodeVec[path[i-1]]));
//             dirVec.push_back(mode(&nodeVec[path[i-1]]));
//             cont1++;
//             cont2 = 1;
//             if(nodeVec[path[i]].next != NULL){
//                 //printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i-1], path[i], direction(&nodeVec[path[i-1]], &nodeVec[path[i]]));
//                 cont2++;
//                 //printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i], path[i+1],direction(&nodeVec[path[i]], &nodeVec[path[i+1]]));
//                 //printf("Modo %d = %s\n\n", path[i], mode(&nodeVec[path[i]]));
//                 dirVec.push_back(mode(&nodeVec[path[i]]));
//                 cont1++;
//                 cont2 = 1;
//             }
//         }
//         else if(nodeVec[path[i]].next == NULL) { //Estamos no ultimo no
//             if(path[i] == 0 || path[i] == 1 || path[i] == 2 || path[i] == 3 || path[i] == 16 || path[i] == 23 || path[i] == 12 || path[i] == 19){
//                 dirVec.push_back("Pick");  //Ultimo modo do vetor de modos (Uma posicao a menos do que o vetor de nos)
//                 //printf("Modo %d = Pick\n", path[i]);
//             }
//             else if(path[i] == 15 || path[i] == 22 || path[i] == 11 || path[i] == 18 || path[i] == 31 || path[i] == 32 || path[i] == 33 || path[i] == 34){
//                 dirVec.push_back("Drop");  
//                 //printf("Modo %d = Drop\n", path[i]);
//             }
//             else{
//                 dirVec.push_back("Error");  
//                 //printf("Modo %d = Error\n", path[i]);
//             }
//             break;
//         }
//     }
//     return dirVec;
// }


// =================================================================


#include "direction.h"

Direction::Direction() {
    nNodes = N_NODES;
}

char Direction::direction(Node *no1, Node *no2) {   
    // printf("1-x:%d-y:%d\n", no1->xpos, no1->ypos);
    // printf("2-x:%d-y:%d\n", no2->xpos, no2->ypos);
        if (no1->xpos < no2->xpos)
        return 'd';
    else if (no1->xpos > no2->xpos)
        return 'u';
    else if (no1->ypos > no2->ypos)
        return 'l';
    else if (no1->ypos < no2->ypos)
        return 'r';
    return 'e';
}

const char* Direction::mode(Node *intermedio){
    if ((direction(intermedio->prev, intermedio) == 'u' && direction(intermedio, intermedio->next) == 'u') ||
        (direction(intermedio->prev, intermedio) == 'd' && direction(intermedio, intermedio->next) == 'd') ||
        (direction(intermedio->prev, intermedio) == 'l' && direction(intermedio, intermedio->next) == 'l') ||
        (direction(intermedio->prev, intermedio) == 'r' && direction(intermedio, intermedio->next) == 'r'))
        return "Line";
    else

    if ((direction(intermedio->prev, intermedio) == 'u' && direction(intermedio, intermedio->next) == 'l') ||
            (direction(intermedio->prev, intermedio) == 'l' && direction(intermedio, intermedio->next) == 'd') ||
            (direction(intermedio->prev, intermedio) == 'd' && direction(intermedio, intermedio->next) == 'r') ||
            (direction(intermedio->prev, intermedio) == 'r' && direction(intermedio, intermedio->next) == 'u'))
        return "Left";
    else

    if ((direction(intermedio->prev, intermedio) == 'u' && direction(intermedio, intermedio->next) == 'r') ||
            (direction(intermedio->prev, intermedio) == 'r' && direction(intermedio, intermedio->next) == 'd') ||
            (direction(intermedio->prev, intermedio) == 'd' && direction(intermedio, intermedio->next) == 'l') ||
            (direction(intermedio->prev, intermedio) == 'l' && direction(intermedio, intermedio->next) == 'u'))
        return "Right";

    else

        return "Error";
}

void Direction::findEdges(int no1) {
    printf("Adj de no %d: ", no1);
    for (int i=0; i<35; i++){
        if(graph[no1][i] == 1){
            printf("%d ", i);
        }
    }
    printf("\n");
}

void Direction::nodeVector(Node *nodeVec){  //Define os ponteiros para os nos anteriores e proximos a NULL e as coordenadas
    for(int i=0; i<nNodes; i++){
        nodeVec[i].prev = NULL;
        nodeVec[i].next = NULL;
        nodeVec[i].xpos = pos[0][i];
        nodeVec[i].ypos = pos[1][i];
    //    if(i == 4 || i == 10 || i == 14 || i == 21 || i == 25 || i == 31) printf("\n");
        // printf("node[%i]:  pos = %d,%d\n", i, nodeVec[i].xpos, nodeVec[i].ypos);
    }
}

std::vector<std::string> Direction::definePath(int *path, int size, Node *nodeVec){
    
    int cont1 = 1, cont2 = 1;
    std::vector<std::string> dirVec;

    if (size <= 1) {
        // Handle the case when size is 0
        // Set initial mode or direction as needed
        // Serial.printf("Modo ");
        // Serial.print(path[0]);
        // Serial.println(" = Fim");
        // Serial.println();
        dirVec.push_back("Fim");
        cont1 = 1;
        cont2 = 1;
        return dirVec;
    }

    // // Serial.printf("Comecar\n");
    for(int i=0; i<size; i++){
        if (i == 0) {
            nodeVec[path[i]].prev = NULL;
            nodeVec[path[i]].next = &nodeVec[path[i+1]];

            
            switch (direction(&nodeVec[path[i]], &nodeVec[path[i+1]]))
            {
            case 'u':
                initialCompass = 0;
                break;

            case 'r':
                initialCompass = 1;
                break;

            case 'd':
                initialCompass = 2;
                break;
            
            case 'l':
                initialCompass = 3;
                break;

            default:
                break;
            }
            // printf("No inicial\n");
        }
        else if (i == (size)-1){
            nodeVec[path[i]].prev = &nodeVec[path[i-1]];
            nodeVec[path[i]].next = NULL;
            // printf("No final\n");
        }
        else{
            nodeVec[path[i]].prev = &nodeVec[path[i-1]];
            nodeVec[path[i]].next = &nodeVec[path[i+1]];
            // printf("No ao meio\n");
        }
    }

    for (int i=0; i<size; i++){
        // // Serial.printf("size is ");
        // // Serial.println(size);
        // // Serial.printf("path[i] is ");
        // // Serial.println(path[i]);
        // // Serial.printf("path[i+1] is ");
        // // Serial.println(path[i+1]);
        if(nodeVec[path[i]].next == NULL){ //Estamos no ultimo no
            // Serial.printf("Modo ");
            // Serial.print(path[i]);
            // Serial.println(" = Fim");
            // Serial.println();
            dirVec.push_back("Fim");
            cont1 = 1;
            cont2 = 1;
            break;
        }

        // MODO FNR (Feira Nacional de Robotica)
        else if(nodeVec[path[i+1]].next == NULL) { //Estamos no penultimo no
            if(path[i+1] == 0 || path[i+1] == 1 || path[i+1] == 2 || path[i+1] == 3){
                std::cout << "Direcao" << cont1 << "." << cont2 << " : " << path[i-1] << " -> " << path[i] << " = " << direction(&nodeVec[path[i-1]], &nodeVec[path[i]]) << std::endl;
                // Serial.println(cont1);
                // Serial.printf(".");
                cont2++;
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i], path[i+1],direction(&nodeVec[path[i]], &nodeVec[path[i+1]]));
                if(mode(&nodeVec[path[i]]) == "Right") {
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = AdjustL");
                    dirVec.push_back("AdjustL");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = TRight");
                    dirVec.push_back("TRight");
                }
                else if(mode(&nodeVec[path[i]]) == "Left") {
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = AdjustL");
                    dirVec.push_back("AdjustL");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = TLeft");
                    dirVec.push_back("TLeft");
                }
                dirVec.push_back("Pick");
            }
            else if(path[i+1] == 16 || path[i+1] == 23 || path[i+1] == 12 || path[i+1] == 19){
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i-1], path[i], direction(&nodeVec[path[i-1]], &nodeVec[path[i]]));
                cont2++;
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i], path[i+1],direction(&nodeVec[path[i]], &nodeVec[path[i+1]]));
                if(mode(&nodeVec[path[i]]) == "Right") {
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = AdjustL");
                    dirVec.push_back("AdjustL");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = TRight");
                    dirVec.push_back("TRight");
                }
                else if(mode(&nodeVec[path[i]]) == "Left") {
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = AdjustL");
                    dirVec.push_back("AdjustL");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = TLeft");
                    dirVec.push_back("TLeft");
                }
                dirVec.push_back("Pick");
            }
            else if(path[i+1] == 15 || path[i+1] == 22 || path[i+1] == 11 || path[i+1] == 18){
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i-1], path[i], direction(&nodeVec[path[i-1]], &nodeVec[path[i]]));
                cont2++;
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i], path[i+1],direction(&nodeVec[path[i]], &nodeVec[path[i+1]]));
                if(mode(&nodeVec[path[i]]) == "Right") {
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = AdjustL");
                    dirVec.push_back("AdjustL");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = TRight");
                    dirVec.push_back("TRight");
                }
                else if(mode(&nodeVec[path[i]]) == "Left") {
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = AdjustL");
                    dirVec.push_back("AdjustL");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = TLeft");
                    dirVec.push_back("TLeft");
                }
                dirVec.push_back("Drop");
            }
            else if(path[i+1] == 31 || path[i+1] == 32 || path[i+1] == 33 || path[i+1] == 34){
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i-1], path[i], direction(&nodeVec[path[i-1]], &nodeVec[path[i]]));
                cont2++;
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i], path[i+1],direction(&nodeVec[path[i]], &nodeVec[path[i+1]]));
                if(mode(&nodeVec[path[i]]) == "Right") {
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = AdjustL");
                    dirVec.push_back("AdjustL");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = TRight");
                    dirVec.push_back("TRight");
                }
                else if(mode(&nodeVec[path[i]]) == "Left") {
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = AdjustL");
                    dirVec.push_back("AdjustL");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = TLeft");
                    dirVec.push_back("TLeft");
                }
                dirVec.push_back("Drop");
            }
            else{
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i-1], path[i], direction(&nodeVec[path[i-1]], &nodeVec[path[i]]));
                cont2++;
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i], path[i+1],direction(&nodeVec[path[i]], &nodeVec[path[i+1]]));
                if(mode(&nodeVec[path[i]]) == "Right") {
                    dirVec.push_back("RLine");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = RLine");
                    // Serial.println();
                }
                else if(mode(&nodeVec[path[i]]) == "Left") {
                    dirVec.push_back("LLine");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = LLine");
                    // Serial.println();
                }
                else{
                    dirVec.push_back("Line");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.printf(" = ");
                    // Serial.println("Line");
                    // Serial.println();
                }
                // Serial.printf("Modo ");
                // Serial.print(path[i]);
                // Serial.println(" = AdjustL");
                dirVec.push_back("AdjustL");
            }
        }

        
        else if(nodeVec[path[i]].prev != NULL && nodeVec[path[i+1]].next != NULL && nodeVec[path[i]].next != NULL){ //Nao e o primeiro, penultimo nem o ultimo
        //                                                                             Como i>2 estamos em algum no entre o quarto e o penultimo (pois o ultimo tem .next = NULL)
        //                                                                             Atribuimos apenas o modo do anterior a ele 
            if (strcmp(mode(&nodeVec[path[i]]), "Error") != 0){
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i-1], path[i], direction(&nodeVec[path[i-1]], &nodeVec[path[i]]));
                cont2++;
                // Serial.printf("Direcao%d.%d : %d -> %d = %c\n", cont1, cont2, path[i], path[i+1],direction(&nodeVec[path[i]], &nodeVec[path[i+1]]));
                if(mode(&nodeVec[path[i]]) == "Right") {
                    dirVec.push_back("RLine");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = RLine");
                    // Serial.println();
                }
                else if(mode(&nodeVec[path[i]]) == "Left") {
                    dirVec.push_back("LLine");
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.println(" = LLine");
                    // Serial.println();
                }
                else{
                    // Serial.printf("Modo ");
                    // Serial.print(path[i]);
                    // Serial.printf(" = ");
                    // Serial.println(mode(&nodeVec[path[i]]));
                    // Serial.println();
                    dirVec.push_back(mode(&nodeVec[path[i]]));
                }
            }
            cont1++;
            cont2 = 1;
        }
        else if(nodeVec[path[i]].prev == NULL && nodeVec[path[i]].next != NULL){ //Estamos no primeiro no e temos um proximo no
            // Serial.println();
            // Serial.printf("Direcao%d : %d -> %d = %c\n", cont1, path[i], path[i+1], direction(&nodeVec[path[i]], &nodeVec[path[i+1]]));
            // Serial.printf("Modo ");
            // Serial.print(path[i]);
            // Serial.println(" = Line");
            // Serial.println();
            dirVec.push_back("Line");
            cont1++;
        }
    }

    return dirVec;
}