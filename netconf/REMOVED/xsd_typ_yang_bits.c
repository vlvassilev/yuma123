

#ifdef CHANGE_BACK_BITS
	/* a bits type is choice on 0 to N empty child element nodes */
	topcon = xml_val_new_struct(NCX_EL_CHOICE, xsd_id);
	if (!topcon) {
	    return ERR_INTERNAL_MEM;
	} else {
	    val_add_child(topcon, val);  /* add early */
	}

	/* set the minOccurs to 0 */
	res = xml_val_add_cattr(XSD_MIN_OCCURS, 0, XSD_ZERO, topcon);
	if (res != NO_ERR) {
	    return res;
	}

	/* set the maxOccurs to the number of bits */
	sprintf((char *)numbuff, "%u", typ_enumdef_count(realtypdef));
	res = xml_val_add_cattr(XSD_MAX_OCCURS, 0, numbuff, topcon);
	if (res != NO_ERR) {
	    return res;
	}

	/* create an empty element definition for each bit */
	for (enu = typ_first_enumdef(realtypdef);
	     enu != NULL;
	     enu = (const typ_enum_t *)dlq_nextEntry(enu)) {
	    /* add an empty element to the choice */
	    node = xsd_new_element(mod, enu->name, 
				   flagtypdef, typdef,
				   TRUE, FALSE);
	    if (!node) {
		return ERR_INTERNAL_MEM;
	    } else {
		val_add_child(node, topcon);
	    }


	    /* add annotation child element */
	    annot = xml_val_new_struct(XSD_ANNOTATION, xsd_id);
	    if (!annot) {
		return ERR_INTERNAL_MEM;
	    } else {
		val_add_child(annot, node);  /* add early */
	    }

	    /* add documentation child element */
	    if (enu->descr || enu->ref) {
		docval = xsd_make_documentation_ref(enu->descr, enu->ref);
		if (!docval) {
		    return ERR_INTERNAL_MEM;
		} else {
		    val_add_child(docval, annot);
		}
	    }

	    /* add appinfo child element */
	    appval = xsd_make_bit_appinfo(enu->pos, 
					  &enu->appinfoQ,
					  enu->status);
	    if (!appval) {
		return ERR_INTERNAL_MEM;
	    } else {
		val_add_child(appval, annot);
	    }
	}
#endif
