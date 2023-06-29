// JSON Import
function dbs_ocmr_json_import(oTarget, oFormat, sFunctionName) {
	"This function accepts a target file and a format on how to import"
	if (oTarget instanceof plc.impextarget) { // 
		try{
			oFormat.DoImportWithFormatAndTarget(oTarget,true); // ????????????
			plw.writetolog("[INFO][dbs_ocmr_json_import] import is finished"); // print this function's name + done
			return "[INFO]["+sFunctionName+"] import is finished"; // print the main function's name + done
		}
		catch (error e){
			plw.writetolog("[ERROR][dbs_ocmr_json_import] error when trying to import the file: "+e); // print the error
			return "[ERROR]["+sFunctionName+"] error when trying to import the file: "+e; // print/return the error???????
		}
	}
}
