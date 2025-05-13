#include <algorithm>
#include <deque>
#include <iostream>
#include <random>
#include <vector>

#include "simulatedAnnealing.hpp"

using namespace std;
using ll = long long;
mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

// ===== パラメータ個体の定義 =====
struct Individual {
    double startTemp;
    double endTemp;
    double swapRate;  // 近傍変化の比率
    double score;

    Individual() {
        startTemp = rng() % 9901 + 100;                          // 100 ~ 10000
        endTemp = pow(10, -((rng() % 5) + 2));                   // 1e-2 ~ 1e-6
        swapRate = uniform_real_distribution<>(0.01, 0.3)(rng);  // 0.01 ~ 0.3
        score = -1;
    }

    void mutate() {
        if (uniform_real_distribution<>(0, 1)(rng) < 0.3)
            startTemp *= uniform_real_distribution<>(0.8, 1.2)(rng);
        if (uniform_real_distribution<>(0, 1)(rng) < 0.3)
            endTemp *= uniform_real_distribution<>(0.8, 1.2)(rng);
        if (uniform_real_distribution<>(0, 1)(rng) < 0.3)
            swapRate *= uniform_real_distribution<>(0.8, 1.2)(rng);
    }
};

// ===== 評価関数（焼きなまし法） =====
double runSimulatedAnnealing(const Individual& ind) {
    auto start = system_clock::now();
    int timeLimit = 15;

    deque<int> currentDeck(N), bestDeck(N);
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
        double elapsed =
            duration_cast<milliseconds>(now - start).count() / 1000.0;
        if (elapsed > timeLimit) break;

        double t = elapsed / timeLimit;
        double temperature = startTemp * pow(endTemp / startTemp, t);

        deque<int> nextDeck = currentDeck;
        int numSwaps =
            uniform_int_distribution<int>(1, int(ind.swapRate * N))(rng);
        for (int k = 0; k < numSwaps; ++k) {
            int i = uniform_int_distribution<int>(0, N - 1)(rng);
            int j = uniform_int_distribution<int>(0, N - 1)(rng);
            swap(nextDeck[i], nextDeck[j]);
        }

        double nextScore = evaluate(nextDeck);
        double diff = nextScore - currentScore;

        if (diff >= 0 || exp(diff / temperature) >
                             uniform_real_distribution<>(0.0, 1.0)(rng)) {
            currentDeck = nextDeck;
            currentScore = nextScore;
            if (nextScore > bestScore) {
                bestScore = nextScore;
                bestDeck = nextDeck;
            }
        }
    }
    return bestScore;
}

// ===== 遺伝的アルゴリズム =====
int main() {
    const int POP_SIZE = 20;
    const int GENERATIONS = 30;
    const int ELITE = 4;

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

    fin.close();

    vector<Individual> population(POP_SIZE);
    for (auto& ind : population) ind.score = runSimulatedAnnealing(ind);

    for (int gen = 0; gen < GENERATIONS; ++gen) {
        sort(population.begin(), population.end(),
             [](const Individual& a, const Individual& b) {
                 return a.score > b.score;
             });

        cerr << "Gen " << gen << " best: " << population[0].score << "\n";

        vector<Individual> newPop;
        for (int i = 0; i < ELITE; ++i) newPop.push_back(population[i]);

        while (newPop.size() < POP_SIZE) {
            // トーナメント選択
            Individual p1 = population[rng() % ELITE];
            Individual p2 = population[rng() % ELITE];

            Individual child;
            child.startTemp = (p1.startTemp + p2.startTemp) / 2;
            child.endTemp = sqrt(p1.endTemp * p2.endTemp);
            child.swapRate = (p1.swapRate + p2.swapRate) / 2;

            child.mutate();
            child.score = runSimulatedAnnealing(child);
            newPop.push_back(child);
        }

        population = newPop;
    }

    auto best = *max_element(population.begin(), population.end(),
                             [](const Individual& a, const Individual& b) {
                                 return a.score < b.score;
                             });

    cout << "Best parameters:\n";
    cout << "startTemp = " << best.startTemp << "\n";
    cout << "endTemp = " << best.endTemp << "\n";
    cout << "swapRate = " << best.swapRate << "\n";
    cout << "score = " << best.score << "\n";
}
