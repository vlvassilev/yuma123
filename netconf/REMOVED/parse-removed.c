

/********************************************************************
* FUNCTION consume_revision_history
* 
* Check if a revision-history clause is present
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod    == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_revision_history (tk_chain_t *tkc,
			      ncx_module_t  *mod)
{
    status_t       res;
    boolean        done;

    /* check if optional revision-history keyword is present */
    res = ncx_consume_tstring(tkc, mod, NCX_EL_REVISION_HISTORY, TRUE);
    if (res!=NO_ERR) {
	/* may be a real error or just skipped */
        return res;
    }

    /* got the token, so the left brace must be present */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
	return res;
    }

    done = FALSE;
    while (!done) {
	res = consume_revhist_entry(tkc, mod);
	if (res != NO_ERR) {
	    done = TRUE;
	}
    }

    /* get the closing right brace */
    if (res==NO_ERR || res==ERR_NCX_SKIPPED) {
	res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    }

    return res;

}  /* consume_revision_history */




/********************************************************************
* FUNCTION consume_header
* 
* Parse the next N tokens as an NCX module header
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod    == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_header (tk_chain_t *tkc,
                    ncx_module_t  *mod)
{
    const xmlChar *val;
    tk_type_t      tktyp;
    uint32         flags;
    status_t       res, retres;
    boolean        done;

    /* Process the 'header' token string */
    res = ncx_consume_tstring(tkc, mod, NCX_EL_HEADER, NCX_REQ);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* Process the opening left brace token */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    flags = 0;
    done = FALSE;
    retres = NO_ERR;

    /* loop through all the sub-clauses present */
    while (!done) {
	/* look ahead to the next token */
	tktyp = tk_next_typ(tkc);
	val = tk_next_val(tkc);

	/* check the next token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* sub-clause assumed */
	case TK_TT_RBRACE:
	    retres = TK_ADV(tkc);
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    done = TRUE;
	    continue;
 	}
	    
	/* Got a token string so check the value */
        if (!xml_strcmp(val, NCX_EL_DESCRIPTION)) {
	    if (flags & B_HDR_DESCRIPTION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'description' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DESCRIPTION, 
					 ncx_save_descr() ? 
					 &mod->hdr.descr : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_DESCRIPTION;
	    }
	} else if (!xml_strcmp(val, NCX_EL_VERSION)) {
	    if (flags & B_HDR_VERSION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the mandatory 'version' field */
	    res = consume_version(tkc, mod, &mod->hdr.version);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_VERSION;
	    }
	} else if (!xml_strcmp(val, NCX_EL_OWNER)) {
	    if (flags & B_HDR_OWNER) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the mandatory 'owner' name */
	    res = ncx_consume_name(tkc, mod, NCX_EL_OWNER, &mod->hdr.owner, 
				   NCX_REQ, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_OWNER;
	    }
	} else if (!xml_strcmp(val, NCX_EL_APPLICATION)) {
	    if (flags & B_HDR_APPLICATION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the mandatory 'application' name */
	    res = ncx_consume_name(tkc, mod, NCX_EL_APPLICATION, 
				   &mod->hdr.app, NCX_REQ, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_APPLICATION;
	    }
	} else if (!xml_strcmp(val, NCX_EL_COPYRIGHT)) {
	    if (flags & B_HDR_COPYRIGHT) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'copyright' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_COPYRIGHT, 
					 ncx_save_descr() ? 
					 &mod->hdr.copyright : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
		continue;
	    } else {
		flags |= B_HDR_COPYRIGHT;
	    }
	} else if (!xml_strcmp(val, NCX_EL_CONTACT_INFO)) {
	    if (flags & B_HDR_CONTACT_INFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'contact-info' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_CONTACT_INFO, 
					 ncx_save_descr() ? 
					 &mod->hdr.contact_info : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_CONTACT_INFO;
	    }

	} else if (!xml_strcmp(val, NCX_EL_NAMESPACE)) {
	    if (flags & B_HDR_NAMESPACE) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'namespace' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_NAMESPACE, 
					 &mod->hdr.ns, NCX_OPT, 
					 NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_NAMESPACE;
	    }
	} else if (!xml_strcmp(val, NCX_EL_LAST_UPDATE)) {
	    if (flags & B_HDR_LAST_UPDATE) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'last-update' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_LAST_UPDATE, 
					 &mod->hdr.last_update, NCX_OPT, 
					 NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_LAST_UPDATE;
	    }
	} else if (!xml_strcmp(val, NCX_EL_REVISION_HISTORY)) {
	    if (flags & B_HDR_REV_HISTORY) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'revision-history' clause is present */
	    res = consume_revision_history(tkc, mod);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_REV_HISTORY;
	    }
	} else if (!xml_strcmp(val, NCX_EL_APPINFO)) {
	    if (flags & B_HDR_APPINFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'appinfo' clause is present */
	    res = ncx_consume_appinfo(tkc, mod, &mod->hdr.appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_APPINFO;
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    done = TRUE;
	}
    }

    /* check if the header was parsed okay */
    if (retres == ERR_NCX_ENTRY_EXISTS) {
	res = TK_ADV(tkc);
    } else if (retres == NO_ERR) {
	if ((flags & M_HDR_MANDATORY) != M_HDR_MANDATORY) {
	    retres = ERR_NCX_DATA_MISSING;
	}
    }

    if (retres != NO_ERR) {
	ncx_print_errormsg(tkc, mod, retres);
    }

    return retres;

}  /* consume_header */


/********************************************************************
* FUNCTION consume_type
* 
* Parse an NCX module 'type' section
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod  == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_type (tk_chain_t *tkc,
                  ncx_module_t   *mod)
{
    typ_template_t  *typ;
    xmlChar         *str;
    const xmlChar   *val;
    tk_type_t        tktyp;
    uint32           flags;
    status_t         res, retres;
    boolean          done, doerr;
    ncx_access_t     maxacc;
    ncx_data_class_t dataclass;
    dlq_hdr_t        appinfoQ;

    /* peek ahead to see if the section is done */
    if (!(tk_next_typ(tkc)==TK_TT_TSTRING && tk_next_val(tkc) &&
          !xml_strcmp(NCX_EL_TYPE, tk_next_val(tkc)))) {
	return ERR_PARS_SECDONE;  
    }

    /* malloc a new type struct */
    typ = typ_new_type();
    if (!typ) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* Already checked for a 'type' start tag, so this should work */
    res = ncx_consume_name(tkc, mod, NCX_EL_TYPE, &typ->name, 
			   NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        typ_free_type(typ);
        return res;
    }

    flags = 0;
    done = FALSE;
    doerr = FALSE;
    retres = NO_ERR;
    dlq_createSQue(&appinfoQ);

    /* loop through all the sub-clauses present */
    while (!done) {
	/* look ahead to the next token */
	tktyp = tk_next_typ(tkc);
	val = tk_next_val(tkc);

	/* check the next token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    done = TRUE;
	    doerr = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* sub-clause assumed */
	case TK_TT_RBRACE:
	    (void)TK_ADV(tkc);
	    done = TRUE;
	    continue;
	default:
	    (void)TK_ADV(tkc);
	    retres = ERR_NCX_WRONG_TKTYPE;
	    done = TRUE;
	    doerr = TRUE;
	    continue;
 	}

	/* Got a token string so check the value */
        if (!xml_strcmp(val, NCX_EL_DESCRIPTION)) {
	    if (flags & B_TYP_DESCRIPTION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'description' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DESCRIPTION, 
					 ncx_save_descr() ? 
					 &typ->descr : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_DESCRIPTION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_CONDITION)) {
	    if (flags & B_TYP_CONDITION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'condition' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_CONDITION, 
					 ncx_save_descr() ? 
					 &typ->condition : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_CONDITION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_SYNTAX)) {
	    if (flags & B_TYP_SYNTAX) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* get start of syntax clause */
	    res = ncx_consume_tstring(tkc, mod, NCX_EL_SYNTAX, NCX_REQ);
	    if (res==NO_ERR) {
		res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
		if (res == NO_ERR) {
		    /* convert the syntax contents to internal format */
		    res = typ_parse_syntax_contents(mod, tkc, typ);
		    if (res == NO_ERR) {
			/* get the 'syntax' end tag */
			res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
		    }
		}
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_SYNTAX;
	    }
        } else if (!xml_strcmp(val, NCX_EL_METADATA)) {
	    if (flags & B_TYP_METADATA) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* check for an optional metadata clause */
	    res = ncx_consume_tstring(tkc, mod, NCX_EL_METADATA, NCX_OPT);
	    if (res == NO_ERR) {
		res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
		if (res == NO_ERR) {
		    /* convert the metadata contents to internal format */
		    res = typ_parse_metadata_contents(mod, tkc, typ);
		    if (res == NO_ERR) {
			/* get the 'metadata' end tag */
			res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
		    }
		}
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_METADATA;
	    }
        } else if (!xml_strcmp(val, NCX_EL_DEFAULT)) {
	    if (flags & B_TYP_DEFAULT) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Check if the optional 'default' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DEFAULT, 
					 &typ->defval, NCX_OPT, 
					 NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_DEFAULT;
	    }
        } else if (!xml_strcmp(val, NCX_EL_MAX_ACCESS)) {
	    if (flags & B_TYP_MAX_ACCESS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Check for the optional 'max-access' field
	     * If missing, then leave as 'not set', and the
	     * parent node will be used to determine the access
	     *
	     * This must be after the syntax clause is parsed and the 
	     * typ.typdef struct has been filled in, 
	     * so saved in 'maxacc' for now
	     */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_MAX_ACCESS, &str, 
					 NCX_OPT, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		maxacc = ncx_get_access_enum(str);
		m__free(str);
		if (maxacc == NCX_ACCESS_NONE) {
		    res = ERR_NCX_WRONG_VAL;
		    doerr = TRUE;
		}
	    }

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_MAX_ACCESS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_DATA_CLASS)) {
	    if (flags & B_TYP_DATA_CLASS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Get the optional 'data-class' field */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DATA_CLASS,
					 &str, NCX_OPT, NCX_NO_WSP, 
					 TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		dataclass = ncx_get_data_class_enum(str);
		m__free(str);
		if (dataclass==NCX_DC_NONE) {
		    res = ERR_NCX_WRONG_VAL;
		    doerr = TRUE;
		}
	    }

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_DATA_CLASS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_UNITS)) {
	    if (flags & B_TYP_UNITS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* check if the optional 'units' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_UNITS, 
					 &typ->units, NCX_OPT, NCX_WSP, 
					 TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_UNITS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_APPINFO)) {
	    if (flags & B_TYP_APPINFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* check if the optional 'appinfo' clause is present 
	     * This must be after the syntax clause is parsed and the 
	     * typ.typdef struct has been filled in, so it is saved in
	     * the temp 'appinfoQ'
	     */
	    res = ncx_consume_appinfo(tkc, mod, &appinfoQ);

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_APPINFO;
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    doerr = TRUE;
	    done = TRUE;
	}
    }

    /* check if the type was parsed okay */
    if (retres == ERR_NCX_ENTRY_EXISTS) {
	doerr = TRUE;
	(void)TK_ADV(tkc);
    } else if (retres == NO_ERR) {
	if ((flags & M_TYP_MANDATORY) != M_TYP_MANDATORY) {
	    doerr = TRUE;
	    retres = ERR_NCX_DATA_MISSING;
	}
    }

    /* check for a duplicate entry error in this module */
    if (retres == NO_ERR && ncx_is_duplicate(mod, typ->name)) {
	doerr = TRUE;
	retres = ERR_NCX_DUP_ENTRY;
    }

    /* finish up the clauses that were cached locally */
    if (retres == NO_ERR) {
	/* check max-access set or not */
	if (flags & B_TYP_MAX_ACCESS) {
	    typ->typdef.maxaccess = maxacc;
	} else {
	    typ->typdef.maxaccess = NCX_ACCESS_NONE;
	}

	/* check data-class set or not */
	if (flags & B_TYP_DATA_CLASS) {
	    typ->typdef.dataclass = dataclass;
	} else {
	    if (typ->typdef.maxaccess == NCX_ACCESS_RO) {
		typ->typdef.dataclass = NCX_DC_STATE;
	    } else {
		typ->typdef.dataclass = NCX_DC_NONE;  /* not set */
	    }
	}

	/* check appinfoQ set or not */
	if (flags & B_TYP_APPINFO) {
	    dlq_block_enque(&appinfoQ, &typ->typdef.appinfoQ);
	}
    } else {
	if (doerr) {
	    ncx_print_errormsg(tkc, mod, retres);
	}
	if (!dlq_empty(&appinfoQ)) {
	    ncx_clean_appinfoQ(&appinfoQ);
	}
	typ_free_type(typ);
	return retres;
    }

#ifdef NCX_PARSE_DEBUG
    log_debug3("\nncx_parse: adding type (%s) to mod (%s)", 
	    typ->name, mod->hdr.modname);
#endif

    /* add the type to the module type Que */
    dlq_enque(typ, &mod->typeQ);
    return NO_ERR;

}  /* consume_type */


/********************************************************************
* FUNCTION consume_rpc_contents
* 
* Parse contents of one NCX module <rpc> section
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   rpc == rpc_template_t to fill in
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_rpc_contents (ncx_module_t *mod,
			  tk_chain_t *tkc,
                          rpc_template_t   *rpc)
{
    const xmlChar *val;
    xmlChar       *str;
    tk_type_t      tktyp;
    uint32         flags;
    status_t       res, retres;
    boolean        done;

    flags = 0;
    done = FALSE;
    retres = NO_ERR;

    while (!done) {
	/* look ahead to the next token */
	tktyp = tk_next_typ(tkc);
	val = tk_next_val(tkc);

	/* check the next token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* sub-clause assumed */
	case TK_TT_RBRACE:
	    retres = TK_ADV(tkc);
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    done = TRUE;
	    continue;
	}
	    
	/* Got a token string so check the value */
        if (!xml_strcmp(val, NCX_EL_DESCRIPTION)) {
	    if (flags & B_RPC_DESCRIPTION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'description' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DESCRIPTION, 
					 ncx_save_descr() ? 
					 &rpc->descr : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_DESCRIPTION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_CONDITION)) {
	    if (flags & B_RPC_CONDITION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'condition' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_CONDITION, 
					 ncx_save_descr() ? 
					 &rpc->condition : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_CONDITION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_RPC_TYPE)) {
	    if (flags & B_RPC_TYPE) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the mandatory <rpc-type> field */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_RPC_TYPE, &str,
					 NCX_REQ, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		if (!xml_strcmp(NCX_EL_OTHER, str)) {
		    rpc->rpc_typ = RPC_TYP_OTHER;
		} else if (!xml_strcmp(NCX_EL_CONFIG, str)) {
		    rpc->rpc_typ = RPC_TYP_CONFIG;
		} else if (!xml_strcmp(NCX_EL_EXEC, str)) {
		    rpc->rpc_typ = RPC_TYP_EXEC;
		} else if (!xml_strcmp(NCX_EL_MONITOR, str)) {
		    rpc->rpc_typ = RPC_TYP_MONITOR;
		} else if (!xml_strcmp(NCX_EL_DEBUG, str)) {
		    rpc->rpc_typ = RPC_TYP_DEBUG;
		} else {
		    res = ERR_NCX_WRONG_VAL;
		}
		m__free(str);
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_TYPE;
	    }
        } else if (!xml_strcmp(val, NCX_EL_INPUT)) {
	    if (flags & B_RPC_INPUT) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the optional 'input' field */
	    res = ncx_consume_mname(tkc, mod, NCX_EL_INPUT, &rpc->in_psd_name,
				    &rpc->in_modstr, NCX_OPT, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		/* check the input PSD name */
		res = psd_locate_template(mod, rpc->in_modstr, 
					  rpc->in_psd_name, &rpc->in_psd);
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_INPUT;
	    }
        } else if (!xml_strcmp(val, NCX_EL_PARMS)) {
	    if (flags & B_RPC_PARMS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the other form of the input PSD, direct parms clause */
	    res = consume_int_psd(tkc, mod, rpc);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_PARMS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_OUTPUT)) {
	    if (flags & B_RPC_OUTPUT) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the optional 'output' field */
	    res = ncx_consume_mname(tkc, mod, NCX_EL_OUTPUT, 
				    &rpc->out_data_name,
				    &rpc->out_modstr, 
				    NCX_OPT, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		/* check the output data name as a type first */
		res = typ_locate_template(mod, rpc->out_modstr, 
					  rpc->out_data_name, 
					  (typ_template_t **)&rpc->out_data);
		if (res == NO_ERR) {
		    rpc->out_datatyp = RPC_OT_TYPE;
		} else {
		    /* check the output data name as a PSD 2nd */
		    res = psd_locate_template(mod, rpc->out_modstr, 
				      rpc->out_data_name, 
				      (psd_template_t **)&rpc->out_data);
		    if (res == NO_ERR) {
			rpc->out_datatyp = RPC_OT_PSD;
		    }
		}
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_OUTPUT;
	    }
        } else if (!xml_strcmp(val, NCX_EL_APPINFO)) {
	    if (flags & B_RPC_APPINFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* check if the optional appinfo clause is present */
	    res = ncx_consume_appinfo(tkc, mod, &rpc->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_APPINFO;
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    done = TRUE;
	}
    }

    /* check if the rpc construct was parsed okay */
    if (retres == ERR_NCX_ENTRY_EXISTS) {
	res = TK_ADV(tkc);
    } else if (retres == NO_ERR) {
	if ((flags & M_RPC_MANDATORY) != M_RPC_MANDATORY) {
	    retres = ERR_NCX_DATA_MISSING;
	} else if ((flags & B_RPC_INPUT) && (flags & B_RPC_PARMS)) {
	    retres = ERR_NCX_EXTRA_CHOICE;
	}
    }

    if (retres == NO_ERR) {
	/* check if output skipped */
	if (!(flags & B_RPC_OUTPUT)) {
	    /* set to the RpcOkReplyType default */
	    rpc->out_modstr = xml_strdup(NC_MODULE);
	    rpc->out_data_name = xml_strdup(NC_OK_REPLY);
	    rpc->out_datatyp = RPC_OT_TYPE;
	}
    } else {
	ncx_print_errormsg(tkc, mod, retres);
    }

    return retres;

} /* consume_rpc_contents */


/********************************************************************
* FUNCTION consume_rpc
* 
* Parse one NCX module <rpc> section
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_rpc (tk_chain_t *tkc,
		 ncx_module_t   *mod)
{
    rpc_template_t  *rpc;
    status_t       res;

    /* peek ahead to see if the section is done */
    if (!(tk_next_typ(tkc)==TK_TT_TSTRING && tk_next_val(tkc) &&
          !xml_strcmp(NCX_EL_RPC, tk_next_val(tkc)))) {
	return ERR_PARS_SECDONE;  
    }

    /* malloc a new RPC struct */
    rpc = rpc_new_template();
    if (!rpc) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);	
	return res;
    }

    /* Already checked for an <rpc> start node, so this should work */
    res = ncx_consume_name(tkc, mod, NCX_EL_RPC, &rpc->name, 
			   NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
        rpc_free_template(rpc);
        return res;
    }

    /* get the rest of the rpc, including the end tag */
    res = consume_rpc_contents(mod, tkc, rpc);
    if (res != NO_ERR) {
	rpc_free_template(rpc);
        return res;
    }

    /* make sure this is not a duplicate */
    if (ncx_is_duplicate(mod, rpc->name)) {
	res = ERR_NCX_DUP_ENTRY;
	ncx_print_errormsg(tkc, mod, res);	
	rpc_free_template(rpc);
	return res;
    }

#ifdef NCX_PARSE_DEBUG
    log_debug3("\nncx_parse: adding RPC (%s) to mod.owner (%s.%s)", 
	      rpc->name, mod->hdr.modname, mod->hdr.owner);
#endif

    /* add the rpc to the module RPC Que */
    dlq_enque(rpc, &mod->rpcQ);
    return NO_ERR;

}  /* consume_rpc */


/********************************************************************
* FUNCTION consume_notif_data
* 
* Parse contents of one NCX module 'notif-data' section
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   not == not_template_t to fill in
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_notif_data (ncx_module_t *mod,
			tk_chain_t *tkc,
			not_template_t   *not)
{
    status_t      res;
    boolean       done;
    typ_notdata_t  *nd;

    /* check for empty list -- which is okay */
    if (tk_next_typ(tkc) == TK_TT_RBRACE) {
        return NO_ERR;
    }

    done = FALSE;
    while (!done) {
        nd = typ_new_notdata();
        if (!nd) {
	    res = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        }

        /* move to an identifier token of some kind */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
            typ_free_notdata(nd);
            return res;
        }

        if (TK_CUR_ID(tkc) || TK_CUR_SID(tkc)) {
            /* module-qualified id or simple id or
             * module-qualified scoped id or scoped id 
             */
            nd->nametyp = TK_CUR_TYP(tkc);
            nd->fullname = xml_strdup(TK_CUR_VAL(tkc));

            /*** TEMP *** NEED TO FIND THE IDENTIFIER
             *** TEMP *** AND FILL IN THE TYPDEF
             *** TEMP ***/

            if (!nd->fullname) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
                typ_free_notdata(nd);
                return res;
            }

            /* save the notif data struct */
            dlq_enque(nd, &not->not_dataQ);
        } else {
            /* wrong token type */
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
            typ_free_notdata(nd);
            return res;
        }

        /* setup the next loop */
        switch (tk_next_typ(tkc)) {
        case TK_TT_RBRACE:
            done = TRUE;
            break;
        case TK_TT_COMMA:
            res = ncx_consume_token(tkc, mod, TK_TT_COMMA);
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                return res;
            }
            break;
        default:
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        }
    }

    return NO_ERR;

} /* consume_notif_data */


/********************************************************************
* FUNCTION consume_notif_contents
* 
* Parse contents of one NCX module <notif> section
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   not == not_template_t to fill in
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_notif_contents (ncx_module_t *mod,
			    tk_chain_t *tkc,
			    not_template_t *not)
{
    status_t       res;

    res = consume_cmn_docs(tkc, mod, &not->descr, &not->condition);
    if (res != NO_ERR) {
	return res;
    }

    /* Get the mandatory <notif-class> field */
    res = ncx_consume_name(tkc, mod, NCX_EL_NOTIF_CLASS, &not->not_class,
          NCX_REQ, TK_TT_SEMICOL);
    if (res != NO_ERR) {
        return res;
    }

    /* Get the mandatory <notif-type> field */
    res = ncx_consume_name(tkc, mod, NCX_EL_NOTIF_TYPE, &not->not_type,
          NCX_REQ, TK_TT_SEMICOL);
    if (res != NO_ERR) {
        return res;
    }

    /* Get the optional <notif-data> field */
    res = ncx_consume_tstring(tkc, mod, NCX_EL_NOTIF_DATA, NCX_OPT);
    switch (res) {
    case NO_ERR:
        /* get rest of the notif-data section */
        res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
        if (res != NO_ERR) {
            return res;
        }
        res = consume_notif_data(mod, tkc, not);
        if (res != NO_ERR) {
            return res;
        }
        /* get rest of the notif-data section */
        res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
        if (res != NO_ERR) {
            return res;
        }
        break;
    case ERR_NCX_SKIPPED:
        break;
    default:
        return res;
    }

    /* get the 'notif' end tag */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    return res;

} /* consume_notif_contents */


/********************************************************************
* FUNCTION consume_notif
* 
* Parse one NCX module <notif> section
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_notif (tk_chain_t *tkc, 
                   ncx_module_t   *mod)
{
    not_template_t  *not;
    status_t       res;

    /* peek ahead to see if the section is done */
    if (!(tk_next_typ(tkc)==TK_TT_TSTRING && tk_next_val(tkc) &&
          !xml_strcmp(NCX_EL_NOTIF, tk_next_val(tkc)))) {
	return ERR_PARS_SECDONE;  
    }

    /* malloc a new NOT struct */
    not = not_new_template();
    if (!not) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* Already checked for an <notif> start node, so this should work */
    res = ncx_consume_name(tkc, mod, NCX_EL_NOTIF, &not->name, 
                           NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
        not_free_template(not);
        return res;
    }

    /* get the rest of the <notif> contents and the end tag */
    res = consume_notif_contents(mod, tkc, not);
    if (res != NO_ERR) {
        not_free_template(not);
        return res;
    }

    /* make sure this is not a duplicate */
    if (ncx_is_duplicate(mod, not->name)) {
	res = ERR_NCX_DUP_ENTRY;
	ncx_print_errormsg(tkc, mod, res);	
	not_free_template(not);
	return res;
    }

#ifdef NCX_PARSE_DEBUG
    log_debug3("\nncx_parse: adding NOT (%s) to mod.owner (%s.%s)", 
	      not->name, mod->hdr.modname, mod->hdr.owner);
#endif

    /* add the rpc to the module RPC Que */
    dlq_enque(not, &mod->notifQ);
    return NO_ERR;

}  /* consume_notif */

