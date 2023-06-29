// get json response
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
