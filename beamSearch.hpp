#pragma once

#include <random>
#include <algorithm>
#include <numeric>
#include <queue>

#include "CardGame.hpp"

using namespace std;
using State = CardGame;

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
