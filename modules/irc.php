<?php

// 
// IRC API
//

define('PUN_ROOT', dirname(__FILE__).'/');
define('PUN_MAIN_IRC', 1);

/* Don't forget to modify the key */
define("IRC_KEYSALT", "THIS IS THE KEY YOU NEED TO MODIFY AND ADD INTO YOUR CONFIGURATION");

require PUN_ROOT.'include/common.php';

function auth()
{
  $key = md5(md5(IRC_KEYSALT) . $_GET['cmd']);
  if (!isset($_GET['key']))
    die('ERR');
  if ($_GET['key'] != $key)
    die('ERR');
  // Key is good, script continues :-)
}

/* New posts */
if ($_GET['cmd'] == "newpost")
  {
    // Check the data are sent by an authorized entity
    auth();
    if (empty($_GET['last']))
      die('ERR');
    $last = intval($_GET['last']);
    $query = $db->query("SELECT p.poster, t.subject, f.forum_name, t.id, p.posted FROM ".$db->prefix."topics AS t INNER JOIN ".$db->prefix."forums AS f ON t.forum_id = f.id INNER JOIN ".$db->prefix."posts AS p ON t.id = p.topic_id WHERE p.posted > '".$last."' AND t.first_post_id != p.id") or die('SQL');
    echo '<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE newpost>
<newpost>';
    while ($new = $db->fetch_assoc($query))
      {
	echo '
  <post>
    <id>'.$new['id'].'</id>
    <forum>'.$new['forum_name'].'</forum>
    <poster>'.$new['poster'].'</poster>
    <subject>'.$new['subject'].'</subject>
    <posted>'.$new['posted'].'</posted>
  </post>';     
      }
    echo '
</newpost>';
  }

/* New topics */
else if ($_GET['cmd'] == "newtopic")
  {
    // Check the data are sent by an authorized entity
    auth();
    if (empty($_GET['last']))
      die('ERR');
    $last = intval($_GET['last']);
    $query = $db->query("SELECT t.poster, t.subject, f.forum_name, t.id, t.posted FROM ".$db->prefix."topics AS t INNER JOIN ".$db->prefix."forums AS f ON t.forum_id = f.id WHERE t.last_post > '".$last."' AND t.first_post_id = t.last_post_id") or die('SQL');
    echo '<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE newtopic>
<newtopic>';
    while ($new = $db->fetch_assoc($query))
      {
	echo '
  <topic>
    <id>'.$new['id'].'</id>
    <forum>'.$new['forum_name'].'</forum>
    <poster>'.$new['poster'].'</poster>
    <subject>'.$new['subject'].'</subject>
    <posted>'.$new['posted'].'</posted>
  </topic>';     
      }
    echo '
</newtopic>';
  }

/*
else if ($_GET['cmd'] == "newuser")
  {
    // Check the data are sent by an authorized entity
    auth();
    if (empty($_GET['last']))
      die('ERR');
    $out = "";
    $last = intval($_GET['last']);
    $query = $db->query("SELECT username FROM `".$db->prefix."users` WHERE `group_id` != 0 AND `registered` > '".$last."'") or die('SQL');
    while ($new = $db->fetch_assoc($query))
      $out .= $new['username'].'[';
    // Delete last separator
    echo substr($out, 0, strlen($out) - 1);
  }
*/
else
  die('CMD');
?>
