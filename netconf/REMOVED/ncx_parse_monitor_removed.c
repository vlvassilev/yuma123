
/********************************************************************
* FUNCTION consume_obj_contents
* 
* Parse the next N nodes as <object> contents
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   obj   == mon_obj_t  in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_obj_contents (ncx_module_t *mod,
			  tk_chain_t *tkc,
			  mon_obj_t *obj)
{
    status_t        res;

    /* Check for the optional <description> field */
    res = consume_cmn_docs(tkc, mod, &obj->descr, &obj->condition);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* Get the mandatory <type> field */
    res = ncx_consume_mname(tkc, mod, NCX_EL_TYPE, &obj->typname, 
         &obj->modstr, NCX_REQ, TK_TT_SEMICOL);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* check this as a type template first */
    res = typ_locate_template(mod, obj->modstr, obj->typname,
			      (typ_template_t **)&obj->odef);
    if (res==NO_ERR) {
	obj->otyp = MON_OT_TYPE;
    } else {
	/* check this as a MON template last */	
	res = mon_locate_template(mod, obj->modstr, obj->typname,
			      (mon_template_t **)&obj->odef);
	if (res==NO_ERR) {
	    obj->otyp = MON_OT_MONITOR;
	} else {
	    /* This is an error; unknown object type */
	    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
	}
    }

    /* get the 'obj' end tag */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }	
    return NO_ERR;

}  /* consume_obj_contents */


/********************************************************************
* FUNCTION consume_obj_cmn
* 
* Common sub-parser for regular objects or choice objects
* Parse the next N nodes as an 'object' subtree
* Create a mon_obj_t struct and add it to the specified mon
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
* OUTPUTS:
*   *ret == pointer to new object, if NO_ERR; NULL otherwise
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_obj_cmn (ncx_module_t *mod,
		     tk_chain_t  *tkc,
		     mon_obj_t  **ret)
{
    mon_obj_t      *obj;
    status_t        res;

    /* get a new object */
    obj = mon_new_obj();
    if (!obj) {
        return SET_ERROR(ERR_INTERNAL_MEM);
    }

    /* the next nodes should be an <object> start tag, name, left brace */
    res = ncx_consume_name(tkc, mod, NCX_EL_OBJECT, &obj->name, 
                           NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
        mon_free_obj(obj);
        return SET_ERROR(res);
    }

    /* get the object contents and end tag */
    res = consume_obj_contents(mod, tkc, obj);
    if (res != NO_ERR) {
        mon_free_obj(obj);
        return res;
    }

    *ret = obj;
    return NO_ERR;

}  /* consume_obj_cmn */


/********************************************************************
* FUNCTION consume_obj
* 
* Parse the next N nodes as a <object> subtree
* Create a mon_obj_t struct and add it to the specified MON
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain in progress
*   mon == mon_template struct that will get the mon_obj_t 
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_obj (ncx_module_t *mod,
		 tk_chain_t  *tkc,
		 mon_template_t  *mon)
{
    mon_obj_t      *obj;
    status_t        res;

    res = consume_obj_cmn(mod, tkc, &obj);
    if (res != NO_ERR) {
        return res;
    }

    /* check if this is a duplicate */
    if (mon_find_obj(mon, obj->name)) {
	mon_free_obj(obj);
	return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

    /***  TBD:  TYPE VALIDATION ***/

    /* set the object ID and bump the object count */
    obj->objid = mon->objcnt++;
        
    /* 1st pass parsing success, add the entry at the end of the queue */
    dlq_enque(obj, &mon->objQ);
    return NO_ERR;

}  /* consume_obj */


/********************************************************************
* FUNCTION consume_cobj
* 
* Parse the next N nodes as an object within a choice or
* choice block decl
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   mon  == monitor struct in progress to verify against
*   mch == mon_choice_t in progress, that will get the mon_obj_t
*          if the mb parm is NULL
*   mb == mon_block_t that will get the mon_obj_t struct
*      == NULL if this is not a choice block decl in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_cobj (ncx_module_t *mod,
		  tk_chain_t *tkc,
		  mon_template_t  *mon,
		  mon_choice_t  *mch,
		  mon_block_t *mb)
{
    mon_obj_t   *obj, *test;
    status_t     res;

    res = consume_obj_cmn(mod, tkc, &obj);
    if (res != NO_ERR) {
        return res;
    }

    /* check if this is a duplicate in the MON in progress */
    if (mon_find_obj(mon, obj->name)) {
	mon_free_obj(obj);
	return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

    /* make sure this object is not already in the choice in progress */
    test = mon_search_choice(mch, obj->name);
    if (test) {
	mon_free_obj(obj);
	return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

    /* make sure this object is not already in the block in progress */
    if (mb) {
        test = mon_search_block(mb, obj->name);
        if (test) {
            mon_free_obj(obj);
            return SET_ERROR(ERR_NCX_DUP_ENTRY);
        }

	obj->blockid = mb->blockid;

        /* passed all tests, save in the block */
        dlq_enque(obj, &mb->blockQ);
    }

    /* set the object ID and bump the object count */
    obj->objid = mon->objcnt++;

    obj->choiceid = mch->choice_id;

    if (!mb) {
	/* passed all tests, save in the choice */
	dlq_enque(obj, &mch->choiceQ);
    }

    return NO_ERR;

}  /* consume_cobj */


/********************************************************************
* FUNCTION consume_mblock
* 
* Parse the next N nodes as a choice block decl
* Create a mon_obj_t struct for each block member 
* and add it to a new mon_block_t, then add that
* to the specified mon_choice_t
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   mon == monitor struct to verify against
*   mch == mon_choice_t that will get the mon_block_t struct
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_mblock (ncx_module_t *mod,
		    tk_chain_t *tkc,
		    mon_template_t  *mon,
		    mon_choice_t *mch)

{
    mon_block_t  *mb;
    status_t      res;
    boolean       done;

    /* consume the opening left bracket of the block decl */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACK);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* get a new choice block header */
    mb = mon_new_block();
    if (!mb) {
        return SET_ERROR(ERR_INTERNAL_MEM);
    }

    mb->blockid = mch->blockcnt++;

    /* get all the objects in this choice block */
    done = FALSE;
    while (!done) {
        res = consume_cobj(mod, tkc, mon, mch, mb);
        if (res != NO_ERR) {
            mon_free_block(mb);
            return SET_ERROR(res);
        }

        if (tk_next_typ(tkc) == TK_TT_RBRACK) {
            done = TRUE;
        }
    }

    /* consume the closing right bracket of the block decl */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACK);
    if (res != NO_ERR) {
        mon_free_block(mb);
        return SET_ERROR(res);
    }

    /* add this block to the choice in progress */
    dlq_enque(mb, &mch->choiceQ);
    return NO_ERR;

}  /* consume_mblock */


/********************************************************************
* FUNCTION consume_msection
* 
* Parse the next N nodes as an 'object' or 'choice' subtree
* Create a mon_obj_t struct and add it to the specified mon
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   mon    == monitor struct that will get the mon_obj_t 
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_msection (ncx_module_t *mod,
		      tk_chain_t *tkc,
		      mon_template_t  *mon)
{
    mon_choice_t    *mch;
    status_t         res;
    boolean          done;

    /* move to the first token */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* this should be an identifier token */
    if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
	res = ERR_NCX_WRONG_TKTYPE;
	ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    /* this identifier should be a 'object' or 'choice' token */
    if (!xml_strcmp(TK_CUR_VAL(tkc), NCX_EL_OBJECT)) {
        /* this is a regular object */
        TK_BKUP(tkc);
        return consume_obj(mod, tkc, mon);
    } else if (!xml_strcmp(TK_CUR_VAL(tkc), NCX_EL_CHOICE)) {
        /* consume the starting left brace of the choice */
        res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
        if (res != NO_ERR) {
            return SET_ERROR(res);
        }
        
        /* get a new object choice header */
        mch = mon_new_choice();
        if (!mch) {
            return SET_ERROR(ERR_INTERNAL_MEM);
        }
	mch->choice_id = mon->choicecnt++;

        /* get all the object choices */
        done = FALSE;
        while (!done) {
            /* next token can be a '[' to start a choice block decl
             * or an 'object' token to start a regular object decl
             */
            if (tk_next_typ(tkc) == TK_TT_LBRACK) {
                res = consume_mblock(mod, tkc, mon, mch);
            } else {
                res = consume_cobj(mod, tkc, mon, mch, NULL);
            }
            if (res != NO_ERR) {
                mon_free_choice(mch);
                return res;
            }

            if (tk_next_typ(tkc) == TK_TT_RBRACE) {
                done = TRUE;
            }
        }

        /* consume the ending right brace of the choice */
        res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
        if (res != NO_ERR) {
            mon_free_choice(mch);
        }

        /* save the object choice in the MON obj Q */
        dlq_enque(mch, &mon->objQ);
        return NO_ERR;
    } else {
        return SET_ERROR(ERR_NCX_WRONG_TKVAL);
    }        
    /*NOTREACHED*/

}  /* consume_msection */


/********************************************************************
* FUNCTION consume_mon
* 
* Parse an NCX module <monitor> section
*
* INPUTS:
*   tkc == token chain
*   mod  == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_mon (tk_chain_t *tkc,
                 ncx_module_t   *mod)
{
    status_t        res;
    mon_template_t *mon;
    boolean         done;

    /* peek ahead to see if the section is done */
    if (!(tk_next_typ(tkc)==TK_TT_TSTRING && tk_next_val(tkc) &&
          !xml_strcmp(NCX_EL_MONITOR, tk_next_val(tkc)))) {
	return ERR_PARS_SECDONE;  
    }

    /* malloc a new MON struct */
    mon = mon_new_template();
    if (!mon) {
	return SET_ERROR(ERR_INTERNAL_MEM);
    }

    /* Already checked for a <monitor> start node, so this should work */
    res = ncx_consume_name(tkc, mod, NCX_EL_MONITOR, &mon->name, 
                           NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
        mon_free_template(mon);
        return SET_ERROR(res);
    }

    res = consume_cmn_docs(tkc, mod, &mon->descr, &mon->condition);
    if (res != NO_ERR) {
        mon_free_template(mon);
        return SET_ERROR(res);
    }

    /* Get the optional <type> field 
     * If missing, then set to the default 'top'
     */
    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_TYPE, &mon->montype, 
         NCX_OPT, NCX_NO_WSP, TK_TT_SEMICOL);
    switch (res) {
    case NO_ERR:
	if (!xml_strcmp(mon->montype, NCX_PSTYP_TOP)) {
	    mon->mon_type = MON_TYP_TOP;
	} else if (!xml_strcmp(mon->montype, NCX_PSTYP_INTERFACE)) {
	    mon->mon_type = MON_TYP_INTERFACE;
	} else {
	    return SET_ERROR(ERR_NCX_WRONG_VAL);
	}
	break;
    case ERR_NCX_SKIPPED:
        mon->montype = xml_strdup(MON_DEF_MONTYPE);
	if (!mon->montype) {
	    return SET_ERROR(ERR_INTERNAL_MEM);
	}
	mon->mon_type = MON_DEF_MONTYPE_ENUM;
        break;
    default:
        return SET_ERROR(res);
    }

    /* Get the objects start tag and left brace */
    res = ncx_consume_tstring(tkc, mod, NCX_EL_OBJECTS, NCX_REQ);
    if (res==NO_ERR) {
        res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    }
    if (res != NO_ERR) {
        mon_free_template(mon);
        return SET_ERROR(res);
    }

    /* get each 'object' or 'choice' subsection */
    done = FALSE;
    while (!done) {
	res = consume_msection(mod, tkc, mon);
        if (res != NO_ERR) {
	    mon_free_template(mon);
	    return res;
	}
        if (tk_next_typ(tkc) == TK_TT_RBRACE) {
            done = TRUE;
        }
    }

    /* get the mandatory 'objects' end tag */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    if (res != NO_ERR) {
	mon_free_template(mon);
	return SET_ERROR(res);
    }

    /* get the mandatory 'monitor' end tag */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    if (res != NO_ERR) {
	mon_free_template(mon);
	return SET_ERROR(res);
    }

    /* make sure this is not a duplicate */
    if (ncx_is_duplicate(mod, mon->name)) {
#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: dropping duplicate MON (%s) for mod.owner (%s.%s)", 
           mon->name, mod->hdr.modname, mod->hdr.owner);
#endif
            mon_free_template(mon);
            return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: adding MON (%s) to mod.owner (%s.%s)", 
           mon->name, mod->hdr.modname, mod->hdr.owner);
#endif

    /* Got a valid entry, add it to the mod->monQ */
    dlq_enque(mon, &mod->monQ);
    return NO_ERR;

}  /* consume_monitor */
