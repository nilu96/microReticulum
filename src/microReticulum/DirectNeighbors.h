#pragma once

#include "Bytes.h"

#include <map>
#include <stdint.h>

namespace RNS {

    class DirectNeighbors {

    private:
        class NeighborEntry {
		public:
			NeighborEntry(bool is_transport) :
				_is_transport(is_transport),
				_state(0),
				_wait_counter(0) {}

			void set_state(int state) {
				if (state < 0) state = 0;
				else if (state > MAX_STATE) state = MAX_STATE;
				_state = (uint8_t)state;
			}
			void set_wait_counter(int wait_counter) {
				if (wait_counter < 0) wait_counter = 0;
				else if (wait_counter > MAX_WAIT_COUNTER) wait_counter = MAX_WAIT_COUNTER;
				_wait_counter = (uint8_t)wait_counter;
			}
			void set_responsive() {
				set_state(0);
				_wait_counter = 0;
			}
			void set_unresponsive() {
				set_state(MAX_STATE);
				set_wait_counter(MAX_WAIT_COUNTER);
			}
			void set_week_unresponsive() {
				set_state(_state + 1);
				if (_wait_counter < WEEK_UNRESPONSIVE_WAIT_VALUE) set_wait_counter(WEEK_UNRESPONSIVE_WAIT_VALUE);
			}
			bool is_unresponsive() const {
				return _state == MAX_STATE;
			}
			void wait() {
				if (_wait_counter == 0) return;
				_wait_counter--;
				if (_wait_counter > 0) return;
				
				if (is_unresponsive()) set_state(MAX_STATE - MAX_STATE/2); // ??
				else set_state(_state - 1);

				if (_state > 0) set_wait_counter(WEEK_UNRESPONSIVE_WAIT_VALUE);
			}

		private:
			bool _is_transport : 1;
			uint8_t _state : 3;
			uint8_t _wait_counter : 4;

			// 0 = unknown/responsive, > 0 = increasing level of unresponsiveness
			// hard maximum is 7 but can be lower if desired, and can be used to adjust sensitivity of unresponsiveness detection
			static constexpr int MAX_STATE = 7; // TODO: need to figure out a good value for this
			static constexpr int MAX_WAIT_COUNTER = 15;
			static constexpr int WEEK_UNRESPONSIVE_WAIT_VALUE = MAX_WAIT_COUNTER / 2; // TODO: need to figure out a good value for this
	    };
	    static_assert(sizeof(NeighborEntry) == 1);
        using NeighborTable = std::map<Bytes, NeighborEntry>;

    private:
		static NeighborTable _neighbors;  // A table for keeping track of neighbor nodes/destinations and their responsiveness
		static constexpr float _neighbors_wait_interval = 60.0 * 45; // TODO: need to figure out a good value for this
        static double _neighbors_last_wait;

    public:
        static void init();
        static void trigger_wait_for_all_neighbors();
        static void set_neighbor_responsive(const Bytes& neighbor_hash, bool is_transport);
        static void set_neighbor_unresponsive(const Bytes& neighbor_hash, bool is_transport);
        static void set_neighbor_week_unresponsive(const Bytes& neighbor_hash, bool is_transport);
        static bool is_neighbor_unresponsive(const Bytes& neighbor_hash);

        inline static NeighborEntry& add_neighbor(const Bytes& neighbor_hash, bool is_transport) { return _neighbors.try_emplace(neighbor_hash, is_transport).first->second; }
        inline static void remove_neighbor(const Bytes& neighbor_hash) { _neighbors.erase(neighbor_hash); }
    };
}
