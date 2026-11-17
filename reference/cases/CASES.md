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
| `entry_big_numbers` | Large sequence number, time and participant are copied as read (64-bit) |
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
