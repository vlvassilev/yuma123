/* This file contains a copy of the port-iterator helper
 * functions part of ovs-ofctl tool part of the openvswitch
 * project with minor modifications.
 * They are used in the ietf-network-bridge-flows
 * module implementation for OpenFlow southbound interface based on
 * the openvswitch conntroller API. The original License text follows.
 */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

struct port_iterator {
    struct vconn *vconn;

    enum { PI_FEATURES, PI_PORT_DESC } variant;
    struct ofpbuf *reply;
    ovs_be32 send_xid;
    bool more;
};

static void
port_iterator_fetch_port_desc(struct port_iterator *pi)
{
    int retval;
    pi->variant = PI_PORT_DESC;
    pi->more = true;

    struct ofpbuf *rq = ofputil_encode_port_desc_stats_request(
        vconn_get_version(pi->vconn), OFPP_ANY);
    pi->send_xid = ((struct ofp_header *) rq->data)->xid;
    retval=vconn_send_block(pi->vconn, rq);
    assert(retval==0);
}

static void
port_iterator_fetch_features(struct port_iterator *pi)
{
    int retval;
    pi->variant = PI_FEATURES;

    /* Fetch the switch's ofp_switch_features. */
    enum ofp_version version = vconn_get_version(pi->vconn);
    struct ofpbuf *rq = ofpraw_alloc(OFPRAW_OFPT_FEATURES_REQUEST, version, 0);
    retval=vconn_transact(pi->vconn, rq, &pi->reply);
    assert(retval==0);

    enum ofptype type;
    if (ofptype_decode(&type, pi->reply->data)
        || type != OFPTYPE_FEATURES_REPLY) {
        ovs_fatal(0, "%s: received bad features reply",
                  vconn_get_name(pi->vconn));
    }
    if (!ofputil_switch_features_has_ports(pi->reply)) {
        /* The switch features reply does not contain a complete list of ports.
         * Probably, there are more ports than will fit into a single 64 kB
         * OpenFlow message.  Use OFPST_PORT_DESC to get a complete list of
         * ports. */
        ofpbuf_delete(pi->reply);
        pi->reply = NULL;
        port_iterator_fetch_port_desc(pi);
        return;
    }

    struct ofputil_switch_features features;
    enum ofperr error = ofputil_pull_switch_features(pi->reply, &features);
    if (error) {
        printf("%s: failed to decode features reply (%s)",
                  vconn_get_name(pi->vconn), ofperr_to_string(error));
        assert(0);
    }
}

/* Initializes 'pi' to prepare for iterating through all of the ports on the
 * OpenFlow switch to which 'vconn' is connected.
 *
 * During iteration, the client should not make other use of 'vconn', because
 * that can cause other messages to be interleaved with the replies used by the
 * iterator and thus some ports may be missed or a hang can occur. */
static void
port_iterator_init(struct port_iterator *pi, struct vconn *vconn)
{
    memset(pi, 0, sizeof *pi);
    pi->vconn = vconn;
    if (vconn_get_version(vconn) < OFP13_VERSION) {
        port_iterator_fetch_features(pi);
    } else {
        port_iterator_fetch_port_desc(pi);
    }
}

/* Obtains the next port from 'pi'.  On success, initializes '*pp' with the
 * port's details and returns true, otherwise (if all the ports have already
 * been seen), returns false.  */
static bool
port_iterator_next(struct port_iterator *pi, struct ofputil_phy_port *pp)
{
    int retval;
    for (;;) {
        if (pi->reply) {
            int retval = ofputil_pull_phy_port(vconn_get_version(pi->vconn),
                                               pi->reply, pp);
            if (!retval) {
                return true;
            } else if (retval != EOF) {
#if 0
                ovs_fatal(0, "received bad reply: %s",
                          ofp_to_string(pi->reply->data, pi->reply->size,
                                        verbosity + 1));
#else
                assert(0);
#endif
            }
        }

        if (pi->variant == PI_FEATURES || !pi->more) {
            return false;
        }

        ovs_be32 recv_xid;
        do {
            ofpbuf_delete(pi->reply);
            retval = vconn_recv_block(pi->vconn, &pi->reply);
            assert(retval==0);
            recv_xid = ((struct ofp_header *) pi->reply->data)->xid;
        } while (pi->send_xid != recv_xid);

        struct ofp_header *oh = pi->reply->data;
        enum ofptype type;
        if (ofptype_pull(&type, pi->reply)
            || type != OFPTYPE_PORT_DESC_STATS_REPLY) {
#if 0
            ovs_fatal(0, "received bad reply: %s",
                      ofp_to_string(pi->reply->data, pi->reply->size,
                                    verbosity + 1));
#else
            assert(0);
#endif
        }

        pi->more = (ofpmp_flags(oh) & OFPSF_REPLY_MORE) != 0;
    }
}

/* Destroys iterator 'pi'. */
static void
port_iterator_destroy(struct port_iterator *pi)
{
    if (pi) {
        while (pi->variant == PI_PORT_DESC && pi->more) {
            /* Drain vconn's queue of any other replies for this request. */
            struct ofputil_phy_port pp;
            port_iterator_next(pi, &pp);
        }

        ofpbuf_delete(pi->reply);
    }
}


