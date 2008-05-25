


#ifdef CPP_DEBUG
/********************************************************************
* FUNCTION cap_printf_XML
*
* printf the capability list as an XML sequence 
*
* INPUTS:
*    caplist == capability list to check
* RETURNS:
*    none
*********************************************************************/
void 
cap_printf_XML (cap_list_t *caplist)
{
    status_t  res;

    res = buf_init_buffer(&dbuff, DBUFF_SIZE);
    if (res != NO_ERR) {
	return;
    }
    res = cap_emit_caplist(&dbuff, caplist);
    if (res==NO_ERR) {
	buf_print_buffer(&dbuff);
    }

    buf_clean_buffer(&dbuff);

} /* cap_printf_XML */
#endif   /* CPP_DEBUG */


/********************************************************************
* FUNCTION emit_cap
*
* sprintf the capability
*
* INPUTS:
*    buffer == buffer to hold sprintf strings
*    ns_id  == namespace ID
*    capname == capability name to emit
*    capval == capability value to emit
* RETURNS:
*    NO_ERR if string fits in buffer and no parameter errors
*********************************************************************/
static status_t emit_cap (buf_buffer_t *buffer,
			  xmlns_id_t  ns_id,
			  const xmlChar *capname,
			  const xmlChar *capval)
{
    status_t   res;

    res = buf_write_tag(buffer, ns_id, capname, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }
    buf_inc_indent_level(buffer);
    res = buf_write_indent(buffer, capval);
    if (res != NO_ERR) {
	return res;
    }
    buf_dec_indent_level(buffer);
    return buf_write_tag(buffer, ns_id, capname, BUF_END_TAG, NULL);

} /* emit_cap */


/********************************************************************
* FUNCTION cap_emit_caplist
*
* sprintf the capability list as an XML sequence to the buffer
*
* INPUTS:
*    buffer == buffer to hold sprintf strings
*    caplist == capability list to emit
* RETURNS:
*    NO_ERR if string fits in buffer and no parameter errors
*********************************************************************/
status_t cap_emit_caplist (buf_buffer_t *buffer,
			   cap_list_t *caplist)
{
    status_t     res;
    cap_rec_t   *cap;
    cap_stdrec_t *scap;
    xmlns_id_t  ns_id;
    int          i;

    /* check parameters */
    if (!caplist || !buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    
    ns_id = xmlns_nc_id();

    /* write the <capabilities> start tag */
    res = buf_write_tag(buffer, ns_id, 
         (const xmlChar *) NC_CAPS_STR, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* adjust the indent count */
    buf_inc_indent_level(buffer);

    /* brute force check the V1 capability */
    if (m__bitset(caplist->cap_std, CAP_BIT_V1)) {
	xmlStrPrintf(uribuff, MAX_URI_SIZE+1, 
		     (const xmlChar *) "%s", NC_URN);
	res = emit_cap(buffer, ns_id, (const xmlChar *) NC_CAP_STR, uribuff);
	if (res != NO_ERR) {
	    return res;
	}
    }

    scap = cap_get_stdrec();
    /* check all the standard capabilities */
    for (i=1; i<CAP_STDID_LAST_MARKER; i++) {
	
	if (m__bitset(caplist->cap_std, scap[i].cap_bitnum)) {
	    if (i==CAP_STDID_URL) {
		if (caplist->cap_protos) {
		    xmlStrPrintf(uribuff, MAX_URI_SIZE+1,
                                 (const xmlChar *) "%s:%s?scheme=%s", 
				 (const xmlChar *)NC_CAP_URN, CAP_NAME_URL, 
				 caplist->cap_protos);
		} else {
		    xmlStrPrintf(uribuff, MAX_URI_SIZE+1,
                                 (const xmlChar *) "%s:%s", 
                                 (const xmlChar *) NC_CAP_URN, CAP_NAME_URL);
		}
	    } else {
		xmlStrPrintf(uribuff, MAX_URI_SIZE+1,
                             (const xmlChar *) "%s:%s", NC_CAP_URN, 
                             scap[i].cap_name);
	    }
	    res = emit_cap(buffer, ns_id, (const xmlChar *) NC_CAP_STR, 
			   uribuff);
	    if (res != NO_ERR) {
		return res;
	    }
	}
    }

    /* check the enterprise capability queue */
    for (cap=(cap_rec_t *)dlq_firstEntry(&caplist->cap_q);
	 cap != NULL; cap=(cap_rec_t *)dlq_nextEntry(cap)) {
	res = emit_cap(buffer, ns_id, (const xmlChar *) NC_CAP_STR, 
		       cap->cap_uri);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* adjust the indent count */
    buf_dec_indent_level(buffer);

    /* write the </capabilities> end tag */
    return buf_write_tag(buffer, ns_id, 
       (const xmlChar *) NC_CAPS_STR, BUF_END_TAG, NULL);

} /* cap_emit_caplist */


