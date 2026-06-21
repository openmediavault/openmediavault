<?php

$finder = PhpCsFixer\Finder::create()
    ->in(__DIR__)
    ->path('usr/sbin')
    ->path('usr/share/php')
    ->path('usr/share/openmediavault')
    ->path('var/www/openmediavault')
    ->name('*.php')
    ->name('*.inc')
    ->name('omv-engined')
    ->name('omv-rpc');

$config = new PhpCsFixer\Config();
return $config->setRules([
    '@PSR12' => true,
    'binary_operator_spaces' => ['default' => 'single_space'],
])
->setFinder($finder);
