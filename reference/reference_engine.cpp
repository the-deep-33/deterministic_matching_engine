#include <iostream>
#include <map>
#include <cstdint>
#include <vector>
#include <string_view>
#include <algorithm>
#include <functional>

constexpr std::int32_t kMaxParticipants         = 1 << 20;
constexpr std::int32_t kMaxOrdersPerParticipant = 200;

enum class Side { Buy, Sell };

enum class BadRequest
{
    TOO_LATE,
    BAD_QTY,
    BAD_PRICE,
    BAD_PARTICIPANT,
    NOT_YOURS,
    TOO_MANY_ORDERS,
};

enum class State { Live, Gone, };

enum class CancelType { User, STP, };

struct Order {
    std::int64_t id;
    std::int64_t participant;
    Side side;
    std::int64_t price;
    std::int64_t quantity;
};

struct Record {
    std::int64_t price;
    Side side;
    State state;
    std::int64_t participant;
};

template<typename First, typename... Rest>
void println(const First& first, const Rest&... rest)
{
    std::cout << first;
    ((std::cout << ' ' << rest), ...);
    std::cout << '\n';
}

constexpr char printSide(Side side)
{
    switch(side)
    {
        case Side::Buy:
            return 'B';
        case Side::Sell:
            return 'S';
    }
    return '?';
}

std::string_view BadReqToText(BadRequest req)
{
    switch(req)
    {
        case BadRequest::BAD_QTY:
            return "BAD_QUANTITY";
        case BadRequest::BAD_PRICE:
            return "BAD_PRICE";
        case BadRequest::BAD_PARTICIPANT:
            return "BAD_PARTICIPANT";
        case BadRequest::NOT_YOURS:
            return "NOT_YOURS";
        case BadRequest::TOO_LATE:
            return "TOO_LATE";
        case BadRequest::TOO_MANY_ORDERS:
            return "TOO_MANY_ORDERS";
    }
    return "?";
}

std::string_view CancelTypeToText(CancelType cancel)
{
    switch(cancel)
    {
        case CancelType::User:
            return "USER";
        case CancelType::STP:
            return "STP";
    }
    return "?";
}

void print_ack(std::int64_t seq, std::int64_t time, std::int64_t participant, std::int64_t order,
               Side side, std::int64_t price, std::int64_t quantity)
{
    println(seq, time, "ACK", participant, order, printSide(side), price, quantity);
}

void print_reject(std::int64_t seq, std::int64_t time, std::int64_t participant, BadRequest req)
{
    println(seq, time, "REJECT", participant, BadReqToText(req));
}

void print_fill(std::int64_t seq, std::int64_t time, std::int64_t participant, std::int64_t order,
                std::int64_t trade, char role, std::int64_t price, std::int64_t quantity, std::int64_t remaining)
{
    println(seq, time, "FILL", participant, order, trade, role, price, quantity, remaining);
}

void print_trade(std::int64_t seq, std::int64_t time, std::int64_t trade, std::int64_t order,
                 std::int64_t price, std::int64_t quantity, Side incoming_side)
{
    println(seq, time, "TRADE", trade, order, price, quantity, printSide(incoming_side));
}

void print_add(std::int64_t seq, std::int64_t time, std::int64_t order, Side side,
               std::int64_t price, std::int64_t quantity)
{
    println(seq, time, "ADD", order, printSide(side), price, quantity);
}

void print_cancelled(std::int64_t seq, std::int64_t time, std::int64_t participant, std::int64_t order,
                     std::int64_t quantity, CancelType cancel)
{
    println(seq, time, "CANCELLED", participant, order, quantity, CancelTypeToText(cancel));
}

void print_remove(std::int64_t seq, std::int64_t time, std::int64_t order, std::int64_t quantity)
{
    println(seq, time, "REMOVE", order, quantity);
}

int main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::map<std::int64_t, std::vector<Order>, std::greater<>> bids;
    std::map<std::int64_t, std::vector<Order>> asks;
    std::map<std::int64_t, Record> history;
    std::int64_t trade_id {1};
    std::int64_t sequence_number;
    std::int64_t timestamp;
    char type;

    while(std::cin >> sequence_number >> timestamp >> type)
    {
        if(type == 'N')
        {
            Order order {};
            char side_char;
            std::cin >> order.participant >> side_char >> order.price >> order.quantity;
            order.side = (side_char == 'B') ? Side::Buy : Side::Sell;

            if(order.quantity < 1 || order.quantity > INT32_MAX)
            {
                print_reject(sequence_number, timestamp, order.participant, BadRequest::BAD_QTY);
                continue;
            }
            if(order.price < 1 || order.price > 100'000'000)
            {
                print_reject(sequence_number, timestamp, order.participant, BadRequest::BAD_PRICE);
                continue;
            }
            if(order.participant < 0 || order.participant >= kMaxParticipants)
            {
                print_reject(sequence_number, timestamp, order.participant, BadRequest::BAD_PARTICIPANT);
                continue;
            }
            std::int64_t participant_count {0};
            for(const auto& [price, orders]: bids)
            {
                for(const auto& temp_order: orders)
                {
                    if(temp_order.participant == order.participant)
                    {
                        ++participant_count;
                    }
                }
            }
            for(const auto& [price, orders]: asks)
            {
                for(const auto& temp_order: orders)
                {
                    if(temp_order.participant == order.participant)
                    {
                        ++participant_count;
                    }
                }
            }
            if(participant_count >= kMaxOrdersPerParticipant)
            {
                print_reject(sequence_number, timestamp, order.participant, BadRequest::TOO_MANY_ORDERS);
                continue;
            }
            order.id = sequence_number;
            print_ack(sequence_number, timestamp, order.participant, order.id, 
                      order.side, order.price, order.quantity);
            history[order.id] = 
                Record 
                {
                    .price          = order.price,
                    .side           = order.side,
                    .state          = State::Live,
                    .participant    = order.participant
                };

            auto crosses = [&](std::int64_t best_price)
            {
                return (order.side == Side::Buy) ? best_price <= order.price : best_price >= order.price;
            };

            auto match = [&](auto& opposite)
            {
                while(!opposite.empty() && crosses(opposite.begin()->first) && order.quantity > 0)
                {
                    std::vector<Order>& orders = opposite.begin()->second;
                    while(!orders.empty() && order.quantity > 0)
                    {
                        Order& first_order = *orders.begin();
                        if(order.participant == first_order.participant)
                        {
                            print_cancelled(sequence_number, timestamp, first_order.participant, 
                                            first_order.id, first_order.quantity, CancelType::STP);
                            print_remove(sequence_number, timestamp, first_order.id, first_order.quantity);
                            history[first_order.id].state = State::Gone;
                            orders.erase(orders.begin());
                            continue;
                        }
                        std::int64_t trade_qty = (order.quantity > first_order.quantity) ? first_order.quantity : order.quantity;
                        print_fill(sequence_number, timestamp, first_order.participant, first_order.id, 
                                   trade_id, 'M', first_order.price, trade_qty, first_order.quantity - trade_qty
                        );
                        print_fill(sequence_number, timestamp, order.participant, order.id, 
                                   trade_id, 'T', first_order.price, trade_qty, order.quantity - trade_qty
                        );
                        print_trade(sequence_number, timestamp, trade_id, first_order.id, 
                                    first_order.price, trade_qty, order.side);

                        order.quantity          -= trade_qty;
                        first_order.quantity    -= trade_qty;

                        if(order.quantity == 0)
                        {
                            history[order.id].state = State::Gone;
                        }
                        if(first_order.quantity == 0)
                        {
                            history[first_order.id].state = State::Gone;
                            orders.erase(orders.begin());
                        }
                        ++trade_id;
                    }
                    if(orders.empty())
                    {
                        opposite.erase(opposite.begin());
                    }
                }
            };

            if(order.side == Side::Buy)
            {
                match(asks);
                if(order.quantity > 0)
                {
                    bids[order.price].push_back(order);
                }
            }
            else
            {
                match(bids);
                if(order.quantity > 0)
                {
                    asks[order.price].push_back(order);
                }
            }
            if(order.quantity > 0)
                print_add(sequence_number, timestamp, order.id, order.side, order.price, order.quantity);

        }
        else if(type == 'C')
        {
            std::int64_t participant {};
            std::int64_t order_id {};
            std::cin >> participant >> order_id;
            if(participant < 0 || participant >= kMaxParticipants)
            {
                print_reject(sequence_number, timestamp, participant, BadRequest::BAD_PARTICIPANT);
                continue;
            }
            auto history_it = history.find(order_id);
            if(history_it == history.end())
            {
                print_reject(sequence_number, timestamp, participant, BadRequest::NOT_YOURS);
                continue;
            }
            Record& current = history_it->second;
            if(current.state == State::Gone)
            {
                print_reject(sequence_number, timestamp, participant, BadRequest::TOO_LATE);
                continue;
            }
            if(current.participant != participant)
            {
                print_reject(sequence_number, timestamp, participant, BadRequest::NOT_YOURS);
                continue;
            }
            std::vector<Order>& orders = (current.side == Side::Buy) ? bids[current.price] : asks[current.price];
            auto order_it = std::find_if(orders.begin(), orders.end(), [order_id](const Order& order){
                return order.id == order_id;
            });
            print_cancelled(sequence_number, timestamp, participant, order_it->id, order_it->quantity, CancelType::User);
            print_remove(sequence_number, timestamp, order_it->id, order_it->quantity);
            orders.erase(order_it);
            current.state = State::Gone;
            if(orders.empty())
            {
                if(current.side == Side::Buy)   bids.erase(current.price);
                else                            asks.erase(current.price);
            }
        }
        else {
            std::cerr << "Malformed line\n";
            return 1;
        }
    }
}