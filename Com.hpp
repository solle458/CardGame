#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "CSVHandler.h"

class Com {
    public:
        std::vector<std::vector<int>> cards;
        Com(const int n = 50){
            CSVFile<int> csv;
            csv.csv_read("deck.csv", false, false, ',');
            cards.resize(n);
            for(int i = 0; i < n; i++){
                cards[i].resize(n);
                for(int j = 0; j < n; j++){
                    cards[i][j] = csv.cell[i][j];
                }
            }
        }

        void show(){
            for(int i = 0; i < cards.size(); i++){
                for(int j = 0; j < cards[i].size(); j++){
                    std::cout << cards[i][j] << " ";
                }
                std::cout << std::endl;
            }
        }
};
