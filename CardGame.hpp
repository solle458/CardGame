#pragma once

#include <algorithm>  // std::shuffle のために追加
#include <iostream>   // show() で使用
#include <memory>     // std::shared_ptr のために追加
#include <numeric>    // std::iota のために追加
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Com.hpp"

constexpr long long INF = 1e18;  // INF は現状使われていないようですが残します
constexpr const int NumberOfCards = 50;
constexpr const int NumberOfMatches =
    50;  // NumberOfMatches も現状 evaluate() 内で直接使われていません
constexpr const int END_TURN = 50;

class CardGame {
   private:
    int turn_ = 0;

   public:
    int game_score_ = 0;
    int first_action_ = -1;
    int now_cards_idx_ = 0;
    double evaluate_score_ = 0;
    // Com com_; // 元のメンバー、削除
    std::vector<int> card_;
    std::vector<int> now_cards_;
    std::shared_ptr<const Com> com_ptr_;  // Comオブジェクトへのshared_ptr

    // CardGame() {} //
    // デフォルトコンストラクタを削除、または適切にshared_ptrを初期化

    CardGame(const int seed, std::shared_ptr<const Com> com_data)
        : com_ptr_(com_data) {
        if (!com_ptr_) {
            // com_ptr_ が null の場合の処理 (エラーハンドリングなど)
            // ここでは例外を投げるか、デフォルトのComオブジェクトを生成するなどの対応が考えられます。
            // 例: throw std::runtime_error("Com data is null");
            // このサンプルでは、簡略化のためそのまま進めますが、実際のプロダクトでは注意が必要です。
        }
        card_.resize(NumberOfCards);
        now_cards_.resize(
            NumberOfCards);  // now_cards_ の内容は0から始まるインデックス
        std::iota(card_.begin(), card_.end(), 1);  // 1から始まるカード
        std::shuffle(card_.begin(), card_.end(), std::mt19937(seed));
        std::iota(now_cards_.begin(), now_cards_.end(),
                  0);  // 0から始まるインデックス
        std::shuffle(now_cards_.begin(), now_cards_.end(), std::mt19937(seed));
    }

    // コピーコンストラクタ (デフォルトで問題ないが、明示的に書くことも可能)
    CardGame(const CardGame& other) = default;
    // {
    //     turn_ = other.turn_;
    //     com_ptr_ = other.com_ptr_; // shared_ptrのコピー
    //     (参照カウントのインクリメント) game_score_ = other.game_score_;
    //     first_action_ = other.first_action_;
    //     now_cards_idx_ = other.now_cards_idx_;
    //     evaluate_score_ = other.evaluate_score_;
    //     card_ = other.card_;
    //     now_cards_ = other.now_cards_;
    // }

    // ムーブコンストラクタ (デフォルトで問題ないが、明示的に書くことも可能)
    CardGame(CardGame&& other) noexcept = default;

    // コピー代入演算子 (デフォルトで問題ないが、明示的に書くことも可能)
    CardGame& operator=(const CardGame& other) = default;

    // ムーブ代入演算子 (デフォルトで問題ないが、明示的に書くことも可能)
    CardGame& operator=(CardGame&& other) noexcept = default;

    bool isDone() const { return this->turn_ == END_TURN; }

    double
    evaluate() {  // 戻り値をdoubleに変更 (evaluate_score_ の型に合わせて)
        double score = 0;
        if (!com_ptr_) {  // 安全のためnullチェック
            // com_ptr_ が null の場合のスコア (例: 0点)
            return this->evaluate_score_ = 0.0;
        }
        // NumberOfMatches が com_ptr_->cards.size() と等しいという前提
        for (const auto& match :
             com_ptr_->cards) {  // com_ptr_ を介してアクセス
            // match.size() が NumberOfCards と等しいという前提
            for (int i = 0; i < std::min((int)match.size(), (int)card_.size());
                 i++) {  // サイズチェックを追加
                if (match[i] < card_[i]) {
                    score++;
                } else if (match[i] == card_[i]) {
                    score += 0.5;
                }
            }
        }
        return this->evaluate_score_ = score;
    }

    void advance(const int action) {
        if (this->now_cards_[now_cards_idx_] < card_.size() &&
            action < card_.size()) {  // Bounds check
            std::swap(card_[this->now_cards_[now_cards_idx_]], card_[action]);
        }
        this->game_score_ = static_cast<int>(
            evaluate());  // evaluate()の戻り値に合わせてキャスト
        this->turn_++;
        this->now_cards_idx_++;
        if (now_cards_idx_ >=
            NumberOfCards) {  // now_cards_idx_ が範囲外にならないように
            now_cards_idx_ = NumberOfCards - 1;
        }
    }

    std::vector<int> legalAction(const int current_card_original_pos_idx)
        const {  // current_card_original_pos_idxが示すものに合わせて引数名変更
        std::vector<int> actions;
        for (int i = 0; i < NumberOfCards; ++i) {
            // current_card_original_pos_idx は card_
            // 配列上の「現在のカードがおかれている場所」のインデックス action
            // は card_ 配列上の「スワップ対象の場所」のインデックス
            if (i == this->now_cards_[now_cards_idx_])
                continue;  // 現在のカードの位置とはスワップしない
            actions.push_back(i);
        }
        return actions;
    }

    void show() const {  // constを追加
        std::cout << "Cards: ";
        for (int i = 0; i < card_.size(); ++i) {
            std::cout << card_[i] << (i == card_.size() - 1 ? "" : ", ");
        }
        std::cout << std::endl << "Score: " << game_score_ << std::endl;
        std::cout << "Turn: " << turn_
                  << ", Next card to place from original position: "
                  << (now_cards_idx_ < now_cards_.size()
                          ? now_cards_[now_cards_idx_]
                          : -1)
                  << std::endl;
    }
};

// 比較演算子は evaluate_score_ を使うので変更なし
bool operator<(const CardGame& lhs, const CardGame& rhs) {
    return lhs.evaluate_score_ < rhs.evaluate_score_;
}
