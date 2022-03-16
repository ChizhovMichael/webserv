<?php

session_start();
// var_dump( $_SESSION);
if (isset($_SESSION['sess_key'])) {
    echo $_SESSION['sess_key'] . '<br>';
}
if (isset($_SESSION['sess_key1'])) {
    echo $_SESSION['sess_key1'] . '<br>';
}
