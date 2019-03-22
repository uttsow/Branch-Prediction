#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <vector>
#include <string>

using namespace std;

class Predictor{

private:
    //Private variable for the Constructor
    vector<long> hexAddr;
    vector<string> prediction;

    //Used for the singular option branches (T, NT);
    pair<long, unsigned long> oneWayBranch(string option);

    //used as a helper function for GShare
    void globalRegisterUpdate(int *globalRegister, const string prediction, int aHistoryLength);

    //Helper function for Tournament
    bool tournmentGshare(vector<int> *aGshareTable, int aIndex, int *aGlobalHistory);
    bool tournmentBimodal(vector<int> *aBimodalTable, int aIndex);


public:
    //Constructor. 2 vector pram, one contains hex addresses and the other is if its T or NT
    Predictor(vector<long> aHexAddr, vector<string> aPrediction);

    /*
        1. Branch always taken. Returns pair <taken, total>
        2. Branch always not taken. returns pair

        both will call a oneWay branch function
    */
    pair<long, unsigned long> alwaysTaken();
    pair<long, unsigned long> alwaysNotTaken();

    //3. Bimodal Predictor with a single bit of history with various table size
    pair<long, unsigned long> bimodalSingle(int tableSize);

    //4. Bimodal Predictor with a double bit of history with various table size;
    pair<long, unsigned long> bimodalDouble(int tableSize);

    //5. GShare predictor
    pair<long, unsigned long> gshare(int aHistoryLength);

    //6. Tournament Predictor
    pair<long, unsigned long> tournament();
    //run the simulation
    void runSimulation(string output);
};

#endif
