//remove entries from table
function remove_entries_from_table(name){
	plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] get table as var");
	var OCM_R_IMPORT_TABELLE = plc.get(name);

	if (OCM_R_IMPORT_TABELLE != undefined && OCM_R_IMPORT_TABELLE != "")
	{
		for(var OCM_R in OCM_R_IMPORT_TABELLE)
		{
			plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] OCM_R for delete: "+OCM_R);
			OCM_R.delete();
		}
	}else{
		plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] wanted table can't be loaded: " + name);
	}
}