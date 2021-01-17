#!/bin/php
<?php
if($argc != 2){ echo "Usage: ./gen_bcrypt_hash.php <PASSPHRASE>\n"; exit(1); }
echo "BCrypt hash(for $argv[1]): ".password_hash($argv[1], PASSWORD_BCRYPT)."\n";
?>
