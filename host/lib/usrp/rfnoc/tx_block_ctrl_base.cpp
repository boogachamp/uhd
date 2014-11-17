//
// Copyright 2014 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <uhd/usrp/rfnoc/tx_block_ctrl_base.hpp>
#include <uhd/utils/msg.hpp>

using namespace uhd;
using namespace uhd::rfnoc;

void tx_block_ctrl_base::setup_tx_streamer(uhd::stream_args_t &args)
{
    UHD_MSG(status) << "tx_block_ctrl_base::setup_tx_streamer() on " << get_block_id() << std::endl;

    // 0. Check if args collides with our own options
    BOOST_FOREACH(const std::string key, _args.keys()) {
        if (args.args.has_key(key) and _args[key] != args.args[key]) {
            throw uhd::runtime_error(
                str(boost::format(
                        "Conflicting options for block %s: Block options require '%s' == '%s',\n"
                        "but streamer requests '%s' == '%s'."
                        ) % get_block_id().get() % key % _args[key] % key % args.args[key]
                )
            );
        }
        args.args[key] = _args[key];
    }

    // 1. Call our own init_tx() function
    // This should modify "args" if necessary.
    _init_tx(args);

    // 2. Check if we're the last block
    if (_is_final_tx_block()) {
        UHD_MSG(status) << "tx_block_ctrl_base::setup_tx_streamer(): Final block, returning. " << std::endl;
        return;
    }

    // 3. Call all downstream blocks
    BOOST_FOREACH(const node_ctrl_base::node_map_pair_t downstream_node, _downstream_nodes) {
        // Make a copy so that modifications downstream aren't propagated upstream
        uhd::stream_args_t new_args = args;
        sptr this_downstream_block_ctrl =
            boost::dynamic_pointer_cast<tx_block_ctrl_base>(downstream_node.second.lock());
        if (this_downstream_block_ctrl) {
            this_downstream_block_ctrl->setup_tx_streamer(new_args);
        }
    }
}
// vim: sw=4 et:
