#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <numeric>

using namespace std;

using ll = long long;
using namespace chrono;

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
int simulatedAnnealing(Individual ind, double timeLimit) {
    auto start = system_clock::now();

    vector<int> currentDeck(N), bestDeck(N);
    iota(currentDeck.begin(), currentDeck.end(), 1);
    random_device rd;
    mt19937 rng(rd());
    shuffle(currentDeck.begin(), currentDeck.end(), rng);

    double currentScore = evaluate(currentDeck);
    double bestScore = currentScore;

    bestDeck = currentDeck;

    double startTemp = ind.startTemp, endTemp = ind.endTemp;

    while (true) {
        auto now = system_clock::now();
        double elapsed = duration_cast<milliseconds>(now - start).count() / 1000.0;
        if (elapsed > timeLimit) break;

        double t = elapsed / timeLimit;
        double temperature = startTemp * pow(endTemp / startTemp, t);

        vector<int> nextDeck = currentDeck;
        for (int i = 0; i < 1; ++i) {
            int idx1 = uniform_int_distribution<int>(0, N - 1)(rng);
            int idx2 = uniform_int_distribution<int>(0, N - 1)(rng);
            swap(nextDeck[idx1], nextDeck[idx2]);
        }
        // int i = uniform_int_distribution<int>(0, N - 1)(rng);
        // int j = uniform_int_distribution<int>(0, N - 1)(rng);
        // swap(nextDeck[i], nextDeck[j]);

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
    return bestScore;
}

int main() {
    ifstream fin("deck.csv");
    if (!fin) {
        cerr << "deck.csv が開けませんでした。" << endl;
        return 1;
    }

    for (int i = 0; i < N; ++i) {
        string line;
        getline(fin, line);
        stringstream ss(line);
        for (int j = 0; j < N; ++j) {
            string val;
            getline(ss, val, ',');
            opponent[i][j] = stoi(val);
        }
    }

    const int POP_SIZE = 20;
    const int GENERATIONS = 30;
    const int ELITE = 4;

    vector<Individual> population(POP_SIZE);
    for (auto& ind : population)
        ind.score = simulatedAnnealing(ind, 2);

    for (int gen = 0; gen < GENERATIONS; ++gen) {
        sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
            return a.score > b.score;
        });

        cerr << "------------------------\n";
        cerr << "Gen " << gen << " best: " << population[0].score << "\n";
        cerr << "startTemp: " << population[0].startTemp << "\n";
        cerr << "endTemp: " << population[0].endTemp << "\n";
        cerr << "------------------------\n";

        vector<Individual> newPop;
        for (int i = 0; i < ELITE; ++i) newPop.push_back(population[i]);

        while (newPop.size() < POP_SIZE) {
            // トーナメント選択
            Individual p1 = population[rng() % ELITE];
            Individual p2 = population[rng() % ELITE];

            Individual child;
            child.startTemp = (p1.startTemp + p2.startTemp) / 2;
            child.endTemp = sqrt(p1.endTemp * p2.endTemp);

            child.mutate();
            child.score = simulatedAnnealing(child, 2);
            newPop.push_back(child);
        }

        population = newPop;
    }

    auto best = *max_element(population.begin(), population.end(),
        [](const Individual& a, const Individual& b) { return a.score < b.score; });

    cout << "Best parameters:\n";
    cout << "startTemp = " << best.startTemp << "\n";
    cout << "endTemp = " << best.endTemp << "\n";
    cout << "score = " << best.score << "\n";
}
