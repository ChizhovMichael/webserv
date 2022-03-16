<?php

setcookie("cookie_example","some_text");

session_start();

$_SESSION['sess_key'] = "sess_value";
// $_SESSION['sess_key1'] = "sess_value1";

phpinfo();
