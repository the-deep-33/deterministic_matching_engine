// matching_engine.cpp
//
// Reference matching engine for the specification "Matching engine: rules and
// formats" (Nov 15, 2026), revised edition with the participant range and the
// open-order limit. One instrument, limit orders only, price then arrival
// time. Reads order messages from standard input, one per line, and writes the
// resulting events to standard output, one per line.
//
// Build:  g++ -std=c++20 -O0 -Wall -Wextra -Wpedantic matching_engine.cpp -o matching_engine
// Run:    ./matching_engine < input.txt > output.txt
//
// ---------------------------------------------------------------------------
// ASSUMPTIONS
// Choices made where the specification was unclear or left a case open, one
// line each.
//
//  1. "Repeat the step below" in New order rule 6 means the paragraph "One matching step", not rule 7 (Rest).
//  2. Sequence numbers are trusted to be positive and strictly increasing, as stated; they are not checked, since no event exists to report a violation.
//  3. The input shape is trusted as stated (field count, whole numbers that fit in signed 64 bits, type N or C, side B or S) and is not validated.
//  4. Every number, time included, is read as a signed 64-bit integer and written in plain decimal, so unusual spellings such as 007 or +7 come out as 7.
//  5. Time is otherwise untouched: it is never compared, ordered or used in arithmetic, only written back.
//  6. A line holding only spaces, tabs or a carriage return counts as empty and is skipped; a carriage return before the newline (CRLF input) is ignored.
//  7. Each output line ends with a single line feed, the last line included.
//  8. The "Seen by" column is not acted on: every event goes to standard output, with no per-participant filtering or routing.
//  9. A REJECT names the sender exactly as sent, even when that value is out of range (BAD_PARTICIPANT), as worked example 7 shows.
// 10. A cancel's order id is not range-checked: zero, negative or otherwise unused ids fall under Cancel case 2 (NOT_YOURS), as does the cancel's own sequence number.
// 11. The open-order limit counts orders, not lots, and applies to new orders only; a cancel is never refused for it.
// 12. Standard output is flushed after each message, so a message's events are out before the next line is read.
// ---------------------------------------------------------------------------

#include <algorithm>
#include <cstdint>
#include <deque>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

// Every number in the input is read as a signed 64-bit integer.
using Number = std::int64_t;

const Number MIN_PRICE = 1;
const Number MAX_PRICE = 100'000'000;
const Number MIN_QUANTITY = 1;
const Number MAX_QUANTITY = 2'147'483'647;
const Number MIN_PARTICIPANT = 0;
const Number MAX_PARTICIPANT = 1'048'575;
const Number MAX_RESTING_PER_PARTICIPANT = 200;

// One record per order ever created. Records are never erased, so for any id
// we can tell "never created" (no record) from "created but gone" (record with
// in_book == false) for the whole run.
struct Order {
    Number participant;  // owner
    char side;           // 'B' or 'S'
    Number price;        // limit price in ticks
    Number remaining;    // lots not yet traded
    bool in_book;        // true only while the order is resting in the book
};

// One side of the book: price -> the line of order ids resting at that price,
// longest-resting first. A price is present only while its line is not empty.
using BookSide = std::map<Number, std::deque<Number>>;

class Engine {
public:
    void new_order(Number seq, Number time, Number participant, char side,
                   Number price, Number quantity) {
        // 1. Check the quantity.
        if (quantity < MIN_QUANTITY || quantity > MAX_QUANTITY) {
            event(seq, time, "REJECT") << ' ' << participant << " BAD_QUANTITY\n";
            return;
        }

        // 2. Check the price.
        if (price < MIN_PRICE || price > MAX_PRICE) {
            event(seq, time, "REJECT") << ' ' << participant << " BAD_PRICE\n";
            return;
        }

        // 3. Check the participant.
        if (!participant_in_range(participant)) {
            event(seq, time, "REJECT") << ' ' << participant << " BAD_PARTICIPANT\n";
            return;
        }

        // 4. Check the open-order limit, counting the orders already resting
        //    when this message arrives.
        if (resting_orders_of(participant) >= MAX_RESTING_PER_PARTICIPANT) {
            event(seq, time, "REJECT") << ' ' << participant << " TOO_MANY_ORDERS\n";
            return;
        }

        // 5. Accept. The order's id is this message's sequence number.
        const Number id = seq;
        Order& incoming = orders_[id];
        incoming = Order{participant, side, price, quantity, false};
        event(seq, time, "ACK") << ' ' << participant << ' ' << id << ' ' << side
                                << ' ' << price << ' ' << quantity << '\n';

        // 6. Match, one step at a time, while the incoming order has remaining
        //    quantity, the opposite side is not empty, and the prices cross.
        BookSide& opposite = (side == 'B') ? sells_ : buys_;
        while (incoming.remaining > 0 && !opposite.empty()) {
            // Best opposite price: lowest sell for a buy, highest buy for a sell.
            const BookSide::iterator best =
                (side == 'B') ? opposite.begin() : std::prev(opposite.end());
            const Number best_price = best->first;
            const bool crosses =
                (side == 'B') ? (best_price <= price) : (best_price >= price);
            if (!crosses) {
                break;
            }

            // First in line: at the best price, the one resting there longest.
            const Number resting_id = best->second.front();
            Order& resting = orders_.at(resting_id);

            if (resting.participant == participant) {
                // Self-trade prevention: cancel the resting order in full.
                const Number gone = resting.remaining;
                remove_from_book(resting_id);
                event(seq, time, "CANCELLED") << ' ' << resting.participant << ' '
                                              << resting_id << ' ' << gone << " STP\n";
                event(seq, time, "REMOVE") << ' ' << resting_id << ' ' << gone << '\n';
            } else {
                // Trade at the resting order's price.
                const Number trade_price = resting.price;
                const Number trade_quantity =
                    std::min(incoming.remaining, resting.remaining);
                const Number trade = next_trade_;
                next_trade_ += 1;

                resting.remaining -= trade_quantity;
                incoming.remaining -= trade_quantity;

                event(seq, time, "FILL")
                    << ' ' << resting.participant << ' ' << resting_id << ' ' << trade
                    << " M " << trade_price << ' ' << trade_quantity << ' '
                    << resting.remaining << '\n';
                event(seq, time, "FILL")
                    << ' ' << participant << ' ' << id << ' ' << trade
                    << " T " << trade_price << ' ' << trade_quantity << ' '
                    << incoming.remaining << '\n';
                event(seq, time, "TRADE")
                    << ' ' << trade << ' ' << resting_id << ' ' << trade_price << ' '
                    << trade_quantity << ' ' << side << '\n';

                // A fully filled resting order leaves the book; a partly filled
                // one stays where it is, still first in line.
                if (resting.remaining == 0) {
                    remove_from_book(resting_id);
                }
            }
        }

        // 7. Rest whatever is left, behind every order already at that price.
        if (incoming.remaining > 0) {
            BookSide& own = (side == 'B') ? buys_ : sells_;
            own[price].push_back(id);
            incoming.in_book = true;
            event(seq, time, "ADD") << ' ' << id << ' ' << side << ' ' << price << ' '
                                    << incoming.remaining << '\n';
        }
    }

    void cancel(Number seq, Number time, Number participant, Number order_id) {
        // 1. The sender is out of range. The order id is not looked at.
        if (!participant_in_range(participant)) {
            event(seq, time, "REJECT") << ' ' << participant << " BAD_PARTICIPANT\n";
            return;
        }

        // 2. No order was ever created with that id.
        const std::map<Number, Order>::iterator found = orders_.find(order_id);
        if (found == orders_.end()) {
            event(seq, time, "REJECT") << ' ' << participant << " NOT_YOURS\n";
            return;
        }
        const Order& order = found->second;

        // 3. The order was created but is no longer in the book.
        if (!order.in_book) {
            event(seq, time, "REJECT") << ' ' << participant << " TOO_LATE\n";
            return;
        }

        // 4. The order is in the book and belongs to a different participant.
        if (order.participant != participant) {
            event(seq, time, "REJECT") << ' ' << participant << " NOT_YOURS\n";
            return;
        }

        // 5. The order is in the book and belongs to the sender.
        const Number gone = order.remaining;
        remove_from_book(order_id);
        event(seq, time, "CANCELLED") << ' ' << participant << ' ' << order_id << ' '
                                      << gone << " USER\n";
        event(seq, time, "REMOVE") << ' ' << order_id << ' ' << gone << '\n';
    }

private:
    // Starts an output line: "<seq> <time> <name>". The caller appends the rest.
    static std::ostream& event(Number seq, Number time, const char* name) {
        return std::cout << seq << ' ' << time << ' ' << name;
    }

    static bool participant_in_range(Number participant) {
        return participant >= MIN_PARTICIPANT && participant <= MAX_PARTICIPANT;
    }

    // How many orders this participant has resting in the book right now,
    // counted by walking both sides of the book.
    Number resting_orders_of(Number participant) const {
        Number count = 0;
        for (const BookSide* side : {&buys_, &sells_}) {
            for (const auto& [price, line] : *side) {
                for (const Number id : line) {
                    if (orders_.at(id).participant == participant) {
                        count += 1;
                    }
                }
            }
        }
        return count;
    }

    // Takes a resting order out of its line and marks it as gone. The other
    // orders at that price keep their relative places.
    void remove_from_book(Number id) {
        Order& order = orders_.at(id);
        BookSide& side = (order.side == 'B') ? buys_ : sells_;
        std::deque<Number>& line = side.at(order.price);
        line.erase(std::find(line.begin(), line.end(), id));
        if (line.empty()) {
            side.erase(order.price);
        }
        order.in_book = false;
    }

    BookSide buys_;                   // resting buy orders
    BookSide sells_;                  // resting sell orders
    std::map<Number, Order> orders_;  // every order ever created, by id
    Number next_trade_ = 1;           // trade numbers run 1, 2, 3, ...
};

int main() {
    Engine engine;
    std::string line;

    while (std::getline(std::cin, line)) {
        // Skip empty lines.
        if (line.find_first_not_of(" \t\r") == std::string::npos) {
            continue;
        }

        std::istringstream fields(line);
        Number seq = 0;
        Number time = 0;
        char type = '\0';
        Number participant = 0;
        fields >> seq >> time >> type >> participant;

        if (type == 'N') {
            char side = '\0';
            Number price = 0;
            Number quantity = 0;
            fields >> side >> price >> quantity;
            engine.new_order(seq, time, participant, side, price, quantity);
        } else if (type == 'C') {
            Number order_id = 0;
            fields >> order_id;
            engine.cancel(seq, time, participant, order_id);
        }

        // This message is now handled completely, events included.
        std::cout.flush();
    }

    return 0;
}
