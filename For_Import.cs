// String object for import
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