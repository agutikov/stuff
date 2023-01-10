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
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include "gtl/include/gtl/phmap.hpp"
#include <unordered_set>


#define ENABLE_HAND_LOOKUP_TABLE 0



using namespace std::chrono_literals;

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

Value& operator++(Value& value)
{
    value = static_cast<Value>(static_cast<int>(value) + 1);
    return value;
}

Value& operator--(Value& value)
{
    value = static_cast<Value>(static_cast<int>(value) - 1);
    return value;
}

constexpr bool operator<(Value lhs, Value rhs)
{
    return int(lhs) < int(rhs);
}

constexpr bool operator>(Value lhs, Value rhs)
{
    return int(lhs) > int(rhs);
}

constexpr bool operator<=(Value lhs, Value rhs)
{
    return int(lhs) <= int(rhs);
}

constexpr bool operator>=(Value lhs, Value rhs)
{
    return int(lhs) >= int(rhs);
}

constexpr bool operator==(Value lhs, Value rhs)
{
    return int(lhs) == int(rhs);
}

constexpr bool operator==(Value lhs, int rhs)
{
    return int(lhs) == rhs;
}

constexpr bool operator!=(Value lhs, Value rhs)
{
    return int(lhs) != int(rhs);
}

constexpr int operator+(Value value, int i)
{
    return int(value) + i;
}

constexpr int operator-(Value value, int i)
{
    return int(value) - i;
}

struct Card
{
    int8_t index = 0; // 0-51

    Card() = default;

    Card(int8_t index) : index(index) {}

    Value value() const
    {
        return static_cast<Value>(index % 13);
    }
    Suit suit() const
    {
        return static_cast<Suit>(index / 13);
    }

    uint64_t GetBitmask() const
    {
        return 1ULL << (index % 13 + (index / 13) * 16);
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
        auto val_it = valueMap.find(s[0]);
        auto suit_it = suitMap.find(s[1]);
        if (val_it == valueMap.end() || suit_it == suitMap.end()) {
            throw std::runtime_error("Invalid card string: " + s);
        }
        return Card(int(val_it->second) + int(suit_it->second) * 13);
    }

    Card& operator++()
    {
        ++index;
        return *this;
    }

    Card& operator--()
    {
        --index;
        return *this;
    }

    Card& operator+(int i)
    {
        index += i;
        return *this;
    }

    Card& operator-(int i)
    {
        index -= i;
        return *this;
    }
};


bool operator<(const Card& lhs, const Card& rhs)
{
    if (lhs.value() == rhs.value()) {
        return lhs.suit() < rhs.suit();
    } else {
        return lhs.value() < rhs.value();
    }
}


bool operator==(const Card& lhs, const Card& rhs)
{
    return lhs.suit() == rhs.suit() && lhs.value() == rhs.value();
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
    os << card.value() << card.suit();
    return os;
}



std::ostream& operator << (std::ostream& os, const std::vector<Card>& cards)
{
    for (const auto& card : cards) {
        os << card << " ";
    }
    return os;
}

template<typename T, size_t N>
std::ostream& operator << (std::ostream& os, const std::array<T, N>& cards)
{
    for (const auto& card : cards) {
        os << card << " ";
    }
    return os;
}

std::random_device g_rd{ "/dev/urandom" };
std::default_random_engine g_rnd{ g_rd() };

struct Deck
{
    std::set<Card> cards;

    Deck()
    {
        for (int i = 0; i < 52; ++i) {
            cards.insert(Card(i));
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


struct Deal
{
    std::vector<Hand> hands;
    std::vector<Card> board;
};

std::ostream& operator<<(std::ostream& os, const Hand& hand)
{
    os << hand.cards[0] << hand.cards[1];
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
        os << "[" << game.hands[0] << "] " << game.board << " [" << game.hands[1] << "] ";
        return os;
    }

    for (size_t i = 0; i < game.hands.size(); ++i) {
        os << "Hand " << i+1 << ": " << game.hands[i] << "; ";
    }
    os << "Table: " << game.board;
    return os;
}


constexpr size_t count_bits_16(uint16_t bits)
{
    size_t count = 0;
    while (bits) {
        bits &= bits - 1;
        count++;
    }
    return count;
}

constexpr std::array<size_t, 1 << 13> g_bitcount_table = []() {
    std::array<size_t, 1 << 13> table;
    for (size_t i = 0; i < table.size(); ++i) {
        table[i] = count_bits_16(i);
    }
    return table;
}();


size_t CountBits(uint16_t bits)
{
    return g_bitcount_table[bits];
}

struct ValueBitSet
{
    uint16_t bits = 0;

    ValueBitSet() = default;

    ValueBitSet(Value value) : bits(ValueMask(value)) {}

    explicit ValueBitSet(uint16_t bits) : bits(bits) {}

    static constexpr uint16_t ValueMask(Value value)
    {
        return 1 << static_cast<uint8_t>(value);
    }
    static constexpr uint16_t ALL_VALUES_MASK = 0x1FFF;
    static constexpr uint16_t StraightMask(Value value)
    {
        if (value == Value::Five) {
            return ValueMask(Value::Five)
                | ValueMask(Value::Four)
                | ValueMask(Value::Three)
                | ValueMask(Value::Two)
                | ValueMask(Value::Ace);
        } else {
            return ValueMask(value)
                | ValueMask(value) >> 1
                | ValueMask(value) >> 2
                | ValueMask(value) >> 3
                | ValueMask(value) >> 4;
        }
    }
    static constexpr uint16_t ROYAL_STRAIGHT_MASK = 0x1F00;
    static constexpr uint16_t CIRCLE_STRAIGHT_MASK = 0x100F;

    static ValueBitSet Straight(Value value)
    {
        return ValueBitSet(StraightMask(value));
    }

    bool HasRoyalStraight() const
    {
        return (bits & ROYAL_STRAIGHT_MASK) == ROYAL_STRAIGHT_MASK;
    }

    bool HasCircleStraight() const
    {
        return bits == CIRCLE_STRAIGHT_MASK;
    }

    operator bool() const
    {
        return bits != 0;
    }

    bool HasValue(Value value) const
    {
        return bits & ValueMask(value);
    }
    bool HasStraight(Value value) const
    {
        uint16_t mask = StraightMask(value);
        return (bits & mask) == mask;
    }
    ValueBitSet FindStraight() const
    {
        uint16_t mask = StraightMask(Value::Ace);
        for (uint8_t i = 0; i < 9; ++i) {
            if ((bits & mask) == mask) {
                return ValueBitSet(mask);
            }
            mask >>= 1;
        }
        mask = StraightMask(Value::Five);
        if ((bits & mask) == mask) {
            return ValueBitSet(mask);
        }
        return {};
    }

    struct SortedValueList
    {
        std::array<Value, 13> values;
        size_t size = 0;
    };

    SortedValueList GetSortedValueList() const
    {
        SortedValueList list;
        for (int value = static_cast<int>(Value::Ace); value >= static_cast<int>(Value::Two); --value) {
            if (bits & ValueMask(static_cast<Value>(value))) {
                list.values[list.size++] = static_cast<Value>(value);
            }
        }
        return list;
    }

    Value GetHighestValue() const
    {
        for (int value = static_cast<int>(Value::Ace); value >= static_cast<int>(Value::Two); --value) {
            if (bits & ValueMask(static_cast<Value>(value))) {
                return static_cast<Value>(value);
            }
        }
        throw std::runtime_error("ValueBitSet is empty");
    }

    template<size_t N>
    ValueBitSet GetHighestValues() const
    {
        ValueBitSet list;
        size_t i = 0;
        for (int value = static_cast<int>(Value::Ace); value >= static_cast<int>(Value::Two); --value) {
            if (bits & ValueMask(static_cast<Value>(value))) {
                list += ValueBitSet(static_cast<Value>(value));
                i++;
            }
            if (i == N) {
                return list;
            }
        }
        throw std::runtime_error("ValueBitSet is empty");
    }

    void Add(Value value)
    {
        bits |= ValueMask(value);
    }

    void Remove(Value value)
    {
        bits &= ~ValueMask(value);
    }

    ValueBitSet TrimToSize(size_t size) const
    {
        auto list = GetSortedValueList();
        ValueBitSet result;
        for (size_t i = 0; i < size; ++i) {
            result.Add(list.values[i]);
        }
        return result;
    }

    ValueBitSet& operator += (Value value)
    {
        Add(value);
        return *this;
    }
    ValueBitSet& operator -= (Value value)
    {
        Remove(value);
        return *this;
    }
    ValueBitSet& operator += (ValueBitSet other)
    {
        bits |= other.bits;
        return *this;
    }
    ValueBitSet& operator -= (ValueBitSet other)
    {
        bits &= ~other.bits;
        return *this;
    }
};

ValueBitSet operator + (ValueBitSet lhs, Value value)
{
    return ValueBitSet(lhs.bits | ValueBitSet::ValueMask(value));
}
ValueBitSet operator - (ValueBitSet lhs, Value value)
{
    return ValueBitSet(lhs.bits & ~ValueBitSet::ValueMask(value));
}
ValueBitSet operator + (ValueBitSet lhs, ValueBitSet rhs)
{
    return ValueBitSet(lhs.bits | rhs.bits);
}
ValueBitSet operator - (ValueBitSet lhs, ValueBitSet rhs)
{
    return ValueBitSet(lhs.bits & ~rhs.bits);
}


std::ostream& operator<<(std::ostream& os, const ValueBitSet& set)
{
    for (int8_t i = 13; i >= 0; --i) {
        if (set.bits & ValueBitSet::ValueMask(Value(i))) {
            os << Value(i);
        }
    }
    return os;
}

struct CardBitSet
{
    static_assert(sizeof(ValueBitSet) * 4 == sizeof(uint64_t), "ValueBitSet must be 16 bits");

    union {
        uint64_t bits;
        ValueBitSet suits[4];
        uint16_t _suits[4];
    } u = {.bits = 0};

    void SortSuits()
    {
        std::sort(std::begin(u._suits), std::end(u._suits), std::greater<uint16_t>());
    }

    int64_t Get52Bits() const
    {
        int64_t suit_values[4] = {
            u.suits[0].bits,
            u.suits[1].bits,
            u.suits[2].bits,
            u.suits[3].bits,
        };
        return suit_values[0]
            | suit_values[1] << (16 - 3)
            | suit_values[2] << (32 - 6)
            | suit_values[3] << (48 - 9);
    }

    int64_t Get40Bits() const
    {
        int64_t result = 0;
        int64_t shift = 1;
        for (int8_t i = 0; i < 52; i++) {
            if (HasCard(i)) {
                result += i * shift;
                shift *= 52;
            }
        }
        return result;
    }

    uint32_t Get32Bits() const
    {
        int64_t result = 0;
        int64_t shift = 1;
        for (int8_t i = 0; i < 52; i++) {
            if (HasCard(i)) {
                result += i * shift;
                shift *= 52;
            }
        }
        return result & 0xFFFFFFFF;
    }

    static CardBitSet FullDeck()
    {
        return CardBitSet{ 0x1FFF1FFF1FFF1FFF };
    }

    static constexpr uint64_t FourOfAKindMask(Value value)
    {
        return uint64_t(ValueBitSet::ValueMask(value))
            | uint64_t(ValueBitSet::ValueMask(value)) << 16
            | uint64_t(ValueBitSet::ValueMask(value)) << 32
            | uint64_t(ValueBitSet::ValueMask(value)) << 48;
    }
    static constexpr uint64_t SuitMask(Suit suit)
    {
        return 0x1FFF << (static_cast<uint8_t>(suit) * 16);
    }

    CardBitSet() = default;

    explicit CardBitSet(uint64_t bits) : u{.bits=bits} {}

    CardBitSet(const std::vector<Card>& cards)
    {
        for (const auto& card : cards) {
            AddCard(card);
        }
    }

    void AddCard(Card card)
    {
        u.bits |= card.GetBitmask();
    }

    void RemoveCard(Card card)
    {
        u.bits &= ~card.GetBitmask();
    }

    bool HasCard(Card card) const
    {
        return u.bits & card.GetBitmask();
    }

    ValueBitSet GetAllValues() const
    {
        return ValueBitSet( u.suits[0].bits | u.suits[1].bits | u.suits[2].bits | u.suits[3].bits );
    }

    ValueBitSet GetSuitValues(Suit suit) const
    {
        return u.suits[static_cast<uint8_t>(suit)];
    }

    std::array<size_t, 4> GetSuitsCount() const
    {
        return {
            CountBits(u.suits[0].bits),
            CountBits(u.suits[1].bits),
            CountBits(u.suits[2].bits),
            CountBits(u.suits[3].bits)
        };
    }
};

std::ostream& operator<<(std::ostream& os, const CardBitSet& set)
{
    for (int8_t i = 0; i < 52; ++i) {
        Card card{i};
        if (set.HasCard(card)) {
            os << card;
        }
    }
    return os;
}

enum class HandType : uint8_t
{                  //  size of descriptor | type     | size
    HighCard = 0,  //             5 cards | mask     |  13
    Pair,          //     1 + 3 = 4 cards | 4 values |  15
    TwoPair,       // 1 + 1 + 1 = 3 cards | 3 values |  12
    ThreeOfAKind,  //     1 + 2 = 3 cards | 3 values |  12
    Straight,      //             1 card  | value    |   4
    Flush,         //             5 cards | mask     |  13
    FullHouse,     //     1 + 1 = 2 cards | 2 values |   8
    FourOfAKind,   //     1 + 1 = 2 cards | 2 values |   8
    StraightFlush, //             1 card  | value    |   4
    RoyalFlush     //             0 cards | -        |   0
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
    std::optional<Value> value;
    std::optional<ValueBitSet> values;

    uint32_t Pack() const
    {
        uint32_t result = static_cast<uint32_t>(type) << 17;
        if (type == HandType::TwoPair) {
            if (values) {
                result |= values->bits << 4;
            }
            if (value) {
                result |= static_cast<uint32_t>(*value);
            }
        } else {
            if (value) {
                result |= static_cast<uint32_t>(*value) << 13;
            }
            if (values) {
                result |= values->bits;
            }
        }
        return result;
    }

    uint32_t Hash() const
    {
        uint32_t hash = static_cast<uint32_t>(type) * 13 * (1 << 13);
        if (type == HandType::TwoPair) {
            if (values) {
                hash += values->bits * 13;
            }
            if (value) {
                hash += static_cast<uint32_t>(*value);
            }
        } else {
            if (value) {
                hash += static_cast<uint32_t>(*value) << 13;
            }
            if (values) {
                hash += values->bits;
            }
        }
        return hash;
    }
    static constexpr size_t MAX_HASH_VALUE =
        static_cast<size_t>(HandType::RoyalFlush)
        * static_cast<size_t>(Value::Ace)
        * 0x1F0;
};

bool operator<(const HandValue& lhs, const HandValue& rhs)
{
    return lhs.Pack() < rhs.Pack();
}
bool operator>(const HandValue& lhs, const HandValue& rhs)
{
    return lhs.Pack() > rhs.Pack();
}

//std::unordered_map<uint64_t, HandValue> g_hand_lookup_table;
//google::dense_hash_map<int64_t, HandValue> g_hand_lookup_table;
gtl::flat_hash_map<uint32_t, HandValue, std::hash<uint32_t>> g_hand_lookup_table;


std::ostream& operator<<(std::ostream& os, const std::vector<Value>& value)
{
    for (const auto& v : value) {
        os << v;
    }
    return os;
}

template<size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<Value, N>& value)
{
    for (const auto& v : value) {
        os << v;
    }
    return os;
}

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
    if (handValue.type == HandType::TwoPair) {
        os << handValue.type << " " << *handValue.values << " " << *handValue.value;
    } else {
        os << handValue.type;
        if (handValue.value) {
            os << " " << *handValue.value;
        }
        if (handValue.values) {
            os << " " << *handValue.values;
        }
    }
    return os;
}


size_t g_lookup_hits = 0;
size_t g_lookup_misses = 0;

HandValue EvaluateCards(const Card* cards, size_t count)
{
    // std::cout << "====================" << std::endl;
    // for (int i = 0; i < count; i++) {
    //     std::cout << cards[i];
    // }
    // std::cout << std::endl;

    CardBitSet card_bits;
    for (size_t i = 0; i < count; ++i) {
        card_bits.AddCard(cards[i]);
    }
    // std::cout << card_bits << std::endl;

#if ENABLE_HAND_LOOKUP_TABLE
    card_bits.SortSuits();
    uint32_t key = card_bits.Get32Bits();

    auto it = g_hand_lookup_table.find(key);
    if (it != g_hand_lookup_table.end()) {
        g_lookup_hits++;
        return it->second;
    }
    g_lookup_misses++;
#endif

    std::array<size_t, 13> value_counts = { 0 };
    for (size_t i = 0; i < count; ++i) {
        value_counts[static_cast<size_t>(cards[i].value())]++;
    }

    std::array<size_t, 4> suit_counts = card_bits.GetSuitsCount();

    ValueBitSet value_bits = card_bits.GetAllValues();
    ValueBitSet::SortedValueList values = value_bits.GetSortedValueList();

    std::optional<Value> three_of_a_kind_value;
    std::array<Value, 3> pair_values;
    size_t pair_count = 0;
    ValueBitSet flush_values;
    ValueBitSet straight_values;
    ValueBitSet straight_flush_values;

    // Count Four of a Kind, Three of a Kind and Pairs
    for (size_t i = 0; i < values.size; ++i) {
        Value value = values.values[i];
        size_t count = value_counts[static_cast<size_t>(value)];
        if (count == 4) {
            value_bits -= value;
            return { HandType::FourOfAKind, value, value_bits.GetHighestValues<1>() };
        } else if (count == 3) {
            if (three_of_a_kind_value) {
                pair_values[pair_count++] = value;
            } else {
                three_of_a_kind_value = value;
            }
        } else if (count == 2) {
            pair_values[pair_count++] = value;
        }
    }

    // Check for Flush
    for (size_t i = 0; i < suit_counts.size(); ++i) {
        if (suit_counts[i] >= 5) {
            flush_values = card_bits.GetSuitValues(static_cast<Suit>(i));
            break;
        }
    }

    // Check for Straight
    if (values.size >= 5) {
        straight_values = value_bits.FindStraight();
    }

    // Check for Straight Flush
    if (straight_values && flush_values) {
        straight_flush_values = flush_values.FindStraight();
        if (straight_flush_values) {
            if (straight_flush_values.HasRoyalStraight()) {
                return { HandType::RoyalFlush };
            } else {
                if (straight_flush_values.HasCircleStraight()) {
                    return { HandType::StraightFlush, Value::Five };
                } else {
                    return { HandType::StraightFlush, straight_flush_values.GetHighestValue() };
                }
            }
        }
    }

    // Trim Flush values to 5
    if (flush_values) {
        flush_values = flush_values.TrimToSize(5);
    }

    // Evaluate Hand
    if (three_of_a_kind_value && pair_count >= 1) {
        return { .type=HandType::FullHouse, .values=ValueBitSet(*three_of_a_kind_value) + pair_values[0] };
    } else if (flush_values) {
        return { .type=HandType::Flush, .values=flush_values };
    } else if (straight_values) {
        if (straight_values.HasCircleStraight()) {
            return { .type=HandType::Straight, .value=Value::Five };
        } else {
            return { .type=HandType::Straight, .value=straight_values.GetHighestValue() };
        }
    } else if (three_of_a_kind_value) {
        value_bits -= *three_of_a_kind_value;
        return { HandType::ThreeOfAKind, *three_of_a_kind_value, value_bits.GetHighestValues<2>() };
    } else if (pair_count >= 2) {
        ValueBitSet pairs = pair_values[0];
        pairs += pair_values[1];
        value_bits -= pairs;
        return { .type=HandType::TwoPair, .value=value_bits.GetHighestValue(), .values = pairs };
    } else if (pair_count == 1) {
        value_bits -= pair_values[0];
        return { HandType::Pair, pair_values[0], value_bits.GetHighestValues<3>() };
    } else {
        return { .type=HandType::HighCard, .values=value_bits.GetHighestValues<5>() };
    }
}

HandValue EvaluateHand(const Hand& hand, const std::vector<Card>& board)
{
    std::vector<Card> cards;
    cards.insert(cards.end(), hand.cards.begin(), hand.cards.end());
    cards.insert(cards.end(), board.begin(), board.end());
    return EvaluateCards(cards.data(), cards.size());
}

int CompareHands(const HandValue& self, const HandValue& opp)
{
    uint32_t lhs = self.Pack();
    uint32_t rhs = opp.Pack();
    if (lhs < rhs) {
        return -1;
    } else if (lhs > rhs) {
        return 1;
    } else {
        return 0;
    }
}

std::vector<HandValue> GetHandValues(const Deal& deal)
{
    std::vector<HandValue> handValues;
    for (const auto& hand : deal.hands) {
        handValues.push_back(EvaluateHand(hand, deal.board));
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
       << "Fail: " << 100.0 * (double)(stats.total - stats.win - stats.draw) / stats.total << "%"
      // << ", count=" << stats.total
    ;
    return os;
}


std::vector<Card> ParseCards(const std::string& str)
{
    std::vector<Card> cards;
    int i = 0;
    while (i < str.size()) {
        if (str[i] == ' ') {
            ++i;
            continue;
        }
        cards.push_back(Card::FromString(str.substr(i, 2)));
        i += 2;
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

void comb(int N, int K)
{
    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    // print integers and permute bitmask
    do {
        for (int i = 0; i < N; ++i) // [0..N-1] integers
        {
            if (bitmask[i]) std::cout << " " << i;
        }
        std::cout << std::endl;
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
}

struct CardsGenerator
{
    std::vector<Card> available;
    std::vector<Card> state;
    std::vector<int> bitmask;
    bool done = false;

    CardsGenerator(int k, const Card* dead_cards = nullptr, int dead_cards_count = 0) :
        bitmask(k, 1)
    {
        CardBitSet dead;
        for (int i = 0; i < dead_cards_count; ++i) {
            dead.AddCard(dead_cards[i]);
        }
        for (int i = 0; i < 52; ++i) {
            if (!dead.HasCard(Card(i))) {
                available.push_back(Card(i));
            }
        }
        state.resize(k);
        bitmask.resize(available.size(), 0);
    }

    bool next()
    {
        if (done) {
            return false;
        }
        int j = 0;
        for (int i = 0; i < available.size(); ++i) // [0..N-1] integers
        {
            if (bitmask[i]) {
                state[j++] = available[i];
            }
        }
        done = !std::prev_permutation(bitmask.begin(), bitmask.end());
        return true;
    }

};


void TestGen()
{
    std::vector<Card> dead_cards = ParseCards("2h3h4h5h6h7h8h9hThJhQhKhAh 2c4c5c6c7c8c9cTcJcQcAc 3d4d5d6d7d8d9dTdJdQdKd 3s4s5s6s7s8s9sTsJsQsKs");
    CardsGenerator gen(2, dead_cards.data(), dead_cards.size());
    int count = 0;
    while (gen.next()) {
        for (Card card : gen.state) {
            std::cout << card;
        }
        std::cout << " ";
        count++;
        if (count % 13 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}


struct HandStats
{
    std::array<size_t, 10> stats;
    size_t total = 0;

    HandStats()
    {
        std::fill(stats.begin(), stats.end(), 0);
    }

    void Add(HandType hand_type)
    {
        stats[int(hand_type)]++;
        total++;
    }

    static constexpr std::array<size_t, 10> required_stats = {
        23'294'460,
        58'627'800,
        31'433'400,
         6'461'620,
         6'180'020,
         4'047'644,
         3'473'184,
           224'848,
            37'260,
             4'324
    };

    void Verify()
    {
        for (int i = 9; i >= 0; --i) {
            if (stats[i] != required_stats[i]) {
                std::cout << "Error: " << HandType(i) << " " << stats[i] << " != " << required_stats[i] << std::endl;
            } else {
                std::cout << "OK: " << HandType(i) << " " << stats[i] << " == " << required_stats[i] << std::endl;
            }
        }
    }
};

std::ostream& operator << (std::ostream& os, const HandStats& stats)
{
    for(int i = 9; i >= 0; --i) {
        os << std::setw(5) << HandType(i) << ": "
            << std::setw(9) << stats.stats[i] << "   "
            << 100.0 * double(stats.stats[i]) / stats.total << "%"
            << std::endl;
    }
    std::cout << "Deals: " << std::setw(9) << stats.total << std::endl;
    return os;
}

constexpr size_t NUM_OF_DISTINCT_HANDS = 4746;

uint32_t g_card_hand_prob[52][NUM_OF_DISTINCT_HANDS] = {{0}};
HandValue g_hand_values[NUM_OF_DISTINCT_HANDS];

void LoopAllDeals()
{
    struct HandTableEntry
    {
        HandValue value;
        size_t times = 0;
        int index = 0;
        std::vector<std::array<Card, 7>> source;
    };
    std::map<uint32_t, HandTableEntry> all_hands;
    CardsGenerator gen(7);

    HandStats hand_stats;
    auto start = std::chrono::system_clock::now();

    while (gen.next()) {
        auto hand_value = EvaluateCards(gen.state.data(), gen.state.size());

        auto it = all_hands.find(hand_value.Pack());
        if (it != all_hands.end()) {
            it->second.times++;
            it->second.source.push_back({
                gen.state[0],
                gen.state[1],
                gen.state[2],
                gen.state[3],
                gen.state[4],
                gen.state[5],
                gen.state[6]
            });
        } else {
            all_hands[hand_value.Pack()] = HandTableEntry{
                .value = hand_value,
                .times = 1,
                .source = {{
                    gen.state[0],
                    gen.state[1],
                    gen.state[2],
                    gen.state[3],
                    gen.state[4],
                    gen.state[5],
                    gen.state[6]
                }}
            };
        }

        hand_stats.Add(hand_value.type);

        if (hand_stats.total % 1'000'000 == 0) {
            std::cout << "=======================================" << std::endl;
            std::cout << hand_stats << std::endl;
        }
    }
    auto end = std::chrono::system_clock::now();
    std::cout << "=======================================" << std::endl;
    std::cout << hand_stats << std::endl;
    std::cout << "Elapsed: " << std::chrono::duration<double>(end - start).count() << " s" << std::endl;

    hand_stats.Verify();

    std::cout << "Lookup Hits: " << g_lookup_hits << std::endl;
    std::cout << "Lookup Misses: " << g_lookup_misses << std::endl;

    int index = 0;
    for (auto& [key, hand] : all_hands) {
        hand.index = index++;
        std::cout << hand.index << ": "
            << hand.value << " |"
            << "  packed: 0x" << std::hex << hand.value.Pack() << std::dec
            << "  hash: " << hand.value.Hash()
            << "  times: " << hand.times
            << std::endl;

        if (hand.index >= NUM_OF_DISTINCT_HANDS) {
            throw std::runtime_error("Exceed NUM_OF_DISTINCT_HANDS");
        }

        g_hand_values[hand.index] = hand.value;
        for (const auto& cards : hand.source) {
            for (const Card& card : cards) {
                g_card_hand_prob[card.index][hand.index]++;
            }
        }
    }
    std::cout << "Total hands: " << all_hands.size() << std::endl;

    for (size_t hand_index = 0; hand_index < NUM_OF_DISTINCT_HANDS; hand_index++){
        std::cout << hand_index << ": " << g_hand_values[hand_index] << std::endl;
        for (int i = 0; i < 52; i++) {
            if (i != 0) {
                std::cout << ", ";
            }
            Card c(i);
            std::cout << c << ": " << g_card_hand_prob[i][hand_index];
        }
        std::cout << std::endl;
    }
}

void TestHash52bit()
{
    std::unordered_set<uint32_t> all_deals;
    std::unordered_set<uint16_t> high_16_bits;
    std::unordered_set<uint16_t> low_16_bits;
    std::unordered_set<uint8_t> high_8_bits[4];
    std::unordered_set<uint32_t> low_24_bits;

    CardsGenerator gen(7);

    uint32_t xor_mask = 0;

    while (gen.next()) {
        CardBitSet set(gen.state);

        set.SortSuits();
        uint32_t bits = set.Get32Bits();

        if (all_deals.find(bits) != all_deals.end()) {
            continue;
        }

        all_deals.insert(bits);

        high_16_bits.insert((bits >> 16) & 0xFFFF);
        low_16_bits.insert(bits & 0xFFFF);
        high_8_bits[0].insert((bits >> 24) & 0xFF);
        high_8_bits[1].insert((bits >> 16) & 0xFF);
        high_8_bits[2].insert((bits >> 8) & 0xFF);
        high_8_bits[3].insert(bits & 0xFF);
        low_24_bits.insert(bits & 0xFFFFFF);

        xor_mask ^= bits;
    }

    std::cout << "32 bit keys: " << all_deals.size() << std::endl;
    std::cout << "High 16 bits: " << high_16_bits.size() << std::endl;
    std::cout << "Low 16 bits: " << low_16_bits.size() << std::endl;
    std::cout << "Highest 8 bits [3]: " << high_8_bits[0].size() << std::endl;
    std::cout << "High 8 bits [2]: " << high_8_bits[1].size() << std::endl;
    std::cout << "Low 8 bits [1]: " << high_8_bits[2].size() << std::endl;
    std::cout << "Lowest 8 bits [0]: " << high_8_bits[3].size() << std::endl;
    std::cout << "Low 24 bits: " << low_24_bits.size() << std::endl;
    if (all_deals.find(xor_mask) != all_deals.end()) {
        std::cout << "XOR mask is in the set!" << std::endl;
    }
    std::cout << "XOR mask: 0x" << std::hex << xor_mask << std::dec << std::endl;
}


void GenerateHandLookupTable()
{
    std::cout << "Generating lookup table..." << std::endl;
    std::cout << "This may take a while..." << std::endl;
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    CardsGenerator gen(7);

    //g_hand_lookup_table.reserve(6009159);
    //g_hand_lookup_table.set_empty_key(0);

    while (gen.next()) {
        CardBitSet hand_bitset(gen.state);
        hand_bitset.SortSuits();
        uint32_t key = hand_bitset.Get32Bits();
        if (g_hand_lookup_table.find(key) != g_hand_lookup_table.end()) {
            continue;
        }
        auto hand_value = EvaluateCards(gen.state.data(), gen.state.size());
        g_hand_lookup_table[key] = hand_value;
    }

    std::cout << "Lookup table size: " << g_hand_lookup_table.size() << std::endl;
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s" << std::endl;
}


Stats EquityHeadsup(const Hand& hand, const std::vector<Card>& board, bool verbose)
{
    Stats stats;
    size_t next_total_output = 2'000'000;

    std::array<Card, 9> deal_state;
    deal_state[0] = hand.cards[0];
    deal_state[1] = hand.cards[1];
    for (int i = 0; i < board.size(); ++i) {
        deal_state[i + 2] = board[i];
    }

    CardsGenerator board_gen(5 - board.size(), deal_state.data(), 2 + board.size());

    while (board_gen.next()) {
        for (int i = 0; i < board_gen.state.size(); ++i) {
            deal_state[2 + board.size() + i] = board_gen.state[i];
        }

        auto our_hand_value = EvaluateCards(deal_state.data(), 7);

        CardsGenerator hand_gen(2, deal_state.data(), 7);

        while(hand_gen.next()) {
            deal_state[7] = hand_gen.state[0];
            deal_state[8] = hand_gen.state[1];

            // std::cout << std::endl;
            // std::cout << "board_gen.state: " << board_gen.state << std::endl;
            // std::cout << "board_gen.available: " << board_gen.available << std::endl;
            // std::cout << "hand_gen.state: " <<  hand_gen.state << std::endl;
            // std::cout << "hand_gen.available: " <<  hand_gen.available << std::endl;
            // std::cout << "deal_state: " << deal_state << std::endl;

            auto hand_value = EvaluateCards(deal_state.data() + 2, 7);

            int cmp = CompareHands(our_hand_value, hand_value);

            stats.total++;
            if (cmp > 0) {
                stats.win++;
            } else if (cmp == 0) {
                stats.draw++;
            }

            if (verbose) {
                PrintGame(Deal{
                    .hands = { hand, Hand{ deal_state[7], deal_state[8] } },
                    .board = { deal_state[2], deal_state[3], deal_state[4], deal_state[5], deal_state[6] }
                });
            }
        }

        if (stats.total >= next_total_output) {
            //std::cout << "==========================" << std::endl;
            //std::cout << hand << " " << board << " : " << stats << "; Total deals: " << stats.total << std::endl;
            next_total_output += 10'000'000;
        }
    }

    return stats;
}


std::vector<Card> RandomCards(int count)
{
    std::vector<Card> cards;
    cards.reserve(count);
    CardBitSet deck = CardBitSet::FullDeck();
    std::uniform_int_distribution<int> distr(0, 51);
    while (cards.size() < count) {
        Card card = Card(distr(g_rnd));
        if (deck.HasCard(card)) {
            cards.push_back(card);
            deck.RemoveCard(card);
        }
    }
    return cards;
}

void DealEquity(const std::vector<Card>& deal, bool verbose)
{
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    auto stats = EquityHeadsup(Hand{ deal[0], deal[1] }, {deal.begin() + 2, deal.end()}, verbose);
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    std::cout << deal << ": " << stats << "; ";
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Total deals: " << stats.total << "; "
        << "Elapsed time: " << elapsed_seconds.count() << "s"
        << std::endl;
}


void RandomDealEquityLoop(int board_cards)
{
    while (true) {
        auto deal = RandomCards(2 + board_cards);
        // std::cout << deal << std::endl;
        DealEquity(deal, false);
    }
}


void TestEval()
{
    static const std::vector<std::pair<std::string, std::string>> tests = {
        {"AsKs QsJsTs Ah Kh", "RF"},

        {"QsJs 9c7c6c Tc 8c", "SF T"},
        {"KdQd 9dJd8d 7d Td", "SF K"},

        {"KsKd 9s9dKc Kh 7s", "4 K 9"},
        {"KsKd 2s2dKc Kh 2c", "4 K 2"},

        {"9hKc Jd7sKs 9c Kh", "FH K9"},
        {"2s2c Ac3d3c 2h 3s", "FH 32"},

        {"Jh8h 5h2c2h Kh 9h", "F KJ985"},
        {"Jc7c JsJh3c Tc 8c", "F JT873"},

        {"Jc9s 7c4s5h Ts 8h", "S J"},
        {"Jc9s 3c4s5h As 2h", "S 5"},

        {"4h6h 4c4dKc 9c Ts", "3 4 KT"},
        {"8c8d 4d2s5d 8s 7d", "3 8 75"},

        {"5d2h Kh9sQd 9h Kd", "2p K9 Q"},
        {"6dJc 6s2hAd Tc 2c", "2p 62 A"},

        {"2d7c JcQdAd 6c 6s", "p 6 AQJ"},
        {"KcAs 3c2s6d Ad Jd", "p A KJ6"},

        {"3cJd 7h8d6d Qc 9h", "h QJ987"},
        {"7cTc 5d2c3h Ah 6s", "h AT765"},
    };

    for (const auto& [deal, expected] : tests) {
        auto cards = ParseCards(deal);
        auto hand_value = EvaluateCards(cards.data(), cards.size());
        std::string hand_value_str = to_string(hand_value);
        if (hand_value_str != expected) {
            std::cout << "FAIL: " << deal << " : " << hand_value_str << " != " << expected << std::endl;
        } else {
            std::cout << "OK: " << deal << " : " << hand_value_str << std::endl;
        }
    }
}


int main(int argc, const char* argv[])
{
    std::string line;
    for (int i = 1; i < argc; ++i) {
        line += std::string(argv[i]) + " ";
    }
    if (!line.empty()) {
        auto deal = ParseCards(line);
        DealEquity(deal, true);
    } else {
        TestEval();
        TestGen();
        //GenerateHandLookupTable();
        //TestHash52bit();
        //LoopAllDeals();
        RandomDealEquityLoop(0);
    }

    return 0;
}

