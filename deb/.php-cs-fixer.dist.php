<?php

$finder = PhpCsFixer\Finder::create()
    ->in(__DIR__)
    ->path('usr/share/php')
    ->path('usr/share/openmediavault')
    ->name('*.php')
    ->name('*.inc');

$config = new PhpCsFixer\Config();
return $config->setRules([
    '@PSR12' => true,
    'binary_operator_spaces' => ['default' => 'single_space'],
])
->setFinder($finder);
