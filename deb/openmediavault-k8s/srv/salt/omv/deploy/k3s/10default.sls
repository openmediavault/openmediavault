# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

# Documentation/Howto:
# https://docs.k3s.io/installation/packaged-components
# https://docs.k3s.io/installation/private-registry#rewrites
# https://docs.k3s.io/helm
# https://github.com/k3s-io/k3s/issues/1086#issuecomment-1342838441
# https://qdnqn.com/how-to-configure-traefik-on-k3s/
# https://community.traefik.io/t/adding-entrypoints-to-a-helm-deployed-traefik-on-k3s/14813/5
# https://kubernetes.io/docs/concepts/storage/volumes/#hostpath-volume-types
# https://kubernetes.io/docs/concepts/storage/persistent-volumes/#persistent-volumes
# https://docs.k3s.io/installation/requirements?os=pi

{% set k8s_config = salt['omv_conf.get']('conf.service.k8s') %}
{% set k8s_lbports_config = salt['omv_conf.get']('conf.service.k8s.lbport') %}
{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}
# {% set email_config = salt['omv_conf.get']('conf.system.notification.email') %}

{% set fqdn = dns_config.hostname | lower %}
{% if dns_config.domainname | length > 0 %}
{% set fqdn = [dns_config.hostname, dns_config.domainname] | join('.') | lower %}
{% endif %}

{% if grains['raspberrypi'] %}
k3s_enable_cgroup_raspi:
  file.replace:
    - name: "/boot/firmware/cmdline.txt"
    - pattern: "^(.*)$"
    - repl: "\\1 cgroup_enable=cpuset cgroup_memory=1 cgroup_enable=memory"
    - backup: False
    - count: 1
    - unless: "grep -q 'cgroup_enable=cpuset cgroup_memory=1 cgroup_enable=memory' /boot/firmware/cmdline.txt"

k3s_cgroups_reboot_required:
  file.touch:
    - name: "/run/reboot-required"
    - onchanges:
      - file: k3s_enable_cgroup_raspi
{% endif %}

create_k3s_manifest_dir:
  file.directory:
    - name: "/var/lib/rancher/k3s/server/manifests/"
    - makedirs: True

cleanup_k3s_manifest_dir:
  module.run:
    - file.find:
      - path: "/var/lib/rancher/k3s/server/manifests/"
      - iname: "openmediavault-*.yaml"
      - delete: "f"

create_k3s_traefik_manifest:
  file.managed:
    - name: "/var/lib/rancher/k3s/server/manifests/openmediavault-traefik.yaml"
    - contents: |
        ---
        apiVersion: helm.cattle.io/v1
        kind: HelmChartConfig
        metadata:
          name: traefik
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          valuesContent: |-
            ports:
              traefik:
                port: 9099
                exposedPort: 9099
{%- for lbport in k8s_lbports_config %}
              {{ lbport.name }}:
                expose:
                  default: {{ lbport.expose | yesno('true,false') }}
                exposedPort: {{ lbport.exposedport }}
                port: {{ lbport.port }}
                protocol: {{ lbport.protocol | upper() }}
{%- if lbport.extravalues | length > 0 %}
                {{ lbport.extravalues | indent(16) }}
{%- endif %}
{%- endfor %}
        ---
        apiVersion: traefik.io/v1alpha1
        kind: Middleware
        metadata:
          name: https-redirect
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          redirectScheme:
            scheme: https
            permanent: true
            port: "{{ k8s_lbports_config | selectattr('name', 'equalto', 'websecure') | first | attr('exposedport') }}"
        ---
        apiVersion: traefik.io/v1alpha1
        kind: TLSStore
        metadata:
          name: default
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
{%- if k8s_config.sslcertificateref | length > 0 %}
          certificates:
            - secretName: userdefined-tls-cert
          defaultCertificate:
            secretName: userdefined-tls-cert
{%- else %}
          certificates:
            - secretName: default-tls-cert
          defaultCertificate:
            secretName: default-tls-cert
{%- endif %}
    - user: root
    - group: root
    - mode: 600

create_k3s_cert_manager_manifest:
  file.managed:
    - name: "/var/lib/rancher/k3s/server/manifests/openmediavault-cert-manager.yaml"
    - contents: |
        ---
        apiVersion: v1
        kind: Namespace
        metadata:
          name: cert-manager
          labels:
            app.kubernetes.io/part-of: openmediavault
        ---
        apiVersion: helm.cattle.io/v1
        kind: HelmChart
        metadata:
          name: cert-manager
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          repo: https://charts.jetstack.io
          chart: cert-manager
          targetNamespace: cert-manager
          valuesContent: |-
            installCRDs: true
        ---
        apiVersion: cert-manager.io/v1
        kind: ClusterIssuer
        metadata:
          name: selfsigned
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          selfSigned: {}
        # ---
        # apiVersion: cert-manager.io/v1
        # kind: ClusterIssuer
        # metadata:
        #   name: letsencrypt-staging
        #  labels:
        #    app.kubernetes.io/part-of: openmediavault
        # spec:
        #   acme:
        #     server: https://acme-staging-v02.api.letsencrypt.org/directory
        #     email: {{ email_config.sender }}
        #     privateKeySecretRef:
        #       name: letsencrypt-issuer-account-key
        #     preferredChain: ""
        #     solvers:
        #       - http01:
        #         ingress:
        #           class: traefik
        ---
        # See https://cert-manager.io/docs/usage/certificate/
        apiVersion: cert-manager.io/v1
        kind: Certificate
        metadata:
          name: default-tls-cert
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          duration: 8760h # 1 year
          dnsNames:
            - "{{ fqdn }}"
            - "*.{{ fqdn }}"
          secretName: default-tls-cert
          issuerRef:
            name: selfsigned
            kind: ClusterIssuer
        # ---
        # apiVersion: cert-manager.io/v1
        # kind: Certificate
        # metadata:
        #   name: host-letsencrypt-staging
        #  labels:
        #    app.kubernetes.io/part-of: openmediavault
        # spec:
        #   commonName: {{ fqdn }}
        #   dnsNames:
        #     - {{ fqdn }}
        #   secretName: host-letsencrypt-staging-cert
        #   issuerRef:
        #     name: letsencrypt-staging
        #     kind: ClusterIssuer
    - user: root
    - group: root
    - mode: 600

create_k3s_misc_manifest:
  file.managed:
    - name: "/var/lib/rancher/k3s/server/manifests/openmediavault-misc.yaml"
    - contents: |
{%- if k8s_config.sslcertificateref | length > 0 %}
{%- set ssl_cert_dir = salt['pillar.get']('default:OMV_SSL_CERTIFICATE_DIR', '/etc/ssl') %}
{%- set ssl_cert_prefix = salt['pillar.get']('default:OMV_SSL_CERTIFICATE_PREFIX', 'openmediavault') %}
        ---
        apiVersion: v1
        kind: Secret
        metadata:
          name: userdefined-tls-cert
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        data:
          tls.crt: {{ ssl_cert_dir | path_join('certs', ssl_cert_prefix ~ k8s_config.sslcertificateref ~ '.crt') | file_base64_encode }}
          tls.key: {{ ssl_cert_dir | path_join('private', ssl_cert_prefix ~ k8s_config.sslcertificateref ~ '.key') | file_base64_encode }}
        type: kubernetes.io/tls
{%- endif %}
    - user: root
    - group: root
    - mode: 600

create_k3s_local_storage_manifest:
  file.managed:
    - name: "/var/lib/rancher/k3s/server/manifests/openmediavault-local-storage.yaml"
    - contents: |
        ---
        apiVersion: storage.k8s.io/v1
        kind: StorageClass
        metadata:
          name: shared-folder
          labels:
            app.kubernetes.io/part-of: openmediavault
        provisioner: rancher.io/local-path
        reclaimPolicy: Retain
        volumeBindingMode: WaitForFirstConsumer
    - user: root
    - group: root
    - mode: 600

create_k3s_config_dir:
  file.directory:
    - name: "/etc/rancher/k3s/"
    - makedirs: True

create_k3s_config:
  file.managed:
    - name: "/etc/rancher/k3s/config.yaml"
    - contents: |
        write-kubeconfig-mode: "0644"
        kube-controller-manager-arg:
          # Removing leader election can reduce CPU usage on a single-node setup.
          - "leader-elect=false"
          - "node-monitor-period=60s"
    - user: root
    - group: root
    - mode: 600

create_k3s_registries:
  file.managed:
    - name: "/etc/rancher/k3s/registries.yaml"
    - contents: |
        mirrors:
          docker.io:
            endpoint:
              - "https://index.docker.io:443/v2"
            rewrite:
              "^bitnami/(.*)": "bitnamilegacy/$1"
    - user: root
    - group: root
    - mode: 600
