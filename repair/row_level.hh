/*
 * Copyright (C) 2018 ScyllaDB
 */

/*
 * This file is part of Scylla.
 *
 * Scylla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scylla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scylla.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>
#include "gms/inet_address.hh"
#include "repair/repair.hh"
#include <seastar/core/distributed.hh>

class row_level_repair_gossip_helper;

namespace service {
class migration_manager;
}

namespace db {

class system_distributed_keyspace;

}

namespace gms {
    class gossiper;
}

class repair_service : public seastar::peering_sharded_service<repair_service> {
    distributed<gms::gossiper>& _gossiper;
    netw::messaging_service& _messaging;
    sharded<database>& _db;
    sharded<db::system_distributed_keyspace>& _sys_dist_ks;
    sharded<db::view::view_update_generator>& _view_update_generator;
    service::migration_manager& _mm;

    shared_ptr<row_level_repair_gossip_helper> _gossip_helper;
    std::unique_ptr<tracker> _tracker;
    bool _stopped = false;

    future<> init_ms_handlers();
    future<> uninit_ms_handlers();
    future<> init_row_level_ms_handlers();
    future<> uninit_row_level_ms_handlers();

public:
    repair_service(distributed<gms::gossiper>& gossiper,
            netw::messaging_service& ms,
            sharded<database>& db,
            sharded<db::system_distributed_keyspace>& sys_dist_ks,
            sharded<db::view::view_update_generator>& vug,
            service::migration_manager& mm, size_t max_repair_memory);
    ~repair_service();
    future<> start();
    future<> stop();

public:
    netw::messaging_service& get_messaging() noexcept { return _messaging; }
    sharded<database>& get_db() noexcept { return _db; }
    service::migration_manager& get_migration_manager() noexcept { return _mm; }
    sharded<db::system_distributed_keyspace>& get_sys_dist_ks() noexcept { return _sys_dist_ks; }
    sharded<db::view::view_update_generator>& get_view_update_generator() noexcept { return _view_update_generator; }
};

future<> row_level_repair_init_messaging_service_handler(distributed<db::system_distributed_keyspace>& sys_dist_ks,
        distributed<db::view::view_update_generator>& view_update_generator, sharded<netw::messaging_service>& ms, sharded<service::migration_manager>& mm);
future<> row_level_repair_uninit_messaging_service_handler();

class repair_info;

future<> repair_cf_range_row_level(repair_info& ri,
        sstring cf_name, utils::UUID table_id, dht::token_range range,
        const std::vector<gms::inet_address>& all_peer_nodes);

future<> shutdown_all_row_level_repair();
