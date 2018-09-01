<?php
// Queries from URL
$devID = $_POST['devID'];
$ison  = $_POST['ison'];
$user  = "4bbfe5";

// HTTP request
$url = "https://cloud.arest.io/{$user}/device?params={{$devID},{$ison}}";
// $data = array('key1' => 'value1', 'key2' => 'value2');

// // use key 'http' even if you send the request to https://...
// $options = array(
//     'http' => array(
//         'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
//         'method'  => 'POST',
//         'content' => http_build_query($data)
//     )
// );
// $context  = stream_context_create($options);
// $response = file_get_contents($url, false, $context);
// if ($response === FALSE) { /* Handle error */ }

// var_dump($response);

$response = file_get_contents($url);
echo $response;
?>