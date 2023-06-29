// create file
function dbs_ocmr_create_json_file(sFileName, sWriteMode, sResult)
{
	"This function accepts a Filepath and Writemode for writing in the JSON, creates the JSON and returns a filepath of the created JSON"
	var pathname oPath = undefined;

	plw.writetolog("[INFO][dbs_ocmr_create_json_file] json file path: " + sFileName);	 // JSON file path is ____ file name_____

	oPath = plw.generateTemporaryFile(sFileName); // opath is being set to a temp file with file name
	plw.writetolog("[INFO][dbs_ocmr_create_json_file] oPath: "+oPath); // print that path
	
	if(oPath instanceof pathname) // if oPath is the type we originally assigned it on line 5 and file name exists
	{
		var fileioLib.oFile oUpdatedSobjIds = new fileioLib.oFile(filepath : oPath,mode : sWriteMode); // updated string object ids are assigned to be written like JSON key value pairs of Path and write mode
		plw.writetolog("[INFO][dbs_ocmr_create_json_file] fileoutputstream: "+oUpdatedSobjIds); // print the key value pairs
		
		with (oUpdatedSobjIds) // ??????????????
		{
			try {
				oUpdatedSobjIds.write(sResult); // write the username, password, and URL from JSON and printed as a Vector in the file with oPath/File path in write mode and the content is sting object ids
				plw.writetolog("[INFO][dbs_ocmr_create_json_file] 'write' was successful!"); // creating a file was successful
			}
			catch (error e){
				plw.writetolog("[ERROR][dbs_ocmr_create_json_file] error when trying to write to file: "+e); // print error
			}
		}
		
		//uncomment for testing if needed
		//plw.downloadFileFromServer(oPath);
	}
	return oPath;
}