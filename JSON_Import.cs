// JSON Import
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