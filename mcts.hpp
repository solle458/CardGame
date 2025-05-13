#pragma once

#include <algorithm>
#include <random>
// #include <format> // C++20以降。古いコンパイラではエラーになる可能性あり。iostreamで代替も可。
#include <iostream> // std::format の代替として
#include <vector>
#include <memory>   // std::unique_ptr, std::shared_ptr
#include <cmath>    // std::sqrt, std::log
#include <limits>   // std::numeric_limits

#include "CardGame.hpp" // CardGame.hpp は shared_ptr<Com> を使うように変更済みと仮定
#include "Timer.hpp"    // Timer.hpp の内容は不明だがそのまま

using State = CardGame; // CardGame.hpp で State = CardGame としているなら不要

class MCTSNode {
public:
    State state_; // CardGameオブジェクト。com_ptr_ を持つ
    int visits_;
    double value_;
    int action_taken_; // このノードに至るために取られたアクション
    MCTSNode* parent_;
    std::vector<std::unique_ptr<MCTSNode>> children_;
    std::vector<int> untried_actions_;
    std::mt19937 rng_;

    // MCTSNode のコンストラクタは State を値で受け取る。
    // この State は既に com_ptr_ を持っている前提。
    MCTSNode(State initial_state, MCTSNode* parent = nullptr, int action_taken = -1,
             unsigned int seed = std::random_device{}())
        : state_(initial_state), // Stateのコピーコンストラクタが呼ばれ、shared_ptrもコピーされる
          visits_(0),
          value_(0.0),
          action_taken_(action_taken),
          parent_(parent),
          rng_(seed) {
        if (!state_.isDone()) {
            // state_.now_cards_ と state_.now_cards_idx_ が public であるか、
            // legalAction に必要な情報を取得する getter が State にある前提。
            // CardGame.hpp の実装では public メンバーなので、このままアクセス可能。
            untried_actions_ = state_.legalAction(state_.now_cards_[state_.now_cards_idx_]);
            std::shuffle(untried_actions_.begin(), untried_actions_.end(), rng_);
        }
    }

    bool hasUntriedActions() const {
        return !untried_actions_.empty();
    }

    bool isFullyExpanded() const {
        return untried_actions_.empty();
    }

    bool isTerminal() const {
        return state_.isDone();
    }

    double getUCTValue(double exploration_weight) const {
        if (visits_ == 0) {
            return std::numeric_limits<double>::max();
        }
        if (!parent_ || parent_->visits_ == 0) { // 親がいない(ルート)か、親の訪問がない場合は特別扱い
            return value_ / visits_ + exploration_weight; // 単純な評価値 + 定数ボーナス
        }
        
        double exploitation = value_ / visits_;
        double exploration = exploration_weight * std::sqrt(std::log(static_cast<double>(parent_->visits_)) / visits_);
        
        // 元の progressive_bias は削除または調整。ここではシンプルに UCB1 に近い形に。
        // double win_rate = value_ / visits_; 
        // double progressive_bias = 0.1 * win_rate;
        // return exploitation + exploration + progressive_bias;
        return exploitation + exploration;
    }

    int selectRandomUntriedAction() {
        if (untried_actions_.empty()) return -1; // アクションがない
        std::uniform_int_distribution<size_t> dist(0, untried_actions_.size() - 1);
        size_t idx = dist(rng_);
        int action = untried_actions_[idx];
        untried_actions_.erase(untried_actions_.begin() + idx);
        return action;
    }

    MCTSNode* selectBestChild(double exploration_weight) {
        MCTSNode* best_child = nullptr;
        double best_value = -std::numeric_limits<double>::max();
        for (auto& child_ptr : children_) {
            double uct_value = child_ptr->getUCTValue(exploration_weight);
            if (uct_value > best_value) {
                best_value = uct_value;
                best_child = child_ptr.get();
            }
        }
        return best_child;
    }
    
    MCTSNode* selectBestChildForPlay() const { // const を追加
        MCTSNode* best_child = nullptr;
        int most_visits = -1;
        for (const auto& child_ptr : children_) { // const auto&
            if (child_ptr->visits_ > most_visits) {
                most_visits = child_ptr->visits_;
                best_child = child_ptr.get();
            }
        }
        return best_child;
    }

    MCTSNode* addChild(int action) {
        State next_state = state_; // Stateのコピー (shared_ptrもコピー)
        next_state.advance(action);
        unsigned int child_seed = rng_(); // 新しいシードを生成
        // std::make_unique を使う
        children_.push_back(std::make_unique<MCTSNode>(next_state, this, action, child_seed));
        return children_.back().get();
    }

    void update(double result) {
        visits_++;
        value_ += result;
    }
};


int monteCarloTreeSearch(const State& initial_state_const, // この initial_state は com_ptr を持つ
                         int time_limit_ms = 1000,
                         int max_simulations = 1000000, // 元のコードでは MCTS 呼び出し時に 10^7 だった
                         double exploration_weight = 1.414) {
    
    // ルートノードを初期化。initial_state_const が持つ com_ptr がコピーされる。
    MCTSNode root(initial_state_const); 
    Timer timer(time_limit_ms); // Timer は long long を取るのでキャストが必要かも
    int simulation_count = 0;
    std::mt19937 simulation_rng(std::random_device{}());

    while (!timer.isTimeUp() && simulation_count < max_simulations) {
        MCTSNode* node = &root;

        // 1. Selection
        while (!node->isTerminal() && node->isFullyExpanded()) {
            node = node->selectBestChild(exploration_weight);
            if (!node) break; // selectBestChild が nullptr を返した場合 (ありえないはずだが安全のため)
        }
        if (!node) continue; // ループを抜ける

        // 2. Expansion
        if (!node->isTerminal() && node->hasUntriedActions()) {
            int action = node->selectRandomUntriedAction();
            if (action != -1) { // 有効なアクションの場合のみ
                 node = node->addChild(action);
            } else if (node->isTerminal()) {
                 // 既に終端ノードだった場合は何もしない (ありえないはずだが安全のため)
            }
        }
         if (!node) continue; // addChild が失敗した場合 (ありえないが)

        // 3. Simulation
        State simulation_state = node->state_; // Stateのコピー (shared_ptrもコピー)
        while (!simulation_state.isDone()) {
            // simulation_state.now_cards_ と simulation_state.now_cards_idx_ が public であるか、
            // legalAction に必要な情報を取得する getter が State にある前提。
            auto legal_actions = simulation_state.legalAction(
                simulation_state.now_cards_[simulation_state.now_cards_idx_]);
            
            if (legal_actions.empty()) break;
            
            std::uniform_int_distribution<size_t> dist(0, legal_actions.size() - 1);
            int random_action = legal_actions[dist(simulation_rng)];
            simulation_state.advance(random_action);
        }

        // 4. Backpropagation
        // simulation_state.evaluate_score_ が public であるか、
        // スコアを取得する getter が State にある前提。
        // CardGame.hpp の実装では public メンバー。
        double result = simulation_state.evaluate_score_;
        MCTSNode* backprop_node = node; // nodeを直接変更しないように
        while (backprop_node != nullptr) {
            backprop_node->update(result);
            backprop_node = backprop_node->parent_;
        }
        simulation_count++;
    }

    MCTSNode* best_child = root.selectBestChildForPlay();
    
    // デバッグ情報 (std::format の代わりに iostream を使う)
    // std::cerr << "Simulations: " << simulation_count << ", Time used: " << timer.elapsedMs() << " ms\n";
    // if (best_child) {
    //     std::cerr << "Best action: " << best_child->action_taken_ 
    //               << " (visits: " << best_child->visits_ 
    //               << ", value: " << (best_child->visits_ > 0 ? best_child->value_ / best_child->visits_ : 0.0) 
    //               << ")\n";
    // }

    if (best_child) {
        return best_child->action_taken_;
    } else {
        // 最良の子が見つからない場合 (例: ルートが既に終端、または未試行手しかない)
        if (!root.untried_actions_.empty()) { // 未試行手があればそれを返す
             std::shuffle(root.untried_actions_.begin(), root.untried_actions_.end(), simulation_rng); // ランダムに選ぶ
            return root.untried_actions_[0];
        } else if (!root.children_.empty()) { // 子はいるが selectBestChildForPlay が null を返した場合 (通常ない)
             return root.children_[0]->action_taken_; // とりあえず最初の子のアクション
        }
        // それでも手がなければ、合法手の最初を返す (main.cpp のフォールバックに近い)
        auto fallback_actions = initial_state_const.legalAction(initial_state_const.now_cards_[initial_state_const.now_cards_idx_]);
        return fallback_actions.empty() ? 0 : fallback_actions[0]; // 0は適当なデフォルト値
    }
}
