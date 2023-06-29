// get json response
function dbs_ocmr_authenticate_and_get_json_response(sUser, sPassword, sUrl)
{
	var plc.BasicAuthorization oAuth = new plc.BasicAuthorization(#user#,sUser,#password#,sPassword); // looks like key values {user: "username"}
	plw.writetolog("[INFO][dbs_ocmr_authenticate_and_get_json_response] authorization: "+oAuth); //print get JSON auth {username: "username", password: "password"}

	var Vector sResult = new vector();	
	try
	{
		if (oAuth instanceof plc.BasicAuthorization) // if variable oAuth is of type that it was assigned to be 
			sResult = rest.json_get(sUrl,oAuth); // then set this variable to the json URL and the username and password and url in that json
	}
	catch (error e) // on the off chance this somehow fails
	{
		plw.writetolog("[ERROR][dbs_ocmr_authenticate_and_get_json_response] error calling rest api: "+e); // print failure with label of this function
	}
	plw.writetolog("[INFO][dbs_ocmr_authenticate_and_get_json_response] return: "+sResult); // print the assigned value of result 
	return sResult; // return the result 

}

// get username, password, and URL from JSON and print as a Vector 