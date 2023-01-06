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

Deal GetDeal(Deck& deck, size_t numPlayers)
{
    std::vector<Card> cards;
    std::copy(deck.cards.begin(), deck.cards.end(), std::back_inserter(cards));

    Deal deal;
    deal.hands.resize(numPlayers);
    for (auto& hand : deal.hands) {
        hand.cards[0] = cards.back();
        cards.pop_back();
        hand.cards[1] = cards.back();
        cards.pop_back();
    }
    deal.table.flop.cards[0] = cards.back();
    cards.pop_back();
    deal.table.flop.cards[1] = cards.back();
    cards.pop_back();
    deal.table.flop.cards[2] = cards.back();
    cards.pop_back();
    deal.table.turn.card = cards.back();
    cards.pop_back();
    deal.table.river.card = cards.back();
    cards.pop_back();

    return deal;
}

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
            os << "High";
            break;
        case HandType::Pair:
            os << "Pair";
            break;
        case HandType::TwoPair:
            os << "2Pair";
            break;
        case HandType::ThreeOfAKind:
            os << "Three";
            break;
        case HandType::Straight:
            os << "Straight";
            break;
        case HandType::Flush:
            os << "Flush";
            break;
        case HandType::FullHouse:
            os << "FullHouse";
            break;
        case HandType::FourOfAKind:
            os << "Four";
            break;
        case HandType::StraightFlush:
            os << "StraightFlush";
            break;
        case HandType::RoyalFlush:
            os << "RoyalFlush";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const HandValue& handValue)
{
    os << handValue.type << " ";
    for (const auto& value : handValue.values) {
        os << value << " ";
    }
    return os;
}

HandValue EvaluateCards(const std::array<Card, 7>& cards)
{
    std::map<Suit, std::set<Value>> suitMap;
    std::map<Value, std::set<Suit>> valueMap;
    std::set<Value> valueSet;
    std::vector<Value> values; // sorted

    for (const auto& card : cards) {
        suitMap[card.suit].insert(card.value);
        valueMap[card.value].insert(card.suit);
        valueSet.insert(card.value);
    }
    for (const auto& value : valueSet) {
        values.push_back(value);
    }
    //std::sort(values.begin(), values.end());

    bool hasFourOfAKind = false;
    Value fourOfAKindValue = Value::Two;
    bool hasThreeOfAKind = false;
    Value threeOfAKindValue = Value::Two;
    std::vector<Value> pairValues;
    bool hasFlush = false;
    bool hasStraight = false;
    Value olderValue = Value::Two;
    Value kicker = Value::Two;

    // Count Four of a Kind, Three of a Kind, Pairs, and Kicker
    for (const auto value : values) {
        if (valueMap[value].size() == 4) {
            hasFourOfAKind = true;
            fourOfAKindValue = value;
        } else if (valueMap[value].size() == 3) {
            hasThreeOfAKind = true;
            threeOfAKindValue = value;
        } else if (valueMap[value].size() == 2) {
            pairValues.push_back(value);
        } else if (valueMap[value].size() == 1) {
            kicker = value;
        }
    }
    std::sort(pairValues.begin(), pairValues.end(), std::less<Value>());

    // Check for Flush
    for (const auto& suit : suitMap) {
        if (suit.second.size() == 5) {
            hasFlush = true;
            olderValue = *suit.second.rbegin();
            break;
        }
    }

    // Check for Straight
    int numConsecutive = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i == 0 || values[i] == values[i-1] + 1) {
            ++numConsecutive;
        } else {
            numConsecutive = 1;
        }
        if (numConsecutive == 5) {
            hasStraight = true;
            olderValue = values[i];
            break;
        } else if (numConsecutive == 4 && values[i] == Value::Ace) {
            hasStraight = true;
            olderValue = Value::Five;
            break;
        }
    }

    // Evaluate Hand
    if (hasStraight && hasFlush) {
        if (olderValue == Value::Ace) {
            return {HandType::RoyalFlush, {Value::Ace}};
        } else {
            return {HandType::StraightFlush, {olderValue}};
        }
    } else if (hasFourOfAKind) {
        return {HandType::FourOfAKind, {fourOfAKindValue, kicker}};
    } else if (hasThreeOfAKind && pairValues.size() >= 1) {
        return {HandType::FullHouse, {threeOfAKindValue, pairValues[0]}};
    } else if (hasFlush) {
        return {HandType::Flush, {olderValue}};
    } else if (hasStraight) {
        return {HandType::Straight, {olderValue}};
    } else if (hasThreeOfAKind) {
        return {HandType::ThreeOfAKind, {threeOfAKindValue, kicker}};
    } else if (pairValues.size() >= 2) {
        return {HandType::TwoPair, {pairValues[0], pairValues[1], kicker}};
    } else if (pairValues.size() == 1) {
        return {HandType::Pair, {pairValues[0], kicker}};
    } else {
        return {HandType::HighCard, {kicker}};
    }
}

HandValue EvaluateHand(const Hand& hand, const Table& table)
{
    std::array<Card, 7> cards;
    cards[0] = hand.cards[0];
    cards[1] = hand.cards[1];
    cards[2] = table.flop.cards[0];
    cards[3] = table.flop.cards[1];
    cards[4] = table.flop.cards[2];
    cards[5] = table.turn.card;
    cards[6] = table.river.card;
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
    if (result == -1) {
        left_state = "WIN";
        right_state = "LOSE";
    } else if (result == 1) {
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

void game()
{
    Deck deck;
    deck.shuffle();

    Deal deal = GetDeal(deck, 2);

    //std::cout << deck << std::endl;

    PrintGame(deal);
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


////////////////////////////////////////////////////////////////////////////////////////////////////


namespace bits
{




struct CardSet
{
    // 13 bits for each suit
    // |0 spades |13 hearts |26 diamonds |39 clubs |52
    uint64_t value_suit_bitmap;


    static constexpr uint64_t SUIT_MASK = 0x0000000000001FFFull;


    // mask for all cards for each suit
    static constexpr uint64_t suit_masks[4] = {
        SUIT_MASK,
        SUIT_MASK << 13,
        SUIT_MASK << 26,
        SUIT_MASK << 39
    };

    static constexpr uint64_t FOUR_OF_A_KIND_MASK = 0x0001000100010001ull;

    // mask for all cards for each value
    static constexpr uint64_t value_masks[13] = {
        FOUR_OF_A_KIND_MASK,
        FOUR_OF_A_KIND_MASK << 1,
        FOUR_OF_A_KIND_MASK << 2,
        FOUR_OF_A_KIND_MASK << 3,
        FOUR_OF_A_KIND_MASK << 4,
        FOUR_OF_A_KIND_MASK << 5,
        FOUR_OF_A_KIND_MASK << 6,
        FOUR_OF_A_KIND_MASK << 7,
        FOUR_OF_A_KIND_MASK << 8,
        FOUR_OF_A_KIND_MASK << 9,
        FOUR_OF_A_KIND_MASK << 10,
        FOUR_OF_A_KIND_MASK << 11,
        FOUR_OF_A_KIND_MASK << 12
    };

    static constexpr uint64_t STRAIGHT_FLUSH_MASK = 0x1Full;
    static constexpr uint64_t ROYAL_FLUSH_MASK = STRAIGHT_FLUSH_MASK << 8;


    CardSet(uint64_t bitmap) : value_suit_bitmap(bitmap) {}

    explicit CardSet(Card d)
    {
        value_suit_bitmap = 1ull << (int(d.suit) * 13 + int(d.value));
    }

    Value GetValue() const
    {
        for (int i = 0; i < 13; ++i) {
            if (value_suit_bitmap & value_masks[i]) {
                return Value(i);
            }
        }
        throw std::runtime_error("Invalid card");
    }

    Suit GetSuit() const
    {
        for (int i = 0; i < 4; ++i) {
            if (value_suit_bitmap & suit_masks[i]) {
                return Suit(i);
            }
        }
        throw std::runtime_error("Invalid card");
    }

    operator Card() const
    {
        return Card{GetValue(), GetSuit()};
    }

    explicit CardSet(const Deck& d)
    {
        value_suit_bitmap = 0;
        for (const auto& c : d.cards) {
            value_suit_bitmap |= CardSet(c).value_suit_bitmap;
        }
    }

    operator Deck() const
    {
        Deck d;
        for (int i = 0; i < 52; ++i) {
            if (value_suit_bitmap & (1ull << i)) {
                d.cards.insert(Card{Value(i % 13), Suit(i / 13)});
            }
        }
        return d;
    }

    explicit CardSet(const Hand& h)
    {
        value_suit_bitmap = CardSet(h.cards[0]).value_suit_bitmap
            | CardSet(h.cards[1]).value_suit_bitmap;
    }

    explicit CardSet(const Table& t)
    {
        value_suit_bitmap = CardSet(t.flop.cards[0]).value_suit_bitmap
            | CardSet(t.flop.cards[1]).value_suit_bitmap
            | CardSet(t.flop.cards[2]).value_suit_bitmap
            | CardSet(t.turn.card).value_suit_bitmap
            | CardSet(t.river.card).value_suit_bitmap;
    }

    CardSet operator + (const CardSet& other) const
    {
        return CardSet(value_suit_bitmap | other.value_suit_bitmap);
    }

    CardSet operator - (const CardSet& other) const
    {
        return CardSet(value_suit_bitmap & ~other.value_suit_bitmap);
    }

    bool isEmpty() const
    {
        return value_suit_bitmap == 0;
    }

    size_t CountCardsOfValue(Value v) const
    {
        uint64_t value_bit = 1ull << int(v);
        size_t count = value_bit & value_suit_bitmap ? 1 : 0;
        value_bit <<= 13;
        count += value_bit & value_suit_bitmap ? 1 : 0;
        value_bit <<= 13;
        count += value_bit & value_suit_bitmap ? 1 : 0;
        value_bit <<= 13;
        count += value_bit & value_suit_bitmap ? 1 : 0;
        return count;
    }

    static size_t CountBits(uint64_t x)
    {
        size_t count = 0;
        while (x) {
            x &= x - 1;
            ++count;
        }
        return count;
    }

    size_t CountCardsOfSuit(Suit s) const
    {
        uint64_t suit_set = value_suit_bitmap& suit_masks[int(s)];
        suit_set >>= int(s) * 13;
        return CountBits(suit_set);
    }

    struct SuitValueMaps
    {
        size_t suit_counts[4];
        size_t value_counts[13];
    };

    SuitValueMaps GetSuitValueMaps() const
    {
        SuitValueMaps maps;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 13; ++j) {
                uint64_t value_bit = 1ull << (i * 13 + j);
                if (value_suit_bitmap & value_bit) {
                    ++maps.suit_counts[i];
                    ++maps.value_counts[j];
                }
            }
        }
        return maps;
    }
};

std::ostream& operator << (std::ostream& os, const CardSet& cs)
{
    os << Deck(cs);
    return os;
}








} // namespace bits


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


void calc_deal(const std::string& line)
{
    OptDeal deal = OptDeal::FromString(line);

    Stats stats = deal.Evaluate();

    std::cout << stats << std::endl;
}


void calc_loop()
{
    std::string line;
    while (std::getline(std::cin, line)) {
        calc_deal(line);
    }
}




int main(int argc, const char* argv[])
{
    g_verbose = true;

    std::string line;
    for (int i = 1; i < argc; ++i) {
        line += std::string(argv[i]) + " ";
    }
    if (!line.empty()) {
        calc_deal(line);
    } else {
        calc_loop();
    }


    return 0;
}
