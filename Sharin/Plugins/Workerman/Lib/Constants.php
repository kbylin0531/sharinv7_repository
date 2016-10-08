<?php
/**
 * This file is part of workerman.
 *
 * Licensed under The MIT License
 * For full copyright and license information, please see the MIT-LICENSE.txt
 * Redistributions of files must retain the above copyright notice.
 *
 * @author    walkor<walkor@workerman.net>
 * @copyright walkor<walkor@workerman.net>
 * @link      http://www.workerman.net/
 * @license   http://www.opensource.org/licenses/mit-license.php MIT License
 */

// For onError callback.
const WORKERMAN_CONNECT_FAIL = 1;
// For onError callback.
const WORKERMAN_SEND_FAIL = 2;

// Compatible with php7
if(!class_exists('Error')) {
    include_once SR_PATH_FRAMEWORK.'/Library/Error.class.php';
}