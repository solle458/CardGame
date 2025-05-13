#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "CSVHandler.h"

class Com {
   public:
    std::vector<std::vector<int>> cards;

    Com(const int n = 50) {
        CSVFile<int> csv;
        // CSVHandler.h が存在しないため、以下の行は実際には動作しませんが、
        // 元のコードの構造を維持しています。
        csv.csv_read("deck.csv", false, false, ',');
        // ダミーデータを使用する場合の処理
        if (csv.cell.empty() && n > 0) {  // csv_readがダミーで中身がない場合
            cards.assign(n, std::vector<int>(n));
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    cards[i][j] = (i + j) % 10 + 1;  // 何らかのダミーデータ
                }
            }
        } else if (!csv.cell.empty()) {
            cards.resize(std::min(n, (int)csv.cell.size()));
            for (int i = 0; i < cards.size(); ++i) {
                cards[i].resize(std::min(n, (int)csv.cell[i].size()));
                for (int j = 0; j < cards[i].size(); ++j) {
                    cards[i][j] = csv.cell[i][j];
                }
            }
        }
        // 元のコードのロジック
        // cards.resize(n);
        // for(int i = 0; i < n; i++){
        //     cards[i].resize(n);
        //     for(int j = 0; j < n; j++){
        //         if (i < csv.cell.size() && j < csv.cell[i].size()) { //
        //         Bounds check
        //             cards[i][j] = csv.cell[i][j];
        //         } else {
        //             cards[i][j] = 0; // Default value if CSV is smaller
        //         }
        //     }
        // }
    }

    // コピーコンストラクタとコピー代入演算子をdeleteすることも検討可能
    // Com(const Com&) = delete;
    // Com& operator=(const Com&) = delete;
    // ムーブコンストラクタとムーブ代入演算子をdeleteすることも検討可能
    // Com(Com&&) = delete;
    // Com& operator=(Com&&) = delete;

    void show() const {  // constを追加
        for (const auto& row : cards) {
            for (int card_val : row) {
                std::cout << card_val << " ";
            }
            std::cout << std::endl;
        }
    }
};
