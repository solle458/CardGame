#pragma once

#include <string>
#include <sstream>
#include <random>
#include <vector>

#include "Com.hpp"

constexpr long long INF = 1e18;
constexpr const int NumberOfCards = 50;
constexpr const int NumberOfMatches = 50;
constexpr const int END_TURN = 50;

class CardGame {
    private:
        int turn_ = 0;

    public:
        int game_score_ = 0;
        int first_action_ = -1;
        int now_cards_idx_ = 0;
        double evaluate_score_ = 0;
        Com com_;
        std::vector<int> card_;
        std::vector<int> now_cards_;
        CardGame() {}
        CardGame(const int seed){
            card_.resize(NumberOfCards);
            now_cards_.resize(NumberOfCards);
            std::iota(card_.begin(), card_.end(), 1);
            std::shuffle(card_.begin(), card_.end(), std::mt19937(seed));
            std::iota(now_cards_.begin(), now_cards_.end(), 0);
            std::shuffle(now_cards_.begin(), now_cards_.end(), std::mt19937(seed));
        }

        bool isDone() const {
            return this->turn_ == END_TURN;
        }

        int evaluate() {
            double score = 0;
            for(auto match : com_.cards){
                for(int i = 0; i < match.size(); i++){
                    if(match[i] < card_[i]){
                        score++;
                    }else if (match[i] == card_[i]){
                        score += 0.5;
                    }
                }
            }
            return this->evaluate_score_ = score;
        }

        void advance(const int action) {
            swap(card_[this->now_cards_[now_cards_idx_]], card_[action]);
            this->game_score_ = evaluate();
            this->turn_++;
            this->now_cards_idx_++;
        }

        std::vector<int> legalAction(const int now) {
            std::vector<int> action;
            for(int i = 0; i < card_.size(); i++){
                if(i == now) continue;
                action.push_back(i);
            }
            return action;
        }

        void show() {
            std::cout << "Cards: ";
            for(int i = 0; i < card_.size(); i++){
                std::cout << card_[i] << ", ";
            }
            std::cout << std::endl << "Score: " << game_score_ << std::endl;
        }
};

