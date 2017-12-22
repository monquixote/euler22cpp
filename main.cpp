#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <numeric>

using namespace std;

struct Card {
    const char suit, val;

    Card(const char val, const char suit) :  val(val), suit(suit) {}
};

enum hand_precidence {high, pair, two_pair, three, straight, flush, full, four, straight_flush};

using Hand = vector<Card>;
using HandValue = pair<hand_precidence, vector<short>>;

Hand get_hand(stringstream &ss){
    string s;
    Hand hand;

    for(int i=0; i<5; i++){
        getline(ss, s, ' ');
        hand.emplace_back(s[0],s[1]);
    }
    return hand;
}

bool is_flush(const Hand &h){
    char first = h.front().suit;
    return  all_of(h.begin()+1, h.end(), [first](Card c){return c.suit==first;});
}

bool is_straight(const vector<short> &counts){
    int count = 0;
    for(auto e: counts){
        switch(e) {
            case 0:
                if (count > 0) {
                    return false;
                }
                break;
            case 1:
                ++count;
                if(count == 5){
                    return true;
                }
                break;
            default:
                return false;
        }
    }
    return false;
}

short card_val2ordinal(const char c){
    const string cards = "23456789TJQKA";
    return (short) cards.find(c,0);
}

string hand_precidence2string(const hand_precidence &h){
    const vector<string> precidence_strings{"High", "Pair", "Two Pairs", "Three of a kind", "Straight", "Flush", "Full house", "Four of a kind", "Straight flush"};
    return precidence_strings[h];
}

vector<short> count_vals(const Hand &h){
    vector<short> counts(13, 0);
    return accumulate(begin(h),end(h),counts,[](vector<short> totals,Card c){ totals[card_val2ordinal(c.val)]++; return totals;});
}

hand_precidence get_dupes(const vector<short> &counts){
    auto max = *max_element(begin(counts),end(counts));

    switch(max) {
        case 1:
            return high;
        case 2:
            return (count(counts.begin(), counts.end(), 2) == 2) ? two_pair : hand_precidence::pair;
        case 3:
            return (find(counts.begin(), counts.end(), 2) != counts.end()) ? full : three;
        case 4:
            return four;
        default:
            throw invalid_argument("No hand found");
    }
}

hand_precidence flush_or_straight(vector<short> &counts, const Hand &h){
    bool flush_hand = is_flush(h);
    bool straight_hand = is_straight(counts);
    if (straight_hand && flush_hand) {
        return straight_flush;
    }
    if (flush_hand) {
        return hand_precidence::flush;
    }
    if (straight_hand) {
        return straight;
    }
    return high;
}

HandValue evaluate_hand(const Hand &h){
    auto counts = count_vals(h);
    auto prec = get_dupes(counts);

    return HandValue{(prec == high) ? flush_or_straight(counts,h) : prec, counts};
}


string handvalue2string(const HandValue &h){
    return hand_precidence2string(h.first);
}

function<short()> make_gen(const vector<short> &counts){
    auto max = *max_element(begin(counts),end(counts));
    return [search = move(max), i = counts.size(), &counts]() mutable -> short  {
        while(search>0){
            while(i>0){
                --i;
                if(counts[i] == search){
                    return i;
                }
            }
            i = counts.size();
            --search;
        }
        return -1;
    };
}

bool compare_counts(const vector<short> &left, const vector<short> &right){
    auto lfunc = make_gen(left);
    auto rfunc = make_gen(right);
    int l,r;
    while(l != -1 && r != -1){
        l = lfunc();
        r = rfunc();
        //cout << ordinal2card_val(l) << " " << ordinal2card_val(r) << endl;
        if(l > r){
            return true;
        }
        if(r > l){
            return false;
        }
}
    return false;
}

bool left_wins(const HandValue &left, const HandValue &right){
    if(left.first != right.first){
        return left.first > right.first;
    }
    return compare_counts(left.second, right.second);
}

bool compare_hands(const Hand &left, const Hand &right){
    auto left_score = evaluate_hand(left);
    auto right_score = evaluate_hand(right);
    //cout << handvalue2string(left_score) << " vs " << handvalue2string(right_score) << endl;

    return left_wins(left_score, right_score);
}

bool process_row(const string &row){
    //cout << row << endl;
    stringstream ss(row);
    auto left = get_hand(ss);
    auto right = get_hand(ss);
    return compare_hands(left, right);
}

int main() {
    clock_t c;
    c = clock();
    ifstream t("p054_poker.txt");
    string s;
    int i = 0;
    while (getline(t,s)) {
        if(process_row(s)){
            i++;
        }
        //cout << (process_row(s) ? "Left" : "Right" ) << " is the winner" << endl;
    }
    cout << "Left won " << i << endl;
    c = clock() - c;
    printf ("It took me %d clicks (%f seconds).\n",c,((float)c)/CLOCKS_PER_SEC);
    return 0;
}