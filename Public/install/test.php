<?php require __DIR__.'/common.php';?>
<body>
<script>
    function testClick(){
        if($('.yes').length!=7){
            alert('您的配置或权限不符合要求');
        }else{
            location.href='./index.php?c=create';
        }
    }
</script>
<div id="nav">
    <div class="inside">
        <p class="name">bjyadmin<span>安装向导</span></p>
        <ul class="schedule">
            <li class="number">1</li>
            <li class="word">使用协议</li>
        </ul>
        <ul class="schedule active">
            <li class="number">2</li>
            <li class="word">环境检测</li>
        </ul>
        <ul class="schedule">
            <li class="number">3</li>
            <li class="word">创建数据</li>
        </ul>
        <ul class="schedule">
            <li class="number">4</li>
            <li class="word">安装完成</li>
        </ul>
    </div>
</div>
<div id="out">
    <div class="inside">
        <div class="box test">
            <h2>环境监测</h2>
            <table class="table table-border">
                <tr>
                    <th width="25%">坏境</th>
                    <th width="25%">最低配置</th>
                    <th width="25%">当前配置</th>
                    <th width="25%">是否符合</th>
                </tr>
                <tr>
                    <td>操作系统</td>
                    <td>不限</td>
                    <td><?php echo php_uname('s'); ?></td>
                    <td class="yes">√</td>
                </tr>
                <tr>
                    <td>php版本</td>
                    <td>&gt;5.3</td>
                    <td><?php echo PHP_VERSION ?></td>
                    <?php $php_version=explode('.', PHP_VERSION); ?>
                    <td class="<?php if(($php_version['0']>=7) || ($php_version['0']>=5 && $php_version['1']>=3))echo 'yes'; ?>">
                        <?php if (($php_version['0']>=7) || ($php_version['0']>=5 && $php_version['1']>=3)): ?>
                            √
                        <?php else: ?>
                            ×
                        <?php endif ?>
                    </td>
                </tr>
            </table>
            <h2>目录权限</h2>
            <table class="table table-border">
                <tr>
                    <th width="25%">坏境</th>
                    <th width="25%">最低配置</th>
                    <th width="25%">当前配置</th>
                    <th width="25%">是否符合</th>
                </tr>
                <?php

                const BASE_DIR = SR_PATH_BASE;
                const UPLOAD_DIR = BASE_DIR.'/Public/Upload';
                const RUNTIME_DIR = BASE_DIR.'/Runtime';
                const CONF_DIR = BASE_DIR.'/Admin/Common/Conf';

                ?>
                ?>
                <tr>
                    <td>./</td>
                    <td>可写</td>
                    <td>
                        <?php if (is_writable(BASE_DIR)): ?>
                            可写
                        <?php else: ?>
                            不可写
                        <?php endif ?>
                    </td>
                    <td class="<?php if(is_writable(BASE_DIR))echo 'yes'; ?>">
                        <?php if (is_writable(BASE_DIR)): ?>
                            √
                        <?php else: ?>
                            ×
                        <?php endif ?>
                    </td>
                </tr>
                <tr>
                    <td>./Upload</td>
                    <td>可写</td>
                    <td>
                        <?php if (is_writable(UPLOAD_DIR)): ?>
                            可写
                        <?php else: ?>
                            不可写
                        <?php endif ?>
                    </td>
                    <td class="<?php if(is_writable(UPLOAD_DIR))echo 'yes'; ?>">
                        <?php if (is_writable(UPLOAD_DIR)): ?>
                            √
                        <?php else: ?>
                            ×
                        <?php endif ?>
                    </td>
                </tr>
                <tr>
                    <td>./Runtime</td>
                    <td>可写</td>
                    <td>
                        <?php if (is_writable(RUNTIME_DIR)): ?>
                            可写
                        <?php else: ?>
                            不可写
                        <?php endif ?>
                    </td>
                    <td class="<?php if(is_writable(RUNTIME_DIR))echo 'yes'; ?>">
                        <?php if (is_writable(RUNTIME_DIR)): ?>
                            √
                        <?php else: ?>
                            ×
                        <?php endif ?>
                    </td>
                </tr>
                <tr>
                    <td>./Public/install</td>
                    <td>可写</td>
                    <td>
                        <?php if (is_writable(INSTALL_DIR)): ?>
                            可写
                        <?php else: ?>
                            不可写
                        <?php endif ?>
                    </td>
                    <td class="<?php if(is_writable(INSTALL_DIR))echo 'yes'; ?>">
                        <?php if (is_writable(INSTALL_DIR)): ?>
                            √
                        <?php else: ?>
                            ×
                        <?php endif ?>
                    </td>
                </tr>
                <tr>
                    <td>./Application/Common/Conf</td>
                    <td>可写</td>
                    <td>
                        <?php if (is_writable(CONF_DIR)): ?>
                            可写
                        <?php else: ?>
                            不可写
                        <?php endif ?>
                    </td>
                    <td class="<?php if(is_writable(CONF_DIR))echo 'yes'; ?>">
                        <?php if (is_writable(CONF_DIR)): ?>
                            √
                        <?php else: ?>
                            ×
                        <?php endif ?>
                    </td>
                </tr>
            </table>
            <p class="agree">
                <a class="btn btn-primary" href="./index.php?c=agreement">上一步</a>
                <a class="btn btn-success" href="javascript:void(0);" onclick="testClick()">下一步</a>
            </p>
        </div>
    </div>
</div>

</body>
</html>