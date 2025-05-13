#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <deque>
#include <unordered_set>
#include <limits>

using namespace std;

using ll = long long;
using namespace chrono;

const int N = 50;
vector<vector<int>> opponent(N, vector<int>(N));

// スコアを計算
double evaluate(const deque<int>& myDeck) {
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
deque<int> simulatedAnnealing(double timeLimit) {
    auto start = system_clock::now();

    deque<int> currentDeck(N), bestDeck(N);
    iota(currentDeck.begin(), currentDeck.end(), 1);
    random_device rd;
    mt19937 rng(rd());
    shuffle(currentDeck.begin(), currentDeck.end(), rng);

    double currentScore = evaluate(currentDeck);
    double bestScore = currentScore;

    bestDeck = currentDeck;

    double startTemp = 8357.22, endTemp = numeric_limits<double>::max(), swapRate = 0.183511;

    while (true) {
        auto now = system_clock::now();
        double elapsed =
            duration_cast<milliseconds>(now - start).count() / 1000.0;
        if (elapsed > timeLimit) break;

        double t = elapsed / timeLimit;
        double temperature = startTemp * pow(endTemp / startTemp, t);

        deque<int> nextDeck = currentDeck;
        int numSwaps =
            uniform_int_distribution<int>(1, int(swapRate * N))(rng);
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
    return bestDeck;
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

    fin.close();
    int game_number = 1;
    const int NumberOfCards = 50;
    vector<deque<int>> high_scores;

    for(int i = 0; i < game_number; ++i) {
        deque<int> result = simulatedAnnealing(15.0); // 実行時間 5秒
        high_scores.push_back(result);
    }
    sort(high_scores.rbegin(), high_scores.rend());

    std::cout << "High Scores (Top 10 or less): " << std::endl;
    for(int i = 0; i < std::min((int)high_scores.size(), 10); i++){
        for(size_t j = 0; j < high_scores[i].size(); ++j) {
            std::cout << high_scores[i][j] << (j == high_scores[i].size() - 1 ? "" : ", ");
        }
        std::cout << std::endl;
    }

    // Enhanced card placement statistics
    std::vector<std::vector<std::pair<double, int>>> best_cards_stats(NumberOfCards, std::vector<std::pair<double, int>>(NumberOfCards, {0.0, 0}));

    for(const auto& game_result : high_scores){
        if (game_result.empty()) continue;
        std::deque<int> card_deck = game_result;
        double score_val = static_cast<double>(card_deck.front());
        card_deck.pop_front();

        double processed_score = pow((score_val - 1300.0) / 10.0, 2.0);
        if (processed_score < 0) processed_score = 0;

        for(int j = 0; j < std::min((int)card_deck.size(), NumberOfCards); j++){
            if (card_deck[j] >= 1 && card_deck[j] <= NumberOfCards) { 
                best_cards_stats[card_deck[j]-1][j].first += processed_score;
                best_cards_stats[card_deck[j]-1][j].second = j;
            }
        }
    }

    std::cout << "\nCard Position Statistics (Card C appears at Position P with Score S):" << std::endl;
    for(int i = 0; i < NumberOfCards; i++){
        std::cout << "Card " << i+1 << ": ";
        for(int j = 0; j < NumberOfCards; j++){
            std::cout << best_cards_stats[i][j].first << (j == NumberOfCards - 1 ? "" : ", ");
        }
        std::cout << std::endl;
    }

    std::vector<int> ensemble_result(NumberOfCards, 0);
    std::vector<bool> position_used(NumberOfCards, false);
    std::unordered_set<int> cards_placed;

    // Improved placement strategy 
    for(int card_val_idx = 0; card_val_idx < NumberOfCards; card_val_idx++){
        std::vector<std::pair<double, int>> card_positions = best_cards_stats[card_val_idx];
        std::sort(card_positions.rbegin(), card_positions.rend());

        bool card_placed = false;
        for(const auto& scored_pos_pair : card_positions){
            int pos_idx = scored_pos_pair.second;
            if(!position_used[pos_idx]){
                ensemble_result[pos_idx] = card_val_idx + 1;
                position_used[pos_idx] = true;
                cards_placed.insert(card_val_idx + 1);
                card_placed = true;
                break;
            }
        }

        // If card couldn't be placed in preferred positions
        if (!card_placed) {
            for(int pos_idx = 0; pos_idx < NumberOfCards; pos_idx++) {
                if(!position_used[pos_idx]) {
                    ensemble_result[pos_idx] = card_val_idx + 1;
                    position_used[pos_idx] = true;
                    cards_placed.insert(card_val_idx + 1);
                    break;
                }
            }
        }
    }

    // Verify and fill any remaining positions
    std::unordered_set<int> all_cards;
    for(int i = 1; i <= NumberOfCards; i++) {
        all_cards.insert(i);
    }

    // Find missing cards
    std::vector<int> missing_cards;
    for(int card : all_cards) {
        if(cards_placed.find(card) == cards_placed.end()) {
            missing_cards.push_back(card);
        }
    }

    // Fill remaining positions with missing cards
    for(int pos_idx = 0; pos_idx < NumberOfCards; pos_idx++) {
        if(ensemble_result[pos_idx] == 0 && !missing_cards.empty()) {
            ensemble_result[pos_idx] = missing_cards.back();
            missing_cards.pop_back();
        }
    }

    std::cout << "\nEnsemble Result (Final Deck): ";
    for(size_t i = 0; i < ensemble_result.size(); ++i){
        std::cout << ensemble_result[i] << (i == ensemble_result.size() - 1 ? "" : ", ");
    }
    std::cout << std::endl;

    return 0;
}
