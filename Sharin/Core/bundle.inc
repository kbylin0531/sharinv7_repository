<?php
namespace Sharin;
use Sharin\Core\Logger;
use Sharin\Core\Storage;
use Sharin\Exceptions\ComponentMissDriverException;
use Throwable;
use Exception;
use Sharin\Core\Trace;
use Sharin\Exceptions\MethodNotFoundException;
use Sharin\Exceptions\ParameterInvalidException;
use Sharin\Library\Response;

/**
 * Class Utils
 * @package Sharin
 */
final class Utils {

    /**
     * 数据签名认证
     * @param  array  $data 被认证的数据
     * @return string       签名
     * @author 麦当苗儿 <zuojiazi@vip.qq.com>
     */
    public static function dataSign(array $data) {
        ksort($data);
        return sha1(http_build_query($data));
    }
    /**
     * 加载显示模板
     * @param string $tpl template name in folder 'Tpl'
     * @param array|null $vars vars array to extract
     * @param bool $clean it will clean the output cache if set to true
     * @param bool $isfile 判断是否是模板文件
     */
    public static function template(string $tpl,array $vars=null,bool $clean=true,bool $isfile=false){
        $clean and ob_get_level() > 0 and ob_end_clean();
        $vars and extract($vars, EXTR_OVERWRITE);
        $path = ($isfile or is_file($tpl))?$tpl:SR_PATH_FRAMEWORK."/Template/{$tpl}.php";
        is_file($path) or $path = SR_PATH_FRAMEWORK.'/Template/systemerror.php';
        include $path;
    }
    /**
     * 将C风格字符串转换成JAVA风格字符串
     * C风格      如： sub_string
     * JAVA风格   如： SubString
     * @param string $str
     * @param bool $orien 转换方向，默认C 风格转 Java 风格
     * @return string 返回风格转换后的类型
     */
    public static function styleStr(string $str,bool $orien=true){
        return $orien? ucfirst(preg_replace_callback('/_([a-zA-Z])/',function($match){return strtoupper($match[1]);},$str)):
            strtolower(ltrim(preg_replace('/[A-Z]/', '_\\0', $str), '_'));
    }

    /**
     * 自动从运行环境中获取URI
     * 直接访问：
     *  http://www.xor.com:8056/                => '/'
     *  http://localhost:8056/_xor/             => '/_xor/'  ****** BUG *******
     * @param bool $reget 是否重新获取，默认为false
     * @return null|string
     */
    public static function pathInfo($reget=false){
        static $uri = '/';
        if($reget or '/' === $uri){
            if(isset($_SERVER['PATH_INFO'])){
                //如果设置了PATH_INFO则直接获取之
                $uri = $_SERVER['PATH_INFO'];
            }else{
                $scriptlen = strlen($_SERVER['SCRIPT_NAME']);
                if(strlen($_SERVER['REQUEST_URI']) > $scriptlen){
                    $pos = strpos($_SERVER['REQUEST_URI'],$_SERVER['SCRIPT_NAME']);
                    if(false !== $pos){
                        //在不支持PATH_INFO...或者PATH_INFO不存在的情况下(URL省略将被认定为普通模式)
                        //REQUEST_URI获取原生的URL地址进行解析(返回脚本名称后面的部分)
                        if(0 === $pos){//PATHINFO模式
                            $uri = substr($_SERVER['REQUEST_URI'], $scriptlen);
                        }else{
                            //重写模式
                            $uri = $_SERVER['REQUEST_URI'];
                        }
                    }
                }else{}//URI短于SCRIPT_NAME，则PATH_INFO等于'/'
            }
        }
        return $uri;
    }

    /**
     * 检查继承关系
     * @param string $myname
     * @param string $parentnm
     * @return bool
     */
    public static function checkExtends(string $myname,string $parentnm){
        if(!class_exists($myname)) return false;
        $list = class_parents($myname,false);
        return isset($list[$parentnm]);
    }

    /**
     * 检查接口实现关系
     * @param $myname
     * @param $interfacename
     * @return bool
     */
    public static function checkImplement(string $myname,string $interfacename){
        if(!class_exists($myname)) return false;
        $list = class_implements($myname,false);
        return isset($list[$interfacename]);
    }

    /**
     * 检查trait使用关系
     * @param $myname
     * @param $traitname
     * @return bool
     */
    public static function checkTraitUse(string $myname,string $traitname){
        if(!class_exists($myname)) return false;
        $list = class_uses($myname,false);
        return isset($list[$traitname]);
    }

    /**
     * 调用类的静态方法
     * 注意，调用callable的时候如果是静态方法，则不能带小括号，就像函数名称一样
     *      例如：$callable = "{$clsnm}::{$method}";将永远返回false
     * @param string $clsnm class name
     * @param string $method method name
     * @return mixed|null
     */
    public static function callStatic($clsnm,$method){
        if(class_exists($clsnm,false)) {
            $callable = "{$clsnm}::{$method}";
            if(is_callable($callable)){
                try{
                    return $clsnm::$method();
                }catch (\Exception $e){
                    if(SR_DEBUG_MODE_ON) Developer::trace($e->getMessage());
                }
            }
        }
        return null;
    }

    /**
     * 转换成php处理文件系统时所用的编码
     * 即UTF-8转GB2312
     * @param string $str 待转化的字符串
     * @param string $strencode 该字符串的编码格式
     * @return string|false 转化失败返回false
     */
    public static function toSystemEncode($str,$strencode='UTF-8'){
        return $strencode === SR_OS_ENCODING? $str : iconv($strencode,SR_OS_ENCODING.'//IGNORE',$str);
    }

    /**
     * 转换成程序使用的编码
     * 即GB2312转UTF-8
     * @param string $str 待转换的字符串
     * @param string $program_encoding
     * @return string|false 转化失败返回false
     */
    public static function toProgramEncode($str, $program_encoding='UTF-8'){
        return $program_encoding === SR_OS_ENCODING? $str : iconv(SR_OS_ENCODING,"{$program_encoding}//IGNORE",$str);
    }

    /**
     * 获取类常量
     * use defined() to avoid error of E_WARNING level
     * @param string $class 完整的类名称
     * @param string $constant 常量名称
     * @param mixed $replacement 不存在时的代替
     * @return mixed
     */
    public static function constant($class,$constant,$replacement=null){
        if(!class_exists($class,true)) return $replacement;
        $constant = "{$class}::{$constant}";
        return defined($constant)?constant($constant):$replacement;
    }

    /**
     * 将参数二的配置合并到参数一种，如果存在参数一数组不存在的配置项，跳过其设置
     * (不会覆盖)
     * @param array $dest 被覆盖者
     * @param array $sourse 覆盖者
     * @param bool $covermode 为true的情况下无论目标健是否存在否会覆盖，否则会先检测目标是否存在
     * @return mixed
     */
    public static function merge(array $dest, array $sourse,bool $covermode=true){
        foreach($sourse as $key=>$val){
            if($covermode){
                //fugei mose
                if (isset($dest[$key]) and is_array($val)) {
                    $dest[$key] = self::merge($dest[$key],$val,true);/* se shuju,zo erbi tyi */
                }else{
                    if(is_int($key)){
                        if(!in_array($val,$dest)) $dest[] = $val;
                    }else{
                        $dest[$key] = $val;
                    }
                }
            }else{
                //kno saiqu wumao sai
                if(isset($dest[$key])){
                    if (is_array($val)) {
                        $dest[$key] = self::merge($dest[$key],$val,false);
                    }else{
                        if(is_int($key)){
                            if(!in_array($val,$dest)) $dest[] = $val;
                        }else{
                            $dest[$key] = $val;
                        }
                    }
                }
            }
        }
        return $dest;
    }

    /**
     * 过滤掉数组中与参数二计算值相等的值，可以是保留也可以是剔除
     * @param array $array
     * @param callable|array|mixed $comparer
     * @param bool $leave
     * @return void
     */
    public static function filter(array &$array, $comparer=null, $leave=true){
        static $result = [];
        $flag = is_callable($comparer);
        $flag2 = is_array($comparer);
        foreach ($array as $key=>$val){
            if($flag?$comparer($key,$val):($flag2?in_array($val,$comparer):($comparer === $val))){
                if($leave){
                    unset($array[$key]);
                }else{
                    $result[$key] = $val;
                }
            }
        }
        $leave or $array = $result;
    }

    /**
     * 从字面商判断$path是否被包含在$scope的范围内
     * @param string $path 路径
     * @param string $scope 范围
     * @return bool
     */
    public static function checkInScope($path, $scope) {
        if (false !== strpos($path, '\\')) $path = str_replace('\\', '/', $path);
        if (false !== strpos($scope, '\\')) $scope = str_replace('\\', '/', $scope);
        $path = rtrim($path, '/');
        $scope = rtrim($scope, '/');
        return (SR_IS_WIN ? stripos($path, $scope) : strpos($path, $scope)) === 0;
    }
}

/**
 * Class Behaviour 行为控制
 * 各个行为组成了应用的生命周期
 * @package Sharin
 */
final class Behaviour {
    /**
     * 标签集合
     * key可能是某种有意义的方法，也可能是一个标识符
     * value可能是闭包函数或者类名称
     * 如果是value类名称，则key可能是其调用的方法的名称（此时会检查这个类中是否存在这个方法），也可能是一个标识符
     * @var array
     */
    private static $tags = [];

    public static function init(array $tags=null){
        if(null !== $tags){
            self::$tags = $tags;
        }
    }

    /**
     * 动态注册行为
     * @param string|array $tag 标签名称，如果是数组则意味着批量导入
     * @param mixed|array $behavior 行为名称,为array类型时将进行批量注册
     * @return void
     */
    public static function register($tag, $behavior=null) {
        if(is_array($tag)){
            self::$tags = array_merge(self::$tags,$tag);
        }else{
            if (!isset(self::$tags[$tag]))  self::$tags[$tag] = [];
            if (is_array($behavior)) {
                self::$tags[$tag] = array_merge(self::$tags[$tag], $behavior);
            } else {
                self::$tags[$tag][] = $behavior;
            }
        }
    }

    /**
     * 监听标签的行为
     * @param string $tag 标签名称
     * @param mixed $params 传入回调闭包或者对象方法的参数
     * @return void
     */
    public static function listen($tag, $params = null) {
        if(SR_DEBUG_MODE_ON) Developer::status($tag);
        if (!empty(self::$tags[$tag])) {
            foreach(self::$tags[$tag] as $name){
                if (false === self::exec($name, $tag, $params)) break; // 如果返回false 则中断行为执行
            }
        }
    }

    /**
     * 执行某个行为
     * @param string $callableorclass 闭包或者类名称
     * @param string $tag 方法名（标签名）
     * @param Mixed $params 方法的参数
     * @return mixed
     * @throws ParameterInvalidException
     */
    private static function exec($callableorclass, $tag , $params = null) {
        static $_instances = [];
        if (is_callable($callableorclass)) {
            //如果是闭包，则直接执行闭包函数
            return $callableorclass($params);
        }elseif(is_string($callableorclass)){
            if(!isset($_instances[$callableorclass])){
                $_instances[$callableorclass] = new $callableorclass();
            }
            $obj = $_instances[$callableorclass];
            if(!is_callable([$obj, $tag])) $tag = 'run';//tag默认是方法名称
            return $obj->$tag($params,$tag);
        }
        throw new ParameterInvalidException($callableorclass,['Closure','string']);
    }
}

/**
 * Class Developer
 * Developer tool to improve performance and debug
 * @package Sharin
 */
final class Developer {

    private static $showTrace = SR_DEBUG_MODE_ON;

    /**
     * @var array
     */
    private static $highlightes = [];
    /**
     * @var array
     */
    private static $_status = [];
    /**
     * @var array
     */
    private static $_traces = [];

    /**
     * Open the page trace
     * @return void
     */
    public static function openTrace(){
        self::$showTrace = true;
    }

    /**
     * Close the page trace
     * @return void
     */
    public static function closeTrace(){
        self::$showTrace = false;
    }

    /**
     * record the runtime's time and memory usage
     * @param null|string $tag tag of runtime point
     * @return void
     */
    public static function status($tag){
        SR_DEBUG_MODE_ON and self::$_status[$tag] = [
            microtime(true),
            memory_get_usage(),
        ];
    }

    /**
     * import status
     * @param string $tag
     * @param array $status
     */
    public static function import($tag,array $status){
        self::$_status[$tag] = $status;
    }

    /**
     * 记录下跟踪信息
     * @param string|mixed $message
     * @param ...
     * @return void
     */
    public static function trace($message=null){
        static $index = 0;
        if(!SR_DEBUG_MODE_ON) return;
        if(null === $message){
            !SR_IS_CLI and self::$showTrace and Trace::show(self::$highlightes,self::$_traces,self::$_status);
        }else{
            $location = debug_backtrace();
            if(isset($location[0])){
                $location = "{$location[0]['file']}:{$location[0]['line']}";
            }else{
                $location = $index ++;
            }
            if(func_num_args() > 1) $message = var_export(func_get_args(),true);
            if(!is_string($message)) $message = var_export($message,true);
            if(isset(self::$_traces[$location])){
                $index ++;//it may called multi-times in some place
                $location = "$location ($index)";
            }
            self::$_traces[$location] = $message;
        }
    }
}

/**
 * Class ClassLoader
 * manage the class auto-loading and initialization
 * @package Sharin
 */
final class ClassLoader {
    /**
     * 加载规则
     * [
     *      0   => '',//加载路径，相对于SR_PATH_BASE
     *      1   => '',//文件后缀名称
     * ]
     * @var array
     */
    protected static $rules = [
        0           => [
            'path'  => 'Library',
        ],
        'Sharin'    => [
            'ext'   => 'srg',
        ],
        'Web'       => [
            'ext'   => 'class.php',
        ],
        'Explorer'  => [
            'ext'   => 'class.php',
        ],
    ];

    /**
     * @var array array of key-valure pairs (name to relative path)
     */
    private static $map = [];

    /**
     * import classes from outer
     * @param array $map
     */
    public static function import(array $map){
        $map and self::$map = array_merge(self::$map,$map);
    }

    public static function addRule(){

    }

    /**
     * default loader for this system
     * @param string $clsnm class name
     * @return void
     */
    public static function load($clsnm){
        if(isset(self::$map[$clsnm])) {
            include_once self::$map[$clsnm];
        }else{
            $pos = strpos($clsnm,'\\');
            if($pos === false){
                $rel = $clsnm;
                $index = 0;
            }else{
                $rel = str_replace('\\', '/', $clsnm);
                $index = substr($clsnm,0,$pos);
            }

            $path = isset(self::$rules[$index]['path'])? self::$rules[$index]['path'] : '';
            $ext  = isset(self::$rules[$index]['ext'])? self::$rules[$index]['ext'] : 'php';//兼容
            $path = SR_PATH_BASE."/{$path}/{$rel}.{$ext}";
            if(is_file($path)) include self::$map[$clsnm] = $path;
        }
        Utils::callStatic($clsnm,'__initializationize');

        //检查是否存在继承关系
        if(Utils::checkExtends($clsnm,Component::class)){
            call_user_func([$clsnm,'__initialization']);
        }
    }

    /**
     * register class autoloader
     * @param callable $autoloader
     * @throws SharinException
     */
    public static function register(callable $autoloader=null){
        if(!spl_autoload_register($autoloader?$autoloader:[static::class,'load'],false,true)){
            SharinException::throwing('Faile to register class autoloader!');
        }
    }

}

/**
 * Class SharinException
 *
 * @package Sharin
 */
class SharinException extends Exception {

    /**
     * SharinException constructor.
     */
    public function __construct(){
        $args = func_get_args();
        if(count($args) > 1){
            $message = var_export($args,true);
        }else{
            $message = $args[0];
        }
        $this->message = is_string($message)?$message:var_export($message,true);
    }

    /**
     * 直接抛出异常信息
     * @param ...
     * @return mixed
     * @throws SharinException
     */
    public static function throwing(){
        $clsnm = static::class;//extend class name
        throw new $clsnm(func_get_args());
    }

    /**
     * 处理错误信息
     * @param $code
     * @param $message
     * @param $file
     * @param $line
     */
    public static function handleError($code, $message, $file, $line){
        Behaviour::listen(ON_ERROR);

        Response::cleanOutput();//清空之前的输出
        if(SR_IS_AJAX) exit("File:{$file}   Line:{$line} Message:{$message}\n");
        if(SR_DEBUG_MODE_ON)  Response::sendHttpStatus(403,'Resource Error!');

        $trace = debug_backtrace();
        $vars = [
            'message'   => "C:{$code}   S:{$message}",
            'position'  => "File:{$file}   Line:{$line}",
            'trace'     => $trace, //be careful
        ];

        if(SR_DEBUG_MODE_ON){
            Utils::template('error',$vars);
        }else{
            Utils::template('user_error');
        }
        Behaviour::listen(ON_ERROR_END);
    }

    /**
     * 处理异常
     * @param Throwable $e
     */
    public static function handleException(Throwable $e) {
        Behaviour::listen(ON_EXCEPTION);

        Response::cleanOutput();
        if(SR_IS_AJAX) {
            $message = $e->getMessage();
            Logger::record($message);
            exit($message);
        }
        SR_DEBUG_MODE_ON or Response::sendHttpStatus(403,'Resource Exception!');

        $trace = $e->getTrace();
        if(!empty($trace[0])){
            empty($trace[0]['file']) and $trace[0]['file'] = 'Unkown file';
            empty($trace[0]['line']) and $trace[0]['line'] = 'Unkown line';

            $vars = [
                'message'   => get_class($e).' : '.$e->getMessage(),
                'position'  => 'File:'.$trace[0]['file'].'   Line:'.$trace[0]['line'],
                'trace'     => $trace,
            ];
            if(SR_DEBUG_MODE_ON){
                Developer::trace($vars);//页面跟踪错误
                Utils::template('exception',$vars);
            }else{
                Utils::template('user_error');
            }
        }else{
            Utils::template('user_error');
        }
        Behaviour::listen(ON_EXCEPTION_END);
    }

}


/**
 * Class Component 组件类
 *
 * 调用静态方法等于直接使用系统内置的驱动，较实例化驱动高效
 *
 * 规则：
 *  一、驱动中存在与实例名称一致的方法
 *      调用示例如下：
 *          Component::{method}();
 *          Component::instantiate()->{method}();
 *  二、静态调用可以基于实例，需要设置
 *      设置了 BASE_ON_INSTANCE = true 时，类加载时会自动初始化驱动
 *
 * @package Sharin
 */
abstract class Component {
    /**
     * 继承类配置
     * @var array
     */
    private static $_conventions = [];

    /**
     * @var object[] 静态驱动列表
     */
    private static $_static_drivers = [];

    /**
     * @var object[] 驱动列表
     */
    private static $_drivers = [];
    /**
     * @var string 组件名称（是该组件的唯一标识符），为空时表示自动获取，否则取该类的静态name属性作为短名称
     */
    protected static $name = null;

    /**
     * @var string 错误信息，只能内部设置
     */
    protected static $_error = '';
    /**
     * 获取组件名称
     * 组件名称是类名称的变体，将"\\"改为"."，并且全部转为小写
     * @return string
     */
    public static function getName(){
        null === static::$name and static::$name = strtolower(str_replace('\\','.',ltrim(static::class,'\\')));
        return static::$name;
    }

    /**
     * 获取错误信息
     * 获取一次后错误信息将被重置
     * @return string
     */
    public static function getError(){
        $error = static::$_error;
        static::$_error = '';
        return $error;
    }
    /**
     * 类自动加载时自动调用
     * [OK]
     * @param array|null $config 参数非空是留给继承类覆盖时调用的
     * @return void
     */
    public static function __initialization(array $config = null){
        static $_convention = [ /* jito wule tigo xinen */
            BASE_ON_DRIVER        => FALSE,   //静态方法是否基于驱动
            BASED_DRIVER_INDEX    => '',      //静态方法基于的驱动角标

            DRIVER_CLASS_LIST     => [],      //驱动类的列表
            DRIVER_CONFIG_LIST    => [],      //驱动类列表参数
        ];
        self::$_conventions[static::class] = null !== $config? Utils::merge($_convention,$config,true) : $_convention;
    }

    /**
     * 获取静态驱动
     * @return object|false 返回驱动，驱动不存在时返回false
     */
    final public static function getStaticDriver(){
        $static = static::class;
        if(!isset(self::$_static_drivers[$static])){
            $instance = self::instance(self::convention('BASED_DRIVER_INDEX',-1));/* -1 kennia mao gei*/
            if(false === $instance){
                return false;
            }
            self::$_static_drivers[$static] = $instance;
        }
        return self::$_static_drivers[$static];
    }

    /**
     * 获取配置（规矩）
     * @param string $name 配置名称，涉及到子配置时使用"."连接
     * @param mixed $alternate 替补，如果目标配置不存在
     * @return mixed
     */
    final public static function convention(string $name='',$alternate=null){
        $clsnm = static::class;
        isset(self::$_conventions[$clsnm]) or self::$_conventions[$clsnm] = [];
        if(empty($name)){
            return self::$_conventions[$clsnm];
        }elseif(strpos($name,'.') !== false){/* ummo yule "."ao qudo lofyi xinen  */
            $target = self::$_conventions[$clsnm];
            $name = explode('.',$name);
            foreach ($name as $nm) {
                if(isset($target[$nm])){
                    $target = $target[$nm];
                }else{
                    return $alternate;
                }
            }
            return $target;
        }else{
            return isset(self::$_conventions[$clsnm][$name]) ? self::$_conventions[$clsnm][$name] : $alternate;
        }
    }

    /**
     * 获取自身的一个驱动实例
     * 来实现自身的操作
     * @param int|string|null $index 角标，如果是null时表示自动获取
     * @return object|false
     */
    final public static function instance($index=null){
        $static = static::class;
        isset(self::$_drivers[$static]) or self::$_drivers[$static] = [];
        if(!isset(self::$_drivers[$static][$index])){
            $classes = self::convention('DRIVER_CLASS_LIST',[]);
            if(null === $index){
                $clsnm = reset($classes);
                if(false === $clsnm) {
                    return false;
                }
            } else {
                if(!isset($classes[$index])){
                    return false;
                }
                $clsnm = $classes[$index];
            }
            $plist = self::convention('DRIVER_CONFIG_LIST',[]);
            $params = isset($plist[$index])? $plist[$index] : reset($plist);
            self::$_drivers[$static][$index] = $params ? new $clsnm($params) : new $clsnm();
        }
        return self::$_drivers[$static][$index];
    }

    /**
     * Use driver method as its static method
     * @param string $method method name
     * @param array $arguments method arguments
     * @return mixed
     * @throws SharinException
     */
    public static function __callStatic(string $method,array $arguments) {
        $driver = static::instance(null);
        if(false === $driver) throw new ComponentMissDriverException(static::class);
        $callable = [$driver, $method];
        if(!is_callable($callable)) throw new MethodNotFoundException(static::class,$method);
        return call_user_func_array($callable, $arguments);
    }

}

/**
 * Class Configger 配置管理类
 * @package Sharin\Core
 */
class Configger {
    /**
     * 配置类型
     * 值使用字符串而不是效率更高的数字是处于可以直接匹配后缀名的考虑
     */
    const TYPE_PHP     = 'php';
    const TYPE_INI     = 'ini';
    const TYPE_YAML    = 'yaml';
    const TYPE_XML     = 'xml';
    const TYPE_JSON    = 'json';

    /**
     * @var array 本类的配置
     */
    private static $config = [
        //是否开启Lite
        'USE_LITE'          => true,
        //Lite针对的类名称的列表
        'LITE_CLASS_LIST'   => [
            'Sharin\\Core\\Storage',
            'Sharin\\Core\\Cache',
        ],
        //运行时配置目录（其下的配置文件只在运行时由程序生成）
        'RUNTIME_CONF_PATH'  => SR_PATH_BASE.'/Data/Runtime/Configuration/',
    ];

    /**
     * @var array 配置缓存
     */
    private static $_cache = [];

    /**
     * 初始化，配置管理类
     * 允许多次调用
     * @param array|null $config
     */
    public static function initialize(array $config=null){
        null !== $config and self::$config = Utils::merge(self::$config,$config,true);

        //清空之前的配置
        self::$_cache = [];

        if(self::$config['USE_LITE']){
            //预处理后的文件
            $litefile = SR_PATH_RUNTIME.'/configger.php';
//            if(!Storage::has($litefile,Storage::READ_ACCESS) or Storage::mtime(SR_PATH_CONFIG) > Storage::mtime($litefile)){ //不能使用Storage的原因是避免Storage过早地出场
            if(!is_file($litefile) or !is_readable($litefile) or filemtime(SR_PATH_CONFIG) > filemtime($litefile)) {
                //缓存文件不存在，或者已经过期（配置目录比缓存文件新旧表示过期）需要重新编译
                if(!empty(self::$config['LITE_CLASS_LIST'])) foreach (self::$config['LITE_CLASS_LIST'] as $clsnm){
                    self::load($clsnm);
                }
                // Closure is not suggest in config file due to var_export could not do well with closure
                // it will be translated to 'method Closure::__set_state()'
                Storage::write($litefile,'<?php return '.var_export(self::$_cache,true).';') or SharinException::throwing("配置无法写入文件'{$litefile}'");

                dumpout('缓存文件不存在，或者已经过期（配置目录比缓存文件新旧表示过期），已经完成lite文件的建立工作', self::$_cache);
            }else{
                //直接读取lite文件
                self::$_cache = self::parse($litefile,self::TYPE_PHP);
                dumpout('直接读取lite文件',class_exists('Sharin\\Core\\Storage',false)? 'Storage已经加载到内存中' : 'Storage未载入内存' ,self::$_cache);
            }
        }else{
            //设么都不做，每次通过load方法加载外部配置都会将配置加入到缓存中
            dumpout('设么都不做，每次通过load方法加载外部配置都会将配置加入到缓存中',self::$_cache);
        }
    }
//------------------------------------ 类配置方法集合 : load <== loadInner + loadOuter -------------------------------------------------------------------------------------
    /**
     * 获取类的完整配置
     * @param string $clsnm 类名称
     * @param bool $is_component 该类是否继承自Component类
     * @return array
     */
    public static function load(string $clsnm,bool $is_component=null){
        if(!isset(self::$_cache[$clsnm])){
            null === $is_component and $is_component = Utils::checkExtends($clsnm,Component::class);
            self::$_cache[$clsnm] = self::loadInner($clsnm,$is_component);
            self::$_cache[$clsnm] = Utils::merge(self::$_cache[$clsnm],self::loadOuter($clsnm,$is_component),true);//内部的配置即完整的，不需要外部额外添加，所以参数三设置为false(错误的，因为针对健为数字的情况，外部配置可能会无效)
        }
        return self::$_cache[$clsnm];
    }

    /**
     * 获取类的内置配置
     *  注：只有Component类及子类才能通过convention方法获取内置配置，其他类只能获取到空的数组，如果类不存在时也会返回空数组
     * @param string $clsnm 类名称
     * @param bool $is_component 该类是否继承自Component类
     * @return array 返回类内部的配置
     */
    public static function loadInner(string $clsnm,bool $is_component=null){
        null === $is_component and $is_component = Utils::checkExtends($clsnm,Component::class);
        return $is_component ? call_user_func([$clsnm,'convention']) : [];
    }

    /**
     * 获取类；类的外部配置（包括外部全局和外部应用两部分）
     * @param string $clsnm 类名称
     * @param bool $is_component 该类是否继承自Component类
     * @return array
     */
    public static function loadOuter(string $clsnm,bool $is_component=null) {
        null === $is_component and $is_component = Utils::checkExtends($clsnm,Component::class);

        //获取类的标识符名称
        $idname = $is_component ?
            call_user_func([$clsnm,'getName']):
            strtolower(str_replace('\\','/',ltrim($clsnm,'\\')));
        //类的短名称与长完整名称添加对应
//        self::$_map[$idname] = $clsnm;

        strpos('.', $idname) !== false and $idname = str_replace('.', '/' ,$idname);

        $path = SR_PATH_BASE."/Config/{$idname}.php";
        //读取全局配置
        $result = Storage::has($path,Storage::READ_ACCESS) ? self::parse($path,self::TYPE_PHP) : [];
        if(defined('SR_PATH_APP')){
            //应用配置读取
            $appconf = SR_PATH_APP."/Config/{$idname}.php";
            if(Storage::has($appconf,Storage::READ_ACCESS) and $appconf = self::parse($appconf,self::TYPE_PHP)){
                is_array($appconf) and $result = Utils::merge($result,$appconf,true);
            }
        }
        return $result;
    }

//------------------------------------ user config of dynamic -------------------------------------------------------------------------------------
    /**
     * read the user-defined config in SR_PATH_RUNTIME
     * @param string $identify config identify
     * @param mixed $replacement
     * @return array
     */
    public static function read(string $identify,$replacement=[]){
        $path = self::_id2path($identify,true);//读取时必须保证文件是存在的
        if($path and Storage::has($path,Storage::READ_ACCESS)){
            return self::parse($path,self::TYPE_PHP);
        }
        return $replacement;
    }

    /**
     * write user-config to file
     * @param string $identify
     * @param array $config
     * @return bool
     */
    public static function write(string $identify,array $config){
        $path = self::_id2path($identify,false);
        return Storage::write($path,'<?php return '.var_export($config,true).';');
    }

    /**
     * 将配置项转换成配置文件路径
     * 配置项中的"."会被替换为目录分隔符"/"
     * @param string $item 配置项
     * @param mixed $check 检查文件是否存在
     * @return false|string 返回配置文件路径，参数二位true并且文件不存在时返回null
     */
    private static function _id2path($item,$check=true){
        strpos($item,'.') and $item = str_replace('.',DIRECTORY_SEPARATOR,$item);
        $path = self::$config['RUNTIME_CONF_PATH']."/{$item}.php";
        return !$check || is_readable($path)?$path:false;
    }


//------------------------------------ 通用工具 -------------------------------------------------------------------------------------

    /**
     * parse config file into php array
     * @param string $path 配置文件的路径
     * @param string|null $type 配置文件的类型,参数为null时根据文件名称后缀自动获取
     * @param callable $parser 配置解析方法 有些格式需要用户自己解析
     * @return array
     */
    public static function parse(string $path,string $type='',callable $parser=null){
        $type or $type = pathinfo($path, PATHINFO_EXTENSION);
        switch ($type) {
            case self::TYPE_PHP:
                $result = include $path;
                break;
            case self::TYPE_INI:
                $result = parse_ini_file($path);
                break;
            case self::TYPE_YAML:
                $result = yaml_parse_file($path);
                break;
            case self::TYPE_XML:
                $result = (array)simplexml_load_file($path);
                break;
            case self::TYPE_JSON:
                $result = json_decode(file_get_contents($path), true);
                break;
            default:
                $result = $parser?$parser($path):SharinException::throwing('无法解析配置文件');
        }
        return is_array($result)? $result : [];
    }
}