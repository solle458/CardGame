#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <numeric>
#include <map>

#include "CSVHandler.h"

using namespace std;

using ll = long long;
using namespace chrono;

map<int, int> cardMap;

const int N = 50;
mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
vector<vector<int>> opponent(N, vector<int>(N));

struct Individual {
    double startTemp;
    double endTemp;
    double score;

    Individual() {
        startTemp = rng() % 9901 + 100;             // 100 ~ 10000
        endTemp = pow(10., -((rng() % 5) + 2.));      // 1e-2 ~ 1e-6
        score = -1;
    }

    void mutate() {
        if (uniform_real_distribution<>(0, 1)(rng) < 0.3) startTemp *= uniform_real_distribution<>(0.8, 1.2)(rng);
        if (uniform_real_distribution<>(0, 1)(rng) < 0.3) endTemp *= uniform_real_distribution<>(0.8, 1.2)(rng);
    }
};

// スコアを計算
double evaluate(const vector<int>& myDeck) {
    double totalScore = 0.0;
    for (int game = 0; game < N; ++game) {
        double gameScore = 0.0;
        for (int i = 0; i < N; ++i) {
            if (myDeck[i] > opponent[game][i]) gameScore += 1.0;
            else if (myDeck[i] == opponent[game][i]) gameScore += 0.5;
        }
        totalScore += gameScore;
    }
    return totalScore;
}

// 焼きなまし法
int simulatedAnnealing(double timeLimit) {
    auto start = system_clock::now();

    vector<int> currentDeck(N), bestDeck(N);
    iota(currentDeck.begin(), currentDeck.end(), 1);
    random_device rd;
    mt19937 rng(rd());
    shuffle(currentDeck.begin(), currentDeck.end(), rng);

    double currentScore = evaluate(currentDeck);
    double bestScore = currentScore;

    bestDeck = currentDeck;

    double startTemp = 7340, endTemp = 1e-06;

    while (true) {
        auto now = system_clock::now();
        double elapsed = duration_cast<milliseconds>(now - start).count() / 1000.0;
        if (elapsed > timeLimit) break;

        double t = elapsed / timeLimit;
        double temperature = startTemp * pow(endTemp / startTemp, t);

        vector<int> nextDeck = currentDeck;
        int i = uniform_int_distribution<int>(0, N - 1)(rng);
        int j = uniform_int_distribution<int>(0, N - 1)(rng);
        swap(nextDeck[i], nextDeck[j]);

        double nextScore = evaluate(nextDeck);
        double diff = nextScore - currentScore;

        if (diff >= 0 || exp(diff / temperature) > uniform_real_distribution<>(0.0, 1.0)(rng)) {
            currentDeck = nextDeck;
            currentScore = nextScore;
            if (nextScore > bestScore) {
                bestScore = nextScore;
                bestDeck = nextDeck;
            }
        }
    }
    cerr << "-------------------------\n";
    cerr << "startTemp: " << startTemp << ", endTemp: " << endTemp << endl;
    cerr << "Best score: " << bestScore << endl;
    for(int i = 0; i < N; ++i) {
        cerr << bestDeck[i] << " ";
    }
    cerr << endl;
    cardMap[bestScore]++;
    return bestScore;
}

int main() {
    CSVFile<int> csv;
    csv.csv_read("deck.csv", false, false, ',');
    for (int i = 0; i < N; ++i)for (int j = 0; j < N; ++j) opponent[i][j] = csv.cell[i][j];
    for(int i = 0; i < 100; ++i) {
        double timeLimit = 2.0;
        simulatedAnnealing(timeLimit);
    }
    for(auto& [key, value] : cardMap) {
        cerr << key << ": " << value << endl;
    }
}
