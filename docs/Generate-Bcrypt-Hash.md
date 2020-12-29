##### How to use REST API's Auth(HTTP Basic Auth) with this framework?

Every password will be stored as a BCrypt Hash digest on the server for security and privacy reasons. 
We are usign BCrypt Password Hashing function [BCrypt Password Hash](https://en.wikipedia.org/wiki/Bcrypt)

Here I will show you how to actually generate the hash and use it for for the REST API endpoints.

First, Generate a BCrypt Hash using a python script provided inside the `./configs/gen_bcrypt_hash.py`
Example, If your about to generate a BCrypt hash for the password `Password987!^` for example:
	`$ python3 ./config/gen_bcrypt_hash.py 'Password987!^'`

Now, Copy the hash. You need to create a JSON file with Username and Password credentials for the REST API Framework to use.
Example:
	```json
		{
			"/post_endpoint" : {
						"user_name_one" : "HASH_FROM_SCRIPT",
						"user_name_two" : "HASH_FROM_SCRIPT"
				           } 

		}
	```

In this case, `/post_endpoint` is the REST API or server's resource which you want a user-agent/client to authenticate itself to access.
If you have any questions, you can checkout a sample from `../test/internal/sample_userpass_cred.json` and `../test/internal/sample_userpass_cred_bcrypt.json`

In your case, You need to generate something similar to `../test/internal/sample_userpass_cred_bcrypt.json`
