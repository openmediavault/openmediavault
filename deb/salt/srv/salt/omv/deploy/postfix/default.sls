# Documentation/Howto:
# http://www.postfix.org/postconf.5.html
# http://www.postfix.org/pipe.8.html
# http://www.gtkdb.de/index_7_727.html
# http://irbs.net/internet/postfix/0503/2148.html
# http://gate.io/blogpost34
# http://www.cyberciti.biz/tips/howto-postfix-flush-mail-queue.html
# http://www.tuxfutter.de/wiki/Einrichten_eines_Mailservers_mit_Postfix
# http://dokuwiki.tachtler.net/doku.php?id=tachtler:postfix_centos_6
# http://serverfault.com/questions/536648/postfix-pass-a-copy-of-an-email-to-a-script-but-deliver-the-original-one-to-mai

# Testing:
# echo "Test" | mail -s "Test subject" -a "From: xxx@yyy.zzz" root
# hostname | mailx -s "root `hostname` `date`" root
# mailq
# postcat -q A705238B4C

# Working with CA certificates:
# - Add new CA certificate to /etc/ssl/certs
# - Modify /etc/ca-certificates.conf
# - Execute update-ca-certificates

{% set config = salt['omv.get_config']('conf.system.email') %}
{% set dirpath = '/srv/salt' | path_join(slspath) %}

# Make sure the hostname configuration is applied.
deploy_postfix_hostname:
  salt.state:
    - tgt: '*'
    - sls: omv.deploy.hostname

include:
{% for file in salt['file.readdir'](dirpath) | sort %}
{% if file | regex_match('^(\d+.+).sls$', ignorecase=True) %}
  - .{{ file | replace('.sls', '') }}
{% endif %}
{% endfor %}

{% if config.enable | to_bool %}

start_posfix_service:
  service.running:
    - name: postfix
    - enable: True

{% else %}

stop_postfix_service:
  service.dead:
    - name: postfix
    - enable: False

{% endif %}
