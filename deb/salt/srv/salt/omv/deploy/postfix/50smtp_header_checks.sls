configure_postfix_smtp_header_checks:
  file.managed:
    - name: "/etc/postfix/smtp_header_checks"
    - contents: |
        # Append the hostname to the email subject.
        /^Subject: (.*)/ REPLACE Subject: [{{ salt['network.get_fqdn']() }}] ${1}
    - user: root
    - group: root
    - mode: 600
    - require:
      - salt: deploy_postfix_hostname
    - watch_in:
      - service: start_posfix_service
