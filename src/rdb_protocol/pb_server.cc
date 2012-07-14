#include "rdb_protocol/pb_server.hpp"

query_server_t::query_server_t(int port, const boost::shared_ptr<semilattice_read_view_t<cluster_semilattice_metadata_t> > &_semilattice_metadata, namespace_repo_t<rdb_protocol_t> * _ns_repo)
    : server(port, boost::bind(&query_server_t::handle, this, _1), INLINE),
      semilattice_metadata(_semilattice_metadata), ns_repo(_ns_repo)
{ }

Response query_server_t::handle(const Query &q) {
    Response res;
    res.set_token(q.token());

    query_language::variable_type_scope_t scope;

    try {
        guarantee(get_type(q, &scope) == query_language::Type::QUERY);
    } catch (query_language::runtime_exc_t &e) {
        res.set_status_code(-3);
        res.add_response("Runtime Exception: " + e.what());
        return res;
    } catch (query_language::type_error_t &e) {
        res.set_status_code(-2);
        res.add_response("Message failed to type check: " + e.what());
        return res;
    }

    query_language::runtime_environment_t runtime_environment(ns_repo, semilattice_metadata);
    try {
        return eval(q, &runtime_environment);
    } catch (query_language::runtime_exc_t &e) {
        res.set_status_code(-4);
        res.add_response("Runtime Exception: " + e.what());
        return res;
    }
}
