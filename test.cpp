#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main(){
    vector<int> result = { 38, 20, 46, 19, 25, 48, 33, 34, 21, 17, 43, 12, 40, 0, 15, 16, 0, 4, 5, 42, 2, 30, 11, 14, 31, 45, 23, 22, 1, 47, 9, 8, 35, 37, 24, 41, 29, 3, 28, 32, 6, 13, 10, 7, 18, 27, 26, 44, 39, 36 };
    sort(result.begin(), result.end());
    cout << "Sorted Result: ";
    for(auto card : result){
        cout << card << ", ";
    }
    cout << endl;
    return 0;
}
