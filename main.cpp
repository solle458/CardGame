#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <deque>
#include <algorithm>
#include <memory> // std::shared_ptr のために追加
#include <string> // std::string のため
#include <unordered_set>

#include "beamSearch.hpp"
// #include "simulatedAnnealing.hpp"
#include "mcts.hpp"
#include "GA.hpp"
#include "Timer.hpp" // Timer.hpp の内容は不明ですが、そのまま使用
#include "CardGame.hpp" // CardGame.hpp (State) を直接インクルード
#include "Com.hpp"      // Com.hpp を直接インクルード

std::vector<std::deque<int>> testAiScore(const int game_number, const std::string& algorithm, std::shared_ptr<const Com> com_data)
{
    int best_score = -1;
    std::vector<int> best_cards_deck; // 変数名を変更 (best_cards -> best_cards_deck)
    std::vector<std::deque<int>> results;
    std::mt19937 mt_for_construct(10); // シードを固定
    Timer timer(-1); // Timerの使い方は元のまま

    for (int i = 0; i < game_number; i++)
    {
        // std::cout << "Game: " << i + 1 << std::endl;

        // State (CardGame) の生成時に com_data を渡す
        auto state = State(mt_for_construct(), com_data);
        timer.restart();

        while (!state.isDone())
        {
            int action = -1;
            if (algorithm == "BeamSearch") {
                action = beamSearch(state, 100, END_TURN);
            }
            else if (algorithm == "MCTS") {
                action = monteCarloTreeSearch(state, 10000000, 1000, 1.414);
            }
            else if (algorithm == "GA") {
                action = geneticAlgorithmSearch(state, 200, 500);
            }
            // else if (algorithm == "SimulatedAnnealing") {
            //     action = simulatedAnnealing(state, 100.0, 0.95, 0.001, 50);
            // }
            else {
                std::cout << "Invalid algorithm" << std::endl;
                return {};
            }
            state.advance(action);
        }

        auto elapsed_sec = timer.elapsedMs();
        std::cout << "Time : " << elapsed_sec << " s" << std::endl; // msからsへ
        state.show();

        auto score = state.game_score_;
        // 結果の保存
        std::deque<int> current_game_cards_deque(state.card_.begin(), state.card_.end());
        current_game_cards_deque.push_front(score);
        results.push_back(current_game_cards_deque);

        if (best_score < score)
        {
            best_score = score;
            best_cards_deck = state.card_;
        }
    }
    // std::cout << "Best Score: " << best_score << std::endl;
    // std::cout << "Best Cards Deck: ";
    // for (size_t i = 0; i < best_cards_deck.size(); ++i)
    // {
    //     std::cout << best_cards_deck[i] << (i == best_cards_deck.size() - 1 ? "" : ", ");
    // }
    return results;
}

void ensemble(int game_number, std::shared_ptr<const Com> com_data) {
    std::vector<std::deque<int>> high_scores;

    auto mcts_results = testAiScore(game_number, "MCTS", com_data);
    for(const auto& res : mcts_results){
        high_scores.push_back(res);
    }
    auto bs_results = testAiScore(game_number, "BeamSearch", com_data);
    for(const auto& res : bs_results){
        high_scores.push_back(res);
    }
    auto ga_results = testAiScore(game_number, "GA", com_data);
    for(const auto& res : ga_results){
        high_scores.push_back(res);
    }
    // auto sa_results = testAiScore(game_number, "SimulatedAnnealing", com_data);
    // for(const auto& res : sa_results){
    //     high_scores.push_back(res);
    // }

    std::sort(high_scores.rbegin(), high_scores.rend());

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

    // Validation
    std::unordered_set<int> final_cards(ensemble_result.begin(), ensemble_result.end());
    if(final_cards.size() != NumberOfCards) {
        std::cout << "WARNING: Not all unique cards placed!" << std::endl;
    }
}


int main()
{
    // Comオブジェクトを一度だけ生成し、shared_ptrで管理
    auto com_data = std::make_shared<const Com>();

    int game_number = 1; // テストのため回数を減らす
    testAiScore(game_number, "SimulatedAnnealing", com_data);
    // ensemble(game_number, com_data); // ensembleも実行する場合
    return 0;
}
