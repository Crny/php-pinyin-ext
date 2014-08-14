php-pinyin-ext
==============

PHP extension for convert Chinese characters to Pinyin

# install

```shell
git clone https://github.com/joychao/php-pinyin-ext
cd php-pinyin-ext/ext
phpize
./configure
make && make install

## add follow line to php.ini
extension=pinyin.so
```

#### check

```shell
php -m | grep pinyin
```

