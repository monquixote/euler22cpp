#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <numeric>
#include <map>

using namespace std;

struct Card {
    const char suit, val;

    Card(const char val, const char suit) :  val(val), suit(suit) {}
};

enum hand_precidence {high, pair, two_pair, three, straight, flush, full, four, straight_flush};
const vector<string> precidence_strings{"High", "Pair", "Two Pairs", "Three of a kind", "Straight", "Flush", "Full house", "Four of a kind", "Straight flush"};

using Hand = vector<Card>;
using HandValue = tuple<hand_precidence, int, int>;

Hand get_hand(const string &row){
    stringstream ss(row);
    string s;
    Hand hand;

    while (getline(ss, s, ' ')) hand.emplace_back(s[0],s[1]);
    return move(hand);
}

bool is_flush(const Hand &h){
    char first = h.front().suit;
    return  all_of(h.begin()+1, h.end(), [first](Card c){return c.suit==first;});
}

bool is_straight(const vector<short> &v){
    short first = v.front();
    return all_of(v.begin(), v.end(), [first](short s) mutable {return s == first++;});
}

const string cards = "23456789TJQKA";

short card_val2ordinal(const char c){
    return (short) cards.find(c,0);
}

string ordinal2card_val(const short s){
    return string(1, cards[s]);
}

string hand_precidence2string(const hand_precidence h){
    return precidence_strings[h];
}

vector<short> count_vals(const vector<short> &vals){
    vector<short> counts(static_cast<unsigned long>(*max_element(begin(vals), end(vals)) + 1), 0);
    return accumulate(begin(vals),end(vals),counts,[](vector<short> totals,short c){ totals[c]++;return totals; });
}

map<int,vector<int>> get_runs(const vector<short> &vals){
    auto count = count_vals(vals);
    map<int,vector<int>> runs{};
    for (int i = 0;i<count.size();++i) {
       if(count[i]!=0){
           runs.try_emplace(count[i], vector<int>());
           runs[count[i]].push_back(i);
       }
    }
    return runs;
}

vector<short> sort_vals(const Hand &h){
    vector<short> vals;
    transform(h.begin(),h.end(),back_inserter(vals),[](Card c){return card_val2ordinal(c.val);});
    sort(vals.begin(),vals.end());
    return vals;
}

HandValue evaluate_hand(const Hand &h){
    bool flush_hand = is_flush(h);
    auto sorted_vals = sort_vals(h);
    bool straight_hand = is_straight(sorted_vals);
    if(straight_hand && flush_hand) {
        return HandValue{straight_flush,sorted_vals.back(),-1};
    }

    auto runs = get_runs(sorted_vals);
    if(runs.count(4)){
        return HandValue{four,runs[4].back(),runs[1].back()};
    }
    if(runs.count(3) && runs.count(2)){
        return HandValue{full, runs[3].back(), runs[2].back()};
    }
    if(flush_hand){
        return HandValue(hand_precidence::flush, sorted_vals.back(), -1);
    }
    if(straight_hand){
        return HandValue(straight, sorted_vals.back(), -1);
    }
    if(runs.count(3)){
        return HandValue{three, runs[3].back(), runs[1].back()};
    }
    if(runs.count(2) && runs[2].size() == 2){
        return HandValue{two_pair, runs[2].back(), runs[2].front()};
    }
    if(runs.count(2)){
        return HandValue{hand_precidence::pair, runs[2].back(), runs[2].back()};
    }
    return HandValue{high,runs[1].back(),-1};
}

string handvalue2string(HandValue h){
    auto [hand, major, minor] = h;
    return hand_precidence2string(hand) + " " + ordinal2card_val(major) + (minor == -1 ? "" : " " + ordinal2card_val(minor));
}

bool left_wins(const HandValue &left, const HandValue &right){
    auto [lhand, lmajor, lminor] = left;
    auto [rhand, rmajor, rminor] = right;
    if(lhand != rhand){
        return lhand > rhand;
    }
    if (lmajor != rmajor){
        return lmajor > rmajor;
    }
    return lminor > rminor;
}

bool compare_hands(const Hand &left, const Hand &right){
    auto left_score = evaluate_hand(left);
    auto right_score = evaluate_hand(right);
    cout << handvalue2string(left_score) << " vs " << handvalue2string(right_score) << endl;
    return left_wins(left_score, right_score);
}

bool process_row(const string &row){
    cout << row << endl;
    auto left = get_hand(row.substr(0,row.length()/2));
    auto right = get_hand(row.substr(row.length()/2+1,row.length()-1));
    return compare_hands(left, right);
}

int main() {
    ifstream t("p054_poker.txt");
    string s;
    while (getline(t,s))
        cout << (process_row(s) ? "Left" : "Right" ) << " is the winner" << endl;
    return 0;
}