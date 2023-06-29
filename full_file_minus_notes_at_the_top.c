// full file

namespace dbsocmr;


//example of global variable
/**************************************
var ObjectClass cUser = plc.opx2user;
**************************************/

//Global setting for the URL
new plw.setting("DBS_OCM_R_SET_DEF_URL",               
	"Default OCM-R Endpoint URL",
	//get_dbs_ini_variable("*DBS_API_OCM-WEBSERVICE-URI*"),	//will be used later on
	"https://gateway.hub.db.de/ocm-r-api/review/dev/digital-portal/ocm-r-api/v1/ocmrapi/",
	"STRING",
	"Setting defining the OCM-R Endpoint URL");
   
//Global setting for the Username
new plw.setting("DBS_OCM_R_SET_DEF_USERNAME",               
	"Default OCM-R Endpoint Username",
	//get_dbs_ini_variable("*DBS_API_OCM-WEBSERVICE-USER*"),	//will be used later on
	"OCM API PM Tool",
	"STRING",
	"Setting defining the default OCM-R Endpoint Username");

//Global setting for Password
new plw.setting("DBS_OCM_R_SET_DEF_PASSWORD",               
	"Default OCM-R Endpoint Password",
	//get_dbs_ini_variable("*DBS_API_OCM-WEBSERVICE-PASSWORD*"), //will be used later on
	"SCXfCOwRUMpheoxUF4fESCXfCOwRUMpheoxUF4fE_*",
	"STRING",
	"Setting defining the default OCM-R Endpoint Password");


//example of a function using the documentation option in the first line in the function

/**************************************

function <name of function>(bDebugModeActive, sLogText)
{
	"This function uses the writetolog function to print traces in the logfile if debug mode is active
  	@param bDebugModeActive Boolean: true or false, activating the logging.
  	@param sLogText String: textto be written to logfile."
	
	
	if(bDebugModeActive)
		plw.WriteToLog(sLogText);
}
**************************************/
function dbs_ocmr_prepare_sobj_data_for_import(oJSONToReformat, plc.impexformat _oFormat, sSOBJ)
{
	"This function accepts a JS object created through rest json API and changes its format to fit the Planisware ImpEx feature
	@param oJSONToReformat Object: Dictionary from OCM-R json file containing a list of SOBJ IDs and update times
	@param oFormat Object: import format used containing the JSON fields
	@param sSOBJ String: string containing the SOBJ number"	


	var Vector vObjResult = new Vector();
	var Vector vSobj = new Vector();
	var oJSONReformat = undefined;
	
	var plc.impexformat oFormat = _oFormat;
	var oJSON = undefined;
	var oError = undefined;	

	plw.writetolog("[INFO][dbs_ocmr_prepare_sobj_data_for_import] json pre-format: " + oJSONToReformat);
	
	if (oJSONToReformat != undefined && oJSONToReformat != false)
	{

		oJSON = oJSONToReformat.result;
		//v0.39
		oError = oJSONToReformat.error;

		if (oJSON != undefined)
		{
		
			//v0.25
			if (oJSON.NUMBER == undefined || oJSON.NUMBER == "")
			{
				oJSON.set("NUMBER",sSOBJ);
			}

			try
			{
	
				vSobj = dbs_ocmr_tokenize_dictionary(oJSON, oFormat);

			}
			catch (error e)
			{
				plw.writetolog("[ERROR][dbs_ocmr_prepare_sobj_data_for_import] error when trying split the JSON: "+e);
			}

		}
		else if (oError != undefined)
		{
			
			plw.writetolog("[ERROR][dbs_ocmr_prepare_sobj_data_for_import] error in JSON response: "+oError.message);
		
		}
				
				
	}
	
	plw.writetolog("[INFO][dbs_ocmr_prepare_sobj_for_import] json post-format: " + vSobj);

	return rest.json(vSobj);
}

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

//v0.03: function created
//v0.04: function updated
//v0.11: function updated
//v0.12: function updated
//v0.13: function updated
//v0.18: function updated
//v0.37: changed acummulated supplement setting
//v0.39: added error handling
//v0.44: automatically create master data
//v0.45: update projects automatically
function dbs_ocmr_get_by_sobj_number()
{
	"This function retrieves the data from OCM-R for all SOBJ elements that were updated after a specific date using the settings defined for URL, Username, and Password"
	
	plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] begin function");

    //v0.55
	//Removing old Data before import new
	//remove_entries_from_table('__USER_TABLE_DBS_UT_OCM-R_PEI_IMP');

	var String sFunctionCallName = "dbs_ocmr_get_by_sobj_number";
	plw.writetolog("[INFO]["+sFunctionCallName+"]"+" - called settings: "+ "\nOCM-R Endpoint URL: " + context.DBS_OCM_R_SET_DEF_URL + ", " + "\nOCM-R Endpoint Username: " + context.DBS_OCM_R_SET_DEF_USERNAME + ", " + "\nOCM-R Endpoint Password: " + context.DBS_OCM_R_SET_DEF_PASSWORD);
	
	var String sUrlPrefix = context.DBS_OCM_R_SET_DEF_URL;
	plw.writetolog("[INFO] URL" + sUrlPrefix);
	
	var String sUrlFunction = context.DBS_CUSET_OCMR_DEF_S_URL_FUNCTION_NUMBER;
	var String sUrlParamQuestionMark = context.DBS_CUSET_OCMR_DEF_URL_S_PARAM_QUESTION_MARK;
	var String sUrlSOBJParam = context.DBS_CUSET_OCMR_DEF_URL_S_SOBJ_NUM_PARAM;

	var String sUrlParamAnd = context.DBS_CUSET_OCMR_DEF_URL_S_PARAM_AND;
	var String sUrlAcumulatedSupplementParam = context.DBS_CUSET_OCMR_DEF_URL_S_SOBJ_ACUMMULATED_PARAM;
	//v0.37
	var String sUrlAcumulatedSupplementParamValue = context.DBS_CUSET_OCMR_DEF_URL_GET_BY_SOBJ_S_ACUM_SUPP_TRUE_OR_FALSE;
	
	var String sUser = context.DBS_OCM_R_SET_DEF_USERNAME;
	var String sPassword = context.DBS_OCM_R_SET_DEF_PASSWORD;
	
	//v0.09 wrong type; result is a vector	
	//	var String sResult = "";
	var Vector sResult = new vector();
	var String sFileContent = "";
	var String sFilePath = "";
	var String sFileName = "";
	var pathname oPath = undefined;
	
	var plc.impexformat oFormat = plc.impexformat.get(context.DBS_CUSET_OCMR_SOBJ_DATA_FORMAT);
	var plc.impextarget oTarget = plc.impextarget.get(context.DBS_CUSET_OCMR_SOBJ_DATA_TARGET);
	
	//var String sSOBJ = context.DBS_CUSET_OCMR_DEF_S_SOBJ; //"SOBJ000000015640" to be replaced with loop in future
	//v0.14: replaced string declaraction above with loop below
	for (var plc.__user_table_dbs_ut_ocm_r_ocm_imp oSOBJ in plc.__user_table_dbs_ut_ocm_r_ocm_imp where oSOBJ.DBS_UA_B_OCMR_API_CALL)
	{
		plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] SOBJ object: "+oSOBJ);
		var String sSOBJ = oSOBJ.NAME;
		
		plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] ?sobj_number="+sSOBJ );
		//var String sUrl = sUrlPrefix+sUrlFunction+"?sobj_number="+sSOBJ+"&acummulated_supplement=true";
		var String sUrl = sUrlPrefix+sUrlFunction+sUrlParamQuestionMark+sUrlSOBJParam+sSOBJ+sUrlParamAnd+sUrlAcumulatedSupplementParam+sUrlAcumulatedSupplementParamValue;
		plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] DEFAULT URL OCM ENDPOINT " +sUrl);
		
		//v0.11: function call
		sResult = dbs_ocmr_authenticate_and_get_json_response(sUser, sPassword, sUrl);
		plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] return: "+sResult);
		
		//v0.39
		if (sResult != undefined && typeof sResult == 'object')
			//dbs_ocmr_prepare_sobj_data_for_import
			sFileContent = dbs_ocmr_prepare_sobj_data_for_import(sResult,oFormat,sSOBJ);
		
		plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] formatted content: "+sFileContent);
		
		if (sFileContent != undefined && sFileContent != "" && sFileContent != "[]")
		{
			//v0.07: from here down was added
			var String sWriteMode = context.DBS_CUSET_OCMR_DEF_WRITE_MODE_OVERWRITE;
			sFilePath = context.DBS_UA_S_OCM_R_JSON_PEI_FILE_PATH;	//using a dedicated file for each import
			sFileName = context.DBS_CUSET_OCMR_PEI_DATA_FILE;

			//v0.12: function call
			oPath = dbs_ocmr_create_json_file(sFileName, sWriteMode, sFileContent);
			
			//v0.07: the rest of this actually runs the import using the target and impexformat
			//we can separate it if needed, but otherwise I think we can leave it here if we want it all in one
			if (context.DBS_UA_S_OCM_R_JSON_PEI_FILE_PATH == oPath.toString())
			{

				plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] impex target: "+oTarget);
				
				plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] impex format: "+oFormat);
				
				//v0.13: function call
				var String sFunctionStatus = dbs_ocmr_json_import(oTarget, oFormat, sFunctionCallName);
				plw.writetolog(sFunctionStatus);
			}
			else
				plw.writetolog("[ERROR][dbs_ocmr_get_by_sobj_number] the file paths do not match: "+plw.char(10)+" - "+oPath+plw.char(10)+" - "+context.DBS_UA_S_OCM_R_JSON_PEI_FILE_PATH);
		}
		else
			plw.writetolog("[ERROR][dbs_ocmr_get_by_sobj_number] the retuned file content is empty");
	}
	
	//v0.44
	dbs_ocmr_create_master_data_all();

	//v0.45
	dbs_ocmr_modify_projects_all();

	plw.writetolog("[INFO][dbs_ocmr_get_by_sobj_number] end of function");
	
	return true;
}
dbs_ocmr_get_by_sobj_number.exportfunction();



































function dbs_ocmr_get_updated_sobj_by_date()
{
	"This function returns a list of SOBJ IDs, which have been update after a specific date using the settings defined for URL, Username, and Password."
	
	var String sFunctionCallName = "dbs_ocmr_get_updated_sobj_by_date";


	plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] begin function");

	//Removing old Data before import new
	remove_entries_from_table('__USER_TABLE_DBS_UT_OCM-R_OCM_IMP');

	var String sFunctionName = "dbs_ocmr_get_updated_sobj_by_date";
	plw.writetolog("[INFO]["+sFunctionName+"]"+" - called settings: "+ "\nOCM-R Endpoint URL: " + context.DBS_OCM_R_SET_DEF_URL + ", " + "\nOCM-R Endpoint Username: " + context.DBS_OCM_R_SET_DEF_USERNAME + ", " + "\nOCM-R Endpoint Password: " + context.DBS_OCM_R_SET_DEF_PASSWORD);
	
	var Date dUpdatedBy = context.DBS_UA_D_OCM_R_UPDATED_DATE;
	plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] date updated by: "+dUpdatedBy);
	
	var String sFormattedDate = context.DBS_CUSET_OCMR_DEF_DATE_PRINT.Call(dUpdatedBy,context.DBS_CUSET_OCMR_DEF_DATE_WANTED_FORMAT);
	plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] formatted date: "+sFormattedDate);
	
	var String Regexp = "[,\.]";
	var String sFinalDate = sFormattedDate.toString().ReplaceRegexp(Regexp,"");
	plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] ?updated_date=" + sFinalDate );

	
	//v0.10 - bugfixing the url building
	var String sUrlPrefix = context.DBS_OCM_R_SET_DEF_URL;
	var String sUrlFunction = context.DBS_CUSET_OCMR_DEF_S_URL_FUNCTION_DATE;
	var String sUrlParamQuestionMark = context.DBS_CUSET_OCMR_DEF_URL_S_PARAM_QUESTION_MARK;
	var String sUrlDateParam = context.DBS_CUSET_OCMR_DEF_URL_S_PARAM_DATE;
	var String sUrlParamAnd = context.DBS_CUSET_OCMR_DEF_URL_S_PARAM_AND;
	var String sUrlAcumulatedSupplementParam = context.DBS_CUSET_OCMR_DEF_URL_S_SOBJ_ACUMMULATED_PARAM;
	//v0.37
	var String sUrlAcumulatedSupplementParamValue = context.DBS_CUSET_OCMR_DEF_URL_GET_BY_DATE_S_ACUM_SUPP_TRUE_OR_FALSE;

	//var String sUrl = sUrlPrefix+sUrlFunction+"?updated_date="+sFinalDate+"&acummulated_supplement=true";
	var String sUrl = sUrlPrefix+sUrlFunction+sUrlParamQuestionMark+sUrlDateParam+sFinalDate+sUrlParamAnd+sUrlAcumulatedSupplementParam+sUrlAcumulatedSupplementParamValue;
	plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] DEFAULT URL OCM ENDPOINT: "+sUrl);
	
	var String sUser = context.DBS_OCM_R_SET_DEF_USERNAME;
	var String sPassword = context.DBS_OCM_R_SET_DEF_PASSWORD;

	//v0.09 wrong type; result is a vector	
	//var String sResult = "";
	var Vector sResult = new vector();
	var String sFileContent = "";
	var String sFilePath = "";
	var String sFileName = "";
	var pathname oPath = undefined;

	var plc.impextarget oTarget = plc.impextarget.get(context.DBS_CUSET_OCMR_SOBJ_BY_DATE_TARGET);
	var plc.impexformat oFormat = plc.impexformat.get(context.DBS_CUSET_OCMR_SOBJ_BY_DATE_FORMAT);


	//v0.11: function call
	sResult = dbs_ocmr_authenticate_and_get_json_response(sUser, sPassword, sUrl);
	plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] return: "+sResult);
	

	//v0.39
	if (sResult != undefined && typeof sResult == 'object')
		try
		{

			sFileContent = dbs_ocmr_prepare_sobj_for_import(sResult, oFormat);

		}
		catch (error e)
		{
			plw.writetolog("[ERROR][dbs_ocmr_get_updated_sobj_by_date] error when trying split the JSON: "+e);
		}
		

	plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] formatted content: "+sFileContent);
	
	if (sFileContent != undefined && sFileContent != "" && sFileContent != "[]")
	{
		//v0.07: from here down was added
		var String sWriteMode = context.DBS_CUSET_OCMR_DEF_WRITE_MODE_OVERWRITE;
		sFilePath = context.DBS_UA_S_OCM_R_JSON_FILE_PATH;
		
		sFileName = context.DBS_CUSET_OCMR_SOBJ_DATA_FILE;
		//v0.12: function call
		oPath = dbs_ocmr_create_json_file(sFileName, sWriteMode, sFileContent);
		
		//v0.07: the rest of this actually runs the import using the target and impexformat
		//we can separate it if needed, but otherwise I think we can leave it here if we want it all in one
		if (context.DBS_UA_S_OCM_R_JSON_FILE_PATH == oPath.toString())
		{
			//DBS_CUSET_OCMR_SOBJ_BY_DATE_TARGET
			plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] impex target: "+oTarget);

			plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] impex format: "+oFormat);
			
			//v0.13: function call
			var String sFunctionStatus = dbs_ocmr_json_import(oTarget, oFormat, sFunctionCallName);
			plw.writetolog(sFunctionStatus);
			
		}
		else
			plw.writetolog("[ERROR][dbs_ocmr_get_updated_sobj_by_date] the file paths do not match: "+plw.char(10)+" - "+oPath+plw.char(10)+" - "+context.DBS_UA_S_OCM_R_JSON_FILE_PATH);
	}
	else
		plw.writetolog("[ERROR][dbs_ocmr_get_updated_sobj_by_date] the retuned file content is empty");
	
	plw.writetolog("[INFO][dbs_ocmr_get_updated_sobj_by_date] end of function");
	if (oPath != undefined)
		return oPath;
	else
		return false;
}
dbs_ocmr_get_updated_sobj_by_date.exportfunction();
























//v0.06 function created
//v0.18 function updated
//v0.39 added error handling
function dbs_ocmr_prepare_sobj_for_import(oJSONToReformat, plc.impexformat _oFormat)
{
	"This function accepts a JS object created through rest json API and changes its format to fit the Planisware ImpEx feature
	@param oJSONToReformat Object: Dictionary from OCM-R json file containing a list of SOBJ IDs and update times
	@param oFormat Object: import format used containing the JSON fields"
	
	var Vector vObjResult = new Vector();
	var Vector vSobj = new Vector();
	//var oJSONReformat = undefined;
	var oResult = undefined;
	var oError = undefined;

	var String sDataSet = context.DBS_CUSET_OCMR_DEF_IMP_FILE;
	var plc.impexformat oFormat = _oFormat;
	var plc.impexformat oDATASET = "DATASET";
	var String sValue = "";
	
	plw.writetolog("[INFO][dbs_ocmr_prepare_sobj_for_import] json pre-format: " + oJSONToReformat);
	
	if (oJSONToReformat != undefined && oJSONToReformat != false)
	{
		oResult = oJSONToReformat.result;
		//v0.39
		oError = oJSONToReformat.error;

		if (oResult != undefined)
		{
			vObjResult = oResult.sobj;

			for (var oSobj in vObjResult)
			{
				var oJSONReformat = {};
				for (var plc.impexattribute oImpAtt in plc.impexformat.get("[OCM-R] OCM Import:OCM-R_IMP_FORMAT").get("IMPEX-ATTRIBUTES"))
				{
					var sField = oImpAtt.IMPORT_ATTRIBUTE;
					plw.writetolog("[INFO][dbs_ocmr_prepare_sobj_for_import] sField: " + sField);
					if (sField == oDATASET){
						
						sValue = sDataSet;
						plw.writetolog("[INFO][dbs_ocmr_prepare_sobj_for_import] sValue: " + sValue);
					}
					else
					{

						sValue = oSobj.get(sField);
						plw.writetolog("[INFO][dbs_ocmr_prepare_sobj_for_import] sValue: " + sValue);
					}
				  	
					  
					oJSONReformat.set(sField, sValue);
	
				}

				
				plw.writetolog("[INFO][dbs_ocmr_prepare_sobj_for_import] sobj post-format: " + oJSONReformat);
				vSobj.push(oJSONReformat);
			}
		}
		else if (oError != undefined)
		{
			plw.writetolog("[ERROR][dbs_ocmr_prepare_sobj_for_import] error in json response: " + oError.message);
			
		}
	}
	
	plw.writetolog("[INFO][dbs_ocmr_prepare_sobj_for_import] json post-format: " + vSobj);

	return rest.json(vSobj);
}

//v0.18: function created
//v0.23: adjusted handling of subdicts
//v0.42: handle variables field
//v0.46: handle commas
function dbs_ocmr_tokenize_dictionary(dictToTokenize, oFormat)
{

	var vResult = new Vector();
	var value = undefined;
	var plc.impexformat oDATASET = "DATASET";
	var String sValue = "";
	var String sDataSet = context.DBS_CUSET_OCMR_DEF_IMP_FILE;

	var filteredDict = {};

	var vAttKeys = new Vector();
	var vImpAttr = oFormat.get("IMPEX-ATTRIBUTES");

	for (var plc.impexattribute oImpAttr in vImpAttr)
	{
		vAttKeys.push(oImpAttr.IMPORT_ATTRIBUTE.toUpperCase());
	}	

	for (var key in vAttKeys)
	{

		if (key == oDATASET){
			value = sDataSet;
		}
		else
		{
			value = dictToTokenize.get(key);
		}

		if (value == undefined || value == "" || value == null)
		{
			continue;
		}
	
		plw.writetolog("[INFO][dbs_ocmr_tokenize_dictionary] found key value pair: " + key + " : " + value);
	
		if (typeof value == 'vector')
		{
			for (var entry in value)
			{

				//v0.42
				if (entry.generic_name != undefined && entry.value != undefined)
				{
					if (entry.generic_name.toUpperCase() in vAttKeys)
					{
						//v0.46
						var variable_value = dbs_ocmr_format_import_attribute(entry.generic_name, entry.value, vImpAttr);
						plw.writetolog("[INFO][dbs_ocmr_tokenize_dictionary] found variable key value pair: " + entry.generic_name + " : " + variable_value);
						filteredDict.set(entry.generic_name, variable_value);
					}
					else
					{
						plw.writetolog("[INFO][dbs_ocmr_tokenize_dictionary] found unknown variable key value pair: " + entry.generic_name + " : " + entry.value);
					}
					continue;
				}

				var subList = dbs_ocmr_tokenize_dictionary(entry, oFormat);
				for (var subEntry in subList)
				{
					if (subEntry.parent == undefined || subEntry.parent == "")
					{
						subEntry.parent = dictToTokenize.sys_id;
					}
					vResult.push(subEntry);
					plw.writetolog("[INFO][dbs_ocmr_tokenize_dictionary] subdict found: " + subEntry);
				}
			}
		}
		else if (typeof value == 'object')
		{
			//v0.23
			if (value.display_value != undefined)
			{
				//v0.46
				var display_value = dbs_ocmr_format_import_attribute(key, value.display_value, vImpAttr);
				filteredDict.set(key, display_value);
			}
			else
			{
			
				var subList = dbs_ocmr_tokenize_dictionary(value, oFormat);
				for (var subEntry in subList)
				{
					if (subEntry.parent == undefined || subEntry.parent == "")
					{
						subEntry.parent = dictToTokenize.sys_id;
					}
					vResult.push(subEntry);
					plw.writetolog("[INFO][dbs_ocmr_tokenize_dictionary] subdict found: " + subEntry);
				}
			
			}
		}
		else
		{
			//v0.46
			value = dbs_ocmr_format_import_attribute(key, value, vImpAttr);
			filteredDict.set(key, value);
			
			//plw.writetolog("[INFO][dbs_ocmr_tokenize_dictionary] subdict found: " + filteredDict);
		}
	}

	vResult.push(filteredDict);

	plw.writetolog("[INFO][dbs_ocmr_tokenize_dictionary] result vector: " + vResult);

	return vResult;
}

//v0.46 function created
function dbs_ocmr_format_import_attribute(String key, value, Vector vImpAttr)
{
	"This function formats a primitive value according to the settings in the associated impex attribute
	@param key String: name of import attribute
	@param value: value of import attribute
	@param vImpAttr Object: Vector of impex-attributes
	"

	var boolean bRemoveCommas = false;

	if (typeof value == 'string')
	{
		for (var plc.impex_attribute oImpAttr in vImpAttr where oImpAttr.IMPORT_ATTRIBUTE == key.toUpperCase())
		{
			bRemoveCommas = oImpAttr.DBS_UA_B_REMOVE_COMMAS;
			break;
		}
	}

	if (bRemoveCommas)
	{
		value = value.replaceregexp(",","");
	}

	return value;
	
}


//v0.11: function created
//v0.39: added error handling
function dbs_ocmr_authenticate_and_get_json_response(sUser, sPassword, sUrl)
{
	"This function accepts a User, Password and target API url as Strings, it returns a JSON result as a vector, from the API call"

	var plc.BasicAuthorization oAuth = new plc.BasicAuthorization(#user#,sUser,#password#,sPassword);
	plw.writetolog("[INFO][dbs_ocmr_authenticate_and_get_json_response] authorization: "+oAuth);

	//v0.09 wrong type; result is a vector	
	//	var String sResult = "";
	var Vector sResult = new vector();	
	
	//v0.39
	try
	{
		if (oAuth instanceof plc.BasicAuthorization)
			sResult = rest.json_get(sUrl,oAuth);
	}
	catch (error e)
	{
		plw.writetolog("[ERROR][dbs_ocmr_authenticate_and_get_json_response] error calling rest api: "+e);
	}
	plw.writetolog("[INFO][dbs_ocmr_authenticate_and_get_json_response] return: "+sResult);
	return sResult;

}

//v0.12: function created
function dbs_ocmr_create_json_file(sFileName, sWriteMode, sResult)
{
	"This function accepts a Filepath and Writemode for writing in the JSON, creates the JSON and returns a filepath of the created JSON"

	var pathname oPath = undefined;

	plw.writetolog("[INFO][dbs_ocmr_create_json_file] json file path: "+sFileName);	
	//oPath = plw.generateTemporaryFile("dbs_updated_sobj_ids.json");
	oPath = plw.generateTemporaryFile(sFileName);
	plw.writetolog("[INFO][dbs_ocmr_create_json_file] oPath: "+oPath);
	

	if(oPath instanceof pathname)
	{
		//as defined in script object FILEIO_LIB_JS2
		// - fileioLib is the namespace
		// - oFile = new plw.fileoutputstream(this.filepath, this.mode)
		// - write = this.filestream.write(data)
		var fileioLib.oFile oUpdatedSobjIds = new fileioLib.oFile(filepath : oPath,mode : sWriteMode);
		plw.writetolog("[INFO][dbs_ocmr_create_json_file] fileoutputstream: "+oUpdatedSobjIds);
		
		with (oUpdatedSobjIds)
		{
			try
			{
				oUpdatedSobjIds.write(sResult);
				plw.writetolog("[INFO][dbs_ocmr_create_json_file] 'write' was successful!");
			}
			catch (error e)
			{
				plw.writetolog("[ERROR][dbs_ocmr_create_json_file] error when trying to write to file: "+e);
			}
		}
		
		//uncomment for testing if needed
		//plw.downloadFileFromServer(oPath);
	}
	return oPath;

}


//v0.13 function created
function dbs_ocmr_json_import(oTarget, oFormat, sFunctionCallName)
{
	"This function accepts a target file and a format on how to import"
	
	if (oTarget instanceof plc.impextarget)
	{
		try
		{
			oFormat.DoImportWithFormatAndTarget(oTarget,true);
			plw.writetolog("[INFO][dbs_ocmr_json_import] import is finished");
			return "[INFO]["+sFunctionCallName+"] import is finished";
		}
		catch (error e)
		{
			plw.writetolog("[ERROR][dbs_ocmr_json_import] error when trying to import the file: "+e);
			return "[ERROR]["+sFunctionCallName+"] error when trying to import the file: "+e;
		}
	}
}

//v0.22 function created
//v0.27 added text keys
function dbs_ocmr_create_projects_selected()
{
	"This function attempts to create a new project for each SOBJ from the OCM-R import table, that was selected"
	
	plw.writetolog("[INFO][dbs_ocmr_create_projects_selected] begin function ");
	
	var vSelection = dbs_ocmr_get_selected_sobj();
	var oFormat = plc.impexformat.get("[OCM-R] PEI Import:OCM-R_IMP_JSON_RESPONSE");
	//v0.27
	var vSuccess = [];
	
	if (vSelection.length == 0)
	{
		//v0.27
		plw.alert(#@"DBS_TK_OCM_ERR_INVALID".current);
		return;
	}
	
	var bInvalid = false; //use flag to track if at least one of the selected SOBJ records are not valid
	var nSuccess = 0;
	var proj = undefined;
	for (var oSobj in vSelection)
	{
		//v0.28: if the selected record is invalid, we don't run the import and we set flag to alert the user
		if (oSOBJ.DBS_UA_B_VALID)
		{
			proj = dbs_ocmr_create_or_modify_project_for_sobj(oSobj, oFormat, false);
			if (proj != false)
			{
				nSuccess++;
				//v0.27

				vSuccess.push(proj);
				
			}
		}
		else
			bInvalid = true;
	}
	
	//v0.28: if at least one of the selected records is invalid, we alert the user
	if (bInvalid)
		plw.alert(plw.write_text_key("DBS_TK_OCM-R_NOT_VALID_PROJECT"));
	
	//v0.27
	for (var oSucc in vSuccess)
	{
		plw.alert(oSucc.NAME + " : " + #@"DBS_TK_MSG_PROJECT_HAS_BEEN_CREATED".current);
	}
	plw.writetolog("[INFO][dbs_ocmr_create_projects_selected] end function ");
}

//v0.22 function created
//v0.27 added text keys
function dbs_ocmr_modify_projects_selected()
{
	"This function attempts to modify the corresponding project for each SOBJ from the OCM-R import table, that was selected"
	
	plw.writetolog("[INFO][dbs_ocmr_modify_projects_selected] begin function ");
	
	var vSelection = dbs_ocmr_get_selected_sobj();
	var oFormat = plc.impexformat.get("[OCM-R] PEI Import:OCM-R_IMP_JSON_RESPONSE");
	
	//v0.27
	var vSuccess = [];
	if (vSelection.length == 0)
	{
		//v0.27
		plw.alert(#@"DBS_TK_OCM_ERR_INVALID".current);
		return;
	}
	
	var bInvalid = false; //use flag to track if at least one of the selected SOBJ records are not valid
	var nSuccess = 0;
	var proj = undefined;
	for (var oSobj in vSelection)
	{
		//v0.28: if the selected record is invalid, we don't run the import and we set flag to alert the user
		if (oSOBJ.DBS_UA_B_VALID)
		{
			proj = dbs_ocmr_create_or_modify_project_for_sobj(oSobj, oFormat, true);
			if (proj != false)
			{
				nSuccess++;
				//v0.27
				vSuccess.push(proj);
				
			}
		}
		else
			bInvalid = true;
	}
	
	//v0.28: if at least one of the selected records is invalid, we alert the user
	if (bInvalid)
		plw.alert(plw.write_text_key("DBS_TK_OCM-R_NOT_VALID_PROJECT"));
	
	//v0.27
	for (var oSucc in vSuccess)
	{
		plw.alert(oSucc.NAME + " : " + #@"DBS_TK_MSG_PROJECT_HAS_BEEN_UPDATED".current);
	}

	plw.writetolog("[INFO][dbs_ocmr_modify_projects_selected] end function ");
}

//v0.22 function created
function dbs_ocmr_get_selected_sobj()
{
	"This function returns a vector containing each SOBJ from the OCM-R import table, that was selected"
	
	var vSelection = new Vector();

	for (var oSelected in plw.selection_get() where oSelected.get("DBS_UT_OCM-R_PEI_IMP").get("NAME") == "")
	{
		vSelection.push(oSelected);
	}

	return vSelection;
	
}

//v0.22 function created
//v0.27 added text keys
//v0.34 changed name mapping
//v0.35 use project template
//v0.38 allowed multi-attribute mapping
//v0.41 moved booking element creation here
//v0.43 added budget lines
//v0.53 adjusted mapping
//v0.54 allow linking existing projects to sobj datasets
function dbs_ocmr_create_or_modify_project_for_sobj(_sobj, plc.impexformat _oFormat, bModify)
{
	"This function handles project creation/modification for one SOBJ from the OCM-R import table
	@param _sobj Object: SOBJ to be created/modified
	@param _oFormat Object: impex-format containing the parameter mapping from OCM-R import to project attributes
	@param bModify Boolean: if true, modify the project, else, create it
	"
	

	plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] begin function ");


	
	var vPlist = undefined;
	var oProject = undefined;
	var oReturn = undefined;
	var act = undefined;
	var o_temp = undefined;
	var suser = "";
    
	var plc.impexformat oFormat = _oFormat;
	
	plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] fetching impex format ");

	var vImpexAtt = oFormat.get("IMPEX-ATTRIBUTES");

	//Mapping
	//v0.34
	var sName = _sobj.DBS_UA_S_NUMBER;
	var sDesc = "";
	//v0.35
	var oTemplate = context.DBS_OCM_SET_DEF_PRJ_TMP;

	if(context.applet.user.name != "" && context.applet.user.name != undefined)
		suser = context.applet.user.name;

	//v0.54
	if (_sobj.DBS_UA_S_SOBJ_PROJ=="") // a project is not yet linked to the OCM-R Data set
	{
		plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] No Project is linked, check if a project has been manually created");
		oProject = plc.Ordo_Project.get(sName);
		if (oProject != undefined) //Project exists
		{
			plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] Project "+oProject.NAME+ " existed but was not linked, linking it to the OCM dataset ");
			_sobj.DBS_UA_S_SOBJ_PROJ = oProject.NAME; 
			//_sobj.DBS_AA_S_PROJEKTLEITER = oProject.OWNER;
			//sOwner = _sobj.DBS_AA_S_PROJEKTLEITER;
		}
	}
	else
	{
		oProject = plc.Ordo_Project.get(_sobj.DBS_UA_S_SOBJ_PROJ);

	}

	if (oProject != undefined && !bModify)
	{
		//v0.27
		plw.alert(sName + " : " + #@"DBS_TK_OCM_ERR_IMPORTED".current);
		return false;
	}
	if (oProject == undefined && bModify)
	{
		//v0.27
		plw.alert(sName + " : " + #@"DBS_TK_OCM-R_ALERT_PROJ_NOT_EXIST".current);
		return false;
	}
	plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] project does not yet exist");

	plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] vImpexAtt length: " + vImpexAtt.length);

	vPlist = new vector();

	var sAttName = "";
	var sAttValue = "";
	var vAttNames = undefined;
	//v0.53
	for (var plc.impexattribute oAtt in vImpexAtt where "plc.ordoproject" in oAtt.DBS_UA_S_TARGET_ENTITY.toLowerCase().split(","))
	{

		//v0.38
		vAttNames = oAtt.DBS_UA_S_TARGET_FIELD.split(",");
		
		for (var sAttName in vAttNames)
		{
			sAttValue = _sobj.get(oAtt.PLANISWARE_ATTRIBUTE);
			plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] found attribute " + sAttName + " with value " + sAttValue);

			if (sAttValue != undefined)
			{
				vPlist.push(sAttName);
				vPlist.push(sAttValue);

			}
		}
	}
	
	plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] vPlist softcoded: " + vPlist.tostring());

	/* v0.34
	if (sName!=undefined)
	{
		vPlist.push("NAME");
		vPlist.push(sName);
	}
	*/

	if (oTemplate!=undefined)
	{
		vPlist.push("TEMPLATE");
		vPlist.push(oTemplate);
	}
	

	if (oProject == undefined)
	{
		try
		{
			oProject = plc.Ordo_Project.makeopx2objectwithplist(vPlist);
		}
		catch (error e)
		{
			//V0.29
			_sobj.DBS_UA_B_IMPORT_ERROR=true;

			plw.writetolog("[ERROR][dbs_ocmr_create_or_modify_project_for_sobj] " + e.tostring());
			//v0.27
			plw.alert(sName + " : " + #@"DBS_TK_OCM-R_ALERT_ERR_PROJ_CREATE".current);
			oProject = false;
		}
	}
	else
	{

	
		//v0.54	
		if (oProject.NAME != sName)
		{
			plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] linked project has a different name. concatenating descriptions");
			var nDescIndex = 0;

			while (nDescIndex < vPlist.length && vPlist[nDescIndex] != "DESC")
			{
				nDescIndex++;
			}

			if (nDescIndex + 1 < vPlist.length)
			{
				sDesc = vPlist[nDescIndex+1];
				sDesc = oProject.DESC + " - " + sDesc;
				vPlist[nDescIndex+1] = sDesc;
				plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] new description: " + sDesc);
			}
		}

		try
		{
			oProject.modifywithplist(vPlist);
		}
		catch (error e)
		{
			//V0.29
			_sobj.DBS_UA_B_IMPORT_ERROR=true;

			plw.writetolog("[ERROR][dbs_ocmr_create_or_modify_project_for_sobj] " + e.tostring());
			//v0.27
			plw.alert(sName + " : " + #@"DBS_TK_OCM-R_ALERT_ERR_PROJ_MODIFY".current);
			oProject = false;
		}
		
		//v0.43
		dbs_ocmr_delete_existed_ocmr_budgetline(oProject, "DBS_ET_OCM_BUDGET");
		dbs_ocmr_delete_existed_ocmr_budgetline(oProject, "DBS_ET_PEI_HK");
		
	}


	
	if (oProject instanceof plc.OrdoProject && oProject != undefined)
	{

		//v0.43
		dbs_ocmr_create_ocmr_budgetline(oProject,_sobj,new symbol("DBS_ET_OCM_BUDGET","KEYWORD"),_sobj.DBS_UA_N_U_NET_TOTAL_SALES_PRICE);
		dbs_ocmr_create_ocmr_budgetline(oProject,_sobj,new symbol("DBS_ET_PEI_HK","KEYWORD"),_sobj.DBS_UA_N_U_TOTAL_COST);

		/*
		dbsocm_create_budget_planned_expenditure(oProject,_sobj,act,new symbol("DBS_ET_OCM_BUDGET","KEYWORD"),_sobj.DBS_AA_N_VERKAUFSPREIS_NETTO);
		dbsocm_create_budget_planned_expenditure(oProject,_sobj,act,new symbol("DBS_ET_PEI_HK","KEYWORD"),_sobj.DBS_AA_N_TOTAL_COST);
		*/

		//v0.41
		dbs_ocmr_pei_create_or_modify_booking_elements(oProject, _sobj, oFormat, undefined);

		//v0.30
		dbs_ocmr_import_peis_for_sobj(oProject, _sobj, oFormat);

		//v0.54
		_sobj.DBS_UA_S_SOBJ_PROJ = oProject.NAME;

		//V0.29
		_sobj.DBS_UA_B_IMPORTED=true;
	}

	oReturn = oProject;
	plw.writetolog("[INFO][dbs_ocmr_create_or_modify_project_for_sobj] end function ");
	

	return oReturn;
}

//v0.30 function created
//v0.40 added booking element creation
//v0.41 moved booking element creation to project creation
function dbs_ocmr_import_peis_for_sobj(plc.OrdoProject oProject, oSobj, plc.impexformat oFormat)
{
	"This function imports the PEI elements associated with a given SOBJ into the project that was created from the SOBJ
	@param oProject Object: Project to import the PEIs into
	@param oSobj Object: parent SOBJ of the PEIs to be imported
	@param oFormat Object: Impex format containing the attribute mapping for the PEIs
	"

	plw.writetolog("[INFO][dbs_ocmr_import_peis_for_sobj] begin function ");
	if (oSobj == undefined)
		return false;

	var oClass = oSobj.findClass();

	var vSuccessEsc = new vector();
	var vFailEsc = new vector();

	var vSuccessPe = new vector();
	var vFailPe = new vector();


	var oEsc = undefined;
	var oPe = undefined;
	var oBooking = undefined;

	
	plw.writetolog("[INFO][dbs_ocmr_import_peis_for_sobj] begin main loop ");
	for (var oPei in oClass where oPei.get("DBS_UT_OCM-R_PEI_IMP").get("DBS_UT_OCM-R_PEI_IMP") == oSobj)
	{
		plw.writetolog("[INFO][dbs_ocmr_import_peis_for_sobj] importing PEI " + oPei.NAME);
		if (oPei.DBS_UA_B_PEI_IS_VALID)
		{
			oEsc = dbs_ocmr_pei_create_or_modify_escalations(oProject, oPei, oFormat);
			if (oEsc != false)
				vSuccessEsc.push(oPei.NAME);
			else
				vFailEsc.push(oPei.NAME);

		}

		if (oPei.DBS_UA_B_PEI_IS_VALID_PLEX)
		{
			oPe = dbs_ocmr_create_or_modify_plex_for_pei(oProject, oPei, oFormat);

			if (oPe != false)
				vSuccessPe.push(oPei.NAME);
			else
				vFailPe.push(oPei.NAME);
		}
	}
	plw.writetolog("[INFO][dbs_ocmr_import_peis_for_sobj] end main loop ");

	if (vSuccessEsc.length>0)
	{			
		plw.alert(vSuccessEsc.length.toString("####")+" "+#@"DBS_TK_MSG_CREATE_ESC".current+" : "+vSuccessEsc.join(", "));
	}
	if (vFailEsc.length>0)
	{			
		plw.alert(vFailEsc.length.toString("####")+" "+#@"DBS_TK_MSG_PEI_FAILED_IMPORT_ESC".current+" : "+vFailEsc.join(", "));
	}

	if (vSuccessPe.length>0)
	{			
		plw.alert(vSuccessPe.length.toString("####")+" "+#@"DBS_TK_MSG_CREATE_PLX".current+" : "+vSuccessPe.join(", "));
	}
	if (vFailPe.length>0)
	{			
		plw.alert(vFailPe.length.toString("####")+" "+#@"DBS_TK_LB_PEI_OCM_ERROR_PLX".current+" : "+vFailPe.join(", "));
	}

	plw.writetolog("[INFO][dbs_ocmr_import_peis_for_sobj] end function ");
}

//v0.19: not OCMR specific
function DBS_JS2_GET_BS_BY_FIELD_AND_VALUE(sBsName,sBsField,sValue)
{
	"This function returns a breakdown structure object using the following parameters:
	1. the breakdown structure name ('OBS', '1BS', etc)
	2. the field name in that breakdown structure ('NAME', 'DESC', etc)
	3. the value of that field"
	
	var oSlot = undefined;
	var sClass = "opx"+sBsName;
	var oClass = undefined; //sClass.findClass();
	var vAttributes = new vector(); //new vector(sBsField);
	var vValues = new vector(); //new vector(sValue);
	var oKeyAtt = undefined;
	
	//v0.28: added checks to confirm parameter values are valid
	if (sBsName != undefined && sBsName != "")
	{
		oClass = sClass.findClass();
		
		if (oClass instanceof objectclass)
		{
			if (sBsField != undefined && sBsField != "")
			{
				vAttributes.push(sBsField);
				if (sValue != undefined && sValue != "")
				{
					vValues.push(sValue);
					oKeyAtt = new keyattribute(oClass,vAttributes,vValues);
					with (oKeyAtt.fromObject())
					{
						for(var oEach in sClass.findClass())
						{
							oSlot = oEach.getSlot(sBsField);
							if (oSlot instanceof objectattribute)
							{
								if (oEach.get(oSlot.representation.toString()) == sValue)
									return true;
							}
						}
					}
				}
			}
		}
	}
	
	return false;
}
DBS_JS2_GET_BS_BY_FIELD_AND_VALUE.exportfunction(["STRING","STRING","STRING"],"BOOLEAN","This function returns a breakdown structure object");


//v0.31 called on display of table cell in OCM Import Data > All OCM-R style
function DBS_JS2_OCMR_BACKGROUND_COLOR_DEFINE(ObjectGrid grid)
{
	plw.writetolog("[INFO][DBS_JS2_OCMR_BACKGROUND_COLOR_DEFINE] start: "+this);
	grid.addColorHandler(function(s) { return DBS_JS2_OCMR_BACKGROUND_COLOR_HANDLER(this, s); });
}

//v0.31 called from DBS_JS2_OCMR_BACKGROUND_COLOR_DEFINE
cached function DBS_JS2_OCMR_BACKGROUND_COLOR_HANDLER(plc.__USER_TABLE_DBS_UT_OCM_R_PEI_IMP oSOBJ, ObjectAttribute oSlot)
{
	"This function displays the background colors in the 'All OCM-R' style of the OCM Imoort Data page"
	
	//plw.writetolog("[INFO][DBS_JS2_OCMR_BACKGROUND_COLOR_HANDLER] start: "+oSOBJ);
	if (oSOBJ.USER_ATTRIBUTE_DBS_UA_B_IMPORTED == true)
	{
		//plw.writetolog("[INFO][DBS_JS2_OCMR_BACKGROUND_COLOR_HANDLER] IMPORTED");
		return plw.color(140,200,140);		// Already imported - shadow
	}
	else
	if (oSOBJ.USER_ATTRIBUTE_DBS_UA_B_IMPORT_ERROR == true)
	{
		//plw.writetolog("[INFO][DBS_JS2_OCMR_BACKGROUND_COLOR_HANDLER] ERROR");
		return plw.color(250,100,100);		// An error in this line occured
	}
	
	//otherwise, no color
	return false;
}


//v0.24 function created
function dbs_ocmr_get_selected_pei()
{
	"This function returns a vector containing each PEI from the OCM-R import table, that was selected"
	
	var vSelection = new Vector();

	for (var oSelected in plw.selection_get() where oSelected.get("DBS_UT_OCM-R_PEI_IMP").get("NAME") != ""
							&& oSelected.get("DBS_UT_OCM-R_PEI_IMP").get("DBS_UT_OCM-R_PEI_IMP").get("NAME") != ""
							&& oSelected.get("DBS_UT_OCM-R_PEI_IMP").get("DBS_UT_OCM-R_PEI_IMP").get("DBS_UT_OCM-R_PEI_IMP").get("NAME") == "")
	{
		vSelection.push(oSelected);
	}

	return vSelection;
}

//v0.24 function created
//v0.27 added text keys
function dbs_ocmr_create_pei_plex_selected()
{
	"This function attempts to create a new planned expenditure for each PEI from the OCM-R import table, that was selected"
	
	plw.writetolog("[INFO][dbs_ocmr_create_pei_plex_selected] begin function ");
	
	var vSelection = dbs_ocmr_get_selected_pei();

	if (vSelection.length == 0)
	{
		//v0.27
		plw.alert(#@"DBS_TK_MSG_INVALID_PEI_IMPORT_CANCELLED".current);
		return;
	}
	
	//v0.27
	var vSuccess = [];
	var vFail = [];

	var nSuccess = 0;
	var oPe = undefined;
	var oProj = undefined;
	var oFormat = plc.impexformat.get("[OCM-R] PEI Import:OCM-R_IMP_JSON_RESPONSE");
	for (var oPei in vSelection)
	{
		oProj = dbs_ocmr_get_project_for_pei(oPei);
		oPe = dbs_ocmr_create_or_modify_plex_for_pei(oProj, oPei, oFormat);
		if (oPe != false)
		{
			nSuccess++;
			//v0.27
			vSuccess.push(oPei.NAME);
		}
		else
		{
			//v0.27
			vFail.push(oPei.NAME);
		}
	}

	//v0.27
	if (vSuccess.length>0)
	{			
		plw.alert(vSuccess.length.toString("####")+" "+#@"DBS_TK_MSG_CREATE_PLX".current+" : "+vSuccess.join(", "));
	}
	if (vFail.length>0)
	{			
		plw.alert(vFail.length.toString("####")+" "+#@"DBS_TK_LB_PEI_OCM_ERROR_PLX".current+" : "+vFail.join(", "));
	}

	plw.writetolog("[INFO][dbs_ocmr_create_pei_plex_selected] end function ");
}

//v0.21 function created
//v0.24 function reworked
//v0.38 allowed multi-attribute mapping
//v0.53 adjusted mapping
function dbs_ocmr_create_or_modify_plex_for_pei(plc.OrdoProject oProject, plc.__user_table_dbs_ut_ocm_r_pei_imp oImport, plc.impexformat _oFormat)
{

	"This function attempts to modify a planned expenditure in the passed project, based on the passed PEI, or create a new one if it doesn't exist yet
	@param oProject Object: Project, where the PE should be created/modified
	@param oImport Object: PEI record to create the PE from
	@param _oFormat Object: Impex format containing the mapping between PEI and PE attributes
	"
	
	var plc.workstructure oActivity = undefined;
	var vector vSuccess = new vector();
	var sExtendibleSymbol = "Extendible";//new symbol("EXTENDIBLE","KEYWORD");

	var oFormat = _oFormat;

	var vImpexAtt = oFormat.get("IMPEX-ATTRIBUTES");
	
	if (oProject instanceof plc.ordoproject)
	{
		plw.writetolog("[INFO][dbs_ocmr_create_or_modify_plex_for_pei] project name: " +oProject.name);
		
		oActivity = oProject.getInternalValue(#TOPLEVEL-WS#).car;
		if (oActivity instanceof plc.workstructure)
		{
			plw.writetolog("[INFO][dbs_ocmr_create_or_modify_plex_for_pei] level 1 activity name: " +oActivity.name);
			
			if (oImport.DBS_UA_B_PEI_IS_VALID_PLEX) //v0.28: added data quality check to confirm PEI is valid for planned expenditures
			{
				var oPei = oImport;
				var oExists = dbs_ocmr_find_pei_planned_expenditures(oPei, oProject);
				var sPeiId = oPei.DBS_UA_S_U_PEI_ID;
				var sPeName = oPei.DBS_UA_S_U_PE_NAME;
				
				var vector vPlist = [
					"WORK-STRUCTURE", oActivity.name,
					"UNIT", "EUR",
					"COMMENT", sPeName + " | " + sPeiId,
					//"START-DATE", oProject.OD,
					//"END-DATE", oProject.ED,
					"DURATION-COMPUTATION", sExtendibleSymbol,
					"TYPE", "STANDARD",
					"DATASET", oProject.name,
				];	
				plw.writetolog("[INFO][dbs_ocmr_create_or_modify_plex_for_pei] vPlist hardcoded: " + vPlist);
				var sAttName = "";
				var sAttValue = "";
				var vAttNames = undefined;
				//v0.53
				for (var plc.impexattribute oAtt in vImpexAtt where "plc.expenditure" in oAtt.DBS_UA_S_TARGET_ENTITY.toLowerCase().split(","))
				{
					//v0.38
					vAttNames = oAtt.DBS_UA_S_TARGET_FIELD_PLEX.split(",");
		
					for (var sAttName in vAttNames)
					{
						sAttValue = oPei.get(oAtt.PLANISWARE_ATTRIBUTE);
						plw.writetolog("[INFO][dbs_ocmr_create_or_modify_plex_for_pei] found attribute " + sAttName + " with value " + sAttValue);
						if (sAttValue != undefined)
						{
							vPlist.push(sAttName);
							vPlist.push(sAttValue);
						}
					}
				}
				plw.writetolog("[INFO][dbs_ocmr_create_or_modify_plex_for_pei] vPlist softcoded: " + vPlist);

				if(oExists == undefined)
				{
					//expenditure not found, so we create new one
					plw.writetolog("[INFO][dbs_ocmr_create_or_modify_plex_for_pei] found expenditure: " +oExists);
					

					try 
					{
						var oNewPE = plc.EXPENDITURE.makeOPX2ObjectWithPlist(vPlist);
						if (oNewPE instanceof plc.EXPENDITURE)
						{
							vSuccess.push(oPEI.printattribute());
							
						}
						//V0.29
						oImport.DBS_UA_B_IMPORTED = true;
					}
					catch (error e)
					{
						
						plw.writetolog("[ERROR][dbs_ocmr_create_or_modify_plex_for_pei] creation error: "+oActivity.name+":"+ e.tostring());
						//V0.29
						oImport.DBS_UA_B_IMPORT_ERROR = true;
						return false;
					}
					finally{
						//nothing to do
					}
				}
				else
				{
					//update of the planned expenditure
					plw.writetolog("[INFO][dbs_ocmr_create_or_modify_plex_for_pei] creating new expenditure for: "+oActivity.name);
					
						
					try
					{
						
						oExists.modifywithplist(vPlist);
						vSuccess.push(oPEI.printattribute());
						//V0.29
						oImport.DBS_UA_B_IMPORTED = true;
					}
					catch(error e)
					{
						
						plw.writetolog("[ERROR][dbs_ocmr_create_or_modify_plex_for_pei] modification error "+oActivity.name+":"+ e.tostring());
						//V0.29
						oImport.DBS_UA_B_IMPORT_ERROR = true;
						return false;
					}
					finally{
						//nothing to do
					}
				}
				
			//we need to add these user attributes
			/*
				oPEI.DBS_AA_B_PEI_IMPORTED = true;
				oPEI.DBS_AA_D_PEI_TIMESTAMP = new date();
			*/
			}
			else
			{
				plw.writetolog("[ERROR][dbs_ocmr_create_or_modify_plex_for_pei] the selected record is not valid for planned expenditure creation: " + oImport);
				return false;
			}
		}
		else
		{
			plw.writetolog("[ERROR][dbs_ocmr_create_or_modify_plex_for_pei] invalid activity: " + oActivity);
			return false;
		}
		
	}
	else
	{
		plw.writetolog("[ERROR][dbs_ocmr_create_or_modify_plex_for_pei] invalid project: " + oProject);
		return false;
	}
}


//v0.21 function created
//v0.24 function reworked
function dbs_ocmr_find_pei_planned_expenditures(oPei, oProject)
{
	"This function attempts to find a planned expenditure matching the passed PEI record within the passed Project
	@param oPei Object: PEI record to find a matching PE for
	@param oProject Object: Project, within which to look for a planned expenditure
	"
	var oActivity  = undefined;
	
	if(oProject instanceof plc.ordoproject)
	{
		oActivity = oProject.getInternalValue(#TOPLEVEL-WS#).car;
		if(oActivity instanceof plc.workstructure)
		{
			var sPeiId = oPei.DBS_UA_S_U_PEI_ID;
			var sPeName = oPei.DBS_UA_S_U_PE_NAME;
			plw.writetolog("[INFO][dbs_ocmr_find_pei_planned_expenditures] PEI ID: '"+sPeiId+"' and PE NAME: '"+sPeName+"'");
			
			//this assumes we are using the existing 'OCM' user attributes on planned expenditure class
			for (var plc.EXPENDITURE oPE in plc.EXPENDITURE where oPE.project == oProject && 
																	oPE.activity == oActivity && 
																	oPE.DBS_L1_AA_S_PEIID == sPeiId && 
																	oPE.DBS_L1_AA_S_PENAME == sPeName)
			{
				return oPE;
			}
		}
	}
	
	return undefined;
}

//v0.21 function created
//v0.24 function reworked
//v0.34 name mapping changed
function dbs_ocmr_get_project_for_pei(oPei)
{
	"This function attempts to find the project associated with a PEI record, if that project already exists
	@param oPei Object: PEI record to find the associated project for
	"

	var oReturn = undefined;
	//v0.34
	var sName = oPei.get("DBS_UT_OCM-R_PEI_IMP").get("DBS_UT_OCM-R_PEI_IMP").get("DBS_UA_S_NUMBER");
	if (sName != undefined || sName != "")
		oReturn = plc.ordoproject.get(sName);
	
	return oReturn;
}

//v0.32 function created
//v0.34 name mapping changed
function dbs_ocmr_get_project_for_ocmr()
{
	"This function finds the project associated with any OCM-R element in the DBS_UT_OCM-R_PEI_IMP table"

	var oSobj = this;
	var oParent = oSobj.get("DBS_UT_OCM-R_PEI_IMP");

	while (oParent != undefined && oParent.get("NAME") != "")
	{
		oSobj = oParent;
		oParent = oSobj.get("DBS_UT_OCM-R_PEI_IMP");
	}

	//v0.34
	var sName = oSobj.get("DBS_UA_S_NUMBER");

	var oProject = undefined;
	var sReturn = "";
	if (sName != undefined && sName != "")
	{
		oProject = plc.ordoproject.get(sName);
	}

	if (oProject != undefined)
	{
		sReturn = oProject.get("NAME");
	}

	return sReturn;
}

dbs_ocmr_get_project_for_ocmr.exportfunction([], "STRING", "Finds the project associated with an OCM-R element");

//v0.26 function created
//v0.27 added text keys
//v0.33 added imported/error flags
function dbs_ocmr_create_pei_escalations_selected()
{

	"This function attempts to create a new escalation for each PEI from the OCM-R import table, that was selected"
	
	plw.writetolog("[INFO][dbs_ocmr_create_pei_escalations_selected] begin function ");
	
	var vSelection = dbs_ocmr_get_selected_pei();
	
	if (vSelection.length == 0)
	{
		//v0.27
		plw.alert(#@"DBS_TK_MSG_INVALID_PEI_IMPORT_CANCELLED".current);
		return;
	}
	
	//v0.27	
	var vSuccess = [];
	var vFail = [];

	var nSuccess = 0;
	var oEsc = undefined;
	var oProj = undefined;
	var oFormat = plc.impexformat.get("[OCM-R] PEI Import:OCM-R_IMP_JSON_RESPONSE");
	for (var oPei in vSelection)
	{
		oProj = dbs_ocmr_get_project_for_pei(oPei);
		oEsc = dbs_ocmr_pei_create_or_modify_escalations(oProj, oPei, oFormat);
		if (oEsc != false)
		{
			nSuccess++;
			//v0.27
			vSuccess.push(oPei.NAME);
			//v0.33
			oPei.DBS_UA_B_IMPORTED = true;
			
			
		}
		else
		{
			//v0.27
			vFail.push(oPei.NAME);
			//v0.33
			oPei.DBS_UA_B_IMPORT_ERROR = true;
		}
	}
	
	//v0.27
	if (vSuccess.length>0)
	{			
		plw.alert(vSuccess.length.toString("####")+" "+#@"DBS_TK_MSG_CREATE_ESC".current+" : "+vSuccess.join(", "));
	}
	if (vFail.length>0)
	{			
		plw.alert(vFail.length.toString("####")+" "+#@"DBS_TK_MSG_PEI_FAILED_IMPORT_ESC".current+" : "+vFail.join(", "));
	}

	plw.writetolog("[INFO][dbs_ocmr_create_pei_escalations_selected] end function ");
}


//v0.52 function created
function dbs_ocmr_find_escalation_for_pei(oProj, oRes, oUnit, oCbs2)
{

	"This function attempts to find an existing escalation in a given project based on resource, cost unit, skillset and CBS2
	@param oProj Object: Project to search for an escalation
	@param oRes Object: the escalation should have this resource
	@param oUnit Object: the escalation should have this other unit
	@param oCbs2 Object: the escalation should have this rate center 2
	"	

	plw.writetolog("[INFO][dbs_ocmr_find_escalation_for_pei] function start");
	var o_prj = oProj;
	var o_res = oRes;
	var o_unit = oUnit;
	
	var o_cbs2 = oCbs2;
	plw.writetolog("[INFO][dbs_ocmr_find_escalation_for_pei] the value of unit+  "+o_unit+" - "+o_prj+" - "+o_res+" - "+o_cbs2);
	
	if (o_prj == undefined || o_res == undefined || o_unit==undefined || o_cbs2==undefined) 
		return undefined;
	
	var v_attributes = ["PROJECT","RESOURCE","OTHER_UNIT","RATE_CENTER2"];
	var v_values = [o_prj,o_res,o_unit,o_cbs2];
	var keyatt = new keyattribute(plc.EXCEPTION_VALUE,v_attributes,v_values);
	
	with (keyatt.fromObject()) 
	{
		for (var elt in plc.EXCEPTION_VALUE where elt.PROJECT == o_prj && 
							elt.resource==o_res && 
							elt.other_unit==o_unit && 
							elt.RATE_CENTER2==o_cbs2) 
		{
			plw.writetolog("[INFO][dbs_ocmr_find_escalation_for_pei] escalation found: " + elt);
			return elt;
		}
	}
	plw.writetolog("[INFO][dbs_ocmr_find_escalation_for_pei] escalation not found");
	
	plw.writetolog("[INFO][dbs_ocmr_find_escalation_for_pei] function end");
	return undefined;
}

//v0.52 function created
function dbs_ocmr_get_unit_for_pei(oPei)
{
	"This function attempts to find the cost unit for a PEI
	@param oPei Object: base PEI to find a cost unit for
	"

	plw.writetolog("[INFO][dbs_ocmr_get_unit_for_pei] function start");
	var o_unit = undefined;
	var s_unit_name = oPei.DBS_UA_S_ROLE;
	
	var v_attributes = new vector("DBS_UA_S_COST_UNIT_OCMR_CODE");
	
	var v_values = new vector(s_unit_name);
	var keyatt = new keyattribute(plc.COST_UNIT,v_attributes,v_values);

		
	// search by ocm-r code
	if (s_unit_name != "")
	{
		plw.writetolog("[INFO][dbs_ocmr_get_unit_for_pei] searching by OCM-R code: " + s_unit_name);
		// search by OCM-R Code
		with(keyatt.fromobject()) for (var elt in plc.COST_UNIT where elt.DBS_UA_S_COST_UNIT_OCMR_CODE == s_unit_name)
		{
			o_unit = elt;
			break;
		}
	}
	else
	{
		plw.writetolog("[INFO][dbs_ocmr_get_unit_for_pei] PEI has no OCM-R code, search not possible");
	}

	// backup if not found by ocm-r code: find by pe name
	if (o_unit == undefined)
	{
		s_unit_name = oPei.DBS_UA_S_U_PE_NAME;
		v_attributes = new vector("DESC");
	
		v_values = new vector(s_unit_name);
		keyatt = new keyattribute(plc.COST_UNIT,v_attributes,v_values);

		if (s_unit_name != "")
		{
			plw.writetolog("[INFO][dbs_ocmr_get_unit_for_pei] searching by PE Name: " + s_unit_name);
			with(keyatt.fromobject()) for (var elt in plc.COST_UNIT where elt.DESC == s_unit_name)
			{
				o_unit = elt;
				break;
			}
		}
	}
	else
	{
		plw.writetolog("[INFO][dbs_ocmr_get_unit_for_pei] PEI has no PE Name, search not possible");
	}


	plw.writetolog("[INFO][dbs_ocmr_get_unit_for_pei] cost unit found: " + o_unit);

	plw.writetolog("[INFO][dbs_ocmr_get_unit_for_pei] function end");
	return o_unit;
}

//v0.52 function created
function dbs_ocmr_get_skill_for_pei(oPei){

	"This function attempts to find the skill for a given PEI
	@param oPei Object: base PEI to find a skill for
	"

	plw.writetolog("[INFO][dbs_ocmr_get_skill_for_pei] function start");
	if (oPei.DBS_UA_S_SKILLSET == undefined || oPei.DBS_UA_S_SKILLSET.trim(" ")=="") return dbs_ocmr_get_default_skill_for_pei();// returns the default skill if defined else undefined
	var s_skill_name = oPei.DBS_UA_S_SKILLSET;
	
	var v_attributes = new vector("DBS_UA_S_SKILL_OCMR_CODE");
	var v_values = new vector(s_skill_name);
	var keyatt = new keyattribute(plc.RESOURCESKILL,v_attributes,v_values);
	
	
	var o_skill = undefined;
	if (o_skill == undefined){
		// search by OCM-R Code
		plw.writetolog("[INFO][dbs_ocmr_get_skill_for_pei] search by OCM-R code: " + s_skill_name);
		//PMGT-794 - Performance optimization tested Ok
		with (keyatt.fromobject()) for (var elt in plc.RESOURCESKILL where elt.DBS_UA_S_SKILL_OCMR_CODE == s_skill_name){
			o_skill = elt;
			break;
		}
	}

	plw.writetolog("[INFO][dbs_ocmr_get_skill_for_pei] function end");

	// if no skill found then search for "Default Skill for PEI OCM Import" skill
	return (o_skill == undefined) ? dbs_ocmr_get_default_skill_for_pei() : o_skill;
}

//v0.52 function created
function dbs_ocmr_get_default_res_for_ocmr_data(){
	"This function attempts to find the default resource for OCM-R data"

	plw.writetolog("[INFO][dbs_ocmr_get_default_res_for_ocmr_data] looking for default resource");

	var tkey = new keyattribute(plc.RESOURCE,"DBS_L1_AA_B_PEI_DFLT_OCM_DATA",true);
	with(tkey.fromobject()) for (var res in plc.RESOURCE where res.DBS_L1_AA_B_PEI_DFLT_OCM_DATA == true) return res;
	return undefined;
}

//v0.52 function created
function dbs_ocmr_create_cbs2_for_pei(oPei,_sName,_sDesc,_sFile,_sParent){

	"This function creates a new CBS2 for a PEI with exception handling
	@param oPei Object: base PEI to create a new CBS2 for
	@param _sName String: name of the new CBS2
	@param _sDesc String: description of the new CBS2
	@param _sFile String: file of the new CBS2
	@param _sParent String: parent of the new CBS2
	"

	plw.writetolog("[INFO][dbs_ocmr_create_cbs2_for_pei] function start");
	var plist = new vector();
	plist.push("FILE");
	plist.push(_sFile);
	plist.push("NAME");
	plist.push(_sName);
	plist.push("DESC");
	plist.push(_sDesc);
	plist.push("PARENT");
	plist.push(_sParent);
	
	var oCBS2 = undefined;
		
	try{
		oCBS2 = plc._INF_PT_CBS2.MakeOPX2ObjectWithPList(plist);
		plw.writetolog("[INFO][dbs_ocmr_create_cbs2_for_pei] creation of CBS2 element for PEI "+_sName+" : ");
	}catch(error e){
		plw.writetolog("[ERROR][dbs_ocmr_create_cbs2_for_pei] Error while creating CBS2 element for PEI "+oPei+" : "+plw.char(10)+e);
		return false;
	}
		
	plw.writetolog("[INFO][dbs_ocmr_create_cbs2_for_pei] function end");
	return oCBS2;
}

//v0.52 function created
function dbs_ocmr_find_cbs2_for_pei(oPei, oProj){
	

	"This function attempts to find the CBS2 for a given PEI, or creates a new one, including parent elements, if one is not found
	@param oPei Object: base PEI to find a CBS2 for
	@param oProj Object: Project to write newly created CBS2 into, if necessary
	"

	plw.writetolog("[INFO][dbs_ocmr_find_cbs2_for_pei] function start");
	var o_prj = oProj;

	//if the project doesn't exist we quit
	if (o_prj == undefined ) return undefined;
	
	var oCBS2Root = undefined;
	var oCBS2Parent = undefined;
	var oCBS2 = undefined;

	//setting defining the default CBS2 element for DB Systel, stored in DBS_DATA which is COST_ACCOUNT_2_DEFAULT_FILE
	oCBS2Root = plc._INF_PT_CBS2.get(context.DBS_SET_DEF_CBS2_ELEMENT);
	var sCBS2Rootfile = context.COST_ACCOUNT_2_DEFAULT_FILE;
	var sRootParent = "";
	
	//if the root element doesn't exist, we have to create it
	if (oCBS2Root == undefined)
	{
		plw.writetolog("[INFO][dbs_ocmr_find_cbs2_for_pei] creating root cbs2 for pei: "+oPei);
		//parameters: name, description, file, parent
		oCBS2Root = dbs_ocmr_create_cbs2_for_pei(oPei,context.DBS_SET_DEF_CBS2_ELEMENT,context.DBS_SET_DEF_CBS2_ELEMENT,sCBS2Rootfile,sRootParent);
	}

	//the PEI CBS2 elements are the children of a project, stored in the project; here we make sure that the project element exists
	oCBS2Parent = plc._INF_PT_CBS2.get(o_prj.name);
	
	var sCBS2Parentname = o_prj.name;
	var sCBS2Parentdesc = o_prj.desc;
	var sCBS2Parentfile = o_prj;
	
	//if the project element doesn't exist, we have to create it
	if (oCBS2Parent == undefined && oCBS2Root != undefined)
	{
		plw.writetolog("[INFO][dbs_ocmr_find_cbs2_for_pei] creating parent cbs2 for pei: "+oPei);
		oCBS2Parent = dbs_ocmr_create_cbs2_for_pei(oPei,sCBS2Parentname,sCBS2Parentdesc,sCBS2Parentfile,oCBS2Root);
	}
	
	//in case we might have to create the CBS2 elements with different names, e.g. PEI+project, we simply change it here
	var sCBS2Name = oPei.DBS_UA_S_NUMBER;
	var sCBS2Desc = oPei.DBS_UA_S_U_PE_NAME;
	
	//the PEI CBS2 elements are the children of a project, stored in the project	
	var oCBS2 = plc._INF_PT_CBS2.get(sCBS2Name);
	
	if (oCBS2 == undefined && oCBS2Root != undefined && oCBS2Parent != undefined)
	{
		plw.writetolog("[INFO][dbs_ocmr_find_cbs2_for_pei] creating cbs2 for pei: "+oPei);
		oCBS2 = dbs_ocmr_create_cbs2_for_pei(oPei,sCBS2Name,sCBS2Desc,sCBS2Parentfile,oCBS2Parent);
	}
	
	plw.writetolog("[INFO][dbs_ocmr_find_cbs2_for_pei] function end");
	return oCBS2;
}


//v0.52 function created
function dbs_ocmr_get_default_skill_for_pei(){

	"This function attempts to find the default skill for PEIs"

	plw.writetolog("[INFO][dbs_ocmr_get_default_skill_for_pei] searching for default PEI skill");

	var default_skill_id = undefined;//plw.gettemporaryattribute("DEFAULT_SKILL_ID_FOR_PEI_OCM_IMP");
	if (default_skill_id == false || default_skill_id == undefined){
		for (var o_skill in plc.RESOURCESKILL where o_skill.DBS_L1_AA_B_DEFAULT_FOR_PEI_OCM_IMP){
			default_skill_id = o_skill.printattribute();
			//default_skill_id.storetemporaryObject("DEFAULT_SKILL_ID_FOR_PEI_OCM_IMP");
			break;
		}
	}
	var o_skill = plc.RESOURCESKILL.get(default_skill_id);
	return o_skill;
}

//v0.52 function created
//v0.53 moved part of value calculation
function dbs_ocmr_calc_value_for_pei(oPei, oUnit)
{

	"This function attempts to calculate the value of the escalation for a given PEI and cost unit
	@param oPei Object: base PEI to calculate escalation value for
	@param oUnit Object: cost unit to use in escalation value calculation
	"

	plw.writetolog("[INFO][dbs_ocmr_calc_value_for_pei] function start");

	var nValue = 1;
	
	var nQuantity = 0;
	var nUnitCost = 1;


	if (oPei != undefined)
		nQuantity = oPei.DBS_UA_N_QUANTITY_IN_DAYS;

	plw.writetolog("[INFO][dbs_ocmr_calc_value_for_pei] nQuantity * nUnitCost = "+ (nQuantity*nUnitCost) );
	if (nQuantity > 0 && nUnitCost > 0)
	{

		if (oPei.DBS_UA_N_U_TARGET_PRICE > 0)
		{
			plw.writetolog("[INFO][dbs_ocmr_calc_value_for_pei] doing calculation based on target price");
			nValue = oPei.DBS_UA_N_U_TARGET_PRICE / (nQuantity * nUnitCost);
		}
		else if (oPei.DBS_UA_N_U_PRICE > 0)
		{
			plw.writetolog("[INFO][dbs_ocmr_calc_value_for_pei] target price invalid, doing calculation based on price");
			nValue = oPei.DBS_UA_N_U_PRICE / (nQuantity * nUnitCost);

		}
		else
		{
			plw.writetolog("[INFO][dbs_ocmr_calc_value_for_pei] no valid price field, calculation not possible");

		}
	}

	plw.writetolog("[INFO][dbs_ocmr_calc_value_for_pei] found value: " + nValue);

	plw.writetolog("[INFO][dbs_ocmr_calc_value_for_pei] function end");
	return nValue;
}

//v0.26 function created
//v0.38 allowed multi-attribute mapping
//v0.52 escalation creation functional
//v0.53 adjusted mapping
function dbs_ocmr_pei_create_or_modify_escalations(oProject, oPei, oFormat)
{

	"This function attempts to modify an escalation in the passed project, based on the passed PEI, or create a new one if it doesn't exist yet
	@param oProject Object: Project, where the escalation should be created/modified
	@param oPei Object: PEI record to create the escalation from
	@param oFormat Object: Impex format containing the mapping between PEI and escalation attributes
	"
	
	plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_escalations] start of function for: "+oPei);
	var o_ocm = undefined;
	var o_prj = oProject;
	var o_default_res = dbs_ocmr_get_default_res_for_ocmr_data();
	var o_unit = dbs_ocmr_get_unit_for_pei(oPei);
	var sUnit = undefined;
	var o_skill = dbs_ocmr_get_skill_for_pei(oPei);
	var oCBS2 = dbs_ocmr_find_cbs2_for_pei(oPei, o_prj);
	var o_value = dbs_ocmr_calc_value_for_pei(oPei, o_unit);
	var o_sd = undefined;
	var o_escalations = dbs_ocmr_find_escalation_for_pei(o_prj, o_default_res, o_unit, oCBS2);
	//var o_escalations = undefined;
	var oPosition = oPei.get("DBS_UT_OCM-R_PEI_IMP");
	
	o_ocm = oPei.get("DBS_UT_OCM-R_PEI_IMP").get("DBS_UT_OCM-R_PEI_IMP");
	
	//if oPei is not valid or related OCM is not valid then we do not do the integration of the PEI (PGMT-727)
	if (o_ocm == undefined || o_ocm.get("NAME") == "") //|| o_ocm.DBS_NF_B_VALID == false || oPei.DBS_NF_B_PEI_IS_VALID == false)
	{
		plw.writetolog("[ERROR][dbs_ocmr_pei_create_or_modify_escalations] invalid PEI or linked OCM: "+oPei);
		return false;
	}
	else
		plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_escalations] the linked OCM record: "+o_ocm);
	
	if (o_prj == undefined)
	{
		plw.writetolog("[ERROR][dbs_ocmr_pei_create_or_modify_escalations] the project is not found for: "+oPei);
		return false;
	}
	else
		plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_escalations] project: "+o_prj);
	
	if (oPei.DBS_UA_B_PEI_IS_VALID) //v0.28: added data quality check to confirm PEI is valid for escalations
	{
		var plist = new vector();
		plist.push("FILE");
		plist.push(o_prj);
		plist.push("RATE_CENTER");// project field
		plist.push(o_prj.printattribute());
		plist.push("RESOURCE");
		plist.push(o_default_res);
		plist.push("OTHER_UNIT");
		plist.push(o_unit);
		plist.push("DBS_RA_ESCALATION_SKILL");
		plist.push(o_skill);
		plist.push("RATE_CENTER2");// CBS2 field
		plist.push(oCBS2.printattribute());
		plist.push("RATE_TABLE");
		plist.push("Standard");
		plist.push("VALUE");
		plist.push(1);
		//v0.53
		plist.push("DBS_L1_AA_S_BUDLEPOS");
		plist.push(oPosition.DBS_UA_S_NUMBER);
		plist.push("DBS_L1_AA_S_BUNDLEBESCHREIBUNG");
		plist.push(oPosition.DBS_UA_S_U_DESCRIPTION);
		plist.push("DBS_L1_AA_S_INTERNESPSPELT");
		plist.push(oPosition.DBS_UA_S_U_INTERNAL_PSP_ELEMENT);

		var vImpexAtt = oFormat.get("IMPEX-ATTRIBUTES");
		
		var sAttName = "";
		var sAttValue = "";
		var vAttNames = undefined;
		//v0.53
		for (var plc.impexattribute oAtt in vImpexAtt where "plc.exception_value" in oAtt.DBS_UA_S_TARGET_ENTITY.toLowerCase().split(","))
		{
			//v0.38
			vAttNames = oAtt.DBS_UA_S_TARGET_FIELD_ESC.split(",");
		
			for (var sAttName in vAttNames)
			{
				sAttValue = oPei.get(oAtt.PLANISWARE_ATTRIBUTE);
				plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_escalations] found attribute " + sAttName + " with value " + sAttValue);
				if (sAttValue != undefined)
				{
					plist.push(sAttName);
					plist.push(sAttValue);
				}
			}
		}
		
		if (o_escalations == undefined)
		{
			plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_escalations] no escalations were found, creating new escalation");
			//creation of the escalation
			try
			{
				o_escalations = plc.EXCEPTION_VALUE.makeOPX2ObjectWithPlist(plist);

				
			}
			catch(error e)
			{
				plw.writetolog("[ERROR][dbs_ocmr_pei_create_or_modify_escalations] error while creating plc.EXCEPTION_VALUE for PEI: "+e);
				oPei.DBS_UA_B_IMPORT_ERROR = true;
				return false;
			}
		}
		else
		{
			plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_escalations] an escalation was found, updating: "+o_escalations);
			//update of the escalation
			try
			{
				o_escalations.modifywithplist(plist);
			}
			catch(error e)
			{
				plw.writetolog("[ERROR][dbs_ocmr_pei_create_or_modify_escalations] error while modifying plc.EXCEPTION_VALUE for PEI: "+e);
				oPei.DBS_UA_B_IMPORT_ERROR = true;
				return false;
			}
		}
		
		if(o_escalations instanceof plc.EXCEPTION_VALUE)
		{
			plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_escalations] the escalation is valid: "+o_escalations);

	
			oPei.DBS_UA_B_IMPORTED = true;
			//v0.53
			o_escalations.VALUE = o_value / o_escalations.DBS_L1_AA_N_ESC_VALUE_UNIT;
		}
		else
		{
			plw.writetolog("[ERROR][dbs_ocmr_pei_create_or_modify_escalations] escalation was not created for PEI: " + oPei);
			oPei.DBS_UA_B_IMPORT_ERROR = true;
			return false;
		}
		// if the function has not already returned then the integration is OK ==> return true
		return true;
	}
	else
	{
		plw.writetolog("[ERROR][dbs_ocmr_pei_create_or_modify_escalations] the PEI is not valid for escalation creation: " + oPei);
		oPei.DBS_UA_B_IMPORT_ERROR = true;
		return false;
	}

	
}
//v0.40 function created
//v0.41 removed validity checks, added tree structure
// create or modify buchungselement if possible, returns true if treatment is OK and false if it fails
function dbs_ocmr_pei_create_or_modify_booking_elements(oProject, oPei, oFormat, oParent)
{
	"Creates a booking element for the given PEI element in the given Project
	@param oProject Object: Target project for the booking element
	@param oPei Object: PEI element to create a booking element from
	@param oFormat Object: Impex format containing attribute mappings
	"

	var o_prj = oProject;
	var s_longid = undefined;
	var o_buchungselement = undefined;



	//var o_buchungselement = this.dbs_pei_find_buchungselement_for_pei_method();
	
	// if no project, creation or update is not possible ==> failed
	if (o_prj == undefined)
	{
		plw.writetolog("[ERROR][dbs_ocmr_pei_create_or_modify_booking_elements] Can't find project for PEI "+oPei);
		return false;
	}
	else
	{
		//s_longid = o_prj.printattribute()+":"+this.DBS_UA_S_U_INTERNAL_PSP_ELEMENT;
		s_longid = oPei.DBS_UA_S_U_INTERNAL_PSP_ELEMENT;
		with (o_prj.fromobject())
		{
			for (var o_book in plc.dbs_pt_buchungselement where o_book.NAME == s_longid)
				o_buchungselement = o_book;
		}
	}
	var plist = new vector();

	if (s_longid != undefined && s_longid != "")
	{
		plist.push("FILE");
		plist.push(o_prj);
		plist.push("NAME");
		plist.push(s_longid);
		plist.push("DESC");
		plist.push(s_longid);
		plist.push("DBS_AA_B_BEBUCHBAR");
		plist.push(true);
		if (oParent != undefined)
		{
			plist.push("PARENT");
			plist.push(oParent);		
		}

		if (o_buchungselement == undefined)
		{
			// create buchungselement
			try
			{
				o_buchungselement = plc.dbs_pt_buchungselement.MakeOPX2ObjectWithPList(plist);
				plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_booking_elements] created new booking element: "+o_buchungselement);
			}
			catch(error e)
			{
				plw.writetolog("[ERROR][dbs_ocmr_pei_create_or_modify_booking_elements] Error while creating plc.dbs_pt_buchungselement for PEI "+oPei+" : "+plw.char(10)+e);
				return false;
			}
		}
		else
		{
			plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_booking_elements] booking element already exists: "+o_buchungselement);
			try
			{
				o_buchungselement.modifywithplist(plist);
				plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_booking_elements] modified booking element: "+o_buchungselement);
			}
			catch(error e)
			{
				plw.writetolog("[ERROR][dbs_ocmr_pei_create_or_modify_booking_elements] Error while modifying plc.dbs_pt_buchungselement for PEI "+oPei+" : "+plw.char(10)+e);
				return false;
			}
		}
	}
	else
	{
		// if the name is empty, we do not create but it's normal ==> return true anyway, else creation of the buchungselement
		plw.writetolog("[INFO][dbs_ocmr_pei_create_or_modify_booking_elements] Can't create an plc.dbs_pt_buchungselement with empty name for PEI "+oPei);
	}
	
	var b_result = true;
	var b_result_child = true;
	// if buchungselement does not exist ==> failed
	if (o_buchungselement == undefined)
	{
		o_buchungselement = oParent;
		b_result = false;
	}

	for (var oChild in plc.__user_table_dbs_ut_ocm_r_pei_imp where oChild.get("dbs_ut_ocm-r_pei_imp").get("NAME") == oPei.NAME)
	{
		b_result_child = dbs_ocmr_pei_create_or_modify_booking_elements(oProject, oChild, oFormat, o_buchungselement);
		b_result = b_result && b_result_child;
	}

	// if the function has not already returned then the integration is OK ==> return true 
	return b_result;
}

//v0.43 function created
function dbs_ocmr_create_ocmr_budgetline(plc.OrdoProject _oProject, plc.__USER_TABLE_DBS_UT_OCM_R_PEI_IMP _sobj, STRING exptype, NUMBER qty)
{
	"This function creates a budget line from a SOBJ
	@param _oProject Object: Target project to contain the new budget line
	@param _sobj Object: Source SOBJ containing the data for the budget line
	@param exptype String: Expenditure type of the budget line to be created
	@param qty Number: quantity of the budget line to be created
	"
	var oProject = _oProject;
	
	if(oProject)
	{
		// Level attribute is not on attribute leist for entity Activity when creating a key attribute, break statement should be sufficient
		// To do in DBS-57
		
		//V2 optimization: prj.getInternalValue(#TOPLEVEL-WS#).car returns the toplevel element of the project
		var act = oProject.getInternalValue(#TOPLEVEL-WS#).car;
		plw.writetolog("[INFO][dbs_ocmr_create_ocmr_budgetline] Budget line creation on activity level1: " +act.name);
		
		//making sure that act is an activity, exptype is an expenditure type and qty is a number (which might be 0)
		if(act && exptype)
		{				
			var ObjectClass cls = plc.BUDGETLINE;
			var Vector o_blvect = [
					#WORK-STRUCTURE#, act.name,
					#UNIT#, "EUR",
					#START-DATE#, oProject.OD,
					#END-DATE#, oProject.ED,
					#QTY#, qty,
					#TYPE#, exptype]; 		//same as #DBS_ET_OCM_BUDGET# and #DBS_ET_PEI_HK#
			
			try 
			{
				var new_bl = cls.makeOPX2ObjectWithPlist(o_blvect);
			}
			catch (error e)
			{
				plw.writetolog("[ERROR][dbs_ocmr_create_ocmr_budgetline] Error creating budget line on "+act.name+" : "+ e.tostring());
				plw.alert("Budget line on "+act.name+" was not created");
				return false;
			}
			
			plw.writetolog("[INFO][dbs_ocmr_create_ocmr_budgetline] Budget line created");
		}
	}
}

//v0.43 function created
function dbs_ocmr_delete_existed_ocmr_budgetline(plc.OrdoProject _oProject, STRING exptype)
{
	"This function deletes budgetlines of a given type in the given project
	@param _oProject Object: Target project
	@param exptype String: delete budget lines of this type
	"

	plw.writetolog("[INFO][dbs_ocmr_delete_existed_ocmr_budgetline] Begin function");
	var oProject = _oProject;
	//var plc.virtualdataset o_dataSet = new plc.virtualdataset();
	//o_dataSet.selected_datasets = oProject;
	var exp_type = plc.EXPENDITURE_TYPE.get(exptype).DESC;
	
	with (oProject.fromObject())
	{
		for (var plc.BUDGETLINE x in plc.BUDGETLINE where x.TYPE == exp_type)
		{
			x.delete();
		}
	}
	plw.writetolog("[INFO][dbs_ocmr_delete_existed_ocmr_budgetline] End function");
}

//v0.44
function dbs_ocmr_create_master_data_all()
{
	"This function creates the necessary master data for all SOBJ in the OCM-R import table"


	//v0.48
	for (var oSobj in plc.__user_table_dbs_ut_ocm_r_pei_imp WHERE oSobj.DBS_UT_OCM_R_PEI_IMP.NAME == "" && !oSobj.DBS_UA_B_VALID)
	{
		dbs_ocmr_create_master_data(oSobj);
	}
}

//v0.44 function created
//v0.48 removed hardcodings
//v0.49 use correct default files
function dbs_ocmr_create_master_data(plc.__user_table_dbs_ut_ocm_r_pei_imp _oSobj)
{
	"This function automatically creates the necessary master data to import the given SOBJ as a Project
	@param _oSobj Object: SOBJ containing the data to be created"

	plw.writetolog("[INFO][dbs_ocmr_create_master_data] Begin function");

	var vPlist = undefined;
	var sName = "";

	//v0.48 new varbiables
	var String sDBCustomer  = context.DBS_CUSET_OCMR_DB_CUSTOMER;
	var String sContractType = context.DBS_CUSET_OCMR_CONTRACT_TYPE;
	var String sPortfolioSegments = context.DBS_CUSET_OCMR_DB_PORTFOLIO_SEGMENTS;
	var String sBusinessModelType = context.DBS_CUSET_OCMR_DB_BUSINESS_MODEL_TYPE;
	var String sDBSystel = context.DBS_CUSET_OCMR_DB_SYSTEL;
	var String sCustomerFile = context.CUSTOMERS_DEFAULT_FILE;

	//v0.49 new default files
	var String sContractTypeFile = context.BREAKDOWN_2_FILE;
	var String sPortfolioSegmentsFile = context.BREAKDOWN_4_FILE;
	var String sBusinessModelTypeFile = context.BREAKDOWN_6_FILE;
	var String sOBSFile = context.OBS_DEFAULT_FILE;

	if (!_oSobj.DBS_UA_B_BS0_VALID)
	{
		vPlist = new Vector();
		sName = _oSobj.DBS_UA_S_U_CUSTOMER;

		if (sName != "" && sName != undefined)
		{
			vPlist.push("NAME");
			vPlist.push(sName);
			vPlist.push("DESC");
			vPlist.push(sName);
			vPlist.push("BREAKDOWN_0");
			vPlist.push(sDBCustomer);
			vPlist.push("DATASET");
			vPlist.push(sCustomerFile);

			plw.writetolog("[INFO][dbs_ocmr_create_master_data] Creating new BS0: " + sName);
			try
			{
				plc.0BS.makeopx2objectwithplist(vPlist);
			}
			catch (error e)
			{
				plw.writetolog("[INFO][dbs_ocmr_create_master_data] Error while creating new BS0: " + e.tostring());
				
			}
		}
	}

	if (!_oSobj.DBS_UA_B_BS2_VALID)
	{
		vPlist = new Vector();
		sName = _oSobj.DBS_UA_S_U_CONTRACT_TYPE;

		if (sName != "" && sName != undefined)
		{
			vPlist.push("NAME");
			vPlist.push(sName);
			vPlist.push("DESC");
			vPlist.push(sName);
			vPlist.push("BREAKDOWN_2");
			vPlist.push(sContractType);
			vPlist.push("DATASET");
			vPlist.push(sContractTypeFile);

			plw.writetolog("[INFO][dbs_ocmr_create_master_data] Creating new BS2: " + sName);
			try
			{
				plc.2BS.makeopx2objectwithplist(vPlist);
			}
			catch (error e)
			{
				plw.writetolog("[INFO][dbs_ocmr_create_master_data] Error while creating new BS2: " + e.tostring());
				
			}
		}
	}
	
	
	if (!_oSobj.DBS_UA_B_BS4_VALID)
	{
		vPlist = new Vector();
		sName = _oSobj.DBS_UA_S_U_PORTFOLIO_SEGMENT;

		if (sName != "" && sName != undefined)
		{
			vPlist.push("NAME");
			vPlist.push(sName);
			vPlist.push("DESC");
			vPlist.push(sName);
			vPlist.push("BREAKDOWN_4");
			vPlist.push(sPortfolioSegments);
			vPlist.push("DATASET");
			vPlist.push(sPortfolioSegmentsFile);

			plw.writetolog("[INFO][dbs_ocmr_create_master_data] Creating new BS4: " + sName);
			try
			{
				plc.4BS.makeopx2objectwithplist(vPlist);
			}
			catch (error e)
			{
				plw.writetolog("[INFO][dbs_ocmr_create_master_data] Error while creating new BS4: " + e.tostring());
				
			}
		}
	}

	if (!_oSobj.DBS_UA_B_BS6_VALID)
	{
		vPlist = new Vector();
		sName = _oSobj.DBS_UA_S_U_BUSINESS_MODEL_TYPE;

		if (sName != "" && sName != undefined)
		{
			vPlist.push("NAME");
			vPlist.push(sName);
			vPlist.push("DESC");
			vPlist.push(sName);
			vPlist.push("BREAKDOWN_6");
			vPlist.push(sBusinessModelType);
			vPlist.push("DATASET");
			vPlist.push(sBusinessModelTypeFile);

			plw.writetolog("[INFO][dbs_ocmr_create_master_data] Creating new BS6: " + sName);
			try
			{
				plc.6BS.makeopx2objectwithplist(vPlist);
			}
			catch (error e)
			{
				plw.writetolog("[INFO][dbs_ocmr_create_master_data] Error while creating new BS6: " + e.tostring());
				
			}
		}
	}

	if (!_oSobj.DBS_UA_B_OBS_VALID)
	{
		vPlist = new Vector();
		sName = _oSobj.DBS_UA_S_U_SALES_AREA;

		if (sName != "" && sName != undefined)
		{
			vPlist.push("NAME");
			vPlist.push(sName);
			vPlist.push("DESC");
			vPlist.push(sName);
			vPlist.push("OBS_ELEMENT");
			vPlist.push(sDBSystel);
			vPlist.push("DATASET");
			vPlist.push(sOBSFile);

			plw.writetolog("[INFO][dbs_ocmr_create_master_data] Creating new OBS element: " + sName);
			try
			{
				plc.OBS_NODE.makeopx2objectwithplist(vPlist);
			}
			catch (error e)
			{
				plw.writetolog("[INFO][dbs_ocmr_create_master_data] Error while creating new OBS element: " + e.tostring());
				
			}
		}
	}

	if (!_oSobj.DBS_UA_B_PRJ_LEAD_VALID)
	{
		vPlist = new Vector();
		sName = _oSobj.DBS_UA_S_U_PROJECT_LEADING_ORGANIZATIONAL_UNIT;

		if (sName != "" && sName != undefined)
		{
			vPlist.push("NAME");
			vPlist.push(sName);
			vPlist.push("DESC");
			vPlist.push(sName);
			vPlist.push("OBS_ELEMENT");
			vPlist.push(sDBSystel);
			vPlist.push("DATASET");
			vPlist.push(sOBSFile);

			plw.writetolog("[INFO][dbs_ocmr_create_master_data] Creating new OBS element: " + sName);
			try
			{
				plc.OBS_NODE.makeopx2objectwithplist(vPlist);
			}
			catch (error e)
			{
				plw.writetolog("[INFO][dbs_ocmr_create_master_data] Error while creating new OBS element: " + e.tostring());
				
			}
		}
	}
	plw.writetolog("[INFO][dbs_ocmr_create_master_data] End function");
}

//v0.50 function created
//v0.51 function updated
function dbs_ocmr_add_user_to_sobj_resp_toolbar_button()
{
	"this function opens the pop-up that allows users to assign an SOBJ to themselves"
	//open the popup
	var link = new hyperlink ("fvalue",
		"Attribute","Identifier",
		"EditorType",context.DBS_CUSET_OCMR_ASSIGN_SOBJ_REPORT, //v0.51 added customer setting
		"popup",true);
	link.go(context);
}

//v0.50 function created
//v0.51 function updated
function dbs_ocmr_add_user_to_sobj_resp_report_button()
{
	"this function is called when the OK button on the pop-up that allows you to assign an SOBJ to yourself is pressed"

	var sSobjs = context.DBS_RA_OCM_R_SOBJ_TO_ASSIGN;
	var sUser = context.applet.user.name;

	//v0.51 allow selecting multiple SOBJ
	var vSobjs = sSobjs.split(",");
	for (var sSobj in vSobjs)
	{
 
		var oSobj = plc.__user_table_dbs_ut_ocm_r_pei_imp.get(sSobj);
		if (oSobj != undefined)
		{
			//only add user if they are not in the list already
			if (!(sUser in oSobj.DBS_RA_OCMR_RESP.split(',')))
			{
				
				if(oSobj.DBS_RA_OCMR_RESP != ""){
					oSobj.DBS_RA_OCMR_RESP = oSobj.DBS_RA_OCMR_RESP+","+sUser;
				}else{
					oSobj.DBS_RA_OCMR_RESP = sUser;
				}
				plw.alert(#@"DBS_TK_S_ADD_SOBJ_USER".current +sUser+ " " + #@"DBS_TK_S_ADD_SOBJ_U_ADDED".current+oSobj.DBS_UA_S_NUMBER);
				
			}
		}

	}
	/*else
	{
		plw.alert(#@"DBS_TK_S_ADD_SOBJ_SELECT".current);

	}
	*/

}

//v0.45 function created
function dbs_ocmr_modify_projects_all()
{
	"This function attempts to modify the corresponding project for each SOBJ from the OCM-R import table, that has a linked project"
	
	plw.writetolog("[INFO][dbs_ocmr_modify_projects_all] begin function ");
	
	var vSelection = dbs_ocmr_get_imported_sobj();
	var oFormat = plc.impexformat.get("[OCM-R] PEI Import:OCM-R_IMP_JSON_RESPONSE");
	
	var vSuccess = [];
	if (vSelection.length == 0)
	{
		plw.writetolog("[INFO][dbs_ocmr_modify_projects_all] " + #@"DBS_TK_OCM_ERR_INVALID".current);
		return;
	}
	
	var bInvalid = false; //use flag to track if at least one of the selected SOBJ records are not valid
	var nSuccess = 0;
	var proj = undefined;
	for (var oSobj in vSelection)
	{
		if (oSOBJ.DBS_UA_B_VALID)
		{
			proj = dbs_ocmr_create_or_modify_project_for_sobj(oSobj, oFormat, true);
			if (proj != false)
			{
				nSuccess++;
				//v0.27
				vSuccess.push(proj);
				
			}
		}
		else
			bInvalid = true;
	}
	
	if (bInvalid)
		plw.writetolog("[INFO][dbs_ocmr_modify_projects_all] " + plw.write_text_key("DBS_TK_OCM-R_NOT_VALID_PROJECT"));
	
	for (var oSucc in vSuccess)
	{
		plw.writetolog("[INFO][dbs_ocmr_modify_projects_all] " + oSucc.NAME + " : " + #@"DBS_TK_MSG_PROJECT_HAS_BEEN_UPDATED".current);
	}

	plw.writetolog("[INFO][dbs_ocmr_modify_projects_all] end function ");
}

//v0.47 function created
function dbs_ocmr_get_imported_sobj()
{
	"This function returns a vector containing each SOBJ from the OCM-R import table, that has a linked project"
	
	var vSelection = new Vector();

	for (var oSelected in plc.__user_table_dbs_ut_ocm_r_pei_imp where oSelected.DBS_UT_OCM_R_PEI_IMP.NAME == "" && oSelected.DBS_UA_B_LINKED_PROJECT_EXISTS)
	{
		vSelection.push(oSelected);
	}

	return vSelection;
	
}
