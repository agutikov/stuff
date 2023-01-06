#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <set>
#include <optional>
#include <sstream>
#include <deque>
#include <queue>


enum class Suit : uint8_t
{
    SPADES = 0,
    HEARTS = 1,
    DIAMONDS = 2,
    CLUBS = 3
};

enum class Value : uint8_t
{
    Two = 0,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Ace
};

bool operator<(Value lhs, Value rhs)
{
    return int(lhs) < int(rhs);
}

bool operator>(Value lhs, Value rhs)
{
    return int(lhs) > int(rhs);
}

bool operator<=(Value lhs, Value rhs)
{
    return int(lhs) <= int(rhs);
}

bool operator>=(Value lhs, Value rhs)
{
    return int(lhs) >= int(rhs);
}

bool operator==(Value lhs, Value rhs)
{
    return int(lhs) == int(rhs);
}

bool operator==(Value lhs, int rhs)
{
    return int(lhs) == rhs;
}

bool operator!=(Value lhs, Value rhs)
{
    return int(lhs) != int(rhs);
}

int operator+(Value value, int i)
{
    return int(value) + i;
}

struct Card
{
    Value value;
    Suit suit;

    static Card FromIndex(int index)
    {
        return { static_cast<Value>(index % 13), static_cast<Suit>(index / 13) };
    }

    int GetIndex() const
    {
        return int(suit) * 13 + int(value);
    }

    static Card FromString(const std::string& s)
    {
        static const std::map<char, Value> valueMap = {
            { '2', Value::Two },
            { '3', Value::Three },
            { '4', Value::Four },
            { '5', Value::Five },
            { '6', Value::Six },
            { '7', Value::Seven },
            { '8', Value::Eight },
            { '9', Value::Nine },
            { 'T', Value::Ten },
            { 'J', Value::Jack },
            { 'Q', Value::Queen },
            { 'K', Value::King },
            { 'A', Value::Ace }
        };
        static const std::map<char, Suit> suitMap = {
            { 's', Suit::SPADES },
            { 'h', Suit::HEARTS },
            { 'd', Suit::DIAMONDS },
            { 'c', Suit::CLUBS }
        };
        return { valueMap.at(s[0]), suitMap.at(s[1]) };
    }
};

bool operator<(const Card& lhs, const Card& rhs)
{
    if (lhs.value == rhs.value) {
        return lhs.suit < rhs.suit;
    } else {
        return lhs.value < rhs.value;
    }
}

bool operator==(const Card& lhs, const Card& rhs)
{
    return lhs.suit == rhs.suit && lhs.value == rhs.value;
}

bool operator!=(const Card& lhs, const Card& rhs)
{
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, Suit suit)
{
    switch (suit) {
        case Suit::SPADES:
            os << "s";
            break;
        case Suit::HEARTS:
            os << "h";
            break;
        case Suit::DIAMONDS:
            os << "d";
            break;
        case Suit::CLUBS:
            os << "c";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Value value)
{
    switch (value) {
        case Value::Ace:
            os << "A";
            break;
        case Value::Two:
            os << "2";
            break;
        case Value::Three:
            os << "3";
            break;
        case Value::Four:
            os << "4";
            break;
        case Value::Five:
            os << "5";
            break;
        case Value::Six:
            os << "6";
            break;
        case Value::Seven:
            os << "7";
            break;
        case Value::Eight:
            os << "8";
            break;
        case Value::Nine:
            os << "9";
            break;
        case Value::Ten:
            os << "T";
            break;
        case Value::Jack:
            os << "J";
            break;
        case Value::Queen:
            os << "Q";
            break;
        case Value::King:
            os << "K";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Card& card)
{
    os << card.value << card.suit;
    return os;
}

std::random_device g_rd{ "/dev/urandom" };
std::default_random_engine g_rnd{ g_rd() };

struct Deck
{
    std::set<Card> cards;

    Deck()
    {
        for (int i = int(Suit::SPADES); i <= int(Suit::CLUBS); ++i) {
            for (int j = int(Value::Two); j <= int(Value::Ace); ++j) {
                cards.insert({ static_cast<Value>(j), static_cast<Suit>(i) });
            }
        }
    }

    std::vector<Card> shuffle() const
    {
        std::vector<Card> result;
        std::copy(cards.begin(), cards.end(), std::back_inserter(result));
        std::shuffle(result.begin(), result.end(), g_rnd);
        return result;
    }

    std::vector<Card> GetAllCards() const
    {
        std::vector<Card> result;
        std::copy(cards.begin(), cards.end(), std::back_inserter(result));
        return result;
    }

    void RemoveCard(const Card& card)
    {
        cards.erase(card);
    }

    template<typename T>
    void RemoveCards(const T& cards)
    {
        for (const auto& card : cards) {
            RemoveCard(card);
        }
    }
};

struct Hand
{
    std::array<Card, 2> cards;

    static Hand FromString(const std::string& s)
    {
        return { Card::FromString(s.substr(0, 2)), Card::FromString(s.substr(2, 2)) };
    }
};

struct Flop
{
    std::array<Card, 3> cards;

    static Flop FromString(const std::string& s)
    {
        return { Card::FromString(s.substr(0, 2)), Card::FromString(s.substr(2, 2)), Card::FromString(s.substr(4, 2)) };
    }
};

struct Turn
{
    Card card;

    static Turn FromString(const std::string& s)
    {
        return { Card::FromString(s) };
    }
};

struct River
{
    Card card;

    static River FromString(const std::string& s)
    {
        return { Card::FromString(s) };
    }
};

struct Table
{
    Flop flop;
    Turn turn;
    River river;
};

struct Deal
{
    std::vector<Hand> hands;
    Table table;
};


std::ostream& operator<<(std::ostream& os, const Flop& flop)
{
    os << flop.cards[0] << flop.cards[1] << flop.cards[2];
    return os;
}

std::ostream& operator<<(std::ostream& os, const Turn& turn)
{
    os << turn.card;
    return os;
}

std::ostream& operator<<(std::ostream& os, const River& river)
{
    os << river.card;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Hand& hand)
{
    os << hand.cards[0] << hand.cards[1];
    return os;
}

std::ostream& operator<<(std::ostream& os, const Table& table)
{
    os << table.flop << " " << table.turn << " " << table.river;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Deck& deck)
{
    for (const auto& card : deck.cards) {
        os << card << ", ";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Deal& game)
{
    if (game.hands.size() == 2) {
        os << "[" << game.hands[0] << "] " << game.table << " [" << game.hands[1] << "] ";
        return os;
    }

    for (size_t i = 0; i < game.hands.size(); ++i) {
        os << "Hand " << i+1 << ": " << game.hands[i] << "; ";
    }
    os << "Table: " << game.table;
    return os;
}

enum class HandType : uint8_t
{
    HighCard = 0,
    Pair,
    TwoPair,
    ThreeOfAKind,
    Straight,
    Flush,
    FullHouse,
    FourOfAKind,
    StraightFlush,
    RoyalFlush
};

bool operator<(HandType lhs, HandType rhs)
{
    return static_cast<uint8_t>(lhs) < static_cast<uint8_t>(rhs);
}

bool operator>(HandType lhs, HandType rhs)
{
    return static_cast<uint8_t>(lhs) > static_cast<uint8_t>(rhs);
}

struct HandValue
{
    HandType type;
    std::vector<Value> values;
};

std::ostream& operator<<(std::ostream& os, HandType type)
{
    switch (type) {
        case HandType::HighCard:
            os << "h";
            break;
        case HandType::Pair:
            os << "p";
            break;
        case HandType::TwoPair:
            os << "2p";
            break;
        case HandType::ThreeOfAKind:
            os << "3";
            break;
        case HandType::Straight:
            os << "S";
            break;
        case HandType::Flush:
            os << "F";
            break;
        case HandType::FullHouse:
            os << "FH";
            break;
        case HandType::FourOfAKind:
            os << "4";
            break;
        case HandType::StraightFlush:
            os << "SF";
            break;
        case HandType::RoyalFlush:
            os << "RF";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const HandValue& handValue)
{
    os << handValue.type << " ";
    for (const auto& value : handValue.values) {
        os << value;
    }
    return os;
}

HandValue EvaluateCards(const std::vector<Card>& cards)
{
    std::map<Suit, std::set<Value>> suitMap;
    std::map<Value, std::set<Suit>> valueMap;
    std::set<Value> valueSet;

    for (const auto& card : cards) {
        suitMap[card.suit].insert(card.value);
        valueMap[card.value].insert(card.suit);
        valueSet.insert(card.value);
    }

    bool hasFourOfAKind = false;
    Value fourOfAKindValue = Value::Two;
    bool hasThreeOfAKind = false;
    Value threeOfAKindValue = Value::Two;
    std::set<Value> pairValues;
    bool hasFlush = false;
    bool hasStraight = false;
    std::vector<Value> handValues; // resulting values from highest to lowest

    // Count Four of a Kind, Three of a Kind, Pairs, and Kicker
    for (const auto value : valueSet) {
        if (valueMap[value].size() == 4) {
            hasFourOfAKind = true;
            fourOfAKindValue = value;
        } else if (valueMap[value].size() == 3) {
            hasThreeOfAKind = true;
            threeOfAKindValue = value;
        } else if (valueMap[value].size() == 2) {
            pairValues.insert(value);
        }
    }

    // Check for Flush
    for (const auto& suit : suitMap) {
        if (suit.second.size() == 5) {
            hasFlush = true;
            for (const auto& value : suit.second) {
                handValues.push_back(value);
            }
            std::sort(handValues.begin(), handValues.end(), std::greater<Value>());
            break;
        }
    }

    // Check for Straight
    if (valueSet.size() >= 5) {
        std::vector<Value> values(valueSet.begin(), valueSet.end());
        std::sort(values.begin(), values.end(), std::greater<Value>());
        bool hasAce = valueSet.find(Value::Ace) != valueSet.end();
        int numConsecutive = 0;
        for (size_t i = 0; i < values.size(); ++i) {
            if (i == 0 || values[i] == values[i-1] + 1) {
                ++numConsecutive;
            } else {
                numConsecutive = 1;
            }
            if (numConsecutive == 5) {
                hasStraight = true;
                for (size_t j = i; j > i - 5; --j) {
                    handValues.push_back(values[j]);
                }
                break;
            } else if (hasAce && numConsecutive == 4 && values[i] == Value::Five) {
                hasStraight = true;
                handValues = {Value::Five, Value::Four, Value::Three, Value::Two, Value::Ace};
                break;
            }
        }
    }

    // Evaluate Hand
    HandType handType = HandType::HighCard;
    if (hasStraight && hasFlush) {
        if (valueSet.find(Value::Ace) != valueSet.end()) {
            handType = HandType::RoyalFlush;
        } else {
            handType = HandType::StraightFlush;
        }
    } else if (hasFourOfAKind) {
        handType = HandType::FourOfAKind;
        handValues = {fourOfAKindValue, fourOfAKindValue, fourOfAKindValue, fourOfAKindValue};
        valueSet.erase(fourOfAKindValue);
        handValues.push_back(*valueSet.rbegin());
    } else if (hasThreeOfAKind && pairValues.size() >= 1) {
        handType = HandType::FullHouse;
        handValues = {threeOfAKindValue, threeOfAKindValue, threeOfAKindValue};
        handValues.push_back(*pairValues.rbegin());
        handValues.push_back(*pairValues.rbegin());
    } else if (hasFlush) {
        handType = HandType::Flush;
    } else if (hasStraight) {
        handType = HandType::Straight;
    } else if (hasThreeOfAKind) {
        handType = HandType::ThreeOfAKind;
        handValues = {threeOfAKindValue, threeOfAKindValue, threeOfAKindValue};
        valueSet.erase(threeOfAKindValue);
        handValues.push_back(*valueSet.rbegin());
        valueSet.erase(*valueSet.rbegin());
        handValues.push_back(*valueSet.rbegin());
    } else if (pairValues.size() >= 2) {
        handType = HandType::TwoPair;
        handValues = {*pairValues.rbegin(), *pairValues.rbegin()};
        valueSet.erase(*pairValues.rbegin());
        pairValues.erase(*pairValues.rbegin());
        handValues.push_back(*pairValues.rbegin());
        handValues.push_back(*pairValues.rbegin());
        valueSet.erase(*pairValues.rbegin());
        pairValues.erase(*pairValues.rbegin());
        handValues.push_back(*valueSet.rbegin());
    } else if (pairValues.size() == 1) {
        handType = HandType::Pair;
        handValues = {*pairValues.rbegin(), *pairValues.rbegin()};
        valueSet.erase(*pairValues.rbegin());
        pairValues.erase(*pairValues.rbegin());
        handValues.push_back(*valueSet.rbegin());
        valueSet.erase(*valueSet.rbegin());
        handValues.push_back(*valueSet.rbegin());
        valueSet.erase(*valueSet.rbegin());
        handValues.push_back(*valueSet.rbegin());
    } else {
        handType = HandType::HighCard;
        handValues = {*valueSet.rbegin()};
        valueSet.erase(*valueSet.rbegin());
        handValues.push_back(*valueSet.rbegin());
        valueSet.erase(*valueSet.rbegin());
        handValues.push_back(*valueSet.rbegin());
        valueSet.erase(*valueSet.rbegin());
        handValues.push_back(*valueSet.rbegin());
        valueSet.erase(*valueSet.rbegin());
        handValues.push_back(*valueSet.rbegin());
    }

    return {handType, handValues};
}

HandValue EvaluateHand(const Hand& hand, const Table& table)
{
    std::vector<Card> cards;
    cards.insert(cards.end(), hand.cards.begin(), hand.cards.end());
    cards.insert(cards.end(), table.flop.cards.begin(), table.flop.cards.end());
    cards.push_back(table.turn.card);
    cards.push_back(table.river.card);
    return EvaluateCards(cards);
}

int CompareHands(const HandValue& self, const HandValue& opp)
{
    if (self.type < opp.type) {
        return -1;
    } else if (self.type > opp.type) {
        return 1;
    } else {
        for (size_t i = 0; i < self.values.size(); ++i) {
            if (self.values[i] < opp.values[i]) {
                return -1;
            } else if (self.values[i] > opp.values[i]) {
                return 1;
            }
        }
        return 0;
    }
}

std::vector<HandValue> GetHandValues(const Deal& deal)
{
    std::vector<HandValue> handValues;
    for (const auto& hand : deal.hands) {
        handValues.push_back(EvaluateHand(hand, deal.table));
    }
    return handValues;
}

template<typename T>
std::string to_string(const T& t)
{
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

void PrintGame(const Deal& deal)
{
    std::vector<HandValue> handValues = GetHandValues(deal);

    int result = CompareHands(handValues[0], handValues[1]);

    std::string left_state;
    std::string right_state;
    if (result == 1) {
        left_state = "WIN";
        right_state = "LOSE";
    } else if (result == -1) {
        left_state = "LOSE";
        right_state = "WIN";
    } else {
        left_state = "TIE";
        right_state = "TIE";
    }

    auto left_hand = to_string(handValues[0]);
    auto right_hand = to_string(handValues[1]);
    auto deal_str = to_string(deal);

    printf(" %5s %15s %s %-15s %-5s\n", left_state.c_str(), left_hand.c_str(), deal_str.c_str(), right_hand.c_str(), right_state.c_str());
}



std::vector<Hand> GenerateAllHands(const Deck& deck)
{
    std::vector<Card> cards = deck.GetAllCards();
    std::vector<Hand> hands;
    for (int i = 0; i < cards.size(); ++i) {
        for (int j = i + 1; j < cards.size(); ++j) {
            hands.push_back({ cards[i], cards[j] });
        }
    }
    return hands;
}

std::vector<Flop> GenerateAllFlops(const Deck& deck)
{
    std::vector<Card> cards = deck.GetAllCards();
    std::vector<Flop> flops;
    for (int i = 0; i < cards.size(); ++i) {
        for (int j = i + 1; j < cards.size(); ++j) {
            for (int k = j + 1; k < cards.size(); ++k) {
                flops.push_back({ cards[i], cards[j], cards[k] });
            }
        }
    }
    return flops;
}

std::vector<Turn> GenerateAllTurns(const Deck& deck)
{
    std::vector<Card> cards = deck.GetAllCards();
    std::vector<Turn> turns;
    for (int i = 0; i < cards.size(); ++i) {
        turns.push_back({ cards[i] });
    }
    return turns;
}

std::vector<River> GenerateAllRivers(const Deck& deck)
{
    std::vector<Card> cards = deck.GetAllCards();
    std::vector<River> rivers;
    for (int i = 0; i < cards.size(); ++i) {
        rivers.push_back({ cards[i] });
    }
    return rivers;
}


struct Stats
{
    size_t win = 0;
    size_t draw = 0;
    size_t total = 0;

    Stats& operator+=(const Stats& other)
    {
        win += other.win;
        draw += other.draw;
        total += other.total;
        return *this;
    }
};

std::ostream& operator<<(std::ostream& os, const Stats& stats)
{
    os << "Win: " << 100.0 * (double)stats.win / stats.total << "%, "
       << "Draw: " << 100.0 * (double)stats.draw / stats.total << "%, "
       << "Fail: " << 100.0 * (double)(stats.total - stats.win - stats.draw) / stats.total << "%, "
       << "count=" << stats.total;
    return os;
}


bool g_verbose = false;

Stats bruteforceRiver(const Hand& hand, const Flop& flop, const Turn& turn, const River& river)
{
    Deck deck;
    deck.RemoveCards(hand.cards);
    deck.RemoveCards(flop.cards);
    deck.RemoveCard(turn.card);
    deck.RemoveCard(river.card);

    Stats stats;

    auto hands2 = GenerateAllHands(deck);
    for (const auto& hand2 : hands2) {
        ++stats.total;

        Deal deal{ {hand, hand2}, {flop, turn, river} };

        if (g_verbose) {
            PrintGame(deal);
        }

        auto values = GetHandValues(deal);
        int result = CompareHands(values[0], values[1]);

        if (result == 0) {
            ++stats.draw;
        } else if (result == 1) {
            ++stats.win;
        }
    }

    return stats;
}

Stats bruteforceTurn(const Hand& hand, const Flop& flop, const Turn& turn)
{
    Deck deck;
    deck.RemoveCards(hand.cards);
    deck.RemoveCards(flop.cards);
    deck.RemoveCard(turn.card);

    Stats stats;

    auto rivers = GenerateAllRivers(deck);
    for (const auto& river : rivers) {
        stats += bruteforceRiver(hand, flop, turn, river);
    }

    return stats;
}

Stats bruteforceFlop(const Hand& hand, const Flop& flop)
{
    Deck deck;
    deck.RemoveCards(hand.cards);
    deck.RemoveCards(flop.cards);

    Stats stats;

    auto turns = GenerateAllTurns(deck);
    for (const auto& turn : turns) {
        stats += bruteforceTurn(hand, flop, turn);
    }

    return stats;
}


Stats bruteforceHand(const Hand& hand)
{
    Deck deck;
    deck.RemoveCards(hand.cards);

    Stats stats;

    auto flops = GenerateAllFlops(deck);
    for (const auto& flop : flops) {
        stats += bruteforceFlop(hand, flop);
    }

    return stats;
}




struct OptDeal
{
    Hand hand;
    Flop flop;
    std::optional<Turn> turn;
    std::optional<River> river;

    static OptDeal FromString(const std::string& str)
    {
        // split string by spaces
        std::vector<std::string> parts;
        std::string part;
        std::istringstream iss(str);
        while (iss >> part) {
            parts.push_back(part);
        }

        if (parts.size() < 2 || parts.size() > 4) {
            throw std::runtime_error("Invalid deal string");
        }

        OptDeal deal;
        deal.hand = Hand::FromString(parts[0]);
        deal.flop = Flop::FromString(parts[1]);
        if (parts.size() >= 3) {
            deal.turn = Turn::FromString(parts[2]);
        }
        if (parts.size() == 4) {
            deal.river = River::FromString(parts[3]);
        }

        return deal;
    }

    Stats Evaluate() const
    {
        if (river) {
            return bruteforceRiver(hand, flop, *turn, *river);
        } else if (turn) {
            return bruteforceTurn(hand, flop, *turn);
        } else {
            return bruteforceFlop(hand, flop);
        }
    }
};

std::ostream& operator << (std::ostream& os, const OptDeal& deal)
{
    os << deal.hand << " " << deal.flop;
    if (deal.turn) {
        os << " " << *deal.turn;
    }
    if (deal.river) {
        os << " " << *deal.river;
    }
    return os;
}


void calc_deal(const std::string& line)
{
    OptDeal deal = OptDeal::FromString(line);

    Stats stats = deal.Evaluate();

    std::cout << deal << " : " << stats << std::endl;
}


void calc_loop()
{
    std::string line;
    while (std::getline(std::cin, line)) {
        calc_deal(line);
    }
}


std::vector<Card> ParseCards(const std::string& str)
{
    std::vector<Card> cards;
    for (int i = 0; i < str.size(); i += 2) {
        cards.push_back(Card::FromString(str.substr(i, 2)));
    }
    return cards;
}


int nCk(int n, int k)
{
    if (k > n) return 0;
    if (k * 2 > n) k = n - k;
    if (k == 0) return 1;

    int nCk = n;
    for (int i = 2; i <= k; i++)
    {
        nCk *= (n - i + 1);
        nCk /= i;
    }

    return nCk;
}

struct CombinationGenerator
{
    std::vector<int> state;
    int n, k;
    size_t combination_counter;

    CombinationGenerator(int n, int k) :
        n(n), k(k)
    {
        combination_counter = nCk(n, k);
        state.resize(k);
        for (int i = 0; i < k; ++i) {
            state[i] = i;
        }
        if (k != 0) state[k - 1]--;
    }

    std::vector<int> next()
    {
        if (k == 0 || combination_counter-- == 0)
        {
            return {};
        }

        for (int i = k - 1; i >= 0; i--)
        {
            state[i]++;

            // If "overflow", move to the element before.
            if (state[i] > n - k + i) continue;

            // Reset the next elements.
            for (int j = i + 1; j < k; j++) state[j] = state[j - 1] + 1;
            break;
        }

        return state;
    }
};

void test_gen()
{
    CombinationGenerator gen(5, 3);

    while (true) {
        auto cards = gen.next();
        if (cards.empty()) {
            break;
        }

        for (auto c : cards) {
            std::cout << c << " ";
        }
        std::cout << std::endl;
    }
}

struct CardsGenerator : public CombinationGenerator
{
    CardsGenerator(int k) :
        CombinationGenerator(52, k)
    {
    }

    std::vector<Card> next()
    {
        auto cards = CombinationGenerator::next();
        std::vector<Card> result;
        for (auto c : cards) {
            result.push_back(Card::FromIndex(c));
        }
        return result;
    }
};



void BruteForceLoop()
{
    CardsGenerator gen(7);

    while (true) {
        auto cards = gen.next();
        if (cards.empty()) {
            break;
        }

        auto hand_value = EvaluateCards(cards);
        for (int i = 0; i < 7; ++i) {
            std::cout << cards[i];
        }
        std::cout << " : " << hand_value << std::endl;
    }
}

OptDeal RandomDeal(bool turn = true, bool river = true)
{
    auto deck = Deck{}.shuffle();

    OptDeal deal{
        .hand{deck[0], deck[1]},
        .flop{deck[2], deck[3], deck[4]}
    };
    if (turn) {
        deal.turn = Turn{deck[5]};
        if (river) {
            deal.river = River{deck[6]};
        }
    }

    return deal;
}

void RandomDealEvalLoop()
{
    while (true) {
        auto deal = RandomDeal(true, false);
        auto stats = deal.Evaluate();
        std::cout << deal << " : " << stats << std::endl;
    }
}


int main(int argc, const char* argv[])
{
    g_verbose = false;

    std::string line;
    for (int i = 1; i < argc; ++i) {
        line += std::string(argv[i]) + " ";
    }
    if (!line.empty()) {
        calc_deal(line);
    } else {
        //BruteForceLoop();
        RandomDealEvalLoop();
    }


    return 0;
}
