# Reference engine scenario cases

Each `<name>.in` is an input and `<name>.out` the exact output the rules doc requires. Expected outputs were worked out by hand from the rules doc, not taken from an engine.

| Case | What it checks |
|---|---|
| `entry_qty_zero` | Quantity 0 is rejected |
| `entry_qty_negative` | Negative quantity is rejected |
| `entry_qty_min` | Quantity 1, the smallest valid, is accepted |
| `entry_qty_max` | Quantity 2,147,483,647, the largest valid, is accepted |
| `entry_qty_over_max` | Quantity 2,147,483,648, one past the limit, is rejected |
| `entry_price_zero` | Price 0 is rejected |
| `entry_price_negative` | Negative price is rejected |
| `entry_price_min` | Price 1, the smallest valid, is accepted |
| `entry_price_max` | Price 100,000,000, the largest valid, is accepted |
| `entry_price_over_max` | Price 100,000,001, one past the limit, is rejected |
| `entry_both_bad` | Bad quantity and bad price: quantity is checked first |
| `entry_reject_leaves_book_untouched` | A rejected sell never rests, so a later buy at its price rests too |
| `entry_participant_zero` | Participant 0 is valid |
| `entry_participant_max` | Participant 1,048,575, the largest valid, is accepted and trades |
| `entry_participant_negative` | Participant -1 is rejected; the order never rests |
| `entry_participant_over_max` | Participant 1,048,576, one past the limit, is rejected; the order never rests |
| `entry_price_and_participant_bad` | Bad price and bad participant: price is checked first |
| `example_7` | Worked example 7 from the rules doc, unchanged |
| `entry_big_numbers` | Large sequence number and time are copied as read (64-bit); the order id is the 64-bit sequence number; participant is the largest valid, 1,048,575 |
| `entry_negative_time` | Time is copied, never interpreted, even when negative |
| `entry_blank_lines` | Empty lines in the input are skipped |
| `cross_buy_below_ask` | Buy one tick below the best ask does not cross; both rest |
| `cross_sell_above_bid` | Sell one tick above the best bid does not cross; both rest |
| `cross_buy_equal` | Buy at exactly the best ask crosses |
| `cross_sell_equal` | Sell at exactly the best bid crosses |
| `cross_sell_trades_at_bid_price` | A sell limited at 100 hitting a bid at 105 trades at 105, the resting price |
| `cross_same_side_never_matches` | Two buys never trade with each other, whatever their prices |
| `match_sell_smaller_than_resting` | Incoming sell smaller than the resting bid: bid keeps the rest |
| `match_buy_larger_rests_leftover` | Incoming buy larger than the resting ask: leftover rests at the buy's price |
| `match_sell_larger_rests_at_own_limit` | Leftover of a sell rests at its own limit (99), not the traded price (100) |
| `match_time_priority_bids` | Two bids at one price: the earlier fills first (incoming sell) |
| `match_partial_keeps_place` | A partly filled resting order stays first in line |
| `match_buy_sweep_stops_at_limit` | Buy at 102 sweeps 101 and 102, stops before 103, rests the rest at 102 |
| `match_sell_sweep_stops_at_limit` | Sell at 102 sweeps 103 and 102, stops before 101, rests the rest at 102 |
| `match_sweep_partial_last_level` | Sweep ends partway into the second level; nothing rests |
| `match_emptied_level_is_gone` | After the ask at 101 fills, a new buy at 101 finds 102 and rests |
| `match_trade_numbers_continue` | Trade numbers run across messages; rejects in between use none |
| `match_price_priority_asks` | Asks arrive at 103, 101, 102: a buy fills them lowest price first |
| `match_price_priority_bids` | Bids arrive at 98, 100, 99: a sell fills them highest price first |
| `stp_sell_own_first_in_line` | Sell side: own bid first in line is cancelled, then the sell trades with the next bid |
| `stp_buy_own_behind_other` | Own ask behind someone else's: trade the other first, then cancel own, rest the rest |
| `stp_sell_own_behind_other` | Mirror of the previous case on the sell side |
| `stp_two_own_in_a_row` | Two own asks in a row are both cancelled; trade number 1 still goes to the first real trade |
| `stp_empties_level_then_next_price` | Self-trade cancel empties 101; matching continues at 102 |
| `stp_then_rest_unchanged` | Only own order opposite: it is cancelled in full (5, larger than the incoming 3) and the incoming rests unchanged |
| `stp_not_triggered_without_cross` | Own orders on both sides that do not cross are left alone |
| `cancel_own_bid` | Owner cancels a resting bid; a sell at its price then rests |
| `cancel_own_ask` | Owner cancels a resting ask; a buy at its price then rests |
| `cancel_someone_elses` | Non-owner cancel is refused and the order still trades afterwards |
| `cancel_unused_ids` | Ids never used (future, zero, negative) get NOT_YOURS |
| `cancel_id_of_cancel_line` | The sequence number of a cancel line was never an order |
| `cancel_id_of_rejected_line` | The sequence number of a rejected new order was never an order |
| `cancel_filled_orders` | Both sides of a full fill: resting (1) and incoming that filled on arrival (2) are TOO_LATE |
| `cancel_gone_by_non_owner_cancelled` | A cancelled order cancelled again by someone else: TOO_LATE, not NOT_YOURS |
| `cancel_gone_by_non_owner_filled` | A filled order cancelled by a third participant: TOO_LATE |
| `cancel_after_stp` | An order removed by self-trade prevention is TOO_LATE to cancel |
| `cancel_middle_of_level` | Cancel the middle of three asks; the next buy fills the first and third |
| `cancel_last_at_level` | Cancel the only ask at 100; a buy at 101 then trades at 101 |
| `cancel_partially_filled` | Cancel reports the remaining quantity, not the original |
| `cancel_rested_leftover` | An order that traded on arrival and rested its leftover cancels that leftover |
| `cancel_twice` | The second cancel of the same order is TOO_LATE |
| `cancel_level_rebuilt_keeps_time_order` | After the level is emptied by a cancel, new orders at that price queue in arrival order |
| `cancel_bad_participant_never_created` | Out-of-range sender cancelling an id never created: BAD_PARTICIPANT, not NOT_YOURS; a cancel of a BAD_PARTICIPANT reject's sequence number is NOT_YOURS |
| `cancel_bad_participant_live_order` | Out-of-range sender cancelling a live order: BAD_PARTICIPANT, and the order still trades afterwards |
| `cancel_bad_participant_gone_order` | Out-of-range sender cancelling a cancelled order: BAD_PARTICIPANT, not TOO_LATE |

## Open-order limit cases

Generated by `reference/gen_cap_cases.py`, since each needs over 200 lines. The script writes the expected output by construction from the rules doc; it does not run an engine. Regenerate with `python3 reference/gen_cap_cases.py reference/cases`.

| Case | What it checks |
|---|---|
| `cap_201st_rejected` | 200 resting, the 201st is TOO_MANY_ORDERS; cancelling its sequence number is NOT_YOURS |
| `cap_is_per_participant` | Participant 1 at the limit does not stop participant 2 |
| `cap_counts_both_sides` | 100 bids plus 100 asks is the limit: a new buy and a new sell are both rejected |
| `cap_trading_order_still_rejected` | At the limit, an order that would trade is rejected too, and the bid it would have hit is untouched |
| `cap_freed_by_cancel` | A user cancel frees one place: next accepted, the one after rejected |
| `cap_freed_by_fill` | A resting order filled in full frees one place |
| `cap_partial_fill_still_counts` | A partly filled resting order still counts |
| `cap_freed_by_stp` | At 199, an own sell is accepted, STP removes an own bid, the sell rests: still 199, so one more is accepted |
| `cap_full_fill_on_arrival_not_counted` | At 199, an order that fills in full on arrival never rests and is not counted |
| `cap_rested_leftover_counts` | At 199, an order that trades and rests its leftover takes the 200th place |
