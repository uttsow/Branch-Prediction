#include "Predictor.hpp"

#include<iostream>
#include<fstream>
#include<sstream>


using namespace std;



int main(int argc, char const *argv[]) {
    vector<long> hexAddrVect;
    vector<string> predictorVect;

    //Invalid usage
    if(argc != 3){
        perror("Need 3 inputs: ./predictors <input_file.txt> <output_file.txt> \n");
        exit(EXIT_FAILURE);
    }

    //Valid usage. read from file line by line
    ifstream infile(argv[1]);
    string line;

    while(getline(infile, line)){
        string addr = line.substr(0, line.find(" "));
        string pred = line.substr(line.find(" ") + 1, line.length());
        //converting from string long
        long hexAddr = stol(addr, nullptr, 16);

        //Store values in the vectors.
        hexAddrVect.push_back(hexAddr);
        predictorVect.push_back(pred);
    }

    Predictor pred(hexAddrVect, predictorVect);
    pred.runSimulation(argv[2]);
    infile.close();

    return 0;
}
