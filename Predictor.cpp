#include "Predictor.hpp"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cmath>


#define TAKEN "T"
#define NOTTAKEN "NT"

//Inital Constructor
Predictor::Predictor(vector<long> aHexAddr, vector<string> aPrediction){
    hexAddr = aHexAddr;
    prediction = aPrediction;

    if(hexAddr.size() != prediction.size()){
        perror("Inputs not same! Input files wrong");
        exit(EXIT_FAILURE);
    }

}

//Helper Function
pair<long, unsigned long> Predictor::oneWayBranch(string option){
    int numCorrect = 0;

    //checks the option and counts total correct
    for(unsigned int i = 0; i < hexAddr.size(); i++){
        if(prediction.at(i) == option){
            numCorrect++;
        }
    }

    pair<long, unsigned long> retVal(numCorrect, hexAddr.size());
    return retVal;
}


pair<long, unsigned long> Predictor::alwaysTaken(){
    return oneWayBranch(TAKEN);
}

pair<long, unsigned long> Predictor::alwaysNotTaken(){
    return oneWayBranch(NOTTAKEN);
}


/*
    Bimodal Predictor with a single bit of history stored in each predictor entry. Determine the prediction accuracy of this predictor for the table size of 16, 32, 128, 256, 512, 1024 and 2048 entries. Assume that the initial state of all prediction counters is “Not Taken” (N)
*/

pair<long, unsigned long> Predictor::bimodalSingle(int tableSize){
    //# correct tracker
    int numCorrect = 0;

    //Table with given size
    vector<string> table(tableSize, NOTTAKEN);

    for(unsigned int i =0; i <prediction.size(); i++){

        //actual prediction
        string actPred = prediction[i];

        //index comes from the address modded with tablsize
        int index = hexAddr[i] % tableSize;

        //use index to find the next prediction
        string currPred = table[index];

        //Predict branch to be not taken initally
        if(currPred == NOTTAKEN){
            if(actPred == NOTTAKEN){    //Correct!
                numCorrect++;
            }else{ // Incorrect. Flip it
                table[index] = TAKEN;
            }
        }else if(currPred == TAKEN){
            if(actPred == TAKEN){
                numCorrect++;
            }else{
                table[index] = NOTTAKEN;
            }
        }
    }

    //returns # of correct and total predictions
    pair<long,long> retVal(numCorrect, hexAddr.size());
    return retVal;
}

/*
    Same as previous but with 2 bits (Saturated)

    Rules:
    0 = Strong not taken
    1 = weakly not taken
    --------------------
    2 = weakly taken
    3 = strongly taken
*/

pair<long, unsigned long> Predictor::bimodalDouble(int tableSize){
    int numCorrect = 0;

    //initalize table to not taken
    vector<int> table(tableSize, 1);

    for(unsigned int i = 0; i < prediction.size(); i++){
        //actual prediction
        string actPred = prediction[i];

        //index comes from the address modded with tablsize
        int index = hexAddr[i] % tableSize;

        //use index to find the next prediction
        int currPred = table[index];

        //check if 2 or under (NOT TAKEN);
        if(currPred < 2 ){
            if(actPred == NOTTAKEN){

                //if its 1, change to strong not taken
                if(currPred == 1){
                    table[index] = 0;

                }
                numCorrect++;
            }else{ //incorrect prediction. it was taken


                table[index] = (currPred + 1);
            }
        }else if(currPred >= 2){
            if(actPred == TAKEN){

                if(currPred == 2){
                    table[index] = 3;
                }
                numCorrect++;
            }else{
                table[index] = (currPred - 1);
            }
        }
    }

    pair<long, unsigned long> retVal(numCorrect, hexAddr.size());
    return retVal;
}



/*
    Global Register (SR) Updater -> This will be used for the GShare Predictor. Helper function
    Steps:
    0. Determine if 0(NT) or 1(T) needs to be added
    1. shift to left by 1
    2. OR the results with the new prediction
    3. use the history length. AND the global register and 2^history - 1. This keeps the length
    4. Update it (pointer);
*/

void Predictor::globalRegisterUpdate(int *globalRegister, const string prediction, int historyLength){
    int globalRegPVal = *globalRegister;

    //1. Ex: 1
    int newBit = (prediction == TAKEN) ? 1: 0;

    //2. Ex: 0101 -> 1010
    globalRegPVal = (int)(globalRegPVal << 1);

    //3. Ex: 1010 -> 1011
    globalRegPVal = (int)(globalRegPVal | newBit);

    //4. History = 3, 2^4 = 10000 -1 = 01111.
    //1011 & 01111 = 01011
    globalRegPVal = (int)(globalRegPVal & ((int)pow(2,historyLength)-1));

    //5. Update value
    *globalRegister = globalRegPVal;


}


/*
    Gshare predictor, where the PC is XOR-ed with the global history bits to generate the index into the predictor table. Fix the table size at 2048 entries and determine the prediction accuracy as a function of the number of bits in the global history register. Vary the history length from 3 bits to 11 bits in 1-bit increments. Assume that the initial state of all prediction counters is “Not Taken” (NT). The global history register should be initialized to contain all zeroes (where 0=NT and 1=T). The global history register should be maintained such that the least significant bit of the register represents the result of the most recent branch, and the most significant bit of the register represents the result of the least recent branch in the history.

    Rules:
    0 = Strong not taken
    1 = weakly not taken
    --------------------
    2 = weakly taken
    3 = strongly taken

*/

pair<long, unsigned long> Predictor::gshare(int aHistoryLength){
    int historyLength = aHistoryLength;
    int numCorrect = 0;
    //fixed table size;
    int tableSize = 2048;


    //inital vector table. initalized to NT (1);
    vector<int> table(tableSize, 1);

    int globalReg = 0;

    for(unsigned int i =0; i < prediction.size(); i++){

        //Inital location of the index
        int initalLoc = hexAddr[i] % tableSize;

        //New Index: XOR PC And Global Register
        int index = initalLoc ^ globalReg;

        //Prediction
        int currPred = table[index];

        //Actual result
        string actPred = prediction[i];

        if(currPred < 2 ){
            if(actPred == NOTTAKEN){

                //if its 1, change to strong not taken
                if(currPred == 1){
                    table[index] = 0;

                }
                numCorrect++;
            }else{ //incorrect prediction. it was taken


                table[index] = (currPred + 1);
            }
        }else if(currPred >= 2){
            if(actPred == TAKEN){

                if(currPred == 2){
                    table[index] = 3;
                }
                numCorrect++;
            }else{
                table[index] = (currPred - 1);
            }
        }
        //update the global register now
        globalRegisterUpdate(&globalReg, actPred, historyLength);
    }

    pair<long, unsigned long> retVal(numCorrect, hexAddr.size());
    return retVal;

}



/*
    Tournament Predictor. The tournament predictor selects between gshare and bimodal predictor for every branch. Configure gshare with 2048-entry table and 11 bits of global history, and configure bimodal predictor with 2048-entry table. Furthermore, configure the selector table with 2048 entries and use the same index as you use for bimodal predictor to index into the selector table (that is, the PC). For each entry in the selector, the two-bit counter encodes the following states: 00 – prefer Gshare, 01 – weakly prefer Gshare, 10 – weakly prefer Bimodal, 11 – prefer bimodal. If the two predictors provide the same prediction, then the corresponding selector counter remains the same. If one of the predictors is correct and the other one is wrong, then the selector’s counter is decremented or incremented to move towards the predictor that was correct. Initialize all the component predictors to “Not Taken” and initialize the selector’s counters to “Weakly prefer Bimodal”.

    Rules:
    GSHARE
    0 = Strong not taken
    1 = weakly not taken
    --------------------
    BIMODAL
    2 = weakly taken
    3 = strongly taken

*/

pair<long, unsigned long> Predictor::tournament(){
    int numCorrect = 0;
    int tableSize = 2048;

    //Tables for each predictor

    //Selector -> Weakly Prefer Bimodal (2);
    vector<int> tournamentTable(tableSize, 2);

    //component -> Not Taken (1);
    vector<int> bimodalTable(tableSize, 1);
    vector<int> gshareTable(tableSize, 1);

    //global register for gshare (11 bits);
    int globalReg = 0;

    for(unsigned int i = 0; i <prediction.size(); i++){
        //inital index
        int index = hexAddr[i] % tableSize;

        //Prediction
        int currPred = tournamentTable[index];

        //components
        bool bimodalRes = tournmentBimodal(&bimodalTable, i);
        bool gshareRes = tournmentGshare(&gshareTable, i, &globalReg);

        //Guess less than 2. Check gShare
        if(currPred <2){
            //if Correct
            if(gshareRes){
                //Check that its not also bimodal and if prediction is 1
                if(!bimodalRes && currPred == 1){
                    tournamentTable[index] = 0;
                }
                //update counter
                numCorrect++;

                //Bimodal was correct. Prediction wrong
            }else if(bimodalRes){


                    tournamentTable[index] = (currPred + 1);


            }
        }else if(currPred >= 2){   //Check bimodal
            //Bimodal correct
            if(bimodalRes){

                if(!gshareRes && currPred == 2){
                    tournamentTable[index] = 3;
                }
                numCorrect++;

            }else if(gshareRes){ //Wrong prediction

                    tournamentTable[index] = (currPred - 1);

            }
        }

    }

    pair<long, unsigned long> retVal(numCorrect, hexAddr.size());
    return retVal;


}


/*
    Helper function for tournment. Basically same as the
    the regular Gshare except it returns bool and has a
    fixed size of 2048 and global history is 11 bits
*/

bool Predictor::tournmentGshare(vector<int> *aGshareTable, int aIndex, int *aGlobalHistory){
    bool retVal = false;
    int tableSize = 2048;
    int historyLength = 11;
    //Inital location of the index
    int initalLoc = hexAddr[aIndex] % tableSize;

    //New Index: XOR PC And Global Register
    int index = initalLoc ^ *aGlobalHistory;

    //Prediction
    int currPred = (*aGshareTable)[index];

    //Actual result
    string actPred = prediction[aIndex];

    if(currPred < 2 ){
        if(actPred == NOTTAKEN){

            //if its 1, change to strong not taken
            if(currPred == 1){
                (*aGshareTable)[index] = 0;

            }
            retVal = true;
        }else{ //incorrect prediction. it was taken
            (*aGshareTable)[index] = (currPred + 1);
            retVal = false;
        }
    }else if(currPred >= 2){
        if(actPred == TAKEN){

            if(currPred == 2){
                (*aGshareTable)[index] = 3;
            }
            retVal = true;
        }else{
            (*aGshareTable)[index] = (currPred - 1);
            retVal = false;
        }
    }
    //update the global register now
    globalRegisterUpdate(aGlobalHistory, actPred, historyLength);
    return retVal;
}


/*
    Helper function for tournment. Basically same as the
    the regular bimodal except it returns bool and has a
    fixed size of 2048
*/
bool Predictor::tournmentBimodal(vector<int> *aBimodalTable, int aIndex){
    bool retVal = true;
    int tableSize = 2048;
    int index = hexAddr[aIndex] % tableSize;
    int currPred = (*aBimodalTable)[index];
    string actPred = prediction[aIndex];

    //check if 2 or under (NOT TAKEN);
    if(currPred < 2 ){
        if(actPred == NOTTAKEN){

            //if its 1, change to strong not taken
            if(currPred == 1){
                (*aBimodalTable)[index] = 0;

            }
            retVal = true;
        }else{ //incorrect prediction. it was taken
            (*aBimodalTable)[index] = (currPred + 1);
            retVal = false;
        }
    }else if(currPred >= 2){
        if(actPred == TAKEN){

            if(currPred == 2){
                (*aBimodalTable)[index] = 3;
            }
            retVal = true;
        }else{
            (*aBimodalTable)[index] = (currPred - 1);
            retVal = false;
        }
    }

    return retVal;

}


//Run the simulator 
void Predictor::runSimulation(string output){
    vector<int> tableSize = {16, 32, 128, 256, 512, 1024, 2048};
    pair<long, unsigned long> ansHolder;
    ofstream outputFile(output);

    if(!(outputFile.is_open())){
        perror("Error opening file, check output file name!\n");
        exit(EXIT_FAILURE);
    }

    //Question 1, 2 (Single outcome)
    //1. Always taken
    ansHolder = alwaysTaken();
    outputFile << ansHolder.first << "," << ansHolder.second << ";\n";

    //2. Always NOT Taken
    ansHolder = alwaysNotTaken();
    outputFile << ansHolder.first << "," << ansHolder.second << ";\n";


    //3. Single bit bimodal
    for(unsigned int i = 0; i < tableSize.size(); i++){
        ansHolder = bimodalSingle(tableSize[i]);
        outputFile << ansHolder.first << "," << ansHolder.second << ";";

        //if else to check if i == end of table size
        outputFile << ((i == tableSize.size() - 1) ? "" : " ");
    }
    outputFile << "\n";

    //4. Double bit
    for(unsigned int i = 0; i < tableSize.size(); i++){
        ansHolder = bimodalDouble(tableSize[i]);
        outputFile << ansHolder.first << "," << ansHolder.second << ";";

        //if else to check if i == end of table size
        outputFile << ((i == tableSize.size() - 1) ? "" : " ");
    }
    outputFile << "\n";


    //5. Gshare
    for(int i = 3; i <= 11; i++){
        ansHolder = gshare(i);
        outputFile << ansHolder.first << "," << ansHolder.second << ";";

        outputFile << ((i == 11) ? "" : " ");
    }
    outputFile << "\n";


    //6. Tournment Predictor
    ansHolder = tournament();
    outputFile << ansHolder.first << "," << ansHolder.second << ";\n";

}
