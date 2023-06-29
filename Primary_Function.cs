// Main Function
function dbs_ocmr_get_updated_sobj_by_date(){
	"This function returns a list of SOBJ IDs, which have been update after a specific date using the settings defined for URL, Username, and Password."
	remove_entries_from_table('__USER_TABLE_DBS_UT_OCM-R_OCM_IMP');

	var String sFunctionName = "dbs_ocmr_get_updated_sobj_by_date";

	var String sUrlPrefix = context.DBS_OCM_R_SET_DEF_URL;
	var String sUrlFunction = context.DBS_CUSET_OCMR_DEF_S_URL_FUNCTION_DATE;
	var String sUrlParamQuestionMark = context.DBS_CUSET_OCMR_DEF_URL_S_PARAM_QUESTION_MARK;
	var String sUrlDateParam = context.DBS_CUSET_OCMR_DEF_URL_S_PARAM_DATE;
	var String sUrlParamAnd = context.DBS_CUSET_OCMR_DEF_URL_S_PARAM_AND;
	var String sUrlAcumulatedSupplementParam = context.DBS_CUSET_OCMR_DEF_URL_S_SOBJ_ACUMMULATED_PARAM;
	var String sUrlAcumulatedSupplementParamValue = context.DBS_CUSET_OCMR_DEF_URL_GET_BY_DATE_S_ACUM_SUPP_TRUE_OR_FALSE;
	var String sUser = context.DBS_OCM_R_SET_DEF_USERNAME;
	var String sPassword = context.DBS_OCM_R_SET_DEF_PASSWORD;

	plw.writetolog("[INFO]["+sFunctionName+"] begin function");
	plw.writetolog("[INFO]["+sFunctionName+"]"+" - called settings: "+ "\nOCM-R Endpoint URL: " + sUrlPrefix + ", " + "\nOCM-R Endpoint Username: " + sUser + ", " + "\nOCM-R Endpoint Password: " + sPassword);
	
	var Date dUpdatedBy = context.DBS_UA_D_OCM_R_UPDATED_DATE;
	plw.writetolog("[INFO]["+sFunctionName+"] date updated by: "+dUpdatedBy);
	
	var String sFormattedDate = context.DBS_CUSET_OCMR_DEF_DATE_PRINT.Call(dUpdatedBy,context.DBS_CUSET_OCMR_DEF_DATE_WANTED_FORMAT);
	plw.writetolog("[INFO]["+sFunctionName+"] formatted date: "+sFormattedDate);
	
	var String Regexp = "[,\.]";
	var String sFinalDate = sFormattedDate.toString().ReplaceRegexp(Regexp,"");
	plw.writetolog("[INFO]["+sFunctionName+"] ?updated_date=" + sFinalDate );

	var String sUrl = sUrlPrefix+sUrlFunction+sUrlParamQuestionMark+sUrlDateParam+sFinalDate+sUrlParamAnd+sUrlAcumulatedSupplementParam+sUrlAcumulatedSupplementParamValue;
	plw.writetolog("[INFO]["+sFunctionName+"] DEFAULT URL OCM ENDPOINT: "+sUrl);
________________________________________________________________________________________________________________________________
	var Vector sResult = new vector();
	var String sFileContent = "";
	var String sFilePath = "";
	var String sFileName = "";
	var pathname oPath = undefined;

	var plc.impextarget oTarget = plc.impextarget.get(context.DBS_CUSET_OCMR_SOBJ_BY_DATE_TARGET);
	var plc.impexformat oFormat = plc.impexformat.get(context.DBS_CUSET_OCMR_SOBJ_BY_DATE_FORMAT);


	//v0.11: function call
	sResult = dbs_ocmr_authenticate_and_get_json_response(sUser, sPassword, sUrl);
	plw.writetolog("[INFO]["+sFunctionName+"] return: "+sResult);
	

	//v0.39
	if (sResult != undefined && typeof sResult == 'object')
		try {
			sFileContent = dbs_ocmr_prepare_sobj_for_import(sResult, oFormat);
		} catch (error e) {
			plw.writetolog("[ERROR]["+sFunctionName+"] error when trying split the JSON: "+e);
		}
		

	plw.writetolog("[INFO]["+sFunctionName+"] formatted content: "+sFileContent);
	
	if (sFileContent != undefined && sFileContent != "" && sFileContent != "[]") {
		//v0.07: from here down was added
		var String sWriteMode = context.DBS_CUSET_OCMR_DEF_WRITE_MODE_OVERWRITE;
		sFilePath = context.DBS_UA_S_OCM_R_JSON_FILE_PATH;
		
		sFileName = context.DBS_CUSET_OCMR_SOBJ_DATA_FILE;
		//v0.12: function call
		oPath = dbs_ocmr_create_json_file(sFileName, sWriteMode, sFileContent);
		
		//v0.07: the rest of this actually runs the import using the target and impexformat
		//we can separate it if needed, but otherwise I think we can leave it here if we want it all in one
		if (sFilePath == oPath.toString()) {
			//DBS_CUSET_OCMR_SOBJ_BY_DATE_TARGET
			plw.writetolog("[INFO]["+sFunctionName+"] impex target: "+oTarget);

			plw.writetolog("[INFO]["+sFunctionName+"] impex format: "+oFormat);
			
			//v0.13: function call
			var String sFunctionStatus = dbs_ocmr_json_import(oTarget, oFormat, sFunctionName);
			plw.writetolog(sFunctionStatus);
			
		} else
			plw.writetolog("[ERROR]["+sFunctionName+"] the file paths do not match: "+plw.char(10)+" - "+oPath+plw.char(10)+" - "+sFilePath);
	}
	else
		plw.writetolog("[ERROR]["+sFunctionName+"] the retuned file content is empty");
	
	plw.writetolog("[INFO]["+sFunctionName+"] end of function");
	if (oPath != undefined)
		return oPath;
	else
		return false;
}
dbs_ocmr_get_updated_sobj_by_date.exportfunction();
