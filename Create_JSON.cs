// create json
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