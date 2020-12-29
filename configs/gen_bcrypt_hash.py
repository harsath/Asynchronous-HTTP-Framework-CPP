import bcrypt 
import sys
password_to_hash = str.encode(sys.argv[1])
bcrypt_hash = bcrypt.hashpw(password_to_hash, bcrypt.gensalt());
print(f"HASH: {bcrypt_hash.decode('utf-8')}")
