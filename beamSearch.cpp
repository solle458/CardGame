#include <iostream>
#include <random>
#include <algorithm>
#include <numeric>
#include <queue>
#include <chrono>

#include "CardGame.hpp"

using namespace std;
using State = CardGame;

bool operator<(const State& lhs, const State& rhs) {
    return lhs.evaluate_score_ < rhs.evaluate_score_;
}

int beamSearch(const State &state, const int beam_width, const int beam_depth){
    priority_queue<State> now_beam;
    State best_state = state;

    now_beam.push(state);
    for(int i = 0; i < beam_depth; i++){
        priority_queue<State> next_beam;
        for(int j = 0; j < beam_width; j++){
            if(now_beam.empty()) break;
            State now_state = now_beam.top();
            now_beam.pop();
            auto legal_action = now_state.legalAction(now_state.now_cards_[now_state.now_cards_idx_]);
            for(auto action : legal_action){
                State next_state = now_state;
                next_state.advance(action);
                next_state.evaluate();
                if(i == 0){
                    next_state.first_action_ = action;
                }
                next_beam.push(next_state);
            }
        }

        now_beam = next_beam;
        best_state = now_beam.top();

        if(best_state.isDone()){
            break;
        }
    }
    return best_state.first_action_;
}

void testAiScore(const int game_number)
{
    mt19937 mt_for_construct(0);
    int best_score = -1;
    vector<int> best_cards;
    for (int i = 0; i < game_number; i++)
    {
        cout << "Game: " << i + 1 << endl;

        auto start_time = chrono::system_clock::now().time_since_epoch().count();
        auto state = State(mt_for_construct());

        while (!state.isDone())
        {
            state.advance(beamSearch(state, 100, END_TURN));
        }

        auto end_time = chrono::system_clock::now().time_since_epoch().count();
        cout << "Time: " << (end_time - start_time) / 1e9 << " seconds" << endl;
        state.show();

        auto score = state.game_score_;
        if (best_score < score)
        {
            best_score = score;
            best_cards = state.card_;
        }
    }
    cout << "Best Score: " << best_score << endl;
    cout << "Best Cards: ";
    for (auto card : best_cards)
    {
        cout << card << ", ";
    }
    cout << endl;
}

int main()
{
    testAiScore(10);
}
