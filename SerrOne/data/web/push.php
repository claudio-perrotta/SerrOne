<?php
// start push - "pushed onto" database
echo "<div>Connessione...</div>\r\n";

// client ip address
$ip = $_SERVER['HTTP_CLIENT_IP']?:($_SERVER['HTTP_X_FORWARDE‌​D_FOR']?:$_SERVER['REMOTE_ADDR']);
echo "<div>Client IP: {$ip}</div>\r\n"; 

// set the default timezone to use. Available since PHP 5.1
date_default_timezone_set('UTC');

$reg_date = date_timestamp_get(date_create());
$timestamp = date('Y-m-d G:i:s', $reg_date);

// prints something like: Wed, 25 Sep 2013 15:28:57 -0700
echo "<div>" . date(DATE_RFC2822, $reg_date) . "</div>\r\n";

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
        User TEXT,
        JSON_S TEXT,
        Time_Stamp TIMESTAMP
        )";

    // use exec() because no results are returned
    $db->exec($sql);
    echo "<div>Tabella creata o gia` esistente</div>\r\n";
}
catch(PDOException $e) {
    echo "<div>Error: " . $e->getMessage() . "</div>\r\n";
}

try {
    // Now we prepare a query
    $statement = $db->prepare("INSERT INTO `devices`(`IP`, `User`, `JSON_S`, `Time_Stamp`) VALUES (:a, :b, :c, :d)");

    // Queries from URL
    $queries = array();
    //echo "<pre>".print_r($_SERVER, TRUE)."</pre>\r\n";
    parse_str($_SERVER['QUERY_STRING'], $queries);
    $token = $queries['token'];
    $user  = $queries['user'];

    // JSON from input
    $jsondata = file_get_contents("php://input");
    if ($jsondata === FALSE) {
        die("<div>Something went wrong with file_get_contents()</div>\r\n");
    }
    $data = (array) json_decode($jsondata, true, 5, JSON_BIGINT_AS_STRING);
    if (!$data) {
        die("<div>Something went wrong with json_decode()</div>\r\n");
    }
    echo "<pre>".print_r($data, TRUE)."</pre>\r\n";

    // We can use $statement to insert data
    $statement->execute(array(
        ':a' => $ip,
        ':b' => $user,
        ':c' => $jsondata,
        ':d' => $timestamp
    ));
    echo "<div>IP: {$ip} | Token: {$token} | User: {$user} | Timestamp: {$timestamp}</div>\r\n";
}
catch(PDOException $e) {
    echo "<div>Error: " . $e->getMessage() . "</div>\r\n";
}

$db = null;
echo "<div>Operazione completata!</div>\r\n";
?>