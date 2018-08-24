<?php
// (pull from client) start pop - "popped off" database
//echo "<div>Connessione...</div>\r\n";

// client ip address
$ip = $_SERVER['HTTP_CLIENT_IP']?:($_SERVER['HTTP_X_FORWARDE‌​D_FOR']?:$_SERVER['REMOTE_ADDR']);
//echo "<div>Client IP: {$ip}</div>\r\n"; 

// set the default timezone to use. Available since PHP 5.1
date_default_timezone_set('UTC');

$reg_date = date_timestamp_get(date_create());
$timestamp = date('Y-m-d G:i:s', $reg_date);

// prints something like: Wed, 25 Sep 2013 15:28:57 -0700
//echo "<div>" . date(DATE_RFC2822, $reg_date) . "</div>\r\n";

// Set these to your login data
$DB_HOST = "127.0.0.1";
$DB_USER = "claudius";
$DB_PASS = "";
$DB_NAME = "my_claudius";

    // We connect to the database using the values above
    $db = new PDO("mysql:host=$DB_HOST;dbname=$DB_NAME", DB_USER, DB_PASS);

    // We tell PDO to report us every error
    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

try {
    // sql to create table
    $sql = "CREATE TABLE IF NOT EXISTS `devices` (
        id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
        IP CHAR(45),
        Token TEXT,
        JSON_S TEXT,
        Time_Stamp TIMESTAMP
        )";

    // use exec() because no results are returned
    $db->exec($sql);
    //echo "<div>Tabella creata o gia` esistente</div>\r\n";
}
catch(PDOException $e) {
    echo "<div>Error: " . $e->getMessage() . "</div>\r\n";
}

try {
    // Getting the transferred data
    $obj = json_decode($_GET["x"], false);

    // Now we prepare a query
    $statement = $db->prepare("SELECT * FROM `devices` WHERE `id`=(SELECT MAX(`id`) FROM `devices`);");
    $statement->execute();

    // Exercise PDOStatement::fetch styles
    $result = $statement->fetch(PDO::FETCH_ASSOC);

    // Echo
    //echo "<div>id: ".$result["id"]." | IP: ".$result["IP"]." | Token: ".$result["Token"]." | Time stamp: ".$result["Time_Stamp"]."</div>\r\n";

    // Result JSON to next function
    $jsondata = $result["JSON_S"];
    header("Content-Type: application/json; charset=UTF-8");
    echo $jsondata;
}
catch(PDOException $e) {
    echo "<div>Error: " . $e->getMessage() . "</div>\r\n";
}

$db = null;
//echo "<div>Operazione completata!</div>\r\n";
?>