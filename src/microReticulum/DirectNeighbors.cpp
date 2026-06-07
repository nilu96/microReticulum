#include "DirectNeighbors.h"

#include "Utilities/OS.h"

using namespace RNS;
using namespace RNS::Utilities;

/*static*/ DirectNeighbors::NeighborTable DirectNeighbors::_neighbors;
/*static*/ double DirectNeighbors::_neighbors_last_wait = 0.0;

/*static*/ void DirectNeighbors::init() {
    _neighbors_last_wait = OS::time();
}

/*static*/ void DirectNeighbors::trigger_wait_for_all_neighbors() {
    double now = OS::time();
    if (now - _neighbors_last_wait < _neighbors_wait_interval) return;
    _neighbors_last_wait = now;

    for (auto& neighbor : _neighbors) {
        neighbor.second.wait();
    }
}

/*static*/ void DirectNeighbors::set_neighbor_responsive(const Bytes& neighbor_hash, bool is_transport) {
    add_neighbor(neighbor_hash, is_transport).set_responsive();
}

/*static*/ void DirectNeighbors::set_neighbor_unresponsive(const Bytes& neighbor_hash, bool is_transport) {
    add_neighbor(neighbor_hash, is_transport).set_unresponsive();
}

/*static*/ void DirectNeighbors::set_neighbor_week_unresponsive(const Bytes& neighbor_hash, bool is_transport) {
    add_neighbor(neighbor_hash, is_transport).set_week_unresponsive();
}

/*static*/ bool DirectNeighbors::is_neighbor_unresponsive(const Bytes& neighbor_hash) {
    auto it = _neighbors.find(neighbor_hash);
    if (it == _neighbors.end()) return false;
    return it->second.is_unresponsive();
}
